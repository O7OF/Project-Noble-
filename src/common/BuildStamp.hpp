#pragma once
//
// Build stamp: identifies exactly which source tree built this executable. Logged once at startup so a
// support log states its own provenance (version, git commit, compile time, build tag). The git commit is
// read at run time from the nearest '.git' found walking up from the executable - source builds (the dev
// builds testers run) name their commit; packaged builds without a .git print 'unknown'. Log-only.
//
#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

#include "common/BuildIdentity.hpp"
#include "common/Version.hpp"

namespace noble {
namespace build_stamp_detail {

inline std::string ReadStampFile(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return {};
    std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    return s;
}

inline std::filesystem::path ExePath() {
    wchar_t buf[MAX_PATH * 2] = {};
    const DWORD n = GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)));
    if (n == 0) return {};
    return std::filesystem::path(std::wstring(buf, n));
}

// The nearest '.git' above the executable (also handles worktrees, where '.git' is a file).
inline std::filesystem::path FindGitEntry() {
    std::error_code ec;
    std::filesystem::path dir = ExePath().parent_path();
    for (int i = 0; i < 8 && !dir.empty(); ++i) {
        const auto dotGit = dir / ".git";
        if (std::filesystem::exists(dotGit, ec)) return dotGit;
        const auto parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }
    return {};
}

inline std::string GitHeadCommit() {
    std::error_code ec;
    const auto dotGit = FindGitEntry();
    if (dotGit.empty()) return {};

    std::filesystem::path gitDir = dotGit;
    if (std::filesystem::is_regular_file(dotGit, ec)) {
        std::string link = ReadStampFile(dotGit);
        constexpr std::string_view kPrefix = "gitdir:";
        if (link.rfind(kPrefix, 0) != 0) return {};
        std::string target = link.substr(kPrefix.size());
        while (!target.empty() && (target.front() == ' ' || target.front() == '\t')) target.erase(target.begin());
        gitDir = std::filesystem::path(target);
        if (gitDir.is_relative()) gitDir = dotGit.parent_path() / gitDir;
    }

    const std::string head = ReadStampFile(gitDir / "HEAD");
    if (head.empty()) return {};
    constexpr std::string_view kRefPrefix = "ref:";
    if (head.rfind(kRefPrefix, 0) != 0) {
        // Detached HEAD: the file itself is the commit.
        return head.substr(0, std::min<size_t>(head.size(), 12));
    }
    std::string ref = head.substr(kRefPrefix.size());
    while (!ref.empty() && (ref.front() == ' ' || ref.front() == '\t')) ref.erase(ref.begin());
    const std::string sha = ReadStampFile(gitDir / std::filesystem::path(ref));
    if (sha.size() >= 7) return sha.substr(0, 12);

    // Loosely packed repository: refs may only exist in packed-refs.
    std::ifstream packed(gitDir / "packed-refs");
    std::string line;
    while (std::getline(packed, line)) {
        if (line.empty() || line.front() == '#' || line.front() == '^') continue;
        const auto sp = line.find(' ');
        if (sp == std::string::npos) continue;
        if (line.substr(sp + 1) == ref)
            return line.substr(0, std::min<size_t>(sp, 12));
    }
    return {};
}

} // namespace build_stamp_detail

// One line: version | commit | compiled date time | wire build tag.
inline std::string BuildStamp() {
    const std::string commit = build_stamp_detail::GitHeadCommit();
    std::string stamp = "version ";
    stamp.append(VERSION_STRING);
    stamp += " | commit ";
    stamp += commit.empty() ? "unknown (no .git beside the build)" : commit;
    stamp += " | compiled ";
    stamp += __DATE__;
    stamp += " ";
    stamp += __TIME__;
    stamp += " | tag ";
    stamp.append(kBuildTag);
    return stamp;
}

} // namespace noble
