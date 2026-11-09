#include "network/NetworkTransport.hpp"
#include "SocialOverlay.hpp"

#include "audio/VoiceChat.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "engine/KeyBindings.hpp"
#include "engine/PadLayouts.hpp"
#include "host/Window.hpp"
#include "NobleStyle.hpp"

#include <imgui.h>

#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <map>
#include <mutex>
#include <vector>

namespace noble::SocialOverlay {
namespace {

constexpr size_t kLogLimit = 200;
constexpr size_t kInputLimit = 400;      // the relay's own cap
constexpr uint64_t kFeedMs = 12000;      // how long a new message stays up with the line closed
constexpr uint64_t kEchoMs = 6000;       // a sent line the relay hasn't repeated by then is marked as not delivered
constexpr int kFeedLines = 6;
constexpr int kVoiceRows = 8;
constexpr float kLineGap = 4.0f;

enum Channel { kAll = 0, kTeam = 1, kParty = 2, kDirect = 3, kChannelCount = 4 };

const char* const kChannelIds[kChannelCount] = { "all", "team", "party", "dm" };
const char* const kChannelTags[kChannelCount] = { "ALL", "TEAM", "PARTY", "DM" };
const char* const kAudience[kChannelCount] = {
    "Everyone in this session",
    "Your team",
    "Your party",
    "Direct Message (/w <player> <msg>)"
};
const ImU32 kChannelColors[kChannelCount] = {
    IM_COL32(236, 242, 248, 255), // All
    IM_COL32(120, 214, 150, 255), // Team
    IM_COL32(196, 162, 255, 255), // Party
    IM_COL32(80, 220, 255, 255)   // DM (Cyan)
};
const ImU32 kSystemColor = IM_COL32(255, 196, 120, 255);
const ImU32 kAccent = IM_COL32(120, 200, 255, 255);

struct Line {
    int channel = kAll;
    bool system = false;
    bool awaitingEcho = false;
    bool undelivered = false;
    std::string time, from, text, target;
    uint64_t at = 0;
};

std::mutex s_mutex;   // the log and names: the relay thread adds to both
std::deque<Line> s_log;
std::map<uint64_t, std::string> s_names;

bool s_typing = false;
int s_channel = kAll;
std::string s_dmTarget;
std::string s_lastDmSender;
std::wstring s_input;
int s_swallowKey = 0;        // characters are dropped until the key that opened the line is released
int s_scroll = 0;            // messages scrolled back from the newest
uint64_t s_closedAt = 0;
bool s_wantsFrame = false;
bool s_demo = false, s_demoOpen = false;
std::map<int, bool> s_wasDown;

uint64_t Now() { return GetTickCount64(); }

std::string Clock() {
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char text[8];
    std::snprintf(text, sizeof(text), "%02d:%02d", t.wHour, t.wMinute);
    return text;
}

std::string ToUtf8(const std::wstring& text) {
    if (text.empty()) return {};
    const int len = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<size_t>((std::max)(len, 0)), '\0');
    if (len > 0) WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(), len, nullptr, nullptr);
    return out;
}

// Text from other players: no control characters, so nobody can break the layout.
std::string Clean(std::string text, size_t limit) {
    text.erase(std::remove_if(text.begin(), text.end(), [](char c) { return static_cast<unsigned char>(c) < 0x20 || c == 0x7F; }), text.end());
    if (text.size() > limit) text.resize(limit);
    return text;
}

int ChannelFromId(const std::string& id) {
    for (int i = 0; i < kChannelCount; ++i) if (id == kChannelIds[i]) return i;
    if (id.rfind("dm:", 0) == 0 || id == "whisper") return kDirect;
    return kAll;
}

void AddLine(Line line) {
    line.time = Clock();
    line.at = Now();
    std::lock_guard<std::mutex> lock(s_mutex);
    s_log.push_back(std::move(line));
    while (s_log.size() > kLogLimit) s_log.pop_front();
}

void AddSystem(const std::string& text) {
    Line line;
    line.system = true;
    line.text = text;
    AddLine(std::move(line));
}

