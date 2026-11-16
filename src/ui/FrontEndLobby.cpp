#include "FrontEndInternal.hpp"
#include "network/InviteCode.hpp"

namespace noble::fe {

struct PendingLobbyEvents {
  bool launch = false;
  std::string map, mode, variant, hostEndpoint, sessionId, sessionToken;
  std::string mapOverride, mapVariant, modFolder, modId;
  int difficulty = 1;
  uint64_t skulls = 0;
  bool roster = false;
  std::vector<LobbyPlayer> rosterPlayers;
  bool closed = false;
  std::string closeReason;
};
static std::mutex s_lobbyEventMutex;
static PendingLobbyEvents s_lobbyEvents;

// The master server keeps a roster per session and the host re-registers it
// every few seconds, so a guest who leaves can linger as a nameless row - which
// the UI then labels "Host" for want of a name. Drop nameless rows that are not
// the host, keep exactly one host row, and give it a name.
static void SanitizeLobbyRoster(std::vector<LobbyPlayer> &roster) {
  roster.erase(std::remove_if(roster.begin(), roster.end(),
                              [](const LobbyPlayer &p) {
                                return p.username.empty() && !p.isHost;
                              }),
               roster.end());
  bool hostSeen = false;
  for (LobbyPlayer &p : roster) {
    if (p.isHost && !hostSeen) {
      hostSeen = true;
      if (p.username.empty())
        p.username = "Host";
    } else if (p.isHost) {
      p.isHost = false;
    }
  }
  if (!hostSeen && !roster.empty()) {
    roster.front().isHost = true;
    if (roster.front().username.empty())
      roster.front().username = "Host";
  }
}

void ClearPendingLobbyEvents() {
  std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
  s_lobbyEvents = PendingLobbyEvents{};
}

void TakePendingLobbyEvents(MenuState &st) {
  PendingLobbyEvents ev;
  {
    std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
    if (!s_lobbyEvents.launch && !s_lobbyEvents.roster && !s_lobbyEvents.closed)
      return;
    ev = std::move(s_lobbyEvents);
    s_lobbyEvents = PendingLobbyEvents{};
  }
  if (ev.closed && !st.isLobbyHost) {
    st.screen = Screen::Main;
    st.lobbySessionId.clear();
    st.lobbySessionToken.clear();
    st.lobbyRoster.clear();
    st.isLobbyHost = false;
    NetworkTransport::Get().StopRelay();
    return;
  }
  if (ev.roster) {
    st.lobbyRoster = std::move(ev.rosterPlayers);
    if (st.isLobbyHost) {
      std::string pName = AuthClient::Get().IsLoggedIn()
                              ? AuthClient::Get().GetUsername()
                              : Settings::Get().Snapshot().playerName;
      if (pName.empty())
        pName = "Host";
      const uint64_t hostPId =
          AuthClient::Get().IsLoggedIn()
              ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
              : 1ULL;
      const std::string hostTag = PlayerServiceTag(Settings::Get().Snapshot());
      const bool hostDev = AuthClient::Get().IsDeveloper();
      bool hostFound = false;
      for (auto &p : st.lobbyRoster) {
        if (p.isHost) {
          p.peerId = hostPId;
          p.machineId = DeviceIdentityStore::Get().deviceId;
          p.username = pName;
          p.serviceTag = hostTag;
          p.isDev = hostDev;
          hostFound = true;
        }
      }
      if (!hostFound) {
        LobbyPlayer hp;
        hp.peerId = hostPId;
        hp.machineId = DeviceIdentityStore::Get().deviceId;
        hp.username = pName;
        hp.serviceTag = hostTag;
        hp.isHost = true;
        hp.isDev = hostDev;
        const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
        hp.rankIndex = Career::RankIndex(Career::Get().credits);
        hp.emblemId = prof.emblemIndex;
        hp.nameplateId = prof.nameplateIndex;
        hp.emblemBackgroundId = prof.emblemBackgroundIndex;
        st.lobbyRoster.insert(st.lobbyRoster.begin(), hp);
      }
    }
  }
  if (ev.launch && !st.lobbyLaunchPending && !st.isLobbyHost) {
    NOBLE_INFO("Lobby launch received: map='{}' mode='{}' variant='{}' "
               "mvar='{}' endpoint='{}' session='{}'",
               ev.map, ev.mode, ev.variant, ev.mapVariant, ev.hostEndpoint,
               ev.sessionId);
    st.lobbyLaunchPending = true;
    st.lobbyLaunchMap = ev.map;
    st.lobbyLaunchMode = ev.mode;
    st.lobbyLaunchVariant = ev.variant;
    st.lobbyLaunchDiff = ev.difficulty;
    st.lobbyLaunchSkulls = ev.skulls;
    st.lobbyLaunchMapOverride = ev.mapOverride;
    st.lobbyLaunchMapVariant = ev.mapVariant;
    st.lobbyLaunchModFolder = ev.modFolder;
    st.lobbyLaunchModId = ev.modId;
    if (st.lobbyHostEndpoint.empty() && !ev.hostEndpoint.empty())
      st.lobbyHostEndpoint = ev.hostEndpoint;
    if (!ev.sessionId.empty())
      st.lobbySessionId = ev.sessionId;
    if (!ev.sessionToken.empty())
      st.lobbySessionToken = ev.sessionToken;
    else if (st.lobbySessionToken.empty())
      st.lobbySessionToken = MasterServerClient::Get().GetCurrentSessionToken();
  }
}

void EnterLobby(OverlayContext &ctx, const ServerEntry &s,
                       const std::string &modId,
                       const std::string &modTitle,
                       const std::filesystem::path &modFolder) {
  ctx.st.isLobbyHost = false;
  ctx.st.lobbySessionId = s.id;
  ctx.st.lobbyHostEndpoint = s.hostAddress + ":" + std::to_string(s.port);
  ctx.st.lobbyMap = s.mapName.empty() ? "20_sword_slayer" : s.mapName;
  ctx.st.lobbyMode = s.gameMode.empty() ? "multiplayer" : s.gameMode;
  ctx.st.lobbyVariantName = s.variantName.empty() ? "Default" : s.variantName;
  ctx.st.lobbyMaxPlayers = s.maxPlayers > 0 ? s.maxPlayers : 16;
  ctx.st.lobbyModId = modId;
  ctx.st.lobbyModTitle = modTitle;
  ctx.st.lobbyModFolder = modFolder;
  ctx.st.lobbyMapOverride.clear();
  ctx.st.lobbyMapVariant.clear();
  ctx.st.lobbyLaunchMapOverride.clear();
  ctx.st.lobbyLaunchMapVariant.clear();
  ctx.st.lobbyLaunchModFolder.clear();
  ctx.st.lobbyLaunchModId.clear();
  ctx.st.lobbyRoster.clear();
  ctx.st.lobbyLaunchPending = false;
  {
    std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
    s_lobbyEvents = PendingLobbyEvents{};
  }
  ctx.st.lastLobbyPoll = ImGui::GetTime();
  NOBLE_INFO("EnterLobby: joining lobby '{}' ({}:{}) mod='{}' folder='{}'",
             s.name, s.hostAddress, s.port, modId, modFolder.string());

  std::string pName = AuthClient::Get().IsLoggedIn()
                          ? AuthClient::Get().GetUsername()
                          : Settings::Get().Snapshot().playerName;
  if (pName.empty())
    pName = "Guest";
  uint64_t fallbackId =
      (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
  if (fallbackId < 2ULL)
    fallbackId = 2ULL;
  uint64_t pId = AuthClient::Get().IsLoggedIn()
                     ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                     : fallbackId;
  std::string sTag = PlayerServiceTag(Settings::Get().Snapshot());

  LobbyState state = MasterServerClient::Get().JoinLobbySync(
      s.id, pId, pName, AuthClient::Get().IsDeveloper(), sTag,
      Career::RankIndex(Career::Get().credits),
      UIManager::GetArmorCustomizer().GetProfile().emblemIndex,
      UIManager::GetArmorCustomizer().GetProfile().emblemBackgroundIndex,
      UIManager::GetArmorCustomizer().GetProfile().nameplateIndex, 0, 11774,
      s.hostAddress, s.port, AuthClient::Get().IsTester());
  if (state.ok) {
    if (!state.sessionId.empty())
      ctx.st.lobbySessionId = state.sessionId;
    ctx.st.lobbyRoster = state.roster;
    ctx.st.lobbyMap = state.map;
    ctx.st.lobbyMode = state.mode;
    ctx.st.lobbyVariantName = state.variant;
    ctx.st.lobbyDifficulty = state.difficulty;
    ctx.st.lobbySkulls = state.skulls;
    if (!state.modTitle.empty())
      ctx.st.lobbyModTitle = state.modTitle;
    if (state.isLan && !state.hostLanIp.empty()) {
      ctx.st.lobbyHostEndpoint =
          state.hostLanIp + ":" + std::to_string(state.hostPort);
    }
    if (state.inProgress || state.launchTriggered) {
      ctx.st.lobbyLaunchPending = true;
      ctx.st.lobbyLaunchMap =
          state.launchMap.empty() ? state.map : state.launchMap;
      ctx.st.lobbyLaunchMode =
          state.launchMode.empty() ? state.mode : state.launchMode;
      ctx.st.lobbyLaunchVariant = state.launchVariant;
      ctx.st.lobbyLaunchMapVariant = state.launchMapVariant;
      ctx.st.lobbyLaunchDiff = state.launchDifficulty;
      ctx.st.lobbyLaunchSkulls = state.launchSkulls;
      ctx.st.lobbyLaunchMapOverride = state.launchMapOverride;
      ctx.st.lobbyLaunchModFolder = state.launchModFolder;
      ctx.st.lobbyLaunchModId =
          state.launchModId.empty() ? state.modId : state.launchModId;
    }
  } else {
    LobbyPlayer hostP;
    hostP.peerId = 1;
    hostP.username = s.name.empty() ? "Host" : s.name;
    hostP.isHost = true;
    hostP.ping = s.ping; // unknown stays -1 (drawn as "-"), never a made-up 25 ms
    ctx.st.lobbyRoster.push_back(hostP);

    LobbyPlayer me;
    me.peerId = pId;
    me.username = pName;
    me.serviceTag = sTag;
    me.isHost = false;
    me.isDev = AuthClient::Get().IsDeveloper();
    me.isTester = AuthClient::Get().IsTester();
    me.ping = s.ping;
    ctx.st.lobbyRoster.push_back(me);
  }

  ctx.st.screen = Screen::PreGameLobby;
}

// A line about each game type for the lobby's list. Only what the type itself
// is: a variant's own settings can't be read yet.

void SetupHostLobby(MenuState &st, const std::string &mapScenario,
                           const std::string &mode, const std::string &varPath,
                           const std::string &varName, int diff,
                           uint64_t skulls, int maxPlayers,
                           const ModEntry *mod,
                           const std::filesystem::path &mapOverride,
                           const std::string &mapTitle,
                           const std::filesystem::path &mapImage,
                           const std::filesystem::path &mapThumb,
                           const std::string &existingSessionId,
                           const std::string &existingSessionToken) {
  st.isLobbyHost = true;
  st.lobbyMap = mapScenario;
  st.lobbyMapOverride = mapOverride;
  st.lobbyMapVariant.clear(); // the Custom Game / Forge screen sets it after
                              // this for a Forge map
  st.lobbyMapTitle = mapTitle;
  st.lobbyMapImage = mapImage;
  st.lobbyMapThumb = mapThumb;
  st.lobbyMode = mode;
  st.lobbyVariantPath = varPath;
  st.lobbyVariantName = varName;
  st.lobbyModId = mod ? mod->workshopId : std::string();
  st.lobbyModTitle = mod ? mod->title : std::string();
  st.lobbyModFolder = mod ? mod->folder : std::filesystem::path();
  st.lobbyDifficulty = diff;
  st.lobbySkulls = skulls;
  // The lobby header, the master advert and the LAN beacon all read this number, so clamp it to the seats
  // the engine session can really declare (16 today; 24/32 need the dev-only 32-player slot patch) instead
  // of advertising a count no session can seat.
  const int effectiveCap = EffectiveMultiplayerCap();
  if (maxPlayers > effectiveCap) {
    NOBLE_INFO("FrontEndLobby: max players requested {} -> session seats {} "
               "(17-32 needs the 32-player slot patch, dev-only)",
               maxPlayers, effectiveCap);
    maxPlayers = effectiveCap;
  }
  st.lobbyMaxPlayers = maxPlayers;
  st.lobbyLaunchPending = false;
  st.lobbyMenuRow = 0;
  NOBLE_INFO("FrontEndMenu: host lobby set up: map '{}' mode '{}' game type "
             "'{}' file '{}'",
             mapScenario, mode, varName,
             std::filesystem::path(varPath).filename().string());
  ClearPendingLobbyEvents();
  st.lastLobbyPoll = ImGui::GetTime();

  HostConfig cfg;
  std::string pName = AuthClient::Get().IsLoggedIn()
                          ? AuthClient::Get().GetUsername()
                          : Settings::Get().Snapshot().playerName;
  if (pName.empty())
    pName = "Host";
  cfg.sessionName = pName + "'s Lobby";
  cfg.gameMode = st.lobbyMode;
  cfg.mapName = st.lobbyMap;
  cfg.gametypeName = st.lobbyVariantName;
  cfg.modId = st.lobbyModId;
  cfg.modTitle = st.lobbyModTitle;
  cfg.maxPlayers = st.lobbyMaxPlayers;
  cfg.port = 11774;
  cfg.mode =
      (st.lobbyNetMode == 1) ? NetworkMode::Online_P2P : NetworkMode::Local_LAN;
  SessionManager::Get().StartHosting(cfg);

  if (st.lobbyNetMode == 1 && MasterServerClient::Get().Enabled()) {
    MasterAdvert adv;
    if (!existingSessionId.empty()) {
      adv.id = existingSessionId;
      adv.token = existingSessionToken;
      NOBLE_INFO("FrontEndMenu: Reusing existing session id '{}' for lobby",
                 existingSessionId);
    }
    adv.name = cfg.sessionName;
    adv.port = cfg.port;
    adv.maxPlayers = cfg.maxPlayers;
    adv.map = cfg.mapName;
    adv.mode = cfg.gameMode;
    adv.variant = st.lobbyVariantName;
    adv.difficulty = st.lobbyDifficulty;
    adv.skulls = st.lobbySkulls;
    adv.isMod = mod != nullptr || !st.lobbyModTitle.empty();
    adv.modId = st.lobbyModId;
    adv.modTitle = st.lobbyModTitle;
    adv.inProgress = false;
    adv.isPrivate = (st.lobbyPrivacy != 0);
    adv.hostPeerId = AuthClient::Get().IsLoggedIn()
                         ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                         : 1ULL;
    adv.hostMachineId = "0x484F535400000001";
    adv.hostUsername = pName;
    adv.hostIsDev = AuthClient::Get().IsDeveloper();
    adv.hostIsTester = AuthClient::Get().IsTester();
    adv.hostServiceTag = PlayerServiceTag(Settings::Get().Snapshot());
    // The host's own row identity, so guests can draw it (rank / emblem /
    // nameplate / team).
    {
      const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
      adv.hostRankIndex = Career::RankIndex(Career::Get().credits);
      adv.hostEmblemId = prof.emblemIndex;
      adv.hostEmblemBackgroundId = prof.emblemBackgroundIndex;
      adv.hostNameplateId = prof.nameplateIndex;
      const auto rec = Career::GetRecord();
      int cd = 0;
      for (const auto &c : Career::Challenges())
        if (c.done)
          ++cd;
      int ct = 0, ctm = 0;
      for (const auto &c : Career::Commendations()) {
        if (c.tracked) {
          ct += c.tier;
          ctm += 6;
        }
      }
      const int commPct = ctm > 0 ? (ct * 100 / ctm) : 0;
      adv.hostGamesPlayed = static_cast<int>(rec.totals.gamesPlayed);
      adv.hostKills = static_cast<int>(rec.totals.kills);
      adv.hostDeaths = static_cast<int>(rec.totals.deaths);
      adv.hostChallengesDone = cd;
      adv.hostCommendations = commPct;
      adv.hostArmoryOwned = static_cast<int>(rec.totals.armoryOwned.size());
    }
    MasterServerClient::Get().StartAdvertising(adv);
    st.lobbySessionId = MasterServerClient::Get().GetCurrentSessionId();
    st.lobbySessionToken = MasterServerClient::Get().GetCurrentSessionToken();
  }

  st.lobbyRoster.clear();
  LobbyPlayer me;
  me.peerId = AuthClient::Get().IsLoggedIn()
                  ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                  : 1ULL;
  me.machineId = DeviceIdentityStore::Get().deviceId;
  me.username = pName;
  me.serviceTag = PlayerServiceTag(Settings::Get().Snapshot());
  me.isHost = true;
  me.isDev = AuthClient::Get().IsDeveloper();
  me.isTester = AuthClient::Get().IsTester();
  me.ping = 0;
  {
    const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
    me.rankIndex = Career::RankIndex(Career::Get().credits);
    me.emblemId = prof.emblemIndex;
    me.emblemBackgroundId = prof.emblemBackgroundIndex;
    me.nameplateId = prof.nameplateIndex;
    const auto rec = Career::GetRecord();
    int cd = 0;
    for (const auto &c : Career::Challenges())
      if (c.done)
        ++cd;
    int ct = 0, ctm = 0;
    for (const auto &c : Career::Commendations()) {
      if (c.tracked) {
        ct += c.tier;
        ctm += 6;
      }
    }
    const int commPct = ctm > 0 ? (ct * 100 / ctm) : 0;
    me.gamesPlayed = static_cast<int>(rec.totals.gamesPlayed);
    me.kills = static_cast<int>(rec.totals.kills);
    me.deaths = static_cast<int>(rec.totals.deaths);
    me.challengesDone = cd;
    me.commendations = commPct;
    me.armoryOwned = static_cast<int>(rec.totals.armoryOwned.size());
  }
  st.lobbyRoster.push_back(me);

  st.screen = Screen::PreGameLobby;
}

// The setup screens (Campaign, Firefight, Custom Game, Forge) all end their
// right column the same way: the two action buttons at the bottom, a strip of
// network/lobby chips above them, and the banner and option rows above that.
// These three heights come from one place so a screen cannot draw the strip on
// top of the rows above it (the overlap on the 2026-10-30 screenshots).


void EnqueueLobbyLaunch(const std::string &lMap, const std::string &lMode,
                        const std::string &lVar, int lDiff, uint64_t lSkulls,
                        const std::string &hIp, uint16_t hPort,
                        const std::string &sId, const std::string &lMapOverride,
                        const std::string &lModFolder,
                        const std::string &lModId,
                        const std::string &lMapVariant) {
  std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
  if (s_lobbyEvents.launch)
    return;
  s_lobbyEvents.launch = true;
  s_lobbyEvents.map = lMap;
  s_lobbyEvents.mode = lMode;
  s_lobbyEvents.variant = lVar;
  s_lobbyEvents.difficulty = lDiff;
  s_lobbyEvents.skulls = lSkulls;
  if (!hIp.empty())
    s_lobbyEvents.hostEndpoint = hIp + ":" + std::to_string(hPort);
  s_lobbyEvents.sessionId = sId;
  s_lobbyEvents.mapOverride = lMapOverride;
  s_lobbyEvents.mapVariant = lMapVariant;
  s_lobbyEvents.modFolder = lModFolder;
  s_lobbyEvents.modId = lModId;
}

void EnqueueLobbyClosed(const std::string &reason) {
  std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
  s_lobbyEvents.closed = true;
  s_lobbyEvents.closeReason = reason;
}

void EnqueueLobbyRoster(std::vector<LobbyPlayer> &&updated) {
  std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
  s_lobbyEvents.roster = true;
  s_lobbyEvents.rosterPlayers = std::move(updated);
}

void RefreshLobbyMode(OverlayContext &ctx) {
  MenuState &st = ctx.st;
  // Key on the mode too: with no variant file chosen the old check re-ran every
  // frame and cleared lobbyTeamsManual, so the host's chosen team was
  // immediately re-dealt (always back to red or blue).
  const std::string key = st.lobbyMode + "|" + st.lobbyVariantPath;
  if (st.lobbyModeKey == key)
    return;
  st.lobbyModeKey = key;
  st.lobbyTeamsOn = (st.lobbyMode == "forge");
  // Forge sessions only honour red and blue (the engine ignores the side
  // override there), so the lobby's team count is pinned to 2 and anyone on a
  // higher team is dealt back onto red/blue.
  if (st.lobbyMode == "forge")
    st.lobbyTeamCount = 2; // Forge lobbies support team dividers by default
  st.lobbyTeamsManual = false;
  st.lobbyModeChanges = 0;
  st.lobbyModeSummary.clear();
  if (st.lobbyVariantPath.empty())
    return;
  const std::filesystem::path path(st.lobbyVariantPath);
  VariantRules::Preset preset;
  const bool isMode =
      VariantRules::IsPresetFile(path) && VariantRules::Load(path, preset);
  const std::filesystem::path base =
      isMode ? VariantBasePath(ctx.reachDir, preset.base) : path;
  std::vector<uint8_t> decoded;
  if (!VariantRules::Decode(base, decoded))
    return;
  if (isMode)
    VariantRules::Apply(preset, decoded.data());
  for (const VariantRules::Rule &rule : VariantRules::Rules()) {
    if (std::strcmp(rule.key, "teams") == 0) {
      st.lobbyTeamsOn = (VariantRules::Read(rule, decoded.data()) != 0) ||
                        (st.lobbyMode == "forge");
    }
    const auto it = preset.values.find(rule.key);
    if (isMode && it != preset.values.end()) {
      ++st.lobbyModeChanges;
      if (st.lobbyModeChanges <= 5)
        st.lobbyModeSummary +=
            std::string(st.lobbyModeSummary.empty() ? "" : ", ") + rule.label +
            " " + RuleValueText(rule, it->second);
    }
  }
  if (st.lobbyModeChanges > 5)
    st.lobbyModeSummary +=
        " and " + std::to_string(st.lobbyModeChanges - 5) + " more";
  // Firefight and Campaign are cooperative modes: teams are strictly disabled.
  if (st.lobbyMode == "firefight" || st.lobbyMode == "campaign") {
    st.lobbyTeamsOn = false;
    st.lobbyTeamCount = 1;
  }
  NOBLE_INFO("FrontEndMenu: lobby game type: teams {}, {} rule(s) changed",
             st.lobbyTeamsOn ? "on" : "off", st.lobbyModeChanges);
}

// Host only: everyone on a team while the variant has teams, nobody when it
// doesn't. Players are dealt in roster order, so a joining player doesn't
// reshuffle anyone. Empty when the roster already matches.
void AssignLobbyTeams(MenuState &st) {
  if (st.lobbyTeamsManual)
    return; // the host chose teams deliberately; don't overwrite them
  if (st.lobbyMode == "firefight" || st.lobbyMode == "campaign") {
    st.lobbyTeamsOn = false;
    st.lobbyTeamCount = 1;
  }
  std::vector<std::pair<uint64_t, int>> changes;
  const int teams = std::clamp(st.lobbyTeamCount, 2, kTeamCount);
  for (size_t i = 0; i < st.lobbyRoster.size(); ++i) {
    const int want = st.lobbyTeamsOn
                         ? static_cast<int>(i % static_cast<size_t>(teams)) + 1
                         : 0;
    if (st.lobbyRoster[i].team != want) {
      st.lobbyRoster[i].team = want;
      changes.emplace_back(st.lobbyRoster[i].peerId, want);
      if (st.lobbyRoster[i].isHost)
        MasterServerClient::Get().SetAdvertHostTeam(want);
    }
  }
  if (changes.empty() || st.teamsPushFailed)
    return;
  if (MasterServerClient::Get().SetTeamsSync(st.lobbySessionId, changes)) {
    NOBLE_INFO("FrontEndMenu: teams updated for {} player(s), teams {}",
               changes.size(), st.lobbyTeamsOn ? "on" : "off");
  } else {
    // An older server: the host still sees teams, and this stops asking every
    // poll.
    st.teamsPushFailed = true;
    NOBLE_WARN("FrontEndMenu: the server didn't accept the team list");
  }
}

// Opens the game options editor on a variant; Custom Game and Firefight share
// it.

void StartLobbyVote(OverlayContext &ctx) {
  MenuState &st = ctx.st;
  st.voteMessage.clear();
  if (st.playlistChoice < 0 ||
      st.playlistChoice >= static_cast<int>(st.playlists.size()))
    return;
  const Playlists::Playlist &playlist = st.playlists[st.playlistChoice];
  std::vector<Playlists::Entry> pool = playlist.entries;
  static std::mt19937 s_random{std::random_device{}()};
  std::shuffle(pool.begin(), pool.end(), s_random);
  st.voteEntries.clear();
  std::vector<std::string> labels;
  for (const Playlists::Entry &entry : pool) {
    if (labels.size() >= 3)
      break;
    ResolvedEntry resolved;
    if (!ResolveEntry(ctx, entry, playlist.randomType, resolved))
      continue;
    Playlists::Entry fixed = entry;
    fixed.variant =
        resolved.variant->path.filename()
            .string(); // a random type is settled before anyone votes
    st.voteEntries.push_back(fixed);
    labels.push_back(resolved.map.name + " - " + resolved.variant->label);
  }
  if (labels.size() < 2) {
    st.voteMessage =
        "A vote needs at least 2 playable entries in the playlist.";
  } else if (!MasterServerClient::Get().StartVoteSync(st.lobbySessionId, labels,
                                                      20)) {
    st.voteMessage = "The server didn't accept the vote. It may need updating.";
    MenuSounds::Error();
  } else {
    NOBLE_INFO("FrontEndMenu: vote opened with {} options", labels.size());
    st.voteCast = -1;
    MenuSounds::Click();
  }
}

// Host only, when time is up. "None of these" on top draws three new options
// once; a second time the lobby stays as it is.
void FinishLobbyVote(OverlayContext &ctx) {
  MenuState &st = ctx.st;
  MasterServerClient::Get().EndVoteSync(st.lobbySessionId);
  std::vector<int> counts = st.voteCounts;
  counts.resize(st.voteEntries.size() + 1, 0);
  const int none = counts.back();
  const int best = *std::max_element(counts.begin(), counts.end() - 1);
  st.voteId.clear();
  if (none > best || best == 0) {
    NOBLE_INFO("FrontEndMenu: vote closed, none of these ({} votes, reroll {})",
               none, st.voteRerolls);
    if (none > best && st.voteRerolls < 1) {
      ++st.voteRerolls;
      StartLobbyVote(ctx);
    } else {
      st.voteRerolls = 0;
      st.voteMessage = "No choice won, so the lobby stays as it is.";
    }
    return;
  }
  std::vector<int> leaders;
  for (int i = 0; i + 1 < static_cast<int>(counts.size()); ++i)
    if (counts[i] == best)
      leaders.push_back(i);
  static std::mt19937 s_random{std::random_device{}()};
  const int winner = leaders[s_random() % leaders.size()];
  st.voteRerolls = 0;
  ResolvedEntry resolved;
  if (ResolveEntry(ctx, st.voteEntries[winner], false, resolved)) {
    NOBLE_INFO("FrontEndMenu: vote closed, option {} won with {} votes", winner,
               best);
    ApplyEntryToLobby(st, resolved);
  }
}

// ---------------------------------------------------------------------------------------------
// Reference-Matched Tactical UI Components (Tile Map Pickers, Variants Drawer,
// Top Telemetry, Game Options)
// ---------------------------------------------------------------------------------------------


static void DrawLobbyMapPickerModal(OverlayContext &ctx, const Ui &ui,
                                    float left, float top, bool back) {
  MenuState &st = ctx.st;
  const bool campaign = (st.lobbyMode == "campaign");
  const bool ff = (st.lobbyMode == "firefight");
  const bool forge = (st.lobbyMode == "forge");
  const char *title = campaign ? "CAMPAIGN MAP SELECT"
                      : ff     ? "FIREFIGHT MAP SELECT"
                      : forge  ? "FORGE MAP SELECT"
                               : "LOBBY MAP SELECT";

  DrawTopTelemetryBar(ui, "MAP SELECT");
  DrawTitle(ui, ImVec2(left, top), title, "LOBBY");
  DrawParty(ui, Settings::Get().Snapshot().playerName,
            st.isLobbyHost ? "HOSTING LOBBY" : "IN LOBBY");

  const std::vector<const ModEntry *> &modList = campaign ? ctx.campaignMods
                                                 : ff     ? ctx.ffMods
                                                          : ctx.mpMods;
  const MenuMap *baseMaps = campaign ? kMissions : ff ? kFirefightMaps : kMaps;
  const int baseCount = campaign ? Count(kMissions)
                        : ff     ? Count(kFirefightMaps)
                                 : Count(kMaps);
  const ModMode modMode = campaign ? ModMode::Campaign
                          : ff     ? ModMode::Firefight
                                   : ModMode::Multiplayer;
  const std::string carrier = campaign ? "m10"
                              : ff     ? "ff60_icecave"
                              : forge  ? "forge_halo"
                                       : "70_boneyard";
  const int forgeCount =
      (!campaign && !ff) ? static_cast<int>(ctx.forgeMaps.size()) : -1;

  static int s_lobbyPickerSource = 0;
  static int s_lobbyPickerIdx = 0;
  static std::filesystem::path s_lobbyPickerMvar;
  static bool s_lobbyPickerInit = false;
  if (!s_lobbyPickerInit) {
    s_lobbyPickerSource = !st.lobbyModFolder.empty() ? 1 : 0;
    s_lobbyPickerMvar = st.lobbyMapVariant;
    s_lobbyPickerInit = true;
  }

  float tabsEnd = 0.0f;
  if (DrawSourceTabs(ui, ImVec2(left + 26.0f * ui.s, top + 72.0f * ui.s),
                     s_lobbyPickerSource, static_cast<int>(modList.size()),
                     forgeCount, &tabsEnd)) {
    st.mapDrawerKey.clear();
    st.mapDrawerFocus = -1;
    st.mapGridScrollY = 0.0f;
  }
  const bool modded = (s_lobbyPickerSource == 1);
  const bool forgeTab = (s_lobbyPickerSource == 2 && forgeCount >= 0);
  const float searchX =
      std::max(left + 26.0f * ui.s + 240.0f * ui.s, tabsEnd + 10.0f * ui.s);
  DrawSearchBox(ui, "##lobby_map_search", ImVec2(searchX, top + 72.0f * ui.s),
                180.0f * ui.s, st.mapSearch, sizeof(st.mapSearch),
                "Search maps & variants");

  const std::vector<MapTileGroup> groups =
      BuildMapTileGroups(ctx, modded, forgeTab, modMode, modList, baseMaps,
                         baseCount, carrier, !campaign && !ff, st.mapSearch);

  const float gridX = left + 24.0f * ui.s;
  const float gridY = top + 114.0f * ui.s;
  const float gridBottom = ui.origin.y + ui.size.y - 68.0f * ui.s;
  const float rightMax = ui.origin.x + ui.size.x * 0.94f;
  const float gridW = (rightMax - gridX) * 0.59f;
  const float colGap = 24.0f * ui.s;
  const float cardX = gridX + gridW + colGap;
  const float cardW = rightMax - cardX;
  const float cardY = std::max(gridY, ui.origin.y + 150.0f * ui.s);
  const float cardH = gridBottom - cardY;

  const TacticalGridResult gRes = DrawTacticalMapGrid(
      ui, st, ImVec2(gridX, gridY), ImVec2(gridW, gridBottom - gridY), groups,
      s_lobbyPickerIdx, s_lobbyPickerMvar, true,
      modded ? "No matching modded maps"
             : (forgeTab ? "No matching Forge maps" : "No maps found"),
      campaign ? "BASE MISSION" : "BASE MAP", false, modMode);

  // Right Preview & Confirm Card
  const MapTileGroup *selGrp =
      groups.empty() ? nullptr
                     : &groups[std::clamp(s_lobbyPickerIdx, 0,
                                          static_cast<int>(groups.size()) - 1)];
  const MapEntry *selBase = selGrp ? &selGrp->base : nullptr;
  const MapEntry *selVar = nullptr;
  if (selGrp && !s_lobbyPickerMvar.empty()) {
    for (const MapEntry &v : selGrp->variants)
      if (v.mvar == s_lobbyPickerMvar) {
        selVar = &v;
        break;
      }
  }
  const ImVec2 rc0(cardX, cardY), rc1(cardX + cardW, cardY + cardH);
  ui.dl->AddRectFilled(rc0, rc1, IM_COL32(8, 14, 22, 215), 2.0f * ui.s);
  NobleUi::DrawTacticalFrame(ui.dl, rc0, rc1, ui.s, false, true, false, false);
  const float previewH =
      std::clamp(cardH * 0.38f, 95.0f * ui.s, cardW * 292.0f / 668.0f);
  DrawMapDetailHeaderCard(ui, rc0, cardW, previewH, selBase, selVar,
                          Upper(st.lobbyMode).c_str());

  const bool canPick = selBase && selBase->enabled;
  const float btnY = rc1.y - 62.0f * ui.s;
  const float bw = (cardW - 32.0f * ui.s - 10.0f * ui.s) * 0.5f;
  const bool applyClick = BigAction(
      ui, "lmp_apply", ImVec2(rc0.x + 16.0f * ui.s, btnY), bw, "SELECT MAP",
      selVar ? selVar->name.c_str() : "Apply to Lobby", true, canPick, false);
  const bool cancelClick = BigAction(
      ui, "lmp_cancel", ImVec2(rc0.x + 16.0f * ui.s + bw + 10.0f * ui.s, btnY),
      bw, "CANCEL", "Return to Lobby", false, true, false);

  if ((gRes.pickedBaseOrVariant || applyClick) && canPick) {
    const MapEntry chosen = selVar ? *selVar : *selBase;
    st.lobbyMap =
        chosen.launchScenario.empty() ? chosen.scenario : chosen.launchScenario;
    st.lobbyMapOverride = chosen.mapOverride;
    st.lobbyMapVariant = selVar ? selVar->mvar : std::filesystem::path{};
    st.lobbyMapTitle =
        (chosen.mapOverride.empty() && st.lobbyMapVariant.empty())
            ? std::string()
            : chosen.name;
    st.lobbyMapImage = chosen.imageLoading;
    st.lobbyMapThumb = chosen.imageThumbnail;
    if (chosen.mod) {
      st.lobbyModId = chosen.mod->workshopId;
      st.lobbyModTitle = chosen.mod->title;
      st.lobbyModFolder = chosen.mod->folder;
    } else {
      st.lobbyModId.clear();
      st.lobbyModTitle.clear();
      st.lobbyModFolder.clear();
    }
    // Keep game type valid on stock maps
    if (st.lobbyMode == "multiplayer" && st.lobbyMapOverride.empty() &&
        !st.lobbyVariantPath.empty() && !ctx.variants.empty()) {
      const std::string vType = VariantPathType(st.lobbyVariantPath);
      if (!vType.empty() && !MapGameTypes::Supports(st.lobbyMap, vType)) {
        for (const MenuVariant &mv : ctx.variants) {
          if (mv.type.empty() || MapGameTypes::Supports(st.lobbyMap, mv.type)) {
            st.lobbyVariantPath = mv.path.string();
            st.lobbyVariantName = mv.label;
            break;
          }
        }
      }
    }
    MasterServerClient::Get().UpdateLobbyAdvert(
        st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty,
        st.lobbySkulls);
    MasterServerClient::Get().UpdateLobbyModAdvert(
        !st.lobbyModId.empty(), st.lobbyModId, st.lobbyModTitle);
    st.lobbyMapPickerOpen = false;
    s_lobbyPickerInit = false;
    st.mapDrawerKey.clear();
    st.mapDrawerFocus = -1;
    MenuSounds::Click();
    return;
  }

  if (cancelClick || (back && st.mapDrawerKey.empty())) {
    st.lobbyMapPickerOpen = false;
    s_lobbyPickerInit = false;
    MenuSounds::Back();
  } else if (back && !st.mapDrawerKey.empty()) {
    st.mapDrawerKey.clear();
    st.mapDrawerFocus = -1;
    MenuSounds::Back();
  }
  DrawHints(ui,
            {{Prompt::Accept, "Select / Expand"},
             {Prompt::Back, "Back"},
             {Prompt::Tabs, "Vanilla / Modded / Forge"}},
            {{InputPrompts::Button::Y, "Y", "Toggle Variants"}});
}

void DrawPreGameLobby(OverlayContext &ctx, const Ui &ui, float left,
                             float top, FrontEndChoice &choice, bool &done,
                             bool back) {
  MenuState &st = ctx.st;
  TakePendingLobbyEvents(st);
  if (st.lobbyMapPickerOpen) {
    DrawLobbyMapPickerModal(ctx, ui, left, top, back);
    return;
  }

  if (st.isLobbyHost) {
    const std::string curId = MasterServerClient::Get().GetCurrentSessionId();
    const std::string curTok =
        MasterServerClient::Get().GetCurrentSessionToken();
    if (!curId.empty())
      st.lobbySessionId = curId;
    if (!curTok.empty())
      st.lobbySessionToken = curTok;
  }

  // Non-blocking asynchronous lobby polling (never stalls the render thread)
  static std::atomic<bool> s_lobbyPollActive{false};
  static std::mutex s_lobbyPollMutex;
  static LobbyState s_latestLobbyState;
  static std::atomic<bool> s_hasNewLobbyState{false};

  const double now = ImGui::GetTime();
  const double lobbyPollInterval =
      st.isLobbyHost ? 8.0
                     : (NetworkTransport::Get().IsRelayConnected() ? 5.0 : 2.5);
  if (now - st.lastLobbyPoll > lobbyPollInterval && !s_lobbyPollActive.load()) {
    st.lastLobbyPoll = now;
    if (st.isLobbyHost && st.lobbySessionId.empty()) {
      st.lobbySessionId = MasterServerClient::Get().GetCurrentSessionId();
      st.lobbySessionToken = MasterServerClient::Get().GetCurrentSessionToken();
    }
    if ((!st.lobbySessionId.empty() ||
         (!st.isLobbyHost && !st.lobbyHostEndpoint.empty())) &&
        MasterServerClient::Get().Enabled()) {
      s_lobbyPollActive.store(true);
      const std::string pollSessionId = st.lobbySessionId;
      const bool pollIsHost = st.isLobbyHost;
      const std::string pollHostEndpoint = st.lobbyHostEndpoint;
      const std::string pName = AuthClient::Get().IsLoggedIn()
                                    ? AuthClient::Get().GetUsername()
                                    : Settings::Get().Snapshot().playerName;
      const uint64_t fallbackId =
          (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL) < 2ULL
              ? 2ULL
              : (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
      const uint64_t pId =
          AuthClient::Get().IsLoggedIn()
              ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
              : fallbackId;
      const std::string sTag = PlayerServiceTag(Settings::Get().Snapshot());
      const bool isDev = AuthClient::Get().IsDeveloper();
      const int rankIdx = Career::RankIndex(Career::Get().credits);
      const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
      const int embIdx = prof.emblemIndex;
      const int embBgIdx = prof.emblemBackgroundIndex;
      const int npIdx = prof.nameplateIndex;

      std::thread([pollSessionId, pollIsHost, pollHostEndpoint, pName, pId,
                   sTag, isDev, rankIdx, embIdx, embBgIdx, npIdx]() {
        LobbyState current =
            MasterServerClient::Get().FetchLobbyStateSync(pollSessionId);
        if (!current.ok && !current.isClosed && !pollIsHost &&
            !pollHostEndpoint.empty()) {
          std::string hostIp = pollHostEndpoint;
          uint16_t hostPort = 11774;
          const size_t colon = hostIp.rfind(':');
          if (colon != std::string::npos) {
            hostPort =
                static_cast<uint16_t>(std::atoi(hostIp.c_str() + colon + 1));
            hostIp = hostIp.substr(0, colon);
          }
          LobbyState rejoined = MasterServerClient::Get().JoinLobbySync(
              pollSessionId, pId, pName.empty() ? "Guest" : pName, isDev, sTag,
              rankIdx, embIdx, embBgIdx, npIdx, 0, 11774, hostIp, hostPort);
          if (rejoined.ok) {
            current = std::move(rejoined);
          }
        }
        {
          std::lock_guard<std::mutex> lk(s_lobbyPollMutex);
          s_latestLobbyState = std::move(current);
          s_hasNewLobbyState.store(true);
        }
        s_lobbyPollActive.store(false);
      }).detach();
    }
  }

  if (s_hasNewLobbyState.exchange(false)) {
    LobbyState current;
    {
      std::lock_guard<std::mutex> lk(s_lobbyPollMutex);
      current = std::move(s_latestLobbyState);
    }
    if (!current.sessionId.empty() && current.sessionId != st.lobbySessionId) {
      st.lobbySessionId = current.sessionId;
    }
    if (current.ok)
      st.lobbyPollFailures = 0;
    if (!current.ok || current.isClosed) {
      if (st.isLobbyHost) {
        // The host owns the session; a failed fetch just means the master
        // server blipped.
      } else if (!current.isClosed) {
        // Never kick a guest while the WebSocket relay is connected or during
        // cross-region / transient HTTP poll drops.
        ++st.lobbyPollFailures;
        if (NetworkTransport::Get().IsRelayConnected()) {
          st.lobbyPollFailures = 1;
        } else if (st.lobbyPollFailures <= 5 ||
                   (st.lobbyPollFailures % 10) == 0) {
          NOBLE_WARN("FrontEndMenu: lobby poll failed ({}, http {}); staying "
                     "in the lobby",
                     st.lobbyPollFailures, current.httpStatus);
        }
      } else {
        // The host explicitly closed or disbanded the session: return to the
        // main menu.
        NOBLE_INFO(
            "FrontEndMenu: lobby closed/disbanded ({}); returning to main menu",
            current.closeReason.empty() ? "host closed" : current.closeReason);
        st.screen = Screen::Main;
        st.lobbySessionId.clear();
        st.lobbySessionToken.clear();
        st.lobbyRoster.clear();
        st.isLobbyHost = false;
        st.lobbyPollFailures = 0;
        NetworkTransport::Get().StopRelay();
        return;
      }
    } else if (current.ok) {
      if (!st.isLobbyHost && !current.roster.empty()) {
        st.lobbyRoster = current.roster;
        SanitizeLobbyRoster(st.lobbyRoster);
      }
      if (!st.isLobbyHost) {
        st.lobbyMap = current.map;
        st.lobbyMode = current.mode;
        st.lobbyVariantName = current.variant;
        st.lobbyDifficulty = current.difficulty;
        st.lobbySkulls = current.skulls;
        if (current.isLan && !current.hostLanIp.empty()) {
          st.lobbyHostEndpoint =
              current.hostLanIp + ":" + std::to_string(current.hostPort);
        }
      }
      if (!current.roster.empty()) {
        st.lobbyRoster = current.roster;
        SanitizeLobbyRoster(st.lobbyRoster);
        if (st.isLobbyHost) {
          std::string pName = AuthClient::Get().IsLoggedIn()
                                  ? AuthClient::Get().GetUsername()
                                  : Settings::Get().Snapshot().playerName;
          if (pName.empty())
            pName = "Host";
          const uint64_t hostPId =
              AuthClient::Get().IsLoggedIn()
                  ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                  : 1ULL;
          const std::string hostTag =
              PlayerServiceTag(Settings::Get().Snapshot());
          const bool hostDev = AuthClient::Get().IsDeveloper();
          bool hostFound = false;
          for (auto &p : st.lobbyRoster) {
            if (p.isHost) {
              hostFound = true;
              p.peerId = hostPId;
              p.machineId = DeviceIdentityStore::Get().deviceId;
              p.username = pName;
              p.serviceTag = hostTag;
              p.isDev = hostDev;
              const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
              p.rankIndex = Career::RankIndex(Career::Get().credits);
              p.emblemId = prof.emblemIndex;
              p.nameplateId = prof.nameplateIndex;
              p.emblemBackgroundId = prof.emblemBackgroundIndex;
              const auto rec = Career::GetRecord();
              p.gamesPlayed = static_cast<int>(rec.totals.gamesPlayed);
              p.kills = static_cast<int>(rec.totals.kills);
              p.deaths = static_cast<int>(rec.totals.deaths);
              int cd = 0;
              for (const auto &c : Career::Challenges())
                if (c.done)
                  ++cd;
              p.challengesDone = cd;
              int ct = 0, ctm = 0;
              for (const auto &c : Career::Commendations()) {
                if (c.tracked) {
                  ct += c.tier;
                  ctm += 6;
                }
              }
              p.commendations = ctm > 0 ? (ct * 100 / ctm) : 0;
              p.armoryOwned = static_cast<int>(rec.totals.armoryOwned.size());
            } else {
              Career::PeerStats peer;
              peer.playerId =
                  !p.machineId.empty()
                      ? p.machineId
                      : (!p.username.empty() ? p.username
                                             : std::to_string(p.peerId));
              peer.playerName = p.username;
              peer.serviceTag = p.serviceTag;
              peer.rank = p.rankIndex;
              peer.gamesPlayed = p.gamesPlayed;
              peer.kills = p.kills;
              peer.deaths = p.deaths;
              peer.challengesDone = p.challengesDone;
              peer.commendations = p.commendations;
              peer.armoryOwned = p.armoryOwned;
              Career::SavePeerStats(peer);
              Career::RegisterPlayerName(p.peerId, p.username, p.serviceTag);
            }
          }
          if (!hostFound) {
            LobbyPlayer hp;
            hp.peerId = hostPId;
            hp.machineId = DeviceIdentityStore::Get().deviceId;
            hp.username = pName;
            hp.serviceTag = hostTag;
            hp.isHost = true;
            hp.isDev = hostDev;
            const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
            hp.rankIndex = Career::RankIndex(Career::Get().credits);
            hp.emblemId = prof.emblemIndex;
            hp.nameplateId = prof.nameplateIndex;
            hp.emblemBackgroundId = prof.emblemBackgroundIndex;
            const auto rec = Career::GetRecord();
            hp.gamesPlayed = static_cast<int>(rec.totals.gamesPlayed);
            hp.kills = static_cast<int>(rec.totals.kills);
            hp.deaths = static_cast<int>(rec.totals.deaths);
            int cd = 0;
            for (const auto &c : Career::Challenges())
              if (c.done)
                ++cd;
            hp.challengesDone = cd;
            int ct = 0, ctm = 0;
            for (const auto &c : Career::Commendations()) {
              if (c.tracked) {
                ct += c.tier;
                ctm += 6;
              }
            }
            hp.commendations = ctm > 0 ? (ct * 100 / ctm) : 0;
            hp.armoryOwned = static_cast<int>(rec.totals.armoryOwned.size());
            st.lobbyRoster.insert(st.lobbyRoster.begin(), hp);
          }
        }
      }
      // A vote: everyone sees it through the session; the host alone closes it
      // and applies the result.
      if (current.voteId != st.voteId)
        st.voteCast = -1;
      st.voteId = current.voteId;
      st.voteOptions = current.voteOptions;
      st.voteCounts = current.voteCounts;
      st.voteMsLeft = current.voteMsLeft;
      if (st.isLobbyHost && !st.voteId.empty() && st.voteMsLeft <= 0)
        FinishLobbyVote(ctx);
      if (st.isLobbyHost && !st.lobbyRoster.empty()) {
        // Every lobby mode: Forge assigns teams too, and a mode without teams
        // deals 0 (no dividers).
        RefreshLobbyMode(ctx);
        AssignLobbyTeams(st);
      }
      // Launch only while the host's match is running. The server keeps the
      // last launch_data when the host re-registers the same address and port
      // for its next lobby, so "launch_data present" alone sent a guest
      // straight into a match that wasn't starting (2026-10-29 review, tested
      // against server.js).
      if (!st.isLobbyHost && current.inProgress) {
        st.lobbyLaunchPending = true;
        st.lobbyLaunchMap =
            current.launchMap.empty() ? current.map : current.launchMap;
        st.lobbyLaunchMode =
            current.launchMode.empty() ? current.mode : current.launchMode;
        st.lobbyLaunchVariant = current.launchVariant;
        st.lobbyLaunchDiff = current.launchDifficulty;
        st.lobbyLaunchSkulls = current.launchSkulls;
        st.lobbyLaunchMapOverride = current.launchMapOverride;
        st.lobbyLaunchMapVariant = current.launchMapVariant;
        st.lobbyLaunchModFolder = current.launchModFolder;
        st.lobbyLaunchModId =
            current.launchModId.empty() ? current.modId : current.launchModId;
      }
    }
  }

  // If launch was signaled (via WebSocket or HTTP poll) and we are guest,
  // launch!
  if (!st.isLobbyHost && st.lobbyLaunchPending && !done) {
    st.lobbyLaunchPending = false;
    choice.play = true;
    for (size_t p = 0; p < st.lobbyRoster.size(); ++p) {
      const auto &player = st.lobbyRoster[p];
      Career::RegisterPlayerName(player.peerId, player.username,
                                 player.serviceTag);
      Career::RegisterPlayerSlot(static_cast<int>(p), player.username,
                                 player.serviceTag);
    }
    choice.host = false;
    // Loading screen roster: host first, then the guests.
    choice.rosterNames.clear();
    for (int pass = 0; pass < 2; ++pass) {
      for (const auto &player : st.lobbyRoster) {
        if ((pass == 0) != player.isHost) continue;
        choice.rosterNames.push_back(player.username.empty() ? "Host" : player.username);
      }
    }
    choice.map = st.lobbyLaunchMap.empty() ? st.lobbyMap : st.lobbyLaunchMap;
    choice.mapTitle = st.lobbyMapTitle;
    choice.mapImage = st.lobbyMapImage;
    choice.mapThumb = st.lobbyMapThumb;
    choice.mode =
        st.lobbyLaunchMode.empty() ? st.lobbyMode : st.lobbyLaunchMode;
    choice.variantPath = st.lobbyLaunchVariant;
    choice.difficulty = st.lobbyLaunchDiff;
    choice.skulls = st.lobbyLaunchSkulls;

    // Resolve mod and modded map override for guest
    std::filesystem::path hostMapOverride =
        !st.lobbyLaunchMapOverride.empty()
            ? std::filesystem::path(st.lobbyLaunchMapOverride)
            : st.lobbyMapOverride;
    std::filesystem::path hostModFolder =
        !st.lobbyLaunchModFolder.empty()
            ? std::filesystem::path(st.lobbyLaunchModFolder)
            : st.lobbyModFolder;
    std::string hostModId =
        !st.lobbyLaunchModId.empty() ? st.lobbyLaunchModId : st.lobbyModId;

    // A map override only applies to the map it belongs to. Leftovers from a
    // previous lobby used to be applied to whatever launched next (a campaign
    // mission loading the previous lobby's mod map, then failing mid-load).
    if (!hostMapOverride.empty()) {
      const std::string overrideStem = hostMapOverride.stem().string();
      const std::string scenario = choice.map;
      const bool belongs = overrideStem == scenario ||
                           overrideStem.rfind(scenario + "_", 0) == 0 ||
                           scenario.rfind(overrideStem + "_", 0) == 0;
      if (!belongs) {
        NOBLE_WARN("FrontEndMenu: ignoring map override '{}' for map '{}' "
                   "(left over from another lobby)",
                   hostMapOverride.filename().string(), scenario);
        hostMapOverride.clear();
        st.lobbyMapOverride.clear();
        st.lobbyLaunchMapOverride.clear();
      }
    }

    if (!hostMapOverride.empty() || !hostModFolder.empty() ||
        !hostModId.empty()) {
      std::filesystem::path resolvedMap;
      std::filesystem::path resolvedModFolder = hostModFolder;
      std::error_code ec;

      if (!hostMapOverride.empty() &&
          std::filesystem::exists(hostMapOverride, ec)) {
        resolvedMap = hostMapOverride;
      }

      const std::string wantMapFile = !hostMapOverride.empty()
                                          ? hostMapOverride.filename().string()
                                          : (choice.map + ".map");

      if (resolvedMap.empty() && !hostModId.empty()) {
        if (const ModEntry *installed =
                ModCatalog::FindByWorkshopId(ctx.mods, hostModId)) {
          resolvedModFolder = installed->folder;
          const auto candidate = installed->folder / "maps" / wantMapFile;
          if (std::filesystem::exists(candidate, ec))
            resolvedMap = candidate;
        }
      }

      if (resolvedMap.empty() && !hostModFolder.empty()) {
        const std::string folderName = hostModFolder.filename().string();
        for (const ModEntry &mod : ctx.mods) {
          if (_stricmp(mod.folder.filename().string().c_str(),
                       folderName.c_str()) == 0 ||
              _stricmp(mod.id.c_str(), folderName.c_str()) == 0) {
            resolvedModFolder = mod.folder;
            const auto candidate = mod.folder / "maps" / wantMapFile;
            if (std::filesystem::exists(candidate, ec)) {
              resolvedMap = candidate;
              break;
            }
          }
        }
      }

      if (resolvedMap.empty()) {
        for (const ModEntry &mod : ctx.mods) {
          const auto candidate = mod.folder / "maps" / wantMapFile;
          if (std::filesystem::exists(candidate, ec)) {
            resolvedMap = candidate;
            resolvedModFolder = mod.folder;
            break;
          }
        }
      }

      if (!resolvedMap.empty()) {
        choice.mapOverride = resolvedMap;
        choice.modFolder = resolvedModFolder;
        st.lobbyMapOverride = resolvedMap;
        st.lobbyModFolder = resolvedModFolder;
        NOBLE_INFO(
            "FrontEndMenu: Guest resolved modded map: '{}' (folder: '{}')",
            choice.mapOverride.string(), choice.modFolder.string());
      } else {
        // The host is on a mod map this PC does not have. Launching the plain
        // map anyway puts the guest in a match on a different map than the host
        // (the engine then errors out or the two desync), so refuse the launch
        // and say which Workshop item is missing instead.
        const bool stockScenario =
            KnownScenario(choice.map, kMissions, Count(kMissions)) ||
            KnownScenario(choice.map, kFirefightMaps, Count(kFirefightMaps)) ||
            KnownScenario(choice.map, kMaps, Count(kMaps));
        if (!stockScenario || !hostModId.empty()) {
          NOBLE_WARN("FrontEndMenu: host's mod map '{}' (mod: '{}') is not on "
                     "this PC; not launching",
                     wantMapFile, hostModId);
          st.lobbyLaunchPending = false;
          return;
        }
        choice.mapOverride.clear();
        choice.modFolder.clear();
        NOBLE_WARN("FrontEndMenu: host launched stock map '{}' without a mod "
                   "map; launching it as is",
                   choice.map);
      }
    } else {
      choice.mapOverride = st.lobbyMapOverride;
      choice.modFolder = st.lobbyModFolder;
    }
    choice.modTitle = st.lobbyModTitle;
    // The host's Forge map arrives either as an uploaded .mvar materialized locally
    // under data/host_map_variants or as a file name matching this PC's forgeMaps.
    choice.mapVariant.clear();
    if (!st.lobbyLaunchMapVariant.empty()) {
      {
        std::error_code mvarEc;
        const std::filesystem::path materialized(st.lobbyLaunchMapVariant);
        if (materialized.is_absolute() &&
            std::filesystem::exists(materialized, mvarEc)) {
          choice.mapVariant = materialized;
        }
      }
      if (choice.mapVariant.empty()) {
        const std::filesystem::path want =
            std::filesystem::path(st.lobbyLaunchMapVariant).filename();
        for (const ForgeMap &fm : ctx.forgeMaps) {
          if (_wcsicmp(fm.path.filename().c_str(), want.c_str()) == 0) {
            choice.mapVariant = fm.path;
            break;
          }
        }
        if (choice.mapVariant.empty()) {
          NOBLE_WARN("FrontEndMenu: host's Forge map '{}' isn't on this PC; "
                     "launching the plain map",
                     want.string());
        }
      }
    }
    NOBLE_INFO("Guest launching from lobby: map='{}' mode='{}' "
               "mapOverride='{}' mod='{}' connect='{}'",
               choice.map, choice.mode, choice.mapOverride.string(),
               choice.modFolder.string(), st.lobbyHostEndpoint);
    choice.connectAddress = st.lobbyHostEndpoint;
    choice.sessionId = st.lobbySessionId;
    if (st.lobbySessionToken.empty())
      st.lobbySessionToken = MasterServerClient::Get().GetCurrentSessionToken();
    choice.sessionToken = st.lobbySessionToken;

    // The host sends its variant as a path on its own PC; use the guest's copy
    // of the same file. Without a variant the guest's launch block has no game
    // variant (the 5% load hang from 2026-10-28).
    if (st.lobbyLaunchVariant.empty() && !st.lobbyVariantName.empty()) {
      st.lobbyLaunchVariant = st.lobbyVariantName;
    }
    if (!st.lobbyLaunchVariant.empty()) {
      const std::filesystem::path want =
          std::filesystem::path(st.lobbyLaunchVariant).filename();
      std::filesystem::path local;
      {
        std::error_code varEc;
        const std::filesystem::path materialized(st.lobbyLaunchVariant);
        if (materialized.is_absolute() && std::filesystem::exists(materialized, varEc)) {
          local = materialized;
        }
      }
      if (local.empty()) {
        for (const auto *list : {&ctx.variants, &ctx.ffVariants})
          for (const MenuVariant &v : *list)
            if (local.empty() &&
                (_wcsicmp(v.path.filename().c_str(), want.c_str()) == 0 ||
                 _stricmp(v.label.c_str(), st.lobbyLaunchVariant.c_str()) == 0))
              local = v.path;
      }
      if (local.empty() &&
          (st.lobbyLaunchMode == "forge" || st.lobbyMode == "forge" ||
           _wcsicmp(want.c_str(), kForgeVariant) == 0)) {
        if (std::filesystem::exists(ctx.forgeVariant))
          local = ctx.forgeVariant;
      }
      if (!local.empty()) {
        choice.variantPath = local.string();
      } else {
        NOBLE_WARN("FrontEndMenu: host's game variant '{}' isn't in this PC's "
                   "variants; launching without it",
                   st.lobbyLaunchVariant);
      }
    } else if (st.lobbyLaunchMode == "forge" || st.lobbyMode == "forge") {
      if (std::filesystem::exists(ctx.forgeVariant))
        choice.variantPath = ctx.forgeVariant.string();
    }
    NOBLE_INFO("FrontEndMenu: Guest executing synchronized lobby launch to {}",
               st.lobbyHostEndpoint);
    done = true;
    return;
  }

  // Same frame as the setup screens: DrawTitle at (left, top), the option
  // column under it, the right column at px/pw, DrawHints at the bottom. Every
  // block has a fixed place, so nothing moves between the empty, one player and
  // full states or between host and guest (design pass, 2026-10-31).
  ui.dl->AddRectFilled(ui.origin,
                       ImVec2(ui.origin.x + ui.size.x, ui.origin.y + ui.size.y),
                       IM_COL32(4, 8, 12, 110));
  const float colX = left + 34.0f * ui.s;
  const float colW = 380.0f * ui.s;
  const float px = ui.origin.x + ui.size.x * 0.47f;
  const float pw = ui.size.x * 0.44f;
  const ImU32 panelFill = IM_COL32(8, 12, 18, 190);
  const ImU32 panelEdge = IM_COL32(170, 200, 220, 120);
  const ImU32 kRuleLine = IM_COL32(255, 255, 255, 50);

  // Resolve Map Display Name. A mod map uses the title from its map JSON; a
  // stock map uses the Reach table. Without this the lobby says "Boneyard" for
  // every Campaign-to-Forge selection because 70_boneyard is only the carrier
  // the engine launches through.
  std::string mapDisplay =
      !st.lobbyMapTitle.empty() ? st.lobbyMapTitle : st.lobbyMap;
  if (st.lobbyMapTitle.empty()) {
    for (int i = 0; i < Count(kMaps); ++i) {
      if (kMaps[i].scenario == st.lobbyMap) {
        mapDisplay = kMaps[i].name;
        break;
      }
    }
    for (int i = 0; i < Count(kFirefightMaps); ++i) {
      if (kFirefightMaps[i].scenario == st.lobbyMap) {
        mapDisplay = kFirefightMaps[i].name;
        break;
      }
    }
    for (int i = 0; i < Count(kMissions); ++i) {
      if (kMissions[i].scenario == st.lobbyMap) {
        mapDisplay = kMissions[i].name;
        break;
      }
    }
  }

  const bool campaign = st.lobbyMode == "campaign";
  const bool forge = st.lobbyMode == "forge";
  const bool firefight = st.lobbyMode == "firefight";
  DrawTitle(ui, ImVec2(left, top),
            campaign    ? "CAMPAIGN"
            : forge     ? "FORGE"
            : firefight ? "FIREFIGHT"
                        : "CUSTOM GAME",
            campaign    ? "CO-OP"
            : forge     ? "EDIT"
            : firefight ? "SURVIVAL"
                        : "MULTIPLAYER");

  const char *subtitle =
      !st.isLobbyHost
          ? (st.lobbyHostEndpoint.empty()
                 ? "Connecting to the host..."
                 : "The host chooses the game and starts the match.")
      : st.lobbyNetMode == 0 ? "Local network game."
      : st.lobbyPrivacy == 0 ? "Public game, listed in the server browser."
                             : "Private game, join by invite.";
  Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(colX, top + 70.0f * ui.s),
       kDim, subtitle);

  struct LobbyMenuOption {
    std::string label;
    std::string value;
    bool isStart;
    bool editable;
    std::string tooltip;
  };
  std::vector<LobbyMenuOption> optRows;
  const char *const kReadyTip =
      "Your own ready state. It isn't sent to the host yet.";
  const char *const kOptionsTip =
      "Uses the game type's own settings. Edit them from GAME OPTIONS on "
      "Custom Game, before hosting.";
  const bool playlistActive =
      st.isLobbyHost && st.playlistChoice >= 0 &&
      st.playlistChoice < static_cast<int>(st.playlists.size());
  // A guest: is the host's game type one this PC has? A host's own game mode
  // isn't sent to guests yet (@deepseek's transport); until it is, say so
  // rather than let the player find out from the match.
  bool hostModeKnown = st.isLobbyHost || st.lobbyVariantName.empty() ||
                       st.lobbyVariantName == "Default";
  if (!hostModeKnown) {
    for (const auto *list : {&ctx.variants, &ctx.ffVariants}) {
      for (const MenuVariant &v : *list) {
        if (_stricmp(v.label.c_str(), st.lobbyVariantName.c_str()) == 0 ||
            _stricmp(v.path.filename().string().c_str(),
                     st.lobbyVariantName.c_str()) == 0)
          hostModeKnown = true;
      }
    }
  }
  const char *const hostModeTip =
      hostModeKnown ? "You have the game type the host picked."
                    : "The host made this game mode, and this PC doesn't have "
                      "it. Downloading it from the host is coming.";

  // Teams are a property of the game type, whatever the lobby mode: read them
  // before the rows are built so the roster (and the TEAM COUNT / CHANGE TEAM
  // rows) agree with the mode from the first frame.
  RefreshLobbyMode(ctx);

  if (campaign) {
    optRows.push_back({"READY", st.lobbyReady ? "READY" : "NOT READY", false,
                       true, kReadyTip});
    optRows.push_back({"MISSION", mapDisplay, false, st.isLobbyHost,
                       "Select which campaign mission to play."});
    int skullCount = 0;
    for (int b = 0; b < 64; ++b)
      if ((st.lobbySkulls >> b) & 1ULL)
        ++skullCount;
    optRows.push_back(
        {"SKULLS",
         skullCount >= kSkullCount ? std::string("ALL ON")
                                   : std::to_string(skullCount) + " ON",
         false, false,
         "Skulls are chosen on the Campaign screen before hosting."});
    static const char *kDiffs[] = {"EASY", "NORMAL", "HEROIC", "LEGENDARY"};
    optRows.push_back({"DIFFICULTY",
                       kDiffs[std::clamp(st.lobbyDifficulty, 0, 3)], false,
                       st.isLobbyHost, "Set campaign difficulty level."});
    if (st.isLobbyHost)
      optRows.push_back({"START CO-OP CAMPAIGN", "", true, true,
                         "Start the selected mission."});
  } else if (forge) {
    optRows.push_back({"GAME TYPE", "BASIC EDITING", false, false,
                       "Forge uses Basic Editing as base."});
    optRows.push_back({"MAP", mapDisplay, false, st.isLobbyHost,
                       "Choose which map to forge."});
    if (st.lobbyTeamsOn) {
      // Forge is red/blue only: the engine ignores side overrides in a Forge
      // session, so the lobby must not offer colours the match cannot show
      // (there is no TEAM COUNT row here for the same reason).
      constexpr int teams = 2;
      if (!st.lobbyRoster.empty()) {
        const LobbyPlayer &selectedPlayer = st.lobbyRoster[std::clamp(
            st.lobbyRosterSelection, 0,
            static_cast<int>(st.lobbyRoster.size()) - 1)];
        const int shownTeam = selectedPlayer.team > 0
                                  ? std::clamp(selectedPlayer.team, 1, teams)
                                  : 0;
        optRows.push_back(
            {"CHANGE TEAM",
             shownTeam > 0 ? std::string(TeamStyleFor(shownTeam).shortName)
                           : std::string("AUTO"),
             false, st.isLobbyHost,
             "Opens the team list: pick a team by name and colour."});
      }
    }
    const std::string forgeOptVal =
        st.lobbyModeChanges > 0
            ? std::to_string(st.lobbyModeChanges) + " CHANGED"
            : (st.forgeCustomVariant.empty() ? "DEFAULTS" : "CUSTOM");
    optRows.push_back({"GAME OPTIONS", forgeOptVal, false, st.isLobbyHost,
                       "Customize player traits, shields, speed, damage, and "
                       "game options for Forge."});
    if (st.isLobbyHost)
      optRows.push_back(
          {"START GAME", "", true, true, "Start the selected Forge session."});
  } else if (firefight) {
    optRows.push_back({"READY", st.lobbyReady ? "READY" : "NOT READY", false,
                       true, kReadyTip});
    optRows.push_back(
        {"GAME TYPE",
         st.lobbyVariantName.empty() ? "NORMAL" : st.lobbyVariantName, false,
         st.isLobbyHost, "Select Firefight game variant."});
    optRows.push_back({"MAP", mapDisplay, false, st.isLobbyHost,
                       "Choose which Firefight map to play."});
    if (st.isLobbyHost)
      optRows.push_back(
          {"GAME OPTIONS", "DEFAULTS", false, false, kOptionsTip});
    else
      optRows.push_back({"HOST MODE", hostModeKnown ? "ON THIS PC" : "UNKNOWN",
                         false, false, hostModeTip});
    if (st.isLobbyHost)
      optRows.push_back(
          {"START FIREFIGHT", "", true, true, "Start the Firefight match."});
  } else { // Custom Game
    optRows.push_back({"READY", st.lobbyReady ? "READY" : "NOT READY", false,
                       true, kReadyTip});
    optRows.push_back(
        {"GAME TYPE",
         st.lobbyVariantName.empty() ? "SLAYER" : st.lobbyVariantName, false,
         st.isLobbyHost, "Choose game type and rule set."});
    optRows.push_back({"MAP", mapDisplay, false, st.isLobbyHost,
                       "Choose which map you want to play."});
    if (st.lobbyTeamsOn) {
      // Multi-team: the host sets how many teams the lobby deals, and any team
      // can be handed out.
      const int teams = std::clamp(st.lobbyTeamCount, 2, kTeamCount);
      char countTip[200];
      std::snprintf(countTip, sizeof(countTip),
                    "This game type assigns teams. %d team%s in this lobby; "
                    "the host deals players across them. "
                    "Halo: Reach's multi-team modes use up to 8.",
                    teams, teams == 1 ? "" : "s");
      optRows.push_back({"TEAM COUNT", std::to_string(teams), false,
                         st.isLobbyHost, countTip});
      if (!st.lobbyRoster.empty()) {
        const LobbyPlayer &selectedPlayer = st.lobbyRoster[std::clamp(
            st.lobbyRosterSelection, 0,
            static_cast<int>(st.lobbyRoster.size()) - 1)];
        const int shownTeam = selectedPlayer.team > 0
                                  ? std::clamp(selectedPlayer.team, 1, teams)
                                  : 0;
        const std::string teamText =
            shownTeam > 0 ? std::string(TeamStyleFor(shownTeam).shortName)
                          : std::string("AUTO");
        char teamTip[220];
        std::snprintf(teamTip, sizeof(teamTip),
                      "Opens the team list for %s: pick a team by name and "
                      "colour. The host owns the teams"
                      " in this lobby.",
                      selectedPlayer.username.empty()
                          ? "the host"
                          : selectedPlayer.username.c_str());
        optRows.push_back(
            {"CHANGE TEAM", teamText, false, st.isLobbyHost, teamTip});
      }
    }
    const std::string modeValue =
        st.lobbyModeChanges > 0
            ? std::to_string(st.lobbyModeChanges) + " CHANGED"
            : std::string("DEFAULTS");
    const std::string modeTip = st.lobbyModeChanges > 0
                                    ? st.lobbyModeSummary
                                    : std::string(kOptionsTip);
    if (playlistActive)
      optRows.push_back(
          {"VOTE", "START", false, false,
           st.lobbyModeChanges > 0
               ? "Vote on what's next. This mode: " + st.lobbyModeSummary
               : "Press to let everyone vote on what to play next from this "
                 "playlist."});
    else if (st.isLobbyHost)
      optRows.push_back({"GAME OPTIONS", modeValue, false, false, modeTip});
    else
      optRows.push_back({"HOST MODE", hostModeKnown ? "ON THIS PC" : "UNKNOWN",
                         false, false, hostModeTip});
    if (st.isLobbyHost)
      optRows.push_back(
          {"START GAME", "", true, true, "Start the selected custom game."});
  }

  const int totalRows = static_cast<int>(optRows.size());
  st.lobbyMenuRow = std::clamp(st.lobbyMenuRow, 0, totalRows - 1);
  if (st.lobbyRoster.empty())
    st.lobbyFocusZone = 0;

  // Controller & Keyboard Navigation
  auto stepRow = [&](int d) {
    st.lobbyMenuRow = (st.lobbyMenuRow + d + totalRows) % totalRows;
    MenuSounds::Scroll();
  };
  if (!st.lobbyPickerOpen && !st.lobbyTeamPickerOpen && !st.lobbyProfileOpen &&
      st.lobbyFocusZone == 0) {
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || UiPad::Down())
      stepRow(1);
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || UiPad::Up())
      stepRow(-1);
  }

