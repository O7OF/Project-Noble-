#pragma once

// Host moderation tools (all modes): kick, rejoin cooldown, bans with a timer,
// host mutes and the host's word filter.
//
// Who a player is:
//   - accountId: the player's account id (lobby roster peerId, > 1 when logged
//     in). Stable across restarts and renames, so bans are kept by it.
//   - machineId: the network machine id of this session (lobby roster
//     machineId / the id on the wire). Random per launch, so it only identifies
//     a player for the current session (kicks, mutes, LAN bans).
//   - name: shown in the ban list; LAN players without an account are also
//     matched by name.
//
// The host enforces everything: Tick() asks any blocked player that is still
// connected to leave (host_remove), repeating every 3 s, so a banned or just
// kicked player who rejoins is sent away again.

#include <cstdint>
#include <string>
#include <vector>

namespace noble::HostModeration {

struct PlayerRef {
  uint64_t accountId = 0; // 0 / 1 = no account
  uint64_t machineId = 0; // this session's id on the wire
  std::string name;
};

struct BanEntry {
  uint64_t accountId = 0;
  uint64_t machineId = 0; // only meaningful for the session it was made in
  std::string name;
  std::string reason;
  int64_t bannedAt = 0; // unix seconds
  int64_t until = 0;    // unix seconds; 0 = permanent
};

// Minutes for the ban choices offered in the menus; 0 = permanent.
inline constexpr int kBanChoices[] = {30, 60, 24 * 60, 0};
std::string BanLengthText(int minutes); // "30 MIN", "1 HOUR", "24 HOURS", "PERMANENT"

// Kick: the player leaves now and may not rejoin for kKickCooldownSeconds.
inline constexpr int kKickCooldownSeconds = 120;
void Kick(const PlayerRef &player, const std::string &reason);
// Ban for `minutes` (0 = permanent). Saved to data\userdata\host_bans.json.
void Ban(const PlayerRef &player, int minutes, const std::string &reason);
void Unban(size_t index);
std::vector<BanEntry> Bans(); // expired bans are dropped

// Why this player may not be in the host's game right now ("" = allowed).
std::string BlockReason(const PlayerRef &player, int *minutesLeft = nullptr);

// Host: call often (lobby frame, in-match frame). Rate-limited inside.
// `players` = everyone currently connected (lobby roster or match peers).
void Tick(const std::vector<PlayerRef> &players);

// Host mutes (voice + text for every player in the game). Session only.
void SetHostMuted(uint64_t machineId, bool muted);
bool IsHostMuted(uint64_t machineId);

// Word filter: the host's built-in list (off by default; the first-start setup
// asks) plus custom words.
// Masks matching words with '*'. Used on chat and on names the host shows.
std::string FilterText(const std::string &text);
bool FilterEnabled();
void SetFilterEnabled(bool on);
std::vector<std::string> CustomWords();
void SetCustomWords(const std::vector<std::string> &words);

// Server / lobby names: always filtered (not a setting), for every player.
// Slurs and common swear words, whole words only, so "assistance" or "class"
// stay as they are; joined words ("BadAss") are split at capitals.
std::string FilterServerName(const std::string &name);

// Chat line as this PC should show it, after the host's rules: false = drop it
// (host-muted player). The host applies its own mute list and filter; a guest
// applies the rules the host sent (host_rules), which covers chat that came
// through the master-server relay without passing through the host.
bool ApplyChatRules(uint64_t senderMachine, std::string &text);

// ---- Local text mute (any player, only on this PC; session only) ----
// Chat lines from that player are dropped here. Matched by machine id when the
// line carries one, else by name.
void SetChatMuted(uint64_t machineId, const std::string &name, bool muted);
bool IsChatMuted(uint64_t machineId, const std::string &name);

// ---- Anti-AFK (host option; every player checks itself) ----
// Minutes of no input before a player leaves the match; 0 = off. Saved with the
// ban list. The host sends it to everyone (host_rules); each guest measures its
// own idle time (keyboard, mouse and controller) and leaves when it runs out.
inline constexpr int kAfkChoices[] = {0, 3, 5, 10};
int AfkMinutes();             // host's setting
void SetAfkMinutes(int minutes);
int GuestAfkMinutes();        // the host's rule as received; 0 when none / stale
double LocalIdleSeconds();    // this PC: seconds since the last input

// ---- Co-hosts (session only) ----
// A co-host may kick and mute for everyone (not ban, not change co-hosts). The
// host keeps the list, sends it with host_rules, and checks every co-host
// request itself before acting on it.
void SetCoHost(uint64_t machineId, bool on);
bool IsCoHost(uint64_t machineId);   // host side
bool LocalIsCoHost();                // guest side, from the host's last host_rules
// Guest co-host: ask the host to act ("kick", "mute", "unmute").
void RequestCoHostAction(const std::string &action, uint64_t targetMachine,
                         const std::string &targetName);

// ---- Lobby access: lock, invite only, password (host side) ----
// The host decides who may join its lobby. Players already admitted (in the
// lobby while it was open, or let in since) always stay admitted, so a locked
// lobby still takes its own players back after a match. Everyone else is sent
// away (host_remove) unless the host invited them (friend invite) or they sent
// the right password. The host's master-server listing only shows a lock badge;
// the check itself runs on the host (see the TODO doc: a server-side check is
// O7's call).
enum class JoinAccess { Open = 0, InviteOnly = 1, Password = 2, Locked = 3 };
JoinAccess Access();
void SetAccess(JoinAccess access);
const char *AccessText(JoinAccess access); // "OPEN", "INVITE ONLY", ...
std::string LobbyPassword();               // host's password ("" = none); saved
void SetLobbyPassword(const std::string &password);
// True when the listing should carry the password badge.
bool AdvertHasPassword();
// A fresh lobby: open, nobody admitted, no invites (memory only).
void BeginLobbyAccess();
// Back from a match as host of the same lobby: restore the access mode and the
// admitted players saved by the lobby before the match.
void RestoreLobbyAccess();
// SocialClient calls this for every friend invite the host sends.
void NoteInvited(const std::string &username);
// Host: call every lobby frame with the roster (not the host). Rate-limited.
void TickLobbyAccess(const std::vector<PlayerRef> &players);
size_t AdmittedCount();

// ---- Lobby access (guest side) ----
// The password the player typed before joining ("" = none). Sent hashed to the
// host after joining the lobby.
void SetJoinPassword(const std::string &password);
// Guest: call after joining a lobby, then every lobby frame.
void OnJoinedLobby();
void GuestLobbyTick();
void OnLobbyAuth(const std::string &json, uint64_t senderMachine); // host
void OnLobbyAdmit(const std::string &json);                        // guest
std::string PasswordHash(const std::string &password);

// Called by NetworkTransport for the moderation control messages.
void OnHostRules(const std::string &json);
void OnCoHostAction(const std::string &json, uint64_t senderMachine);

// Guest: the message the host's removal leaves for the main menu (a match is a
// separate process, so it goes through data\userdata\host_notice.txt).
std::string RemovalText(const std::string &kind, const std::string &reason,
                        int minutes);
void WriteRemovalNotice(const std::string &text);
std::string TakeRemovalNotice(); // "" when there is none; deletes the file

} // namespace noble::HostModeration