// On the relay thread. A line we sent comes back from the server: that confirms it instead of showing it twice.
void OnChat(const NetworkTransport::ChatMessage& message) {
    const int channel = ChannelFromId(message.channel);
    const std::string text = Clean(message.text, kInputLimit * 3);
    const std::string from = Clean(message.from, 32);
    const std::string target = Clean(message.target, 32);
    const std::string me = Settings::Get().Snapshot().playerName;

    if (channel == kDirect) {
        if (!target.empty() && _stricmp(target.c_str(), me.c_str()) != 0 && _stricmp(from.c_str(), me.c_str()) != 0) {
            return; // Not for us
        }
        if (_stricmp(from.c_str(), me.c_str()) != 0) {
            s_lastDmSender = from;
            if (s_dmTarget.empty()) s_dmTarget = from;
        }
    }

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (Line& line : s_log) {
            if (line.awaitingEcho && line.channel == channel && line.text == text &&
                (channel != kDirect || line.target == target)) {
                line.awaitingEcho = false;
                line.undelivered = false;
                line.from = from;
                return;
            }
        }
    }
    Line line;
    line.channel = channel;
    line.from = from;
    line.text = text;
    line.target = target;
    AddLine(std::move(line));
}

bool OurWindowHasFocus() {
    // Container window mode: the launcher window that hosts a match is the foreground one, not this
    // process's child window, so ask Window::IsForeground() before falling back to the process check.
    if (Window::IsForeground(Window::ActiveHwnd())) return true;
    HWND foreground = GetForegroundWindow();
    DWORD process = 0;
    if (foreground) GetWindowThreadProcessId(foreground, &process);
    return process == GetCurrentProcessId();
}