  // A vanilla map only hosts the game types it was built for (Invasion on Sword
  // Base crashed the engine), so the lobby's cycling skips what the Custom Game
  // screen refuses. Modded lobbies are never restricted.
  auto comboAllowed = [&](const std::string &scenario,
                          const std::string &variantPath) {
    if (campaign || firefight || forge || !st.lobbyModFolder.empty() ||
        variantPath.empty())
      return true;
    const std::string type = VariantPathType(variantPath);
    return type.empty() || MapGameTypes::Supports(scenario, type);
  };

  // The pre-game map row offers the maps from the host's selected mod first,
  // then the stock Reach table. A Campaign-to-Forge map keeps its own title and
  // art even though the engine launches it through 70_boneyard; a stock choice
  // clears the override again.
  struct LobbyMapChoice {
    std::string
        launch; // scenario the engine launches (the carrier for a mod map)
    std::string label; // title to show
    std::filesystem::path overrideFile;
    std::filesystem::path image;
    std::filesystem::path thumb;
    std::filesystem::path mvar; // Forge map laid over `launch`
  };
  const MenuMap *baseMaps =
      campaign ? kMissions : (firefight ? kFirefightMaps : kMaps);
  const int baseCount =
      campaign ? Count(kMissions)
               : (firefight ? Count(kFirefightMaps) : Count(kMaps));
  const std::string carrier =
      campaign ? "m10" : (firefight ? "ff60_icecave" : "70_boneyard");
  const ModEntry *lobbyMod = nullptr;
  for (const ModEntry &mod : ctx.mods) {
    if ((!st.lobbyModFolder.empty() && mod.folder == st.lobbyModFolder) ||
        (!st.lobbyModId.empty() && !mod.workshopId.empty() &&
         mod.workshopId == st.lobbyModId)) {
      lobbyMod = &mod;
      break;
    }
  }
  std::vector<LobbyMapChoice> mapChoices;
  auto addMapChoice = [&](LobbyMapChoice choice) {
    for (const LobbyMapChoice &existing : mapChoices) {
      if (existing.launch == choice.launch &&
          existing.overrideFile == choice.overrideFile &&
          existing.label == choice.label) {
        return;
      }
    }
    mapChoices.push_back(std::move(choice));
  };
  if (lobbyMod) {
    const std::vector<ModMap> &listed = campaign    ? lobbyMod->campaign
                                        : firefight ? lobbyMod->firefight
                                                    : lobbyMod->multiplayer;
    const std::vector<MapEntry> maps =
        ModMaps(*lobbyMod, listed, baseMaps, baseCount, carrier);
    for (const MapEntry &m : maps) {
      if (m.header || !m.enabled || m.mapOverride.empty())
        continue;
      addMapChoice({m.launchScenario.empty() ? m.scenario : m.launchScenario,
                    m.name, m.mapOverride,
                    m.imageLarge.empty() ? m.imageThumbnail : m.imageLarge,
                    m.imageThumbnail});
    }
  }
  if (!st.lobbyMapOverride.empty()) {
    addMapChoice({st.lobbyMap,
                  st.lobbyMapTitle.empty() ? st.lobbyMap : st.lobbyMapTitle,
                  st.lobbyMapOverride, st.lobbyMapImage, st.lobbyMapThumb});
  }
  if (!st.lobbyMapVariant.empty()) { // the lobby's Forge map stays one of the
                                     // choices after cycling away
    addMapChoice({st.lobbyMap,
                  st.lobbyMapTitle.empty() ? st.lobbyMapVariant.stem().string()
                                           : st.lobbyMapTitle,
                  {},
                  st.lobbyMapImage,
                  st.lobbyMapThumb,
                  st.lobbyMapVariant});
  }
  for (int i = 0; i < baseCount; ++i) {
    addMapChoice({baseMaps[i].scenario, baseMaps[i].name, {}, {}, {}});
  }