bool Pressed(int vk) {
    const bool down = vk > 0 && vk < 256 && (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool& was = s_wasDown[vk];
    const bool edge = down && !was;
    was = down;
    return edge;
}

void OpenLine(int channel, int key) {
    s_typing = true;
    s_channel = channel;
    s_input.clear();
    s_scroll = 0;
    s_swallowKey = key;
    Window::SetCharCapture(true);
    VoiceChat::SetPushToTalkBlocked(true);
}

void CloseLine() {
    s_typing = false;
    s_input.clear();
    s_scroll = 0;
    s_closedAt = Now();
    Window::SetCharCapture(false);
    VoiceChat::SetPushToTalkBlocked(false);
}

void SendLine() {
    std::string text = Clean(ToUtf8(s_input), kInputLimit * 3);
    if (text.find_first_not_of(' ') == std::string::npos) return;

    int channel = s_channel;
    std::string target = s_dmTarget;

    // Check for slash commands
    if (text[0] == '/') {
        size_t firstSpace = text.find(' ');
        std::string cmd = (firstSpace == std::string::npos) ? text.substr(1) : text.substr(1, firstSpace - 1);
        std::string rest = (firstSpace == std::string::npos) ? "" : text.substr(firstSpace + 1);
        while (!rest.empty() && rest.front() == ' ') rest.erase(rest.begin());

        std::string cmdLower = cmd;
        for (char& c : cmdLower) c = static_cast<char>(::tolower(c));

        if (cmdLower == "w" || cmdLower == "dm" || cmdLower == "msg" || cmdLower == "whisper") {
            size_t restSpace = rest.find(' ');
            if (restSpace == std::string::npos || restSpace == 0) {
                AddSystem("Usage: /w <player> <message>");
                return;
            }
            target = rest.substr(0, restSpace);
            text = rest.substr(restSpace + 1);
            while (!text.empty() && text.front() == ' ') text.erase(text.begin());
            if (text.empty()) return;
            channel = kDirect;
            s_dmTarget = target;
        } else if (cmdLower == "r" || cmdLower == "reply") {
            if (s_lastDmSender.empty()) {
                AddSystem("No one has messaged you yet to reply to.");
                return;
            }
            if (rest.empty()) {
                AddSystem("Usage: /r <message>");
                return;
            }
            target = s_lastDmSender;
            text = rest;
            channel = kDirect;
            s_dmTarget = target;
        } else if (cmdLower == "all" || cmdLower == "a") {
            channel = kAll;
            text = rest;
            if (text.empty()) { s_channel = kAll; return; }
        } else if (cmdLower == "team" || cmdLower == "t") {
            channel = kTeam;
            text = rest;
            if (text.empty()) { s_channel = kTeam; return; }
        } else if (cmdLower == "party" || cmdLower == "p") {
            channel = kParty;
            text = rest;
            if (text.empty()) { s_channel = kParty; return; }
        }
    }

    if (channel == kDirect && target.empty()) {
        AddSystem("No recipient set. Use /w <player> <message> or select a friend from the Social menu to send a DM.");
        return;
    }

    const std::string me = Settings::Get().Snapshot().playerName;
    Line line;
    line.channel = channel;
    line.from = me;
    line.text = text;
    line.target = target;

    if (s_demo) {
        AddLine(std::move(line));
        return;
    }

    if (NetworkTransport::Get().IsRelayConnected()) {
        line.awaitingEcho = true;
        NetworkTransport::Get().SendChat(kChannelIds[channel], text, me, target);
        AddLine(std::move(line));
    } else {
        line.awaitingEcho = false;
        line.undelivered = false;
        AddLine(std::move(line));
    }
}

void SeedDemo() {
    const auto add = [](int channel, const char* from, const char* text) {
        Line line;
        line.channel = channel;
        line.from = from;
        line.text = text;
        AddLine(std::move(line));
    };
    add(kAll, "Carter-A259", "Noble Team, mount up.");
    add(kTeam, "Kat-B320", "Two on the ridge, moving to the sword room.");
    add(kAll, "Jun-A266", "gg");
    add(kParty, "Emile-A239", "Anyone staying for another round after this one?");
    add(kTeam, "Kat-B320", "Overshield is up in ten seconds, grab it before they cross the bridge and push the top lift.");
    add(kAll, "Jorge-052", "Say the word.");
}

// Drawing ---------------------------------------------------------------------------------------------------------------

void DrawMic(ImDrawList* dl, ImVec2 c, float s, ImU32 color) {
    const float w = 3.2f * s, top = 6.5f * s, bottom = 1.0f * s;
    dl->AddRectFilled(ImVec2(c.x - w, c.y - top), ImVec2(c.x + w, c.y + bottom), color, w);
    const float r = 5.5f * s;
    dl->PathArcTo(ImVec2(c.x, c.y - 1.0f * s), r, 0.0f, 3.14159265f, 12);
    dl->PathStroke(color, 0, 1.5f * s);
    dl->AddLine(ImVec2(c.x, c.y + 4.5f * s), ImVec2(c.x, c.y + 7.5f * s), color, 1.5f * s);
    dl->AddLine(ImVec2(c.x - 3.0f * s, c.y + 7.5f * s), ImVec2(c.x + 3.0f * s, c.y + 7.5f * s), color, 1.5f * s);
}

void DrawVoiceRow(ImDrawList* dl, ImFont* font, float x, float y, float w, float h, float s, const std::string& name, const char* note, bool local, double time) {
    const float pulse = 0.72f + 0.28f * static_cast<float>(std::sin(time * 7.0));
    const ImU32 fill = local ? IM_COL32(20, 62, 92, 225) : IM_COL32(10, 20, 30, 215);
    const ImU32 edge = local ? IM_COL32(120, 200, 255, 230) : IM_COL32(120, 214, 150, 170);
    dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), fill, 3.0f * s);
    dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 3.0f * s, y + h), edge, 3.0f * s);
    const ImU32 mic = local ? IM_COL32(143, 216, 255, static_cast<int>(255 * pulse)) : IM_COL32(120, 214, 150, static_cast<int>(255 * pulse));
    DrawMic(dl, ImVec2(x + 20.0f * s, y + h * 0.5f), s, mic);
    const float size = 16.0f * s;
    const float textY = y + (h - size) * 0.5f;
    dl->PushClipRect(ImVec2(x, y), ImVec2(x + w - 4.0f * s, y + h), true);
    dl->AddText(font, size, ImVec2(x + 36.0f * s, textY), IM_COL32(240, 246, 252, 255), name.c_str());
    if (note && *note) {
        const float nameW = font->CalcTextSizeA(size, FLT_MAX, 0.0f, name.c_str()).x;
        dl->AddText(font, 13.0f * s, ImVec2(x + 36.0f * s + nameW + 10.0f * s, y + (h - 13.0f * s) * 0.5f), IM_COL32(170, 200, 220, 255), note);
    }
    dl->PopClipRect();
}

void DrawVoice(ImDrawList* dl, ImFont* font, float s, float top) {
    std::vector<std::string> names;
    bool local = VoiceChat::IsTransmitting();
    if (s_demo) {
        local = true;
        names = { "Kat-B320", "Carter-A259", "Jun-A266" };
    } else {
        const std::vector<uint64_t> ids = VoiceChat::ActiveTalkers();
        std::lock_guard<std::mutex> lock(s_mutex);
        for (const uint64_t id : ids) {
            const auto it = s_names.find(id);
            names.push_back(it != s_names.end() && !it->second.empty() ? it->second : "Player");
        }
    }
    if (!local && names.empty()) return;

    const double time = ImGui::GetTime();
    const float x = 24.0f * s, w = 300.0f * s, h = 30.0f * s, gap = 4.0f * s;
    float y = top * s;
    if (local) {
        const SettingsData d = Settings::Get().Snapshot();
        char note[64];
        std::snprintf(note, sizeof(note), "%s", d.voiceOpenMic ? "OPEN MIC" : "TRANSMITTING");
        DrawVoiceRow(dl, font, x, y, w, h, s, "YOU", note, true, time);
        y += h + gap;
    }
    const int shown = (std::min)(static_cast<int>(names.size()), kVoiceRows);
    for (int i = 0; i < shown; ++i) {
        DrawVoiceRow(dl, font, x, y, w, h, s, names[static_cast<size_t>(i)], nullptr, false, time + i * 0.7);
        y += h + gap;
    }
    if (static_cast<int>(names.size()) > shown) {
        const std::string more = "+" + std::to_string(names.size() - shown) + " more talking";
        dl->AddText(font, 14.0f * s, ImVec2(x + 8.0f * s, y), IM_COL32(190, 210, 225, 230), more.c_str());
    }
}

std::string LineText(const Line& line) {
    if (line.system) return line.time + "  " + line.text;
    if (line.channel == kDirect) {
        const std::string me = Settings::Get().Snapshot().playerName;
        if (!line.target.empty() && _stricmp(line.from.c_str(), me.c_str()) == 0) {
            return line.time + "  [DM to " + line.target + "] " + line.from + ": " + line.text;
        } else {
            return line.time + "  [DM from " + line.from + "] " + line.from + ": " + line.text;
        }
    }
    return line.time + "  [" + kChannelTags[line.channel] + "] " + line.from + ": " + line.text;
}

ImU32 Faded(ImU32 color, float alpha) {
    const int a = static_cast<int>(static_cast<float>((color >> IM_COL32_A_SHIFT) & 0xFF) * alpha);
    return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a) << IM_COL32_A_SHIFT);
}

// Newest at the bottom, working up until the room runs out.
void DrawLines(ImDrawList* dl, ImFont* font, float x, float bottom, float top, float width, float size, const std::vector<Line>& lines, bool feed, uint64_t now, float s) {
    float y = bottom;
    for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
        std::string text = LineText(*it);
        if (it->undelivered) text += "  (not delivered)";
        const float h = font->CalcTextSizeA(size, FLT_MAX, width, text.c_str()).y;
        y -= h;
        if (y < top) break;
        float alpha = 1.0f;
        if (feed) {
            const uint64_t age = now - it->at;
            if (age > kFeedMs - 2000) alpha = (std::max)(0.0f, static_cast<float>(kFeedMs - age) / 2000.0f);
        }
        const ImU32 base = it->system || it->undelivered ? kSystemColor : kChannelColors[it->channel];
        dl->AddText(font, size, ImVec2(x + 1.0f * s, y + 1.0f * s), Faded(IM_COL32(0, 0, 0, 220), alpha), text.c_str(), nullptr, width);
        dl->AddText(font, size, ImVec2(x, y), Faded(base, alpha), text.c_str(), nullptr, width);
        y -= kLineGap * s;
    }
}