  auto cycleOption = [&](int dir) {
    const auto &row = optRows[st.lobbyMenuRow];
    if (!row.editable || row.isStart)
      return;
    if (row.label == "READY") {
      st.lobbyReady = !st.lobbyReady;
      MenuSounds::Change();
    } else if (row.label == "TEAM COUNT") {
      if (!st.isLobbyHost)
        return;
      st.lobbyTeamCount =
          st.lobbyTeamCount >= kTeamCount ? 2 : st.lobbyTeamCount + 1;
      st.lobbyTeamsManual = false; // re-deal across the new number of teams
      NOBLE_INFO("FrontEndMenu: lobby team count set to {}", st.lobbyTeamCount);
      MenuSounds::Change();
    } else if (row.label == "CHANGE TEAM") {
      if (!st.isLobbyHost || st.lobbyRoster.empty())
        return;
      LobbyPlayer &player = st.lobbyRoster[std::clamp(
          st.lobbyRosterSelection, 0,
          static_cast<int>(st.lobbyRoster.size()) - 1)];
      const int teams = std::clamp(st.lobbyTeamCount, 2, kTeamCount);
      const int current = std::clamp(player.team, 0, teams);
      const int next =
          current <= 0 ? 1 : ((current - 1 + dir + teams) % teams) + 1;
      player.team = next;
      st.lobbyTeamsManual =
          true; // stop the automatic deal from undoing this on the next poll
      if (player.isHost)
        MasterServerClient::Get().SetAdvertHostTeam(next);
      if (st.lobbySessionId.empty() ||
          !MasterServerClient::Get().SetTeamsSync(st.lobbySessionId,
                                                  {{player.peerId, next}})) {
        NOBLE_INFO("FrontEndMenu: team for '{}' set to {} locally (no session "
                   "to sync)",
                   player.username, next);
      }
      MenuSounds::Change();
    } else if (row.label == "MISSION" || row.label == "MAP") {
      if (mapChoices.empty())
        return;
      const int count = static_cast<int>(mapChoices.size());
      int cur = 0;
      bool found = false;
      for (int i = 0; i < count; ++i) {
        if (mapChoices[i].launch == st.lobbyMap &&
            mapChoices[i].overrideFile == st.lobbyMapOverride &&
            mapChoices[i].mvar == st.lobbyMapVariant) {
          cur = i;
          found = true;
          break;
        }
      }
      if (!found && !st.lobbyMapTitle.empty()) {
        for (int i = 0; i < count; ++i) {
          if (mapChoices[i].label == st.lobbyMapTitle) {
            cur = i;
            found = true;
            break;
          }
        }
      }
      if (!found) {
        for (int i = 0; i < count; ++i) {
          if (mapChoices[i].launch == st.lobbyMap &&
              mapChoices[i].overrideFile.empty() &&
              mapChoices[i].mvar.empty()) {
            cur = i;
            break;
          }
        }
      }
      for (int tries = 0; tries < count; ++tries) {
        cur = (cur + dir + count) % count;
        const LobbyMapChoice &candidate = mapChoices[cur];
        if (!candidate.overrideFile.empty() ||
            comboAllowed(candidate.launch, st.lobbyVariantPath))
          break;
      }
      const LobbyMapChoice &chosen = mapChoices[cur];
      st.lobbyMap = chosen.launch;
      st.lobbyMapOverride = chosen.overrideFile;
      st.lobbyMapVariant = chosen.mvar;
      st.lobbyMapTitle = chosen.overrideFile.empty() && chosen.mvar.empty()
                             ? std::string()
                             : chosen.label;
      st.lobbyMapImage = chosen.image;
      st.lobbyMapThumb = chosen.thumb;
      if (lobbyMod && !chosen.overrideFile.empty()) {
        st.lobbyModFolder = lobbyMod->folder;
        st.lobbyModId = lobbyMod->workshopId;
        st.lobbyModTitle = lobbyMod->title;
      } else {
        st.lobbyModFolder.clear();
        st.lobbyModId.clear();
        st.lobbyModTitle.clear();
      }
      MasterServerClient::Get().UpdateLobbyAdvert(
          st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty,
          st.lobbySkulls);
      MasterServerClient::Get().UpdateLobbyModAdvert(
          !st.lobbyModId.empty(), st.lobbyModId, st.lobbyModTitle);
      MenuSounds::Change();
    } else if (row.label == "GAME TYPE") {
      const std::vector<MenuVariant> &varList =
          firefight ? ctx.ffVariants : ctx.variants;
      if (!varList.empty()) {
        const int count = static_cast<int>(varList.size());
        int cur = 0;
        for (int i = 0; i < count; ++i) {
          if (varList[i].label == st.lobbyVariantName) {
            cur = i;
            break;
          }
        }
        for (int tries = 0; tries < count; ++tries) {
          cur = (cur + dir + count) % count;
          if (comboAllowed(st.lobbyMap, varList[cur].path.string()))
            break;
        }
        st.lobbyVariantPath = varList[cur].path.string();
        st.lobbyVariantName = varList[cur].label;
        MasterServerClient::Get().UpdateLobbyAdvert(
            st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty,
            st.lobbySkulls);
        MenuSounds::Change();
      }
    } else if (row.label == "DIFFICULTY") {
      st.lobbyDifficulty = (st.lobbyDifficulty + dir + 4) % 4;
      MasterServerClient::Get().UpdateLobbyAdvert(
          st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty,
          st.lobbySkulls);
      MenuSounds::Change();
    }
  };