void DrawChat(ImDrawList* dl, ImFont* font, float s, float chatX, float chatBottom) {
    const uint64_t now = Now();
    std::vector<Line> lines;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (Line& line : s_log) {
            if (line.awaitingEcho && now - line.at > kEchoMs) {
                line.awaitingEcho = false;
                line.undelivered = true;
            }
        }
        if (s_typing) {
            const int count = static_cast<int>(s_log.size());
            s_scroll = std::clamp(s_scroll, 0, (std::max)(count - 1, 0));
            const int end = count - s_scroll;
            lines.assign(s_log.begin(), s_log.begin() + end);
        } else {
            for (const Line& line : s_log) if (s_demo || now - line.at < kFeedMs) lines.push_back(line);
            if (static_cast<int>(lines.size()) > kFeedLines) lines.erase(lines.begin(), lines.end() - kFeedLines);
        }
    }
    const float x = chatX * s, w = 560.0f * s, pad = 10.0f * s, size = 16.0f * s;
    const float bottom = chatBottom * s;
    if (!s_typing) {
        if (!lines.empty()) {
            // A faint backing, so the feed stays readable over a bright map or a panel with its own text.
            float total = 0.0f;
            for (const Line& line : lines) total += font->CalcTextSizeA(size, FLT_MAX, w - pad * 2.0f, LineText(line).c_str()).y + kLineGap * s;
            total = (std::min)(total, 190.0f * s);
            dl->AddRectFilled(ImVec2(x, bottom - total - 4.0f * s), ImVec2(x + w, bottom + 4.0f * s), IM_COL32(4, 10, 16, 165), 3.0f * s);
            DrawLines(dl, font, x + pad, bottom, bottom - 190.0f * s, w - pad * 2.0f, size, lines, !s_demo, now, s);
        }
        return;
    }

    // The header says who this reaches, the log sits under it, and the line being typed is at the foot.
    const float headH = 30.0f * s, inputH = 36.0f * s, hintH = 22.0f * s, height = 340.0f * s;
    const float top = bottom - height;
    dl->AddRectFilled(ImVec2(x, top), ImVec2(x + w, bottom), IM_COL32(7, 14, 22, 228), 4.0f * s);
    dl->AddRect(ImVec2(x, top), ImVec2(x + w, bottom), IM_COL32(120, 160, 190, 120), 4.0f * s);
    const ImU32 color = kChannelColors[s_channel];
    dl->AddRectFilled(ImVec2(x, top), ImVec2(x + w, top + headH), IM_COL32(16, 30, 44, 245), 4.0f * s);
    dl->AddRectFilled(ImVec2(x, top), ImVec2(x + 4.0f * s, top + headH), color);
    std::string head = std::string("TO ") + kChannelTags[s_channel];
    if (s_channel == kDirect && !s_dmTarget.empty()) head += " (" + s_dmTarget + ")";
    dl->AddText(font, 17.0f * s, ImVec2(x + 14.0f * s, top + 6.0f * s), color, head.c_str());
    const float headW = font->CalcTextSizeA(17.0f * s, FLT_MAX, 0.0f, head.c_str()).x;
    std::string aud = (s_channel == kDirect && !s_dmTarget.empty()) ? ("Direct Message to " + s_dmTarget) : kAudience[s_channel];
    dl->AddText(font, 14.0f * s, ImVec2(x + 26.0f * s + headW, top + 8.0f * s), IM_COL32(190, 210, 225, 255), aud.c_str());

    const float inputTop = bottom - hintH - inputH;
    const float logBottom = inputTop - 6.0f * s;
    dl->PushClipRect(ImVec2(x, top + headH), ImVec2(x + w, logBottom), true);
    if (lines.empty()) {
        dl->AddText(font, size, ImVec2(x + pad, logBottom - size), IM_COL32(150, 170, 185, 255), "No messages yet.");
    } else {
        DrawLines(dl, font, x + pad, logBottom, top + headH, w - pad * 2.0f, size, lines, false, now, s);
    }
    dl->PopClipRect();

    // The line itself. A long one scrolls so its end and the caret stay in view.
    const ImVec2 box0(x + pad, inputTop), box1(x + w - pad, inputTop + inputH);
    dl->AddRectFilled(box0, box1, IM_COL32(2, 6, 12, 255), 3.0f * s);
    dl->AddRect(box0, box1, color, 3.0f * s);
    const std::string typed = ToUtf8(s_input);
    const float textW = font->CalcTextSizeA(size, FLT_MAX, 0.0f, typed.c_str()).x;
    const float room = box1.x - box0.x - 16.0f * s;
    const float textX = box0.x + 8.0f * s - (std::max)(0.0f, textW - room);
    const float textY = box0.y + (inputH - size) * 0.5f;
    dl->PushClipRect(box0, box1, true);
    if (typed.empty()) dl->AddText(font, size, ImVec2(box0.x + 8.0f * s, textY), IM_COL32(120, 140, 155, 255), "Type a message");
    else dl->AddText(font, size, ImVec2(textX, textY), IM_COL32(245, 248, 252, 255), typed.c_str());
    if (std::fmod(ImGui::GetTime(), 1.0) < 0.55) {
        const float caret = textX + textW + 1.0f * s;
        dl->AddLine(ImVec2(caret, textY), ImVec2(caret, textY + size), kAccent, 1.5f * s);
    }
    dl->PopClipRect();

    char hint[96];
    std::snprintf(hint, sizeof(hint), "Enter send   Tab change channel   PgUp/PgDn scroll   Esc close   %zu/%zu", s_input.size(), kInputLimit);
    dl->AddText(font, 13.0f * s, ImVec2(x + pad, bottom - hintH + 4.0f * s), IM_COL32(160, 180, 195, 255), hint);
    if (s_scroll > 0) {
        const std::string older = std::to_string(s_scroll) + " newer below";
        const float olderW = font->CalcTextSizeA(13.0f * s, FLT_MAX, 0.0f, older.c_str()).x;
        dl->AddText(font, 13.0f * s, ImVec2(x + w - pad - olderW, top + 9.0f * s), kAccent, older.c_str());
    }
}

} // namespace

void Start() {
    static bool s_started = false;
    if (s_started) return;
    s_started = true;
    NetworkTransport::Get().SetChatCallback(&OnChat);
    const wchar_t* cmd = GetCommandLineW();
    s_demoOpen = wcsstr(cmd, L"-social-demo-open") != nullptr;
    s_demo = s_demoOpen || wcsstr(cmd, L"-social-demo") != nullptr;
    if (s_demo) {
        SeedDemo();
        if (s_demoOpen) {
            OpenLine(kTeam, 0);
            s_input = L"On my way to the sword room, cover the lift";
        }
    }
}

void Poll(bool allowOpen) {
    const bool focus = OurWindowHasFocus();
    const uint64_t now = Now();
    bool recent = false;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (auto it = s_log.rbegin(); it != s_log.rend(); ++it) {
            if (now - it->at >= kFeedMs) break;
            recent = true;
            break;
        }
    }
    if (s_typing) {
        if (s_swallowKey > 0 && (GetAsyncKeyState(s_swallowKey) & 0x8000) == 0) s_swallowKey = 0;
        const std::wstring typed = Window::TakeTypedChars();
        if (focus) {
            if (Pressed(VK_ESCAPE)) {
                if (!s_demoOpen) CloseLine();
            } else if (Pressed(VK_RETURN)) {
                SendLine();
                if (!s_demoOpen) CloseLine();
            } else {
                if (Pressed(VK_TAB)) s_channel = (s_channel + 1) % kChannelCount;
                if (Pressed(VK_PRIOR)) s_scroll += 5;
                if (Pressed(VK_NEXT)) s_scroll = (std::max)(0, s_scroll - 5);
            }
        }
        if (s_typing && s_swallowKey == 0) {
            for (const wchar_t c : typed) {
                if (c == L'\b') {
                    if (!s_input.empty()) s_input.pop_back();
                } else if (c >= 0x20 && c != 0x7F && s_input.size() < kInputLimit) {
                    s_input.push_back(c);
                }
            }
        }
    } else {
        Pressed(VK_ESCAPE);
        Pressed(VK_RETURN);
        Pressed(VK_TAB);
        const int keys[4] = {
            KeyBindings::EffectiveKey(KeyBindings::kActionChatAll), KeyBindings::EffectiveKey(KeyBindings::kActionChatTeam),
            KeyBindings::EffectiveKey(KeyBindings::kActionChatParty), KeyBindings::EffectiveKey(KeyBindings::kActionChatOpen) };
        const bool allowed = allowOpen && focus && !(ImGui::GetCurrentContext() && ImGui::GetIO().WantTextInput);
        static const int s_channels[4] = { kAll, kTeam, kParty, -1 };
        for (int i = 0; i < 4; ++i) {
            if (Pressed(keys[i]) && allowed && !s_typing) OpenLine(s_channels[i] < 0 ? s_channel : s_channels[i], keys[i]);
        }
    }
    s_wantsFrame = s_typing || recent || s_demo || VoiceChat::IsTransmitting() || !VoiceChat::ActiveTalkers().empty();
}