  if (!st.lobbyPickerOpen && !st.lobbyTeamPickerOpen && !st.lobbyProfileOpen &&
      st.lobbyFocusZone == 0) {
    const auto &curRow = optRows[st.lobbyMenuRow];
    const bool isStepper = curRow.editable && !curRow.isStart;
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true) || UiPad::Left()) {
      if (isStepper)
        cycleOption(-1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true) || UiPad::Right()) {
      if (isStepper)
        cycleOption(1);
      else if (!st.lobbyRoster.empty()) {
        st.lobbyFocusZone = 1;
        MenuSounds::Scroll();
      }
    }
  }

  auto startMatch = [&]() {
    const std::string variantFileName =
        st.lobbyVariantPath.empty()
            ? ""
            : std::filesystem::path(st.lobbyVariantPath).filename().string();
    {
      const std::string curId = MasterServerClient::Get().GetCurrentSessionId();
      const std::string curTok =
          MasterServerClient::Get().GetCurrentSessionToken();
      if (!curId.empty())
        st.lobbySessionId = curId;
      if (!curTok.empty())
        st.lobbySessionToken = curTok;
    }
    // Refresh the roster once at launch so the host's peer barrier counts every
    // guest that is actually in the lobby, not a snapshot that is up to 1.2 s
    // old.
    if (!st.lobbySessionId.empty() && MasterServerClient::Get().Enabled()) {
      const LobbyState current =
          MasterServerClient::Get().FetchLobbyStateSync(st.lobbySessionId);
      if (current.ok && !current.roster.empty())
        st.lobbyRoster = current.roster;
    }
    std::string variantBody;
    bool variantIsBase64 = false;
    std::string variantSha256;
    if (!st.lobbyVariantPath.empty()) {
      std::error_code varEc;
      const auto varSize =
          std::filesystem::file_size(st.lobbyVariantPath, varEc);
      if (!varEc && varSize > 0 && varSize <= 512 * 1024) {
        std::ifstream in(st.lobbyVariantPath, std::ios::binary);
        if (in) {
          std::vector<uint8_t> raw(
              (std::istreambuf_iterator<char>(in)),
              std::istreambuf_iterator<char>());
          variantSha256 =
              NetworkTransport::Sha256Hex(raw.data(), raw.size());
          if (VariantRules::IsPresetFile(st.lobbyVariantPath)) {
            variantBody.assign(raw.begin(), raw.end());
          } else if (NetworkTransport::Base64Encode(raw.data(), raw.size(),
                                                    variantBody)) {
            variantIsBase64 = true;
          }
        }
      }
    }
    std::string mapVariantBody;
    std::string mapVariantSha256;
    if (!st.lobbyMapVariant.empty()) {
      std::error_code mvarEc;
      const auto mvarSize =
          std::filesystem::file_size(st.lobbyMapVariant, mvarEc);
      if (!mvarEc && mvarSize > 0 && mvarSize <= 2 * 1024 * 1024) {
        std::ifstream in(st.lobbyMapVariant, std::ios::binary);
        if (in) {
          std::vector<uint8_t> raw(
              (std::istreambuf_iterator<char>(in)),
              std::istreambuf_iterator<char>());
          if (NetworkTransport::Base64Encode(raw.data(), raw.size(),
                                             mapVariantBody)) {
            mapVariantSha256 =
                NetworkTransport::Sha256Hex(raw.data(), raw.size());
          }
        }
      }
    }
    try {
      MasterServerClient::Get().LaunchLobbySync(
          st.lobbySessionId, st.lobbyMap, st.lobbyMode, variantFileName,
          st.lobbyDifficulty, st.lobbySkulls, variantBody,
          st.lobbyMapOverride.string(), st.lobbyModFolder.string(),
          st.lobbyModId, st.lobbyMapVariant.filename().string(),
          variantIsBase64, variantSha256, mapVariantBody, mapVariantSha256);
    } catch (const std::exception &e) {
      NOBLE_ERROR("FrontEndMenu: LaunchLobbySync exception: {}", e.what());
    } catch (...) {
      NOBLE_ERROR("FrontEndMenu: LaunchLobbySync unknown exception");
    }
    NOBLE_INFO("Lobby start broadcast: session='{}' map='{}' mode='{}' "
               "variant='{}' difficulty={} skulls=0x{:X} roster={}",
               st.lobbySessionId, st.lobbyMap, st.lobbyMode, variantFileName,
               st.lobbyDifficulty, st.lobbySkulls, st.lobbyRoster.size());

    choice.play = true;
    choice.host = true;
    for (size_t p = 0; p < st.lobbyRoster.size(); ++p) {
      const auto &player = st.lobbyRoster[p];
      Career::RegisterPlayerName(player.peerId, player.username,
                                 player.serviceTag);
      Career::RegisterPlayerSlot(static_cast<int>(p), player.username,
                                 player.serviceTag);
    }
    choice.sessionId = st.lobbySessionId;
    choice.sessionToken = st.lobbySessionToken;
    choice.hostOnline = (st.lobbyNetMode == 1);
    choice.hostPublic = (st.lobbyPrivacy == 0);
    choice.hostMaxPlayers = st.lobbyMaxPlayers;
    choice.expectedPeers = 0;
    for (const auto &player : st.lobbyRoster) {
      if (!player.isHost)
        ++choice.expectedPeers;
    }
    // Loading screen roster: host first, then the guests (the order the reference layout shows).
    choice.rosterNames.clear();
    for (int pass = 0; pass < 2; ++pass) {
      for (const auto &player : st.lobbyRoster) {
        if ((pass == 0) != player.isHost) continue;
        choice.rosterNames.push_back(player.username.empty() ? "Host" : player.username);
      }
    }
    choice.map = st.lobbyMap;
    choice.mapOverride = st.lobbyMapOverride;
    choice.mapVariant = st.lobbyMapVariant;
    choice.mapTitle = st.lobbyMapTitle;
    choice.mapImage = st.lobbyMapImage;
    choice.mapThumb = st.lobbyMapThumb;
    choice.mode = st.lobbyMode;
    choice.variantPath = st.lobbyVariantPath;
    choice.difficulty = st.lobbyDifficulty;
    choice.skulls = st.lobbySkulls;
    // The host picked this lobby's map from a mod; the relaunched match must
    // read that mod first.
    choice.modFolder = st.lobbyModFolder;
    choice.modTitle = st.lobbyModTitle;
    RequestMatchLaunch(choice, done, choice);
  };

  // A rotation that starts by itself. B or Esc stops the countdown for this
  // lobby, and a vote holds it.
  if (st.autoStartAt > 0.0 && st.isLobbyHost) {
    if (back) {
      st.autoStartAt = 0.0;
      back = false;
      NOBLE_INFO("FrontEndMenu: next-match countdown cancelled");
      MenuSounds::Back();
    } else if (!st.voteId.empty()) {
      st.autoStartAt += ImGui::GetIO().DeltaTime;
    } else if (ImGui::GetTime() >= st.autoStartAt) {
      st.autoStartAt = 0.0;
      NOBLE_INFO("FrontEndMenu: countdown finished, starting the next match");
      startMatch();
    }
  }
  bool pickerJustOpened =
      false; // the press that opened the list must not also pick from it
  auto triggerAction = [&]() {
    const auto &cur = optRows[st.lobbyMenuRow];
    if (cur.isStart) {
      startMatch();
    } else if (cur.label == "VOTE") {
      StartLobbyVote(ctx);
    } else if ((cur.label == "MAP" || cur.label == "MISSION") && cur.editable) {
      st.lobbyMapPickerOpen = true;
      st.mapSearch[0] = 0;
      st.mapDrawerKey.clear();
      st.mapDrawerFocus = -1;
      st.mapGridScrollY = 0.0f;
      MenuSounds::Click();
    } else if (cur.label == "GAME OPTIONS" && cur.editable) {
      if (forge) {
        MenuVariant forgeMv;
        forgeMv.path = st.forgeCustomVariant.empty() ? ctx.forgeVariant
                                                     : st.forgeCustomVariant;
        forgeMv.label =
            st.forgeCustomVariant.empty() ? "Basic Editing" : "Forge Custom";
        forgeMv.custom = !st.forgeCustomVariant.empty();
        OpenGameOptions(ctx, forgeMv);
      } else {
        const std::vector<MenuVariant> &varList =
            firefight ? ctx.ffVariants : ctx.variants;
        if (!varList.empty()) {
          int pick = firefight ? st.ffVariant : st.variant;
          for (int i = 0; i < static_cast<int>(varList.size()); ++i) {
            if ((!st.lobbyVariantPath.empty() &&
                 varList[static_cast<size_t>(i)].path.string() ==
                     st.lobbyVariantPath) ||
                (!st.lobbyVariantName.empty() &&
                 varList[static_cast<size_t>(i)].label ==
                     st.lobbyVariantName)) {
              pick = i;
              break;
            }
          }
          OpenGameOptions(
              ctx, varList[std::clamp(
                       pick, 0, static_cast<int>(varList.size()) - 1)]);
        }
      }
    } else if (cur.label == "GAME TYPE" && cur.editable) {
      st.lobbyPickerOpen = true;
      pickerJustOpened = true;
      MenuSounds::Click();
    } else if (cur.label == "CHANGE TEAM" && cur.editable) {
      st.lobbyTeamPickerOpen = true;
      st.lobbyTeamPickerOpenedAt = ImGui::GetTime();
      if (!st.lobbyRoster.empty()) {
        const LobbyPlayer &picked = st.lobbyRoster[std::clamp(
            st.lobbyRosterSelection, 0,
            static_cast<int>(st.lobbyRoster.size()) - 1)];
        st.lobbyTeamPickerRow =
            std::clamp(picked.team, 1,
                       std::clamp(st.lobbyTeamCount, 2, kTeamCount)) -
            1;
      }
      MenuSounds::Click();
    } else {
      cycleOption(1);
    }
  };

  if (!st.lobbyPickerOpen && !st.lobbyTeamPickerOpen && !st.lobbyProfileOpen &&
      st.lobbyFocusZone == 0 && AcceptPressed()) {
    UiPad::ConsumeAccept();
    triggerAction();
  }

  // Option rows. The start button keeps one place whichever mode's list is
  // above it.
  const float rowsY = top + 108.0f * ui.s;
  const float rowStep = 36.0f * ui.s;
  // The start button sits under however many option rows this mode has
  // (Campaign, Forge and Custom Game each have a different count, and the team
  // rows added two more) - a fixed 4-row offset drew it over them.
  int optionRowCount = 0;
  for (const auto &r : optRows) {
    if (!r.isStart)
      ++optionRowCount;
  }
  const float startY =
      rowsY + rowStep * static_cast<float>(optionRowCount) + 10.0f * ui.s;
  const bool mouseMoved = ImGui::GetIO().MouseDelta.x != 0.0f ||
                          ImGui::GetIO().MouseDelta.y != 0.0f;
  float curY = rowsY;
  float teamRowY = -1.0f;
  const bool pickerWasOpen = st.lobbyPickerOpen;
  const bool teamPickerWasOpen = st.lobbyTeamPickerOpen;
  const bool voteUp = !st.voteId.empty();
  if (voteUp) {
    // The vote takes the option rows' place for everyone: three choices and
    // "none of these".
    st.lobbyPickerOpen = true;
    st.voteWasUp = true;
    const int choices = static_cast<int>(st.voteOptions.size()) + 1;
    char head[64];
    std::snprintf(head, sizeof(head), "VOTE: WHAT'S NEXT?   %d s",
                  std::max(0, (st.voteMsLeft + 999) / 1000));
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(colX, rowsY - 26.0f * ui.s),
         kAccent, head);
    st.voteRow = std::clamp(st.voteRow, 0, choices - 1);
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down()) {
      st.voteRow = (st.voteRow + 1) % choices;
      MenuSounds::Scroll();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up()) {
      st.voteRow = (st.voteRow + choices - 1) % choices;
      MenuSounds::Scroll();
    }
    int cast = AcceptPressed() ? st.voteRow : -1;
    for (int i = 0; i < choices; ++i) {
      const ImVec2 r0(colX, rowsY + i * 40.0f * ui.s),
          r1(colX + colW, r0.y + 36.0f * ui.s);
      ImGui::SetCursorScreenPos(r0);
      ImGui::PushID(i);
      if (ImGui::InvisibleButton("##vote_option", ImVec2(colW, 36.0f * ui.s)))
        cast = i;
      if (ImGui::IsItemHovered() && mouseMoved)
        st.voteRow = i;
      ImGui::PopID();
      ui.dl->AddRectFilled(r0, r1,
                           st.voteCast == i ? IM_COL32(52, 112, 156, 230)
                                            : IM_COL32(8, 12, 18, 170));
      if (st.voteRow == i)
        ui.dl->AddRect(r0, r1, kAccent, 0.0f, 0, 2.0f * ui.s);
      const std::string label = i < static_cast<int>(st.voteOptions.size())
                                    ? st.voteOptions[i]
                                    : std::string("None of these");
      Text(ui, NobleStyle::TitleFont(), 17.0f,
           ImVec2(r0.x + 10.0f * ui.s, r0.y + 8.0f * ui.s), kWhite,
           Ellipsize(NobleStyle::TitleFont(), 17.0f * ui.s, Upper(label),
                     colW - 60.0f * ui.s)
               .c_str());
      const std::string count = std::to_string(
          i < static_cast<int>(st.voteCounts.size()) ? st.voteCounts[i] : 0);
      Text(ui, NobleStyle::TitleFont(), 17.0f,
           ImVec2(r1.x - 26.0f * ui.s, r0.y + 8.0f * ui.s), kAccent,
           count.c_str());
    }
    if (cast >= 0) {
      st.voteCast = cast;
      uint64_t peer = 1;
      if (AuthClient::Get().IsLoggedIn())
        peer = static_cast<uint64_t>(AuthClient::Get().GetUserId());
      else if (!st.isLobbyHost) {
        peer = MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL;
        if (peer < 2ULL)
          peer = 2ULL;
      }
      if (!MasterServerClient::Get().CastVoteSync(st.lobbySessionId, peer,
                                                  cast))
        NOBLE_WARN("FrontEndMenu: the vote wasn't accepted");
      MenuSounds::Click();
    }
  } else if (st.voteWasUp) { // the vote just closed: the normal rows come back
    st.voteWasUp = false;
    st.lobbyPickerOpen = false;
  }
  if (st.lobbyPickerOpen && !voteUp) {
    // The game types this map can host, from the variants that ship with the
    // game. Nothing here is invented: the settings inside a variant can't be
    // read until the variant viewer exists.
    const std::vector<MenuVariant> &varList =
        firefight ? ctx.ffVariants : ctx.variants;
    const bool anyMap =
        campaign || firefight || forge || !st.lobbyModFolder.empty();
    if (pickerJustOpened) {
      const MenuVariant *current = nullptr;
      for (const MenuVariant &v : varList)
        if (v.label == st.lobbyVariantName)
          current = &v;
      BeginVariantList(st, varList, st.lobbyMap, anyMap, current);
    }
    VariantRow shown;
    const MenuVariant *picked = DrawVariantList(
        ui, st, varList, st.lobbyMap, anyMap, ImVec2(colX, rowsY),
        colW - 10.0f * ui.s, 32.0f, 5, !pickerJustOpened, shown);
    DrawVariantInfo(
        ui,
        ImVec2(colX, rowsY + 40.0f * ui.s + 32.0f * ui.s * 5.0f + 10.0f * ui.s),
        colW, shown, true);
    if (picked) {
      st.lobbyVariantPath = picked->path.string();
      st.lobbyVariantName = picked->label;
      MasterServerClient::Get().UpdateLobbyAdvert(
          st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty,
          st.lobbySkulls);
      st.lobbyPickerOpen = false;
      MenuSounds::Change();
    }
    if (back) {
      st.lobbyPickerOpen = false;
      MenuSounds::Back();
    }
  }
  for (int i = 0; i < totalRows && !pickerWasOpen; ++i) {
    const auto &row = optRows[i];
    const bool focused = st.lobbyFocusZone == 0 && st.lobbyMenuRow == i;
    if (row.isStart) {
      if (BigAction(ui, "lobby_start", ImVec2(colX, startY), colW,
                    row.label.c_str(), row.tooltip.c_str(), true, true,
                    focused)) {
        st.lobbyFocusZone = 0;
        st.lobbyMenuRow = i;
        startMatch();
      }
      // Only a moved mouse takes the focus: a pointer left resting on a row
      // used to pull the controller's focus back to that row every frame.
      if (ImGui::IsItemHovered() && mouseMoved) {
        st.lobbyFocusZone = 0;
        st.lobbyMenuRow = i;
      }
      continue;
    }
    char rowId[32];
    std::snprintf(rowId, sizeof(rowId), "##lobby_row_%d", i);
    const float h = 32.0f * ui.s;
    const ImVec2 r0(colX, curY), r1(colX + colW, curY + h);
    ImGui::SetCursorScreenPos(r0);
    // While the team dropdown is up it owns the mouse: the rows underneath are
    // drawn but inert, so a click on a team never also hits START GAME / the
    // row below it.
    if (!st.lobbyTeamPickerOpen &&
        ImGui::InvisibleButton(rowId, ImVec2(colW, h))) {
      st.lobbyFocusZone = 0;
      st.lobbyMenuRow = i;
      if (row.label == "CHANGE TEAM" || row.label == "GAME OPTIONS" ||
          row.label == "VOTE" || row.label == "GAME TYPE") {
        triggerAction();
      } else {
        cycleOption(ImGui::GetMousePos().x < colX + colW * 0.48f + 28.0f * ui.s
                        ? -1
                        : 1);
      }
    }
    if (!st.lobbyTeamPickerOpen && ImGui::IsItemHovered() && mouseMoved &&
        !focused) {
      st.lobbyFocusZone = 0;
      st.lobbyMenuRow = i;
      MenuSounds::Scroll();
    }
    // The same plate, focus band, accent edge, font and arrows as OptionRow; a
    // row that can't change has no arrows.
    ui.dl->AddRectFilled(r0, r1, IM_COL32(8, 12, 18, 165));
    if (focused) {
      NobleUi::DrawFocusHighlight(ui.dl, r0, r1, ui.s, true);
    } else if (st.lobbyMenuRow == i) {
      ui.dl->AddRectFilled(r0, ImVec2(r0.x + 2.0f * ui.s, r1.y),
                           IM_COL32(143, 216, 255, 110));
    }
    Text(ui, NobleStyle::TitleFont(), 18.0f,
         ImVec2(r0.x + 10.0f * ui.s, r0.y + 6.0f * ui.s),
         focused ? kWhite : kDim, row.label.c_str());
    const float vx = colX + colW * 0.48f, vw = colW * 0.52f, aw = 28.0f * ui.s;
    const float cy = r0.y + h * 0.5f, ah = 6.0f * ui.s;
    const bool teamRow = row.label == "CHANGE TEAM";
    if (teamRow) {
      // A dropdown, not a stepper: one chevron on the right.
      teamRowY = r0.y;
      const float cx = vx + vw - aw * 0.6f;
      ui.dl->AddTriangleFilled(ImVec2(cx - ah, cy - ah * 0.35f),
                               ImVec2(cx + ah, cy - ah * 0.35f),
                               ImVec2(cx, cy + ah), focused ? kWhite : kDim);
    } else if (row.editable) {
      ui.dl->AddTriangleFilled(
          ImVec2(vx + aw * 0.65f, cy - ah), ImVec2(vx + aw * 0.65f, cy + ah),
          ImVec2(vx + aw * 0.3f, cy), focused ? kWhite : kDim);
      const float rx = vx + vw - aw;
      ui.dl->AddTriangleFilled(
          ImVec2(rx + aw * 0.35f, cy - ah), ImVec2(rx + aw * 0.35f, cy + ah),
          ImVec2(rx + aw * 0.7f, cy), focused ? kWhite : kDim);
    }
    const std::string value =
        Upper(Ellipsize(NobleStyle::TitleFont(), 18.0f * ui.s, row.value,
                        vw - aw * 2.0f - 4.0f * ui.s));
    const float tw =
        TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, value.c_str());
    ui.dl->PushClipRect(ImVec2(vx + aw, r0.y), ImVec2(vx + vw - aw, r1.y),
                        true);
    Text(ui, NobleStyle::TitleFont(), 18.0f,
         ImVec2(std::max(vx + aw, vx + (vw - tw) * 0.5f), r0.y + 6.0f * ui.s),
         row.editable ? kWhite : kDim, value.c_str());
    ui.dl->PopClipRect();
    curY += rowStep;
  }
  if (!st.isLobbyHost) {
    BigAction(ui, "lobby_waiting", ImVec2(colX, startY), colW,
              "WAITING FOR HOST", "The match starts when the host starts it.",
              true, false);
  }

  // ---- CHANGE TEAM dropdown: this mode's teams, by name and colour.
  // Enter/click opens it, Up/Down moves, Enter picks, Back/Esc closes. Never a
  // stepper, and the choice survives the lobby's team re-deal.
  if (!st.isLobbyHost)
    st.lobbyTeamPickerOpen = false;
  if (st.lobbyTeamPickerOpen) {
    const int teams = std::clamp(st.lobbyTeamCount, 2, kTeamCount);
    const float pickRowH = 30.0f * ui.s;
    const float panelH2 = teams * pickRowH + 8.0f * ui.s;
    const float anchorY = teamRowY >= 0.0f ? teamRowY + 34.0f * ui.s : rowsY;
    const float dy = std::min(anchorY, top + 540.0f * ui.s - panelH2);
    const ImVec2 p0(colX, dy), p1(colX + colW, dy + panelH2);
    // Blocker in front of everything the panel covers (submitted first, so the
    // team rows below win the pointer).
    ImGui::SetCursorScreenPos(
        ImVec2(colX - 40.0f * ui.s, rowsY - 40.0f * ui.s));
    ImGui::InvisibleButton(
        "##team_pick_blocker",
        ImVec2(colW + 80.0f * ui.s,
               startY + 60.0f * ui.s - (rowsY - 40.0f * ui.s)));
    const bool acceptClicks =
        ImGui::GetTime() - st.lobbyTeamPickerOpenedAt > 0.15;
    ui.dl->AddRectFilled(ImVec2(p0.x + 2.0f * ui.s, p0.y + 3.0f * ui.s),
                         ImVec2(p1.x + 2.0f * ui.s, p1.y + 3.0f * ui.s),
                         IM_COL32(0, 0, 0, 120));
    ui.dl->AddRectFilled(p0, p1, IM_COL32(6, 10, 16, 248), 5.0f * ui.s);
    ui.dl->AddRect(p0, p1, kAccent, 5.0f * ui.s, 0, 2.0f * ui.s);
    st.lobbyTeamPickerRow = std::clamp(st.lobbyTeamPickerRow, 0, teams - 1);
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down()) {
      st.lobbyTeamPickerRow = (st.lobbyTeamPickerRow + 1) % teams;
      MenuSounds::Scroll();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up()) {
      st.lobbyTeamPickerRow = (st.lobbyTeamPickerRow + teams - 1) % teams;
      MenuSounds::Scroll();
    }
    int picked = -1;
    // Only a press made while the dropdown was already up counts: the same
    // Enter that opened it must not also pick the highlighted team (that closed
    // it again on the frame it appeared).
    if (teamPickerWasOpen && acceptClicks && AcceptPressed())
      picked = st.lobbyTeamPickerRow;
    const int currentTeam =
        st.lobbyRoster.empty()
            ? 0
            : st.lobbyRoster[std::clamp(
                                 st.lobbyRosterSelection, 0,
                                 static_cast<int>(st.lobbyRoster.size()) - 1)]
                  .team;
    for (int t = 1; t <= teams; ++t) {
      const int idx = t - 1;
      const ImVec2 r0(p0.x + 5.0f * ui.s, p0.y + 4.0f * ui.s + idx * pickRowH);
      const ImVec2 r1(p1.x - 5.0f * ui.s, r0.y + pickRowH - 2.0f * ui.s);
      ImGui::SetCursorScreenPos(r0);
      ImGui::PushID(2000 + t);
      if (acceptClicks && ImGui::InvisibleButton(
                              "##team_pick", ImVec2(r1.x - r0.x, r1.y - r0.y)))
        picked = idx;
      if (ImGui::IsItemHovered() && mouseMoved)
        st.lobbyTeamPickerRow = idx;
      ImGui::PopID();
      const TeamStyle &style = TeamStyleFor(t);
      ui.dl->AddRectFilled(r0, r1,
                           idx == st.lobbyTeamPickerRow
                               ? IM_COL32(44, 68, 96, 240)
                               : IM_COL32(12, 18, 26, 220));
      ui.dl->AddRectFilled(ImVec2(r0.x, r0.y), ImVec2(r0.x + 6.0f * ui.s, r1.y),
                           style.header);
      Text(ui, NobleStyle::TitleFont(), 17.0f,
           ImVec2(r0.x + 14.0f * ui.s, r0.y + 4.0f * ui.s), kWhite, style.name);
      if (t == currentTeam) {
        const char *equippedTag = "EQUIPPED";
        const float nw =
            TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s, equippedTag);
        Text(ui, NobleStyle::SmallFont(), 13.0f,
             ImVec2(r1.x - nw - 8.0f * ui.s, r0.y + 7.0f * ui.s), kAccent,
             equippedTag);
      }
    }
    if (picked >= 0 && !st.lobbyRoster.empty()) {
      const int team = picked + 1;
      LobbyPlayer &player = st.lobbyRoster[std::clamp(
          st.lobbyRosterSelection, 0,
          static_cast<int>(st.lobbyRoster.size()) - 1)];
      player.team = team;
      st.lobbyTeamsManual =
          true; // the lobby's automatic deal must not undo a hand-picked team
      if (player.isHost)
        MasterServerClient::Get().SetAdvertHostTeam(team);
      if (!st.lobbySessionId.empty()) {
        MasterServerClient::Get().SetTeamsSync(st.lobbySessionId,
                                               {{player.peerId, team}});
      }
      NOBLE_INFO("FrontEndMenu: team for '{}' set to {} ({})", player.username,
                 team, TeamStyleFor(team).shortName);
      st.lobbyTeamPickerOpen = false;
      MenuSounds::Change();
    }
    if (back) {
      st.lobbyTeamPickerOpen = false;
      MenuSounds::Back();
    }
  }
  // What the focused row does; the start button already says it on its own
  // second line.
  float lobbyTipY = startY + 58.0f * ui.s;
  if (!hostModeKnown && !pickerWasOpen) {
    // Greyed until the host can send its mode: a plain row, because nothing can
    // be pressed yet.
    const ImVec2 d0(colX, lobbyTipY), d1(colX + colW, lobbyTipY + 32.0f * ui.s);
    ui.dl->AddRectFilled(d0, d1, IM_COL32(8, 12, 18, 150));
    Text(ui, NobleStyle::TitleFont(), 18.0f,
         ImVec2(d0.x + 10.0f * ui.s, d0.y + 6.0f * ui.s), kDisabled,
         "DOWNLOAD MODE");
    const char *soon = "Coming later";
    Text(ui, nullptr, 15.0f,
         ImVec2(d1.x - TextWidth(nullptr, 15.0f * ui.s, soon) - 10.0f * ui.s,
                d0.y + 7.0f * ui.s),
         kDisabled, soon);
    lobbyTipY += 38.0f * ui.s;
  }
  if (!pickerWasOpen && !st.lobbyTeamPickerOpen &&
      !optRows[st.lobbyMenuRow].isStart) {
    Text(ui, nullptr, 15.0f, ImVec2(colX, lobbyTipY), kDim,
         optRows[st.lobbyMenuRow].tooltip.c_str(), colX + colW);
  }
  if (playlistActive && !pickerWasOpen) {
    const Playlists::Playlist &active = st.playlists[st.playlistChoice];
    const std::string line =
        "PLAYLIST: " + active.name + (active.shuffle ? "  (shuffled)" : "");
    Text(ui, NobleStyle::SmallFont(), 15.0f,
         ImVec2(colX, lobbyTipY + 22.0f * ui.s), kAccent, line.c_str(),
         colX + colW);
  }
  if (!st.voteMessage.empty() && !pickerWasOpen)
    Text(ui, nullptr, 15.0f, ImVec2(colX, lobbyTipY + 42.0f * ui.s),
         IM_COL32(255, 190, 120, 255), st.voteMessage.c_str(), colX + colW);
  if (st.autoStartAt > 0.0 && st.isLobbyHost && !pickerWasOpen) {
    char countdown[64];
    std::snprintf(countdown, sizeof(countdown),
                  "Next match in %d s. B or Esc cancels.",
                  std::max(0, static_cast<int>(st.autoStartAt -
                                               ImGui::GetTime() + 0.999)));
    Text(ui, NobleStyle::TitleFont(), 17.0f,
         ImVec2(colX, lobbyTipY + 62.0f * ui.s), kAccent, countdown);
  }

  // Friends and the invite strip sit at the foot of the left column, above the
  // prompts.
  const float netH = 44.0f * ui.s;
  const float netY =
      ui.origin.y + ui.size.y - 58.0f * ui.s - 8.0f * ui.s - netH;
  const float friendRowH = 25.0f * ui.s;
  const float friendsH = 26.0f * ui.s + friendRowH * 4.0f;
  const float friendsY = netY - 8.0f * ui.s - friendsH;

  const ImVec2 fr0(colX, friendsY), fr1(colX + colW, friendsY + friendsH);
  ui.dl->AddRectFilled(fr0, fr1, panelFill, 5.0f * ui.s);
  ui.dl->AddRect(fr0, fr1, panelEdge, 5.0f * ui.s);
  Text(ui, NobleStyle::TitleFont(), 16.0f,
       ImVec2(fr0.x + 10.0f * ui.s, fr0.y + 4.0f * ui.s), kWhite, "FRIENDS");
  ui.dl->AddLine(ImVec2(fr0.x, fr0.y + 26.0f * ui.s),
                 ImVec2(fr1.x, fr0.y + 26.0f * ui.s), kRuleLine, 1.0f);
  const bool lobbyTyping =
      ImGui::GetIO().WantTextInput; // these keys are letters in a search box
  const bool friendsPressed =
      (!lobbyTyping && ImGui::IsKeyPressed(ImGuiKey_F, false)) ||
      UiPad::ActionY() ||
      (ImGui::IsMouseClicked(0) &&
       ImGui::IsMouseHoveringRect(fr0, ImVec2(fr1.x, fr0.y + 26.0f * ui.s),
                                  false));
  if (friendsPressed) {
    st.socialPreviousScreen = Screen::PreGameLobby;
    st.screen = Screen::Social;
    MenuSounds::Click();
  }
  float friendY = friendsY + 26.0f * ui.s;
  const auto realFriends = SocialClient::Get().GetFriends();
  if (realFriends.empty()) {
    Text(ui, NobleStyle::SmallFont(), 15.0f,
         ImVec2(fr0.x + 10.0f * ui.s, friendY + 5.0f * ui.s), kDim,
         "No friends online.");
  }
  int friendIdx = 0;
  for (const auto &f : realFriends) {
    if (friendIdx >= 4)
      break;
    Text(ui, NobleStyle::SmallFont(), 15.0f,
         ImVec2(fr0.x + 10.0f * ui.s, friendY + 4.0f * ui.s), kWhite,
         f.username.c_str());
    if (st.isLobbyHost) {
      const float invW = 64.0f * ui.s;
      const float invH = 20.0f * ui.s;
      ImGui::SetCursorScreenPos(
          ImVec2(fr1.x - invW - 6.0f * ui.s, friendY + 2.5f * ui.s));
      ImGui::PushID(1500 + friendIdx);
      if (ImGui::Button("INVITE", ImVec2(invW, invH))) {
        std::string myIp = UPnP::Get().GetExternalIP();
        if (myIp.empty() || myIp == "Unknown")
          myIp = "127.0.0.1";
        std::string invMode = "Custom Game";
        if (campaign)
          invMode = "Campaign";
        else if (firefight)
          invMode = "Firefight";
        else if (forge)
          invMode = "Forge";
        SocialClient::Get().SendInviteAsync(
            f.username, myIp, 11774,
            [](bool ok, const std::string &msg) {
              (void)msg;
              if (ok)
                MenuSounds::Click();
            },
            invMode, st.lobbyMap, st.lobbySessionId);
      }
      ImGui::PopID();
    }
    friendY += friendRowH;
    ++friendIdx;
  }

  // The invite strip never prints the address: it copies it. A public IP on
  // screen ends up in screenshots and streams.
  const std::string extIp = UPnP::Get().GetExternalIP();
  const bool ipKnown = !extIp.empty() && extIp != "Unknown";
  const std::string hostEp = st.isLobbyHost
                                 ? (ipKnown ? extIp + ":11774" : std::string())
                                 : st.lobbyHostEndpoint;
  const ImVec2 net0(colX, netY), net1(colX + colW, netY + netH);
  ui.dl->AddRectFilled(net0, net1, panelFill, 5.0f * ui.s);
  ui.dl->AddRect(net0, net1, panelEdge, 5.0f * ui.s);
  const char *netTitle = st.isLobbyHost   ? "HOSTING"
                         : hostEp.empty() ? "CONNECTING"
                                          : "CONNECTED";
  // A shareable join code works for public AND private (unlisted) lobbies. It is just the session id in a short,
  // typo-tolerant format - the joiner resolves the host through the master server, so no IP is exposed in the code.
  std::string joinCode;
  if (st.isLobbyHost && st.lobbySessionId.size() == 16) {
    InviteCodeInfo codeInfo;
    codeInfo.sessionId = st.lobbySessionId;
    codeInfo.isPrivate = st.lobbyPrivacy != 0;
    joinCode = EncodeInviteCode(codeInfo);
  }
  const char *netDesc =
      st.isLobbyHost
          ? ((st.lobbyNetMode == 1) ? "Online peer-to-peer" : "Local network")
          : "Peer-to-peer";
  if (!joinCode.empty())
    netDesc = "Share this code - anyone who has it can join.";
  Text(ui, NobleStyle::TitleFont(), 16.0f,
       ImVec2(net0.x + 10.0f * ui.s, net0.y + 3.0f * ui.s), kWhite, netTitle);
  static double s_inviteCopiedAt = -10.0;
  const bool justCopied = now - s_inviteCopiedAt < 2.0;
  Text(ui, NobleStyle::SmallFont(), 14.0f,
       ImVec2(net0.x + 10.0f * ui.s, net0.y + 23.0f * ui.s),
       justCopied ? kAccent : kDim,
       justCopied ? (joinCode.empty() ? "Invite copied to the clipboard." : "Join code copied to the clipboard.")
                  : netDesc);
  if (!joinCode.empty())
    Text(ui, NobleStyle::TitleFont(), 15.0f,
         ImVec2(net0.x + 10.0f * ui.s, net0.y + 39.0f * ui.s), NobleUi::kAmber, joinCode.c_str());
  bool copyPressed = false;
  if (!hostEp.empty() || !joinCode.empty()) {
    const float cpW = 112.0f * ui.s, cpH = 28.0f * ui.s;
    ImGui::SetCursorScreenPos(
        ImVec2(net1.x - cpW - 8.0f * ui.s, net0.y + 8.0f * ui.s));
    ImGui::PushID("btn_copy_inv");
    copyPressed = ImGui::Button(joinCode.empty() ? "COPY INVITE" : "COPY CODE", ImVec2(cpW, cpH));
    ImGui::PopID();
    copyPressed = copyPressed ||
                  (!lobbyTyping && ImGui::IsKeyPressed(ImGuiKey_X, false)) ||
                  UiPad::ActionX();
  }
  if (copyPressed) {
    ImGui::SetClipboardText(joinCode.empty() ? ("/connect " + hostEp).c_str() : joinCode.c_str());
    s_inviteCopiedAt = now;
    MenuSounds::Click();
  }

  // Right column: the map banner, then the session's roster in a panel of fixed
  // size, so a filling lobby never moves anything.
  const float bannerY = top + 96.0f * ui.s;
  const float bannerH = 92.0f * ui.s;
  std::string kicker =
      !st.lobbyModTitle.empty() ? "MOD: " + Upper(st.lobbyModTitle)
      : campaign                ? std::string("HALO: REACH \xC2\xB7 CAMPAIGN")
      : forge ? std::string("HALO: REACH \xC2\xB7 FORGE")
              : "HALO: REACH \xC2\xB7 " + Upper(st.lobbyVariantName.empty()
                                                    ? "MULTIPLAYER"
                                                    : st.lobbyVariantName);
  const std::filesystem::path bannerImage =
      st.lobbyMapImage.empty() ? st.lobbyMapThumb : st.lobbyMapImage;
  HeroBanner(ui, ImVec2(px, bannerY), pw, bannerH, st.lobbyMap, kicker,
             mapDisplay.c_str(), "", bannerImage);

  const float headY = bannerY + bannerH + 8.0f * ui.s;
  const float headH = 28.0f * ui.s;
  const float slotH = 40.0f * ui.s;
  const float slotGap = 2.0f * ui.s;
  const float slotPad = 6.0f * ui.s;
  const float teamHeadH = 20.0f * ui.s;
  const float panelH =
      headH + teamHeadH + slotPad + (slotH + slotGap) * 8.0f - slotGap;
  ui.dl->AddRectFilled(ImVec2(px, headY), ImVec2(px + pw, headY + panelH),
                       panelFill);
  ui.dl->AddRect(ImVec2(px, headY), ImVec2(px + pw, headY + panelH),
                 st.lobbyFocusZone == 1 ? IM_COL32(143, 216, 255, 220)
                                        : panelEdge,
                 0.0f, 0, st.lobbyFocusZone == 1 ? 1.8f * ui.s : 1.0f);
  Text(ui, NobleStyle::TitleFont(), 18.0f,
       ImVec2(px + 10.0f * ui.s, headY + 4.0f * ui.s), kWhite,
       "MY GAME SESSION");
  // Count over the seats the engine can really declare, never a stale/demo number above that. A
  // test/demo roster that already exceeds the cap still needs its rows to fit, so the denominator is the
  // larger of the effective cap and the roster actually in.
  const int headerCap =
      (std::min)(st.lobbyMaxPlayers, EffectiveMultiplayerCap());
  const int rosterCount = static_cast<int>(st.lobbyRoster.size());
  const int slotCount =
      std::clamp(std::max(headerCap, rosterCount), 1, 128);
  char countBuf[32];
  std::snprintf(countBuf, sizeof(countBuf), "%zu/%d", st.lobbyRoster.size(),
                slotCount);
  const float countW =
      TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, countBuf);
  Text(ui, NobleStyle::TitleFont(), 18.0f,
       ImVec2(px + pw - countW - 10.0f * ui.s, headY + 4.0f * ui.s), kWhite,
       countBuf);
  ui.dl->AddLine(ImVec2(px, headY + headH), ImVec2(px + pw, headY + headH),
                 kRuleLine, 1.0f);
  // A local LAN lobby has no master-server roster, so this panel cannot see players before the match: say
  // that instead of letting "1 player" read as the whole session (players join in game).
  if (st.isLobbyHost && st.lobbyNetMode == 0) {
    Text(ui, NobleStyle::SmallFont(), 14.0f,
         ImVec2(px + 10.0f * ui.s, headY + panelH + 6.0f * ui.s), kDim,
         "LAN SESSION - PLAYERS JOIN IN GAME; THE FULL ROSTER IS IN THE MATCH",
         pw - 20.0f * ui.s);
  }

  const int openSlots = std::max(0, slotCount - rosterCount);
  const auto &ownProf = UIManager::GetArmorCustomizer().GetProfile();
  const std::string localName = AuthClient::Get().IsLoggedIn()
                                    ? AuthClient::Get().GetUsername()
                                    : Settings::Get().Snapshot().playerName;
  const std::string localDevId = DeviceIdentityStore::Get().deviceId;
  const std::string localLobbyMachineId =
      std::to_string(MasterServerClient::GetLobbyMachineId());
  const uint64_t localPeerId =
      AuthClient::Get().IsLoggedIn()
          ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
          : (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);

  auto isLocal = [&](const LobbyPlayer &p) {
    if (st.isLobbyHost)
      return p.isHost;
    if (p.isHost)
      return false;
    if (!p.machineId.empty() &&
        (p.machineId == localDevId || p.machineId == localLobbyMachineId))
      return true;
    if (p.peerId != 0 && p.peerId == localPeerId)
      return true;
    return false;
  };

  // One list, in Reach's lobby style: when the game type assigns teams each
  // team gets its own divider bar and its players underneath, and every team
  // the lobby deals is listed even while it is empty - so a host alone in the
  // lobby still sees RED / BLUE / ... instead of one undivided row. Without
  // teams it is a plain list. Only the entries in view are drawn, so a
  // 128-player roster costs the same as a full 16.
  constexpr int kVisibleRows = 8;
  struct RosterEntry {
    enum class Kind { Header, Player, Open } kind;
    int team = 0;
    int index = -1;
  };
  const int lobbyTeams =
      st.lobbyTeamsOn ? std::clamp(st.lobbyTeamCount, 2, kTeamCount) : 1;
  std::vector<RosterEntry> entries;
  if (st.lobbyTeamsOn) {
    for (int t = 1; t <= lobbyTeams; ++t) {
      entries.push_back({RosterEntry::Kind::Header, t, -1});
      int members = 0;
      for (int i = 0; i < rosterCount; ++i) {
        const int team = st.lobbyRoster[i].team > 0
                             ? std::clamp(st.lobbyRoster[i].team, 1, lobbyTeams)
                             : 1;
        if (team != t)
          continue;
        entries.push_back({RosterEntry::Kind::Player, t, i});
        ++members;
      }
      if (members == 0)
        entries.push_back({RosterEntry::Kind::Open, t, -1});
    }
  } else {
    for (int i = 0; i < rosterCount; ++i)
      entries.push_back({RosterEntry::Kind::Player, 0, i});
  }
  // Spare seats at the end, capped so a 128-player lobby does not become a
  // hundred OPEN rows.
  for (int i = 0, shown = 0; i < openSlots && shown < kVisibleRows;
       ++i, ++shown) {
    entries.push_back({RosterEntry::Kind::Open, 0, -1});
  }
  auto entryOfPlayer = [&](int rosterIndex) {
    for (size_t e = 0; e < entries.size(); ++e) {
      if (entries[e].kind == RosterEntry::Kind::Player &&
          entries[e].index == rosterIndex)
        return static_cast<int>(e);
    }
    return -1;
  };
  auto keepInView = [&](int rosterIndex) {
    const int entry = entryOfPlayer(rosterIndex);
    if (entry < 0)
      return;
    int &scroll = st.lobbyRosterScroll[0];
    if (entry < scroll)
      scroll = entry;
    if (entry >= scroll + kVisibleRows)
      scroll = entry - kVisibleRows + 1;
  };
  if (rosterCount > 0) {
    st.lobbyRosterSelection =
        std::clamp(st.lobbyRosterSelection, 0, rosterCount - 1);
    const bool prev = !lobbyTyping && UiPad::TabPrev();
    const bool next = !lobbyTyping && UiPad::TabNext();
    if (prev != next) {
      st.lobbyRosterSelection =
          (st.lobbyRosterSelection + (next ? 1 : -1) + rosterCount) %
          rosterCount;
      keepInView(st.lobbyRosterSelection);
      MenuSounds::Scroll();
    }
    if (!st.lobbyPickerOpen && !st.lobbyTeamPickerOpen &&
        !st.lobbyProfileOpen && st.lobbyFocusZone == 1) {
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || UiPad::Down()) {
        st.lobbyRosterSelection = (st.lobbyRosterSelection + 1) % rosterCount;
        keepInView(st.lobbyRosterSelection);
        MenuSounds::Scroll();
      }
      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || UiPad::Up()) {
        st.lobbyRosterSelection =
            (st.lobbyRosterSelection + rosterCount - 1) % rosterCount;
        keepInView(st.lobbyRosterSelection);
        MenuSounds::Scroll();
      }
      if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false) || UiPad::Left()) {
        st.lobbyFocusZone = 0;
        MenuSounds::Scroll();
      }
      if (AcceptPressed()) {
        UiPad::ConsumeAccept();
        st.lobbyProfileOpen = true;
        MenuSounds::Click();
      }
    }
    const bool pageUp =
        ImGui::IsKeyPressed(ImGuiKey_PageUp, false) || UiPad::PageUp();
    const bool pageDown =
        ImGui::IsKeyPressed(ImGuiKey_PageDown, false) || UiPad::PageDown();
    if (pageUp != pageDown) {
      // Page by players, not by rows: a page may cross a team divider.
      const int step = pageDown ? kVisibleRows : -kVisibleRows;
      st.lobbyRosterSelection =
          std::clamp(st.lobbyRosterSelection + step, 0, rosterCount - 1);
      keepInView(st.lobbyRosterSelection);
      MenuSounds::Scroll();
    }
  }

  // The selected player's card: Start or Tab, Back closes it. Only the local
  // player's own numbers are known here.
  const bool profileWasOpen = st.lobbyProfileOpen;
  if (rosterCount > 0 && !pickerWasOpen &&
      (ImGui::IsKeyPressed(ImGuiKey_Tab, false) ||
       ImGui::IsKeyPressed(ImGuiKey_GamepadStart, false))) {
    st.lobbyProfileOpen = !st.lobbyProfileOpen;
    MenuSounds::Click();
  }
  if (rosterCount == 0)
    st.lobbyProfileOpen = false;
  bool rosterOverflows = false;
  const float teamY = headY + headH;
  const float listY = teamY + teamHeadH;
  const float listH = (slotH + slotGap) * kVisibleRows - slotGap;
  if (st.lobbyProfileOpen) {
    const LobbyPlayer &player = st.lobbyRoster[st.lobbyRosterSelection];
    const bool local = isLocal(player);
    const float cardX = px + slotPad, cardW = pw - slotPad * 2.0f;
    float y = headY + headH + slotPad;
    const int rank =
        player.rankIndex >= 0
            ? player.rankIndex
            : (local ? Career::RankIndex(Career::Get().credits) : -1);
    PlayerRow(ui, ImVec2(cardX, y), cardW, rank,
              player.emblemId > 0 ? player.emblemId
                                  : (local ? ownProf.emblemIndex : 0),
              player.nameplateId > 0 ? player.nameplateId
                                     : (local ? ownProf.nameplateIndex : 0),
              player.team, player.serviceTag, player.username, player.isDev,
              false,
              player.emblemBackgroundId > 0
                  ? player.emblemBackgroundId
                  : (local ? ownProf.emblemBackgroundIndex : 0),
              local ? UIManager::GetArmorCustomizer().EmblemColorsNow()
                    : UiArt::EmblemColors{},
              player.isTester);
    y += slotH + 14.0f * ui.s;
    auto statLine = [&](const char *label, const std::string &value) {
      Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(cardX + 4.0f * ui.s, y),
           kDim, label);
      const float w =
          TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, value.c_str());
      Text(ui, NobleStyle::TitleFont(), 18.0f,
           ImVec2(cardX + cardW - w - 4.0f * ui.s, y - 2.0f * ui.s), kWhite,
           value.c_str());
      y += 28.0f * ui.s;
    };
    const auto &ranks = Career::Ranks();
    statLine("RANK", rank >= 0 && rank < static_cast<int>(ranks.size())
                         ? std::string(ranks[rank].name)
                         : std::string("Not shared yet"));
    if (local) {
      const Career::Record rec = Career::GetRecord();
      int challengesDone = 0, challengeCount = 0, tiers = 0, tierMax = 0;
      for (const auto &c : Career::Challenges()) {
        ++challengeCount;
        if (c.done)
          ++challengesDone;
      }
      for (const auto &c : Career::Commendations()) {
        if (c.tracked) {
          tiers += c.tier;
          tierMax += 6;
        }
      }
      statLine("GAMES PLAYED", std::to_string(rec.totals.gamesPlayed));
      statLine("KILLS / DEATHS", std::to_string(rec.totals.kills) + " / " +
                                     std::to_string(rec.totals.deaths));
      const int totalChallenges = challengeCount > 0 ? challengeCount : 4;
      statLine("CHALLENGES DONE NOW", std::to_string(challengesDone) + " / " +
                                          std::to_string(totalChallenges));
      statLine("COMMENDATIONS",
               tierMax > 0 ? std::to_string(tiers * 100 / tierMax) + "%"
                           : std::string("0%"));
      size_t boughtCount = rec.totals.armoryOwned.size();
      if (rec.totals.armoryUnlockAll && boughtCount == 0) {
        boughtCount = 179; // Total catalog count when unlock-all is active
      }
      statLine("ARMORY ITEMS BOUGHT", std::to_string(boughtCount));
    } else {
      int games = player.gamesPlayed;
      int kills = player.kills;
      int deaths = player.deaths;
      int challenges = player.challengesDone;
      int commendations = player.commendations;
      int armoryItems = player.armoryOwned;

      Career::PeerStats cached;
      if (Career::LoadPeerStats(player.machineId, cached) ||
          Career::LoadPeerStats(player.username, cached)) {
        if (games <= 0)
          games = static_cast<int>(cached.gamesPlayed);
        if (kills <= 0)
          kills = static_cast<int>(cached.kills);
        if (deaths <= 0)
          deaths = static_cast<int>(cached.deaths);
        if (challenges <= 0)
          challenges = cached.challengesDone;
        if (commendations <= 0)
          commendations = cached.commendations;
        if (armoryItems <= 0)
          armoryItems = cached.armoryOwned;
      }

      statLine("GAMES PLAYED", std::to_string(games));
      statLine("KILLS / DEATHS",
               std::to_string(kills) + " / " + std::to_string(deaths));
      statLine("CHALLENGES DONE NOW", std::to_string(challenges) + " / 4");
      statLine("COMMENDATIONS", std::to_string(commendations) + "%");
      statLine("ARMORY ITEMS BOUGHT", std::to_string(armoryItems));
    }

    y += 10.0f * ui.s;
    if ((local || st.isLobbyHost) && st.lobbyTeamsOn) {
      // The same dropdown as the lobby's CHANGE TEAM row: pick the team by name
      // and colour.
      if (ActionButton(ui, "lobby_change_team", ImVec2(cardX, y), cardW,
                       "CHANGE TEAM", false)) {
        st.lobbyTeamPickerOpen = true;
        st.lobbyTeamPickerRow =
            std::clamp(player.team, 1,
                       std::clamp(st.lobbyTeamCount, 2, kTeamCount)) -
            1;
        MenuSounds::Click();
      }
      y += 36.0f * ui.s;
    }
  }
  if (!st.lobbyProfileOpen) {
    const float listX = px + slotPad;
    const float listW = pw - slotPad * 2.0f;
    const float barW = 6.0f * ui.s;
    const float rowW = listW - barW;
    const float headHdr = 24.0f * ui.s; // a team divider bar

    // Wheel, Right-Stick, and interactive scrollbar scroll the roster without
    // snapping back to the selected player until selection changes.
    int &scroll = st.lobbyRosterScroll[0];
    const int total = static_cast<int>(entries.size());
    const int selectedEntry = entryOfPlayer(st.lobbyRosterSelection);
    static int s_lastRosterSelEntry = -1;
    static float s_rosterScrollEntries = 0.0f;
    if (selectedEntry != s_lastRosterSelEntry) {
      s_lastRosterSelEntry = selectedEntry;
      if (selectedEntry >= 0) {
        if (selectedEntry < scroll)
          scroll = selectedEntry;
        if (selectedEntry >= scroll + kVisibleRows)
          scroll = selectedEntry - kVisibleRows + 1;
      }
      s_rosterScrollEntries = static_cast<float>(std::max(0, scroll));
    }
    if (ImGui::IsMouseHoveringRect(ImVec2(listX, listY),
                                   ImVec2(listX + listW, listY + listH),
                                   false)) {
      const float wheel = ImGui::GetIO().MouseWheel;
      if (wheel != 0.0f)
        s_rosterScrollEntries -= wheel;
    }
    const float rStickRoster = UiPad::AnalogScrollY(false);
    if (rStickRoster != 0.0f) {
      s_rosterScrollEntries += rStickRoster / std::max(1.0f, slotH);
    }
    auto entryHeight = [&](const RosterEntry &e) {
      return e.kind == RosterEntry::Kind::Header ? headHdr : slotH;
    };

    s_rosterScrollEntries =
        std::clamp(s_rosterScrollEntries, 0.0f,
                   static_cast<float>(std::max(0, total - 1)));
    scroll = std::clamp(static_cast<int>(s_rosterScrollEntries + 0.5f), 0,
                        std::max(0, total - 1));
    float used = 0.0f;
    int firstHidden = total;
    for (int e = scroll; e < total; ++e) {
      const float h = entryHeight(entries[e]);
      if (used + h > listH + 0.5f) {
        firstHidden = e;
        break;
      }
      used += h + slotGap;
    }
    if (firstHidden < total)
      scroll = std::min(scroll, std::max(0, firstHidden - 1));
    const bool overflow = scroll > 0 || firstHidden < total;
    rosterOverflows = overflow;

    float y = listY;
    for (int e = scroll; e < total; ++e) {
      const RosterEntry &entry = entries[e];
      const float h = entryHeight(entry);
      if (y + h > listY + listH + 0.5f)
        break;
      const ImVec2 s0(listX, y);
      const ImVec2 s1(listX + rowW, y + h);
      if (entry.kind == RosterEntry::Kind::Header) {
        // Team divider: a colour bar with the team's name and how many players
        // it holds.
        const TeamStyle &style = TeamStyleFor(entry.team);
        ui.dl->AddRectFilledMultiColor(s0, s1, style.header,
                                       IM_COL32(10, 14, 20, 220),
                                       IM_COL32(10, 14, 20, 220), style.rowAlt);
        ui.dl->AddRectFilled(ImVec2(s0.x, s0.y),
                             ImVec2(s0.x + 4.0f * ui.s, s1.y), style.header);
        int members = 0;
        for (const LobbyPlayer &p : st.lobbyRoster) {
          const int team =
              p.team > 0 ? std::clamp(p.team, 1, std::max(2, lobbyTeams)) : 1;
          if (team == entry.team)
            ++members;
        }
        Text(ui, NobleStyle::SmallFont(), 14.0f,
             ImVec2(s0.x + 12.0f * ui.s, s0.y + 4.0f * ui.s), kWhite,
             style.name);
        const std::string teamCount = std::to_string(members);
        const float teamCountW =
            TextWidth(NobleStyle::SmallFont(), 14.0f * ui.s, teamCount.c_str());
        Text(ui, NobleStyle::SmallFont(), 14.0f,
             ImVec2(s1.x - teamCountW - 10.0f * ui.s, s0.y + 4.0f * ui.s),
             IM_COL32(220, 232, 245, 235), teamCount.c_str());
      } else if (entry.kind == RosterEntry::Kind::Open) {
        ui.dl->AddRectFilled(s0, s1, IM_COL32(255, 255, 255, 8), 4.0f * ui.s);
        ui.dl->AddRect(s0, s1, IM_COL32(255, 255, 255, 24), 4.0f * ui.s);
        Text(ui, NobleStyle::SmallFont(), 14.0f,
             ImVec2(s0.x + 12.0f * ui.s, s0.y + 12.0f * ui.s), kDisabled,
             "OPEN");
      } else {
        const int idx = entry.index;
        const LobbyPlayer &player = st.lobbyRoster[idx];
        const bool local = isLocal(player);
        const bool selected = st.lobbyRosterSelection == idx;
        const int teamForRow =
            st.lobbyTeamsOn ? (player.team > 0 ? player.team : entry.team) : 0;
        // The team's row tint behind the row, so the section reads as one block
        // even when a row is selected.
        ui.dl->AddRectFilled(s0, s1,
                             selected ? IM_COL32(36, 78, 112, 240)
                                      : TeamStyleFor(teamForRow).rowBase);
        PlayerRow(ui, ImVec2(s0.x + 4.0f * ui.s, s0.y), rowW - 4.0f * ui.s,
                  player.rankIndex >= 0
                      ? player.rankIndex
                      : (local ? Career::RankIndex(Career::Get().credits) : -1),
                  player.emblemId > 0 ? player.emblemId
                                      : (local ? ownProf.emblemIndex : 0),
                  player.nameplateId > 0 ? player.nameplateId
                                         : (local ? ownProf.nameplateIndex : 0),
                  teamForRow, player.serviceTag,
                  (local
                       ? (Settings::Get().Snapshot().playerName.empty()
                              ? std::string("Noble Six")
                              : Settings::Get().Snapshot().playerName)
                       : (player.username.empty()
                              ? std::string(player.isHost ? "Host" : "Spartan")
                              : player.username)),
                  player.isDev, selected,
                  player.emblemBackgroundId > 0
                      ? player.emblemBackgroundId
                      : (local ? ownProf.emblemBackgroundIndex : 0),
                  local ? UIManager::GetArmorCustomizer().EmblemColorsNow()
                        : UiArt::EmblemColors{},
                  player.isTester);
        if (selected) {
          if (st.lobbyFocusZone == 1)
            NobleUi::DrawFocusHighlight(ui.dl, s0, s1, ui.s, true);
          else
            ui.dl->AddRect(s0, s1, kAccent, 4.0f * ui.s, 0, 1.5f * ui.s);
        }
        {
          uint64_t machine =
              1000ULL +
              static_cast<uint64_t>(
                  idx); // demo rosters have no ids; only real ids ever talk
          try {
            if (!player.machineId.empty())
              machine = std::stoull(player.machineId, nullptr, 0);
          } catch (...) {
          }
          SocialOverlay::DrawPlayerVoiceIcon(
              ui.dl, ImVec2(s1.x - 20.0f * ui.s, (s0.y + s1.y) * 0.5f), ui.s,
              machine, local);
        }
        char slotId[32];
        std::snprintf(slotId, sizeof(slotId), "##lobby_slot_%d", idx);
        ImGui::SetCursorScreenPos(s0);
        if (ImGui::InvisibleButton(slotId, ImVec2(rowW, h))) {
          if (st.lobbyRosterSelection == idx) {
            st.lobbyProfileOpen = !st.lobbyProfileOpen;
          } else {
            st.lobbyRosterSelection = idx;
            st.lobbyProfileOpen = true;
          }
          MenuSounds::Scroll();
        }
      }
      y += h + slotGap;
    }
    if (overflow) {
      const float trackX = listX + listW - 5.0f * ui.s;
      const int shown = std::max(1, std::min(total - scroll, kVisibleRows));
      const int maxEntries = std::max(1, total - shown);
      float scrollPx = s_rosterScrollEntries * slotH;
      const float maxScrollPx = static_cast<float>(maxEntries) * slotH;
      NobleUi::DrawInteractiveScrollbar(
          ui.dl, "##lobby_roster_sb",
          ImVec2(trackX, listY),
          ImVec2(trackX + 5.0f * ui.s, listY + listH),
          scrollPx, maxScrollPx,
          static_cast<float>(shown) * slotH,
          static_cast<float>(total) * slotH, ui.s);
      s_rosterScrollEntries = std::clamp(scrollPx / std::max(1.0f, slotH),
                                         0.0f, static_cast<float>(maxEntries));
      scroll = std::clamp(static_cast<int>(s_rosterScrollEntries + 0.5f), 0,
                          maxEntries);
    }
  }

  // Back / Cancel Lobby Handling: the host returns to the setup screen of the
  // mode it came from.
  if (back && teamPickerWasOpen) {
    UiPad::ConsumeBack();
    // The dropdown already consumed Back this frame.
  } else if (back && st.lobbyProfileOpen) {
    UiPad::ConsumeBack();
    st.lobbyProfileOpen = false;
    MenuSounds::Back();
  } else if (back && !pickerWasOpen && !profileWasOpen) {
    UiPad::ConsumeBack();
    st.lobbyLaunchPending = false;
    ClearPendingLobbyEvents();
    if (st.isLobbyHost) {
      MasterServerClient::Get().StopAdvertisingSync();
      SessionManager::Get().StopHosting();
      NetworkTransport::Get().StopRelay();
      st.lobbySessionId.clear();
      st.lobbySessionToken.clear();
      st.lobbyRoster.clear();
      st.screen = campaign    ? Screen::Campaign
                  : firefight ? Screen::Firefight
                  : forge     ? Screen::Forge
                              : Screen::CustomGame;
    } else {
      uint64_t fallbackId =
          (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
      if (fallbackId < 2ULL)
        fallbackId = 2ULL;
      uint64_t pId = AuthClient::Get().IsLoggedIn()
                         ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                         : fallbackId;
      if (!st.lobbySessionId.empty()) {
        MasterServerClient::Get().LeaveLobbySync(st.lobbySessionId, pId);
      }
      NetworkTransport::Get().StopRelay();
      st.lobbySessionId.clear();
      st.lobbySessionToken.clear();
      st.lobbyHostEndpoint.clear();
      st.lobbyRoster.clear();
      st.screen = Screen::Servers;
    }
    st.focusRow = 0;
    MenuSounds::Back();
  }

  std::vector<HintExtra> extras = {{InputPrompts::Button::Y, "F", "Friends"}};
  if (!hostEp.empty() || !joinCode.empty())
    extras.push_back({InputPrompts::Button::X, "X", joinCode.empty() ? "Copy invite" : "Copy code"});
  if (rosterOverflows)
    extras.push_back({InputPrompts::Button::RT, "PGDN", "Page"});
  if (rosterCount > 0 && !pickerWasOpen)
    extras.push_back({InputPrompts::Button::Start, "TAB",
                      st.lobbyProfileOpen ? "Close card" : "Player card"});
  if (pickerWasOpen)
    DrawHints(ui, {{Prompt::Accept, "Select"}, {Prompt::Back, "Back"}});
  else
    DrawHints(ui,
              {{Prompt::Accept, "Select"},
               {Prompt::Back, "Back"},
               {Prompt::Tabs, "Players"}},
              extras);
}


} // namespace noble::fe