void OpenChannel(int channel) {
    if (channel >= 0 && channel < kChannelCount) s_channel = channel;
    OpenLine(s_channel, 0);
}

void OpenDirectMessage(const std::string& friendName) {
    if (!friendName.empty()) s_dmTarget = friendName;
    s_channel = kDirect;
    OpenLine(kDirect, 0);
}

bool WantsFrame() { return s_wantsFrame; }

bool IsTyping() { return s_typing; }

bool OwnsEscape() { return s_typing || Now() - s_closedAt < 300; }

void Draw(float voiceTop, float chatX, float chatBottom) {
    if (!s_wantsFrame || !ImGui::GetCurrentContext()) return;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float s = display.y / 1080.0f;
    ImFont* font = NobleStyle::SmallFont() ? NobleStyle::SmallFont() : ImGui::GetFont();
    DrawVoice(dl, font, s, voiceTop);
    DrawChat(dl, font, s, chatX, chatBottom);
}

void DrawPlayerVoiceIcon(ImDrawList* dl, ImVec2 center, float scale, uint64_t machineId, bool local) {
    const float pulse = 0.72f + 0.28f * static_cast<float>(std::sin(ImGui::GetTime() * 7.0));
    const int alpha = static_cast<int>(255 * pulse);
    if (local) {
        if (s_demo || VoiceChat::IsTransmitting()) DrawMic(dl, center, scale, IM_COL32(143, 216, 255, alpha));
        return;
    }
    if (machineId == 0) return;
    if (VoiceChat::IsPeerMuted(machineId)) {
        DrawMic(dl, center, scale, IM_COL32(150, 160, 170, 200));
        dl->AddLine(ImVec2(center.x - 8.0f * scale, center.y - 8.0f * scale), ImVec2(center.x + 8.0f * scale, center.y + 8.0f * scale), IM_COL32(230, 90, 90, 255), 2.0f * scale);
        return;
    }
    if ((s_demo && machineId % 2 == 1) || VoiceChat::IsPeerTalking(machineId)) DrawMic(dl, center, scale, IM_COL32(120, 214, 150, alpha));
}

void SetPlayerName(uint64_t machineId, const std::string& name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_names[machineId] = Clean(name, 32);
}

} // namespace noble::SocialOverlay
