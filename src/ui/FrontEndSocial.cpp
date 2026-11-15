#include "FrontEndInternal.hpp"

namespace noble::fe {

void DrawSocialScreen(OverlayContext &ctx, const Ui &ui, float left,
                             float top, FrontEndChoice &choice, bool &done,
                             bool back) {
  (void)left;
  (void)top;
  (void)choice;
  (void)done;
  MenuState &st = ctx.st;

  DrawTopTelemetryBar(ui, "SOCIAL");

  const float px = ui.origin.x + 24.0f * ui.s;
  const float py = ui.origin.y + 88.0f * ui.s;
  const float panelW = ui.size.x - 48.0f * ui.s;
  const float panelH = ui.origin.y + ui.size.y - py - 62.0f * ui.s;

  ui.dl->AddRectFilled(ui.origin,
                       ImVec2(ui.origin.x + ui.size.x, ui.origin.y + ui.size.y),
                       IM_COL32(4, 8, 12, 224));
  DrawTitle(ui, ImVec2(px, ui.origin.y + 12.0f * ui.s), "COMMUNITY", "SOCIAL");

  const bool modalBlocked = g_showAccountModal || g_showDevMenu ||
                            ModDownloadPrompt::Get().IsActive() ||
                            ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
  const bool typing = ImGui::GetIO().WantTextInput || SocialOverlay::IsTyping();

  // Top-right Account Status Pill (always interactive so controller & mouse can
  // log in, switch account, or check ID cleanly)
  const bool loggedIn = AuthClient::Get().IsLoggedIn();
  std::string userTag =
      loggedIn ? AuthClient::Get().GetUsername() : "Not logged in";
  char statBuf[128];
  if (loggedIn) {
    std::snprintf(statBuf, sizeof(statBuf), "ONLINE: %s  (ID: %d)",
                  userTag.c_str(), AuthClient::Get().GetUserId());
  } else {
    std::snprintf(statBuf, sizeof(statBuf), "OFFLINE  -  SIGN IN REQUIRED");
  }
  const float pillFontPx = 14.0f * ui.s;
  const float tagW =
      TextWidth(NobleStyle::SmallFont(), pillFontPx, statBuf) + 34.0f * ui.s;
  const float actBtnW = (loggedIn ? 110.0f : 164.0f) * ui.s;
  const float topBtnH = 30.0f * ui.s;
  const float topY = ui.origin.y + 20.0f * ui.s;
  const ImVec2 statP0(ui.origin.x + ui.size.x - 24.0f * ui.s - tagW, topY);
  const ImVec2 statP1(ui.origin.x + ui.size.x - 24.0f * ui.s, topY + topBtnH);
  ui.dl->AddRectFilled(statP0, statP1,
                       loggedIn ? IM_COL32(14, 34, 24, 225)
                                : IM_COL32(34, 16, 18, 225),
                       3.0f * ui.s);
  NobleUi::DrawTacticalFrame(ui.dl, statP0, statP1, ui.s, false, loggedIn,
                             false, false);
  ui.dl->AddCircleFilled(
      ImVec2(statP0.x + 14.0f * ui.s, (statP0.y + statP1.y) * 0.5f),
      4.0f * ui.s,
      loggedIn ? IM_COL32(95, 235, 130, 255) : IM_COL32(235, 95, 95, 255));
  Text(ui, NobleStyle::SmallFont(), 14.0f,
       ImVec2(statP0.x + 24.0f * ui.s, topY + 7.0f * ui.s),
       loggedIn ? IM_COL32(165, 245, 185, 245) : IM_COL32(245, 170, 165, 245),
       statBuf);

  {
    const ImVec2 accP0(statP0.x - actBtnW - 10.0f * ui.s, topY);
    const ImVec2 accP1(statP0.x - 10.0f * ui.s, topY + topBtnH);
    ImGui::SetCursorScreenPos(accP0);
    const bool accClicked = ImGui::InvisibleButton("##social_account_btn",
                                                   ImVec2(actBtnW, topBtnH));
    const bool accHovered = ImGui::IsItemHovered();
    ui.dl->AddRectFilled(accP0, accP1,
                         accHovered ? IM_COL32(42, 74, 108, 220)
                                    : IM_COL32(18, 30, 44, 220),
                         2.0f * ui.s);
    NobleUi::DrawTacticalFrame(ui.dl, accP0, accP1, ui.s, false, !loggedIn,
                               accHovered, false);
    const char *accLbl = loggedIn ? "ACCOUNT" : "LOG IN / REGISTER";
    const float lw = TextWidth(NobleStyle::SmallFont(), 14.0f * ui.s, accLbl);
    Text(ui, NobleStyle::SmallFont(), 14.0f,
         ImVec2((accP0.x + accP1.x - lw) * 0.5f, topY + 7.0f * ui.s),
         loggedIn ? kWhite : NobleUi::kAmber, accLbl);
    if (accClicked || (!modalBlocked && !typing && UiPad::ActionY())) {
      OpenAccountModal();
      MenuSounds::Click();
    }
  }

  // Prominent join-by-code entry on the Social screen as well: same modal the Main Menu / Server Browser use.
  {
    const float joinW = 184.0f * ui.s;
    const ImVec2 jc0(statP0.x - actBtnW - 10.0f * ui.s - joinW - 10.0f * ui.s, topY);
    const ImVec2 jc1(jc0.x + joinW, topY + topBtnH);
    ImGui::SetCursorScreenPos(jc0);
    const bool joinClicked = ImGui::InvisibleButton("##social_join_code_btn", ImVec2(joinW, topBtnH));
    const bool joinHovered = ImGui::IsItemHovered();
    ui.dl->AddRectFilled(jc0, jc1,
                         joinHovered ? IM_COL32(42, 74, 108, 235) : IM_COL32(18, 30, 44, 225), 2.0f * ui.s);
    NobleUi::DrawTacticalFrame(ui.dl, jc0, jc1, ui.s, false, true, joinHovered, false);
    const char *joinLbl = "JOIN WITH CODE";
    const float jlw = TextWidth(NobleStyle::SmallFont(), 14.0f * ui.s, joinLbl);
    Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2((jc0.x + jc1.x - jlw) * 0.5f, topY + 7.0f * ui.s),
         NobleUi::kAmber, joinLbl);
    if (joinClicked && !modalBlocked && !typing) {
      st.screen = Screen::Servers;
      UIManager::GetServerBrowser().OpenJoinCodeModal();
      MenuSounds::Click();
    }
  }

  // Main Tactical Panel Container
  const ImVec2 pc0(px, py), pc1(px + panelW, py + panelH);
  ui.dl->AddRectFilled(pc0, pc1, IM_COL32(8, 14, 22, 195), 2.0f * ui.s);
  NobleUi::DrawTacticalFrame(ui.dl, pc0, pc1, ui.s, false, false, false, false);

  const float padX = 18.0f * ui.s;
  const float padY = 14.0f * ui.s;
  const float innerW = panelW - padX * 2.0f;

  // Data snapshot for badges & lists
  const auto invites = SocialClient::Get().GetInvites();
  auto friends = SocialClient::Get().GetFriends();
  static const bool s_friendsDemo =
      wcsstr(GetCommandLineW(), L"-social-friends-demo") !=
      nullptr; // test: rows without an account
  if (s_friendsDemo && friends.empty()) {
    friends.emplace_back();
    friends.back().username = "Kat-B320";
    friends.emplace_back();
    friends.back().username = "Carter-A259";
  }
  const auto incoming = SocialClient::Get().GetIncomingRequests();
  const auto outgoing = SocialClient::Get().GetOutgoingRequests();

  // Controller & keyboard focus model (navigable rows with horizontal actions)
  static int s_focusRow = 0;
  static int s_focusAction = 0;
  static int s_prevTab = -1;
  static int s_lastRowCount = 0;
  static int s_actionCounts[256] = {};
  static bool s_focusQuickAddBox = false;
  static bool s_focusSearchBox = false;

  // Tab switching via LB/RB or Q/E (guarded against modals and text boxes)
  if (!modalBlocked && !typing) {
    if (UiPad::TabPrev() || ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
      st.socialTab = (st.socialTab + 2) % 3;
      MenuSounds::Tab(-1);
    }
    if (UiPad::TabNext() || ImGui::IsKeyPressed(ImGuiKey_E, false)) {
      st.socialTab = (st.socialTab + 1) % 3;
      MenuSounds::Tab(1);
    }
  }
  if (s_prevTab != st.socialTab) {
    s_prevTab = st.socialTab;
    s_focusRow = 0;
    s_focusAction = 0;
  }

  ImGui::SetCursorScreenPos(ImVec2(px + padX, py + padY));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::BeginChild("##social_panel", ImVec2(innerW, panelH - padY * 2.0f),
                    ImGuiChildFlags_None,
                    ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse);

  // 1. Tactical Tab Header + Utility Actions
  const float tabH = 38.0f * ui.s;
  const float tabW = std::min(210.0f * ui.s, (innerW - 220.0f * ui.s) / 3.0f);
  const float tabGap = 8.0f * ui.s;
  const ImVec2 tabOrigin = ImGui::GetCursorScreenPos();

  const std::string tabLabels[3] = {
      "FRIENDS  (" + std::to_string(friends.size()) + ")",
      "REQUESTS  (" + std::to_string(incoming.size() + outgoing.size()) + ")",
      "FIND PLAYERS"};
  for (int t = 0; t < 3; ++t) {
    ImGui::PushID(t);
    const ImVec2 t0(tabOrigin.x + static_cast<float>(t) * (tabW + tabGap),
                    tabOrigin.y);
    const ImVec2 t1(t0.x + tabW, t0.y + tabH);
    const bool active = (st.socialTab == t);
    ImGui::SetCursorScreenPos(t0);
    if (NobleUi::DrawTile(ui.dl, "##soc_tab", t0, t1, tabLabels[t].c_str(),
                          nullptr, active, active, true,
                          t == 1 && !incoming.empty() ? "NEW" : nullptr)) {
      st.socialTab = t;
      MenuSounds::Tab(t > s_prevTab ? 1 : -1);
    }
    ImGui::PopID();
  }
  // Refresh button (top right of tab bar, also bound to X / F5)
  {
    const float refW = 132.0f * ui.s;
    const ImVec2 r0(tabOrigin.x + innerW - refW, tabOrigin.y);
    const ImVec2 r1(tabOrigin.x + innerW, tabOrigin.y + tabH);
    ImGui::SetCursorScreenPos(r0);
    const bool refClick =
        ImGui::InvisibleButton("##social_refresh", ImVec2(refW, tabH));
    const bool refHov = ImGui::IsItemHovered();
    ui.dl->AddRectFilled(
        r0, r1, refHov ? IM_COL32(36, 62, 92, 220) : IM_COL32(16, 28, 42, 210),
        2.0f * ui.s);
    NobleUi::DrawTacticalFrame(ui.dl, r0, r1, ui.s, false, false, refHov,
                               false);
    const char *refLbl = "REFRESH";
    const float gw =
        InputPrompts::Current() == InputPrompts::Device::Gamepad
            ? InputPrompts::ButtonWidth(NobleStyle::SmallFont(), 18.0f * ui.s,
                                        InputPrompts::Button::X)
            : InputPrompts::KeyWidth(NobleStyle::SmallFont(), 18.0f * ui.s,
                                     "F5");
    const float rw = TextWidth(NobleStyle::SmallFont(), 15.0f * ui.s, refLbl);
    const float totalW = gw + 8.0f * ui.s + rw;
    float cx = (r0.x + r1.x - totalW) * 0.5f;
    const float cy = r0.y + (tabH - 18.0f * ui.s) * 0.5f;
    if (InputPrompts::Current() == InputPrompts::Device::Gamepad) {
      cx += InputPrompts::DrawButton(ui.dl, NobleStyle::SmallFont(),
                                     ImVec2(cx, cy), 18.0f * ui.s,
                                     InputPrompts::Button::X) +
            8.0f * ui.s;
    } else {
      cx += InputPrompts::DrawKey(ui.dl, NobleStyle::SmallFont(),
                                  ImVec2(cx, cy), 18.0f * ui.s, "F5") +
            8.0f * ui.s;
    }
    Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(cx, r0.y + 10.0f * ui.s),
         kWhite, refLbl);
    if (refClick ||
        (!modalBlocked && !typing &&
         (UiPad::ActionX() || ImGui::IsKeyPressed(ImGuiKey_F5, false)))) {
      SocialClient::Get().RefreshFriendsAsync();
      SocialClient::Get().PollInvitesAsync();
      MenuSounds::Click();
    }
  }

  // Rule under tabs
  const float ruleY = tabOrigin.y + tabH + 8.0f * ui.s;
  ui.dl->AddLine(ImVec2(tabOrigin.x, ruleY),
                 ImVec2(tabOrigin.x + innerW, ruleY),
                 IM_COL32(143, 216, 255, 55), 1.0f);

  // Clamp focus against previous frame's row/action counts before rendering
  if (s_lastRowCount > 0) {
    s_focusRow = std::clamp(s_focusRow, 0, s_lastRowCount - 1);
    const int maxActs = std::max(1, s_actionCounts[s_focusRow]);
    s_focusAction = std::clamp(s_focusAction, 0, maxActs - 1);
  } else {
    s_focusRow = 0;
    s_focusAction = 0;
  }

  bool navMoved = false;
  bool acceptNow = false;
  if (!modalBlocked && !typing && s_lastRowCount > 0) {
    if (UiPad::Up()) {
      s_focusRow = (s_focusRow - 1 + s_lastRowCount) % s_lastRowCount;
      const int maxActs = std::max(1, s_actionCounts[s_focusRow]);
      s_focusAction = std::clamp(s_focusAction, 0, maxActs - 1);
      navMoved = true;
      MenuSounds::Scroll();
    } else if (UiPad::Down()) {
      s_focusRow = (s_focusRow + 1) % s_lastRowCount;
      const int maxActs = std::max(1, s_actionCounts[s_focusRow]);
      s_focusAction = std::clamp(s_focusAction, 0, maxActs - 1);
      navMoved = true;
      MenuSounds::Scroll();
    } else if (UiPad::Left() && s_actionCounts[s_focusRow] > 1) {
      const int maxActs = s_actionCounts[s_focusRow];
      s_focusAction = (s_focusAction - 1 + maxActs) % maxActs;
      MenuSounds::Scroll();
    } else if (UiPad::Right() && s_actionCounts[s_focusRow] > 1) {
      const int maxActs = s_actionCounts[s_focusRow];
      s_focusAction = (s_focusAction + 1) % maxActs;
      MenuSounds::Scroll();
    }
    if (UiPad::AcceptPressed()) {
      acceptNow = true;
      UiPad::ConsumeAccept();
    }
  }

  // Reserve bottom footer height for status telemetry bar
  std::string statusMsg = SocialClient::Get().GetLastStatusMessage();
  const float footerH = statusMsg.empty() ? 0.0f : 34.0f * ui.s;
  const float scrollTop = ruleY + 10.0f * ui.s;
  const float scrollH = (py + panelH - padY) - scrollTop - footerH -
                        (footerH > 0.0f ? 8.0f * ui.s : 0.0f);

  ImGui::SetCursorScreenPos(ImVec2(tabOrigin.x, scrollTop));
  ImGui::BeginChild("##social_scroll",
                    ImVec2(innerW, std::max(120.0f * ui.s, scrollH)),
                    ImGuiChildFlags_None, ImGuiWindowFlags_None);
  if (!modalBlocked && !typing) {
    UiPad::ScrollWindow(false, true);
  }

  int rowIdx = 0;
  int nextActionCounts[256] = {};
  const bool showPadFocus =
      !modalBlocked &&
      (InputPrompts::Current() == InputPrompts::Device::Gamepad || navMoved ||
       s_lastRowCount > 0);

  auto regRow = [&](int actionCount, bool &outRowFocused, bool &outRowHovered) {
    const int r = rowIdx++;
    if (r < 256)
      nextActionCounts[r] = std::max(1, actionCount);
    outRowFocused = showPadFocus && (r == s_focusRow);
    outRowHovered = false;
    return r;
  };

  auto drawCardBtn = [&](int r, int actIdx, int actTotal, ImVec2 b0, ImVec2 b1,
                         const char *btnId, const char *label, ImU32 accentBg,
                         bool destructive = false) -> bool {
    (void)actTotal;
    const bool activeFocus = (r == s_focusRow && actIdx == s_focusAction);
    ImGui::SetCursorScreenPos(b0);
    const bool clicked =
        ImGui::InvisibleButton(btnId, ImVec2(b1.x - b0.x, b1.y - b0.y));
    const bool hov = !modalBlocked && ImGui::IsItemHovered();
    if (hov) {
      s_focusRow = r;
      s_focusAction = actIdx;
    }
    ImU32 fill = accentBg;
    if (hov || activeFocus) {
      fill =
          destructive ? IM_COL32(115, 36, 36, 235) : IM_COL32(42, 86, 128, 235);
    }
    ui.dl->AddRectFilled(b0, b1, fill, 2.0f * ui.s);
    NobleUi::DrawTacticalFrame(ui.dl, b0, b1, ui.s, activeFocus, false, hov,
                               true);
    const bool showPadGlyph =
        activeFocus && !modalBlocked &&
        InputPrompts::Current() == InputPrompts::Device::Gamepad;
    const float lw = TextWidth(NobleStyle::SmallFont(), 14.0f * ui.s, label);
    const float glyphH = 15.0f * ui.s;
    const float glyphW =
        showPadGlyph
            ? InputPrompts::ButtonWidth(NobleStyle::SmallFont(), glyphH,
                                        InputPrompts::Button::A) +
                  5.0f * ui.s
            : 0.0f;
    float cx = (b0.x + b1.x - (glyphW + lw)) * 0.5f;
    if (showPadGlyph) {
      cx += InputPrompts::DrawButton(
                ui.dl, NobleStyle::SmallFont(),
                ImVec2(cx, b0.y + (b1.y - b0.y - glyphH) * 0.5f), glyphH,
                InputPrompts::Button::A) +
            5.0f * ui.s;
    }
    Text(ui, NobleStyle::SmallFont(), 14.0f,
         ImVec2(cx, b0.y + (b1.y - b0.y - 14.0f * ui.s) * 0.5f), kWhite, label);
    const bool padFire =
        acceptNow && r == s_focusRow && actIdx == s_focusAction;
    return clicked || padFire;
  };

  const float scrollW = ImGui::GetContentRegionAvail().x;
  const auto DrawSectionLabel = [&](const char *lbl, ImU32 barCol) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float h = 24.0f * ui.s;
    ui.dl->AddRectFilled(p, ImVec2(p.x + 3.5f * ui.s, p.y + 18.0f * ui.s),
                         barCol);
    Text(ui, NobleStyle::SmallFont(), 14.5f,
         ImVec2(p.x + 10.0f * ui.s, p.y + 2.0f * ui.s),
         IM_COL32(180, 212, 238, 240), lbl);
    ui.dl->AddLine(ImVec2(p.x, p.y + h), ImVec2(p.x + scrollW, p.y + h),
                   IM_COL32(143, 216, 255, 45), 1.0f);
    ImGui::Dummy(ImVec2(scrollW, h + 6.0f * ui.s));
  };

  // =========================================================================
  // INVITE BANNER CARDS (Always prioritized at top when present)
  // =========================================================================
  if (!invites.empty()) {
    char invHdr[64];
    std::snprintf(invHdr, sizeof(invHdr), "INCOMING GAME INVITES (%zu)",
                  invites.size());
    DrawSectionLabel(invHdr, IM_COL32(236, 178, 56, 255));

    for (size_t i = 0; i < invites.size(); ++i) {
      const auto &inv = invites[i];
      ImGui::PushID(static_cast<int>(500 + i));
      bool rFoc = false, rHov = false;
      const int r = regRow(2, rFoc, rHov);

      const float cardH = 60.0f * ui.s;
      const ImVec2 c0 = ImGui::GetCursorScreenPos();
      const ImVec2 c1(c0.x + scrollW, c0.y + cardH);
      ui.dl->AddRectFilled(
          c0, c1, rFoc ? IM_COL32(34, 44, 52, 235) : IM_COL32(18, 28, 38, 220),
          3.0f * ui.s);
      ui.dl->AddRectFilled(c0, ImVec2(c0.x + 4.0f * ui.s, c1.y),
                           IM_COL32(236, 178, 56, 245));
      NobleUi::DrawTacticalFrame(ui.dl, c0, c1, ui.s, rFoc, true, false, true);

      // Sender + Match details (no IPs ever rendered)
      const float textX = c0.x + 18.0f * ui.s;
      Text(ui, NobleStyle::TitleFont(), 19.0f,
           ImVec2(textX, c0.y + 10.0f * ui.s), kWhite,
           (inv.senderUsername + "  INVITED YOU TO JOIN").c_str());
      const std::string sub =
          "MODE: " +
          (inv.mode.empty() ? std::string("Multiplayer") : inv.mode) +
          (inv.map.empty() ? std::string("") : "   |   MAP: " + inv.map);
      Text(ui, NobleStyle::SmallFont(), 13.5f,
           ImVec2(textX, c0.y + 34.0f * ui.s), NobleUi::kAmber, sub.c_str());

      // Action buttons
      const float btnH = 30.0f * ui.s;
      const float btnY = c0.y + (cardH - btnH) * 0.5f;
      const float joinW = 122.0f * ui.s;
      const float disW = 98.0f * ui.s;
      const float gap = 8.0f * ui.s;
      const ImVec2 dis0(c1.x - 14.0f * ui.s - disW, btnY);
      const ImVec2 dis1(dis0.x + disW, btnY + btnH);
      const ImVec2 join0(dis0.x - gap - joinW, btnY);
      const ImVec2 join1(join0.x + joinW, btnY + btnH);

      if (drawCardBtn(r, 0, 2, join0, join1, "##inv_join", "JOIN GAME",
                      IM_COL32(28, 92, 62, 225))) {
        ServerEntry s;
        s.id = inv.sessionId;
        s.name = inv.senderUsername + "'s Game";
        s.hostAddress = inv.hostIp;
        s.port = inv.hostPort;
        s.mapName = inv.map.empty() ? "20_sword_slayer" : inv.map;
        s.gameMode = inv.mode.empty() ? "multiplayer" : inv.mode;
        s.maxPlayers = 16;
        EnterLobby(GetOverlayCtx(), s);
        SocialClient::Get().ClearInviteAsync(inv.senderUsername);
        MenuSounds::Click();
      }
      if (drawCardBtn(r, 1, 2, dis0, dis1, "##inv_dismiss", "DISMISS",
                      IM_COL32(42, 24, 28, 220), true)) {
        SocialClient::Get().ClearInviteAsync(inv.senderUsername);
        MenuSounds::Back();
      }

      if (rFoc && navMoved)
        ImGui::SetScrollHereY(0.45f);
      ImGui::SetCursorScreenPos(c0);
      ImGui::Dummy(ImVec2(scrollW, cardH + 8.0f * ui.s));
      ImGui::PopID();
    }
    ImGui::Dummy(ImVec2(0, 6.0f * ui.s));
  }

  // =========================================================================
  // TAB CONTENTS
  // =========================================================================
  if (st.socialTab == 0) {
    // FRIENDS TAB: Quick Add Bar (Navigable Row)
    static char s_quickAddBuf[64] = "";
    {
      bool rFoc = false, rHov = false;
      const int r = regRow(2, rFoc, rHov);
      const float barH = 48.0f * ui.s;
      const ImVec2 b0 = ImGui::GetCursorScreenPos();
      const ImVec2 b1(b0.x + scrollW, b0.y + barH);
      ui.dl->AddRectFilled(b0, b1, IM_COL32(14, 22, 32, 210), 2.0f * ui.s);
      NobleUi::DrawTacticalFrame(ui.dl, b0, b1, ui.s, rFoc, false, false, true);

      const float addBtnW = 148.0f * ui.s;
      const float inputW =
          std::max(180.0f * ui.s, scrollW - addBtnW - 34.0f * ui.s);
      const float ctrlY = b0.y + 9.0f * ui.s;
      ImGui::SetCursorScreenPos(ImVec2(b0.x + 12.0f * ui.s, ctrlY));
      ImGui::SetNextItemWidth(inputW);
      if (s_focusQuickAddBox) {
        ImGui::SetKeyboardFocusHere();
        s_focusQuickAddBox = false;
      }
      bool enterAdd = ImGui::InputTextWithHint(
          "##quick_add_friend", "Add friend by username or IP address...",
          s_quickAddBuf, sizeof(s_quickAddBuf),
          ImGuiInputTextFlags_EnterReturnsTrue);
      if (rFoc && s_focusAction == 0) {
        NobleUi::DrawFocusHighlight(ui.dl, ImGui::GetItemRectMin(),
                                    ImGui::GetItemRectMax(), ui.s, true);
        if (acceptNow) {
          s_focusQuickAddBox = true;
          MenuSounds::Click();
        }
      }

      const ImVec2 ab0(b1.x - 12.0f * ui.s - addBtnW, ctrlY);
      const ImVec2 ab1(b1.x - 12.0f * ui.s, ctrlY + 30.0f * ui.s);
      const bool addFired =
          drawCardBtn(r, 1, 2, ab0, ab1, "##btn_add_friend", "ADD FRIEND",
                      IM_COL32(28, 66, 98, 225));
      if ((addFired || enterAdd) && s_quickAddBuf[0] != '\0') {
        SocialClient::Get().AddFriendDirect(s_quickAddBuf);
        s_quickAddBuf[0] = '\0';
        MenuSounds::Click();
      }
      if (rFoc && navMoved)
        ImGui::SetScrollHereY(0.25f);
      ImGui::SetCursorScreenPos(b0);
      ImGui::Dummy(ImVec2(scrollW, barH + 10.0f * ui.s));
    }

    char frHdr[64];
    std::snprintf(frHdr, sizeof(frHdr), "ROSTER / FRIENDS (%zu)",
                  friends.size());
    DrawSectionLabel(frHdr, NobleUi::kAccent);

    if (friends.empty()) {
      const float empH = 88.0f * ui.s;
      const ImVec2 e0 = ImGui::GetCursorScreenPos();
      const ImVec2 e1(e0.x + scrollW, e0.y + empH);
      ui.dl->AddRectFilled(e0, e1, IM_COL32(12, 18, 28, 175), 3.0f * ui.s);
      NobleUi::DrawTacticalFrame(ui.dl, e0, e1, ui.s, false, false, false,
                                 false);
      Text(ui, NobleStyle::TitleFont(), 18.0f,
           ImVec2(e0.x + 20.0f * ui.s, e0.y + 18.0f * ui.s), kWhite,
           "NO FRIENDS ADDED YET");
      Text(ui, NobleStyle::SmallFont(), 14.5f,
           ImVec2(e0.x + 20.0f * ui.s, e0.y + 46.0f * ui.s), kDim,
           "Add a Spartan by username/IP above, or switch to FIND PLAYERS "
           "[LB/RB].");
      ImGui::Dummy(ImVec2(scrollW, empH + 8.0f * ui.s));
    } else {
      std::string myIp = UPnP::Get().GetExternalIP();
      if (myIp.empty() || myIp == "Unknown")
        myIp = "127.0.0.1";

      for (size_t i = 0; i < friends.size(); ++i) {
        const auto &f = friends[i];
        const bool hasGame = !f.endpoint.empty();
        const int actCount = hasGame ? 3 : 2;
        ImGui::PushID(static_cast<int>(i));
        bool rFoc = false, rHov = false;
        const int r = regRow(actCount, rFoc, rHov);

        const float cardH = 58.0f * ui.s;
        const ImVec2 c0 = ImGui::GetCursorScreenPos();
        const ImVec2 c1(c0.x + scrollW, c0.y + cardH);
        const bool cardHover = ImGui::IsMouseHoveringRect(c0, c1, false);
        ui.dl->AddRectFilled(c0, c1,
                             rFoc ? IM_COL32(22, 38, 54, 232)
                                  : (cardHover ? IM_COL32(16, 28, 42, 220)
                                               : IM_COL32(11, 19, 30, 210)),
                             3.0f * ui.s);
        ui.dl->AddRectFilled(c0, ImVec2(c0.x + 4.0f * ui.s, c1.y),
                             hasGame ? IM_COL32(82, 225, 130, 240)
                                     : IM_COL32(95, 175, 225, 190));
        NobleUi::DrawTacticalFrame(ui.dl, c0, c1, ui.s, rFoc, hasGame,
                                   cardHover, true);

        // Monogram Spartan/Sangheili Shield Badge
        const float badgeS = 36.0f * ui.s;
        const ImVec2 bg0(c0.x + 16.0f * ui.s, c0.y + (cardH - badgeS) * 0.5f);
        const ImVec2 bg1(bg0.x + badgeS, bg0.y + badgeS);
        ui.dl->AddRectFilled(bg0, bg1, IM_COL32(22, 38, 56, 235), 3.0f * ui.s);
        ui.dl->AddRect(bg0, bg1,
                       hasGame ? IM_COL32(95, 225, 135, 190)
                               : IM_COL32(143, 216, 255, 120),
                       3.0f * ui.s);
        char initCh[3] = {f.username.empty()
                              ? '?'
                              : static_cast<char>(std::toupper(
                                    static_cast<unsigned char>(f.username[0]))),
                          0, 0};
        const float iw =
            TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, initCh);
        Text(ui, NobleStyle::TitleFont(), 18.0f,
             ImVec2((bg0.x + bg1.x - iw) * 0.5f, bg0.y + 8.0f * ui.s),
             hasGame ? IM_COL32(140, 245, 170, 255) : kAccent, initCh);

        // Name + Status Pill (never display raw endpoint IP: privacy safe)
        const float infoX = bg1.x + 14.0f * ui.s;
        Text(ui, NobleStyle::TitleFont(), 19.0f,
             ImVec2(infoX, c0.y + 9.0f * ui.s), kWhite, f.username.c_str());
        const char *stTxt = hasGame ? "IN LOBBY / JOINABLE" : "ONLINE / FRIEND";
        const ImU32 stCol = hasGame ? IM_COL32(115, 235, 150, 235)
                                    : IM_COL32(143, 216, 255, 190);
        Text(ui, NobleStyle::SmallFont(), 13.5f,
             ImVec2(infoX, c0.y + 33.0f * ui.s), stCol, stTxt);

        // Right-aligned tactical action buttons
        const float btnH = 30.0f * ui.s;
        const float btnY = c0.y + (cardH - btnH) * 0.5f;
        const float btnW = 94.0f * ui.s;
        const float gap = 8.0f * ui.s;
        int idx = 0;

        float rightEdge = c1.x - 14.0f * ui.s;
        const ImVec2 rem0(rightEdge - btnW, btnY);
        const ImVec2 rem1(rightEdge, btnY + btnH);
        rightEdge = rem0.x - gap;

        ImVec2 join0{}, join1{};
        if (hasGame) {
          join0 = ImVec2(rightEdge - btnW, btnY);
          join1 = ImVec2(rightEdge, btnY + btnH);
          rightEdge = join0.x - gap;
        }
        const ImVec2 inv0(rightEdge - btnW, btnY);
        const ImVec2 inv1(rightEdge, btnY + btnH);

        if (drawCardBtn(r, idx++, actCount, inv0, inv1, "##fr_inv", "INVITE",
                        IM_COL32(28, 66, 98, 220))) {
          std::string invMode = "Custom Game";
          if (st.lobbyMode == "campaign")
            invMode = "Campaign";
          else if (st.lobbyMode == "firefight")
            invMode = "Firefight";
          else if (st.lobbyMode == "forge")
            invMode = "Forge";
          if (s_friendsDemo)
            NOBLE_INFO(
                "FrontEndMenu: -social-friends-demo INVITE pressed for {}",
                f.username);
          else
            SocialClient::Get().SendInviteAsync(
                f.username, myIp, 11774,
                [](bool ok, const std::string &msg) {
                  (void)msg;
                  if (ok)
                    MenuSounds::Click();
                },
                invMode, st.lobbyMap, st.lobbySessionId);
          MenuSounds::Click();
        }
        if (hasGame) {
          if (drawCardBtn(r, idx++, actCount, join0, join1, "##fr_join", "JOIN",
                          IM_COL32(28, 92, 62, 225))) {
            ServerEntry s;
            s.name = f.username + "'s Game";
            const size_t colon = f.endpoint.rfind(':');
            s.hostAddress = colon != std::string::npos
                                ? f.endpoint.substr(0, colon)
                                : f.endpoint;
            s.port = static_cast<uint16_t>(
                colon != std::string::npos
                    ? std::atoi(f.endpoint.c_str() + colon + 1)
                    : 11774);
            s.mapName = "20_sword_slayer";
            s.gameMode = "multiplayer";
            s.maxPlayers = 16;
            EnterLobby(GetOverlayCtx(), s);
            MenuSounds::Click();
          }
        }
        if (drawCardBtn(r, idx++, actCount, rem0, rem1, "##fr_rem", "REMOVE",
                        IM_COL32(42, 22, 26, 220), true) &&
            !s_friendsDemo) {
          SocialClient::Get().RemoveFriendAsync(f.username);
          MenuSounds::Back();
        }

        if (rFoc && navMoved)
          ImGui::SetScrollHereY(0.45f);
        ImGui::SetCursorScreenPos(c0);
        ImGui::Dummy(ImVec2(scrollW, cardH + 7.0f * ui.s));
        ImGui::PopID();
      }
    }
  } else if (st.socialTab == 1) {
    // REQUESTS TAB
    char inHdr[64];
    std::snprintf(inHdr, sizeof(inHdr), "INCOMING REQUESTS (%zu)",
                  incoming.size());
    DrawSectionLabel(inHdr, NobleUi::kAmber);

    if (incoming.empty()) {
      Text(ui, NobleStyle::SmallFont(), 14.5f,
           ImVec2(ImGui::GetCursorScreenPos().x + 8.0f * ui.s,
                  ImGui::GetCursorScreenPos().y + 2.0f * ui.s),
           kDim, "No pending incoming friend requests.");
      ImGui::Dummy(ImVec2(scrollW, 28.0f * ui.s));
    } else {
      for (size_t i = 0; i < incoming.size(); ++i) {
        const auto &req = incoming[i];
        ImGui::PushID(static_cast<int>(100 + i));
        bool rFoc = false, rHov = false;
        const int r = regRow(2, rFoc, rHov);

        const float cardH = 54.0f * ui.s;
        const ImVec2 c0 = ImGui::GetCursorScreenPos();
        const ImVec2 c1(c0.x + scrollW, c0.y + cardH);
        ui.dl->AddRectFilled(c0, c1,
                             rFoc ? IM_COL32(24, 38, 52, 232)
                                  : IM_COL32(12, 20, 30, 215),
                             3.0f * ui.s);
        ui.dl->AddRectFilled(c0, ImVec2(c0.x + 4.0f * ui.s, c1.y),
                             NobleUi::kAmber);
        NobleUi::DrawTacticalFrame(ui.dl, c0, c1, ui.s, rFoc, false, false,
                                   true);

        Text(ui, NobleStyle::TitleFont(), 18.0f,
             ImVec2(c0.x + 18.0f * ui.s, c0.y + 8.0f * ui.s), kWhite,
             req.username.c_str());
        Text(ui, NobleStyle::SmallFont(), 13.5f,
             ImVec2(c0.x + 18.0f * ui.s, c0.y + 30.0f * ui.s), NobleUi::kAmber,
             "Sent you a friend request");

        const float btnH = 28.0f * ui.s;
        const float btnY = c0.y + (cardH - btnH) * 0.5f;
        const float btnW = 104.0f * ui.s;
        const ImVec2 dec0(c1.x - 14.0f * ui.s - btnW, btnY);
        const ImVec2 dec1(dec0.x + btnW, btnY + btnH);
        const ImVec2 acc0(dec0.x - 8.0f * ui.s - btnW, btnY);
        const ImVec2 acc1(acc0.x + btnW, btnY + btnH);

        if (drawCardBtn(r, 0, 2, acc0, acc1, "##req_acc", "ACCEPT",
                        IM_COL32(28, 92, 62, 225))) {
          SocialClient::Get().AcceptFriendRequestAsync(req.username);
          MenuSounds::Click();
        }
        if (drawCardBtn(r, 1, 2, dec0, dec1, "##req_dec", "DECLINE",
                        IM_COL32(42, 24, 28, 220), true)) {
          SocialClient::Get().RemoveFriendAsync(req.username);
          MenuSounds::Back();
        }

        if (rFoc && navMoved)
          ImGui::SetScrollHereY(0.45f);
        ImGui::SetCursorScreenPos(c0);
        ImGui::Dummy(ImVec2(scrollW, cardH + 7.0f * ui.s));
        ImGui::PopID();
      }
    }

    ImGui::Dummy(ImVec2(0, 10.0f * ui.s));
    char outHdr[64];
    std::snprintf(outHdr, sizeof(outHdr), "OUTGOING REQUESTS (%zu)",
                  outgoing.size());
    DrawSectionLabel(outHdr, IM_COL32(143, 216, 255, 190));

    if (outgoing.empty()) {
      Text(ui, NobleStyle::SmallFont(), 14.5f,
           ImVec2(ImGui::GetCursorScreenPos().x + 8.0f * ui.s,
                  ImGui::GetCursorScreenPos().y + 2.0f * ui.s),
           kDim, "No pending outgoing requests.");
      ImGui::Dummy(ImVec2(scrollW, 28.0f * ui.s));
    } else {
      for (size_t i = 0; i < outgoing.size(); ++i) {
        const auto &req = outgoing[i];
        ImGui::PushID(static_cast<int>(200 + i));
        bool rFoc = false, rHov = false;
        const int r = regRow(1, rFoc, rHov);

        const float cardH = 52.0f * ui.s;
        const ImVec2 c0 = ImGui::GetCursorScreenPos();
        const ImVec2 c1(c0.x + scrollW, c0.y + cardH);
        ui.dl->AddRectFilled(c0, c1,
                             rFoc ? IM_COL32(22, 34, 48, 230)
                                  : IM_COL32(11, 18, 26, 210),
                             3.0f * ui.s);
        NobleUi::DrawTacticalFrame(ui.dl, c0, c1, ui.s, rFoc, false, false,
                                   true);

        Text(ui, NobleStyle::TitleFont(), 18.0f,
             ImVec2(c0.x + 18.0f * ui.s, c0.y + 8.0f * ui.s), kWhite,
             req.username.c_str());
        Text(ui, NobleStyle::SmallFont(), 13.5f,
             ImVec2(c0.x + 18.0f * ui.s, c0.y + 29.0f * ui.s), kDim,
             "Awaiting player acceptance");

        const float btnH = 28.0f * ui.s;
        const float btnY = c0.y + (cardH - btnH) * 0.5f;
        const float btnW = 104.0f * ui.s;
        const ImVec2 can0(c1.x - 14.0f * ui.s - btnW, btnY);
        const ImVec2 can1(can0.x + btnW, btnY + btnH);
        if (drawCardBtn(r, 0, 1, can0, can1, "##req_can", "CANCEL",
                        IM_COL32(42, 24, 28, 220), true)) {
          SocialClient::Get().RemoveFriendAsync(req.username);
          MenuSounds::Back();
        }

        if (rFoc && navMoved)
          ImGui::SetScrollHereY(0.45f);
        ImGui::SetCursorScreenPos(c0);
        ImGui::Dummy(ImVec2(scrollW, cardH + 7.0f * ui.s));
        ImGui::PopID();
      }
    }
  } else if (st.socialTab == 2) {
    // FIND PLAYERS TAB: Search Bar
    {
      bool rFoc = false, rHov = false;
      const int r = regRow(2, rFoc, rHov);
      const float barH = 48.0f * ui.s;
      const ImVec2 b0 = ImGui::GetCursorScreenPos();
      const ImVec2 b1(b0.x + scrollW, b0.y + barH);
      ui.dl->AddRectFilled(b0, b1, IM_COL32(14, 22, 32, 210), 2.0f * ui.s);
      NobleUi::DrawTacticalFrame(ui.dl, b0, b1, ui.s, rFoc, false, false, true);

      const float srchBtnW = 138.0f * ui.s;
      const float inputW =
          std::max(180.0f * ui.s, scrollW - srchBtnW - 34.0f * ui.s);
      const float ctrlY = b0.y + 9.0f * ui.s;
      ImGui::SetCursorScreenPos(ImVec2(b0.x + 12.0f * ui.s, ctrlY));
      ImGui::SetNextItemWidth(inputW);
      if (s_focusSearchBox) {
        ImGui::SetKeyboardFocusHere();
        s_focusSearchBox = false;
      }
      bool enterPressed = ImGui::InputTextWithHint(
          "##search_player", "Search community players by username...",
          st.socialSearchBuf, sizeof(st.socialSearchBuf),
          ImGuiInputTextFlags_EnterReturnsTrue);
      if (rFoc && s_focusAction == 0) {
        NobleUi::DrawFocusHighlight(ui.dl, ImGui::GetItemRectMin(),
                                    ImGui::GetItemRectMax(), ui.s, true);
        if (acceptNow) {
          s_focusSearchBox = true;
          MenuSounds::Click();
        }
      }

      const ImVec2 sb0(b1.x - 12.0f * ui.s - srchBtnW, ctrlY);
      const ImVec2 sb1(b1.x - 12.0f * ui.s, ctrlY + 30.0f * ui.s);
      const bool btnSearch =
          drawCardBtn(r, 1, 2, sb0, sb1, "##btn_search_player", "SEARCH",
                      IM_COL32(28, 66, 98, 225));
      if ((btnSearch || enterPressed) && st.socialSearchBuf[0] != '\0') {
        st.socialSearching = true;
        std::string q = st.socialSearchBuf;
        SocialClient::Get().SearchUsersAsync(
            q, [&st](bool ok, const std::vector<UserSearchResult> &results) {
              (void)ok;
              st.socialSearchResults = results;
              st.socialSearching = false;
            });
        MenuSounds::Click();
      }
      if (rFoc && navMoved)
        ImGui::SetScrollHereY(0.25f);
      ImGui::SetCursorScreenPos(b0);
      ImGui::Dummy(ImVec2(scrollW, barH + 10.0f * ui.s));
    }

    if (st.socialSearching) {
      Text(ui, NobleStyle::TitleFont(), 17.0f, ImGui::GetCursorScreenPos(),
           kAccent, "SEARCHING PLAYERS...");
      ImGui::Dummy(ImVec2(scrollW, 30.0f * ui.s));
    } else if (!st.socialSearchResults.empty()) {
      char resHdr[64];
      std::snprintf(resHdr, sizeof(resHdr), "SEARCH RESULTS (%zu)",
                    st.socialSearchResults.size());
      DrawSectionLabel(resHdr, NobleUi::kAccent);

      for (size_t i = 0; i < st.socialSearchResults.size(); ++i) {
        const auto &u = st.socialSearchResults[i];
        ImGui::PushID(static_cast<int>(300 + i));
        bool rFoc = false, rHov = false;
        const int r = regRow(1, rFoc, rHov);

        const float cardH = 54.0f * ui.s;
        const ImVec2 c0 = ImGui::GetCursorScreenPos();
        const ImVec2 c1(c0.x + scrollW, c0.y + cardH);
        ui.dl->AddRectFilled(c0, c1,
                             rFoc ? IM_COL32(22, 38, 54, 232)
                                  : IM_COL32(11, 19, 30, 210),
                             3.0f * ui.s);
        NobleUi::DrawTacticalFrame(ui.dl, c0, c1, ui.s, rFoc, false, false,
                                   true);

        Text(ui, NobleStyle::TitleFont(), 18.5f,
             ImVec2(c0.x + 18.0f * ui.s, c0.y + 8.0f * ui.s), kWhite,
             u.username.c_str());
        char idStr[48];
        std::snprintf(idStr, sizeof(idStr), "NOBLE ID: #%d", u.id);
        Text(ui, NobleStyle::SmallFont(), 13.5f,
             ImVec2(c0.x + 18.0f * ui.s, c0.y + 30.0f * ui.s), kDim, idStr);

        const float btnH = 28.0f * ui.s;
        const float btnY = c0.y + (cardH - btnH) * 0.5f;
        const float btnW = 132.0f * ui.s;
        const ImVec2 ab0(c1.x - 14.0f * ui.s - btnW, btnY);
        const ImVec2 ab1(ab0.x + btnW, btnY + btnH);
        if (drawCardBtn(r, 0, 1, ab0, ab1, "##srch_add", "ADD FRIEND",
                        IM_COL32(28, 66, 98, 225))) {
          SocialClient::Get().SendFriendRequestAsync(u.username);
          MenuSounds::Click();
        }

        if (rFoc && navMoved)
          ImGui::SetScrollHereY(0.45f);
        ImGui::SetCursorScreenPos(c0);
        ImGui::Dummy(ImVec2(scrollW, cardH + 7.0f * ui.s));
        ImGui::PopID();
      }
    } else if (st.socialSearchBuf[0] != '\0') {
      char noneStr[128];
      std::snprintf(noneStr, sizeof(noneStr), "No players found matching '%s'.",
                    st.socialSearchBuf);
      Text(ui, NobleStyle::SmallFont(), 15.0f, ImGui::GetCursorScreenPos(),
           IM_COL32(230, 175, 110, 235), noneStr);
      ImGui::Dummy(ImVec2(scrollW, 30.0f * ui.s));
    } else {
      const float hintH = 74.0f * ui.s;
      const ImVec2 h0 = ImGui::GetCursorScreenPos();
      const ImVec2 h1(h0.x + scrollW, h0.y + hintH);
      ui.dl->AddRectFilled(h0, h1, IM_COL32(12, 18, 28, 165), 3.0f * ui.s);
      NobleUi::DrawTacticalFrame(ui.dl, h0, h1, ui.s, false, false, false,
                                 false);
      Text(ui, NobleStyle::SmallFont(), 15.0f,
           ImVec2(h0.x + 18.0f * ui.s, h0.y + 26.0f * ui.s), kDim,
           "Type a player's Noble username above and press SEARCH to send a "
           "friend request.");
      ImGui::Dummy(ImVec2(scrollW, hintH + 6.0f * ui.s));
    }
  }

  // Persist current frame row/action topology for crisp next-frame clamping
  s_lastRowCount = std::min(rowIdx, 256);
  for (int k = 0; k < s_lastRowCount; ++k)
    s_actionCounts[k] = nextActionCounts[k];

  ImGui::EndChild(); // ##social_scroll

  // Footer status telemetry pill
  if (!statusMsg.empty()) {
    const float fy = py + panelH - padY - 30.0f * ui.s;
    const ImVec2 f0(tabOrigin.x, fy);
    const ImVec2 f1(tabOrigin.x + innerW, fy + 30.0f * ui.s);
    ui.dl->AddRectFilled(f0, f1, IM_COL32(14, 30, 24, 215), 2.0f * ui.s);
    ui.dl->AddRect(f0, f1, IM_COL32(75, 200, 120, 140), 2.0f * ui.s);
    Text(ui, NobleStyle::SmallFont(), 14.0f,
         ImVec2(f0.x + 12.0f * ui.s, f0.y + 7.0f * ui.s),
         IM_COL32(140, 240, 165, 245), ("STATUS: " + statusMsg).c_str());
  }

  ImGui::EndChild(); // ##social_panel
  ImGui::PopStyleColor();

  if (back && !modalBlocked && !typing) {
    UiPad::ConsumeBack();
    st.screen = st.socialPreviousScreen;
    st.focusRow = 0;
    MenuSounds::Back();
  }
  DrawHints(ui,
            {{Prompt::Accept, "Select"},
             {Prompt::Back, "Back"},
             {Prompt::Tabs, "Switch Tab"}},
            {{InputPrompts::Button::X, "F5", "Refresh"},
             {InputPrompts::Button::Y, "TAB", "Account"}},
            0.01875f); // the full-width panel's edge
}

// Shown once, the first time Noble starts (spec 4.12). New players get a short
// setup; anyone else goes straight to the menu. Noble has no graphics presets,
// so the spec's preset step is left out rather than offering one that does
// nothing. Returns true while it's up, and then the menu behind it takes no
// input.
bool DrawFirstStart(const Ui &ui) {
  static const bool s_forced =
      wcsstr(GetCommandLineW(), L"-first-start") != nullptr;
  static bool s_forcedDone = false;
  const SettingsData saved = Settings::Get().Snapshot();
  if (saved.firstStartDone && (!s_forced || s_forcedDone))
    return false;

  static int s_step = 0, s_focus = 0;
  static bool s_controller = false, s_loaded = false;
  static float s_sensitivity = 1.0f;
  static char s_name[16] = "", s_tag[5] = "";
  if (!s_loaded) {
    s_loaded = true;
    std::snprintf(s_name, sizeof(s_name), "%s", saved.playerName.c_str());
    std::snprintf(s_tag, sizeof(s_tag), "%s", saved.serviceTag.c_str());
  }
  const auto finish = [&](bool setup) {
    SettingsData d = Settings::Get().Snapshot();
    d.firstStartDone = true;
    d.lastSeenVersion = NobleChangelog::CurrentChangelogVersionKey();
    if (setup) {
      if (s_name[0])
        d.playerName = s_name;
      if (s_tag[0])
        d.serviceTag = s_tag;
      if (s_controller)
        d.padLookSpeedX = d.padLookSpeedY = s_sensitivity;
      else
        d.mouseSensitivity = s_sensitivity;
    }
    Settings::Get().Update(d);
    if (setup) {
      const SettingsData updated = Settings::Get().Snapshot();
      DeviceIdentity id = DeviceIdentityStore::Get();
      id.playerName = updated.playerName;
      id.serviceTag = updated.serviceTag;
      DeviceIdentityStore::Save(id);
      AuthClient::Get().SetPlayerName(updated.playerName);
      if (UIManager::HasArmorCustomizer()) {
        UIManager::GetArmorCustomizer().SetServiceTag(updated.serviceTag);
      }
    }
    s_forcedDone = true;
    s_step = 0;
    s_focus = 0;
    NOBLE_INFO("FrontEndMenu: first start answered (setup: {})", setup);
    MenuSounds::Click();
  };

  ImGuiIO &io = ImGui::GetIO();
  const float s = ui.s;
  const ImVec2 size(720.0f * s, 440.0f * s);
  ImGui::SetNextWindowPos(ImVec2(ui.origin.x + (ui.size.x - size.x) * 0.5f,
                                 ui.origin.y + (ui.size.y - size.y) * 0.5f));
  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowFocus();
  ImGui::SetNextWindowBgAlpha(0.97f);
  ImGui::Begin("##firststart", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus);
  const bool typing = io.WantTextInput;
  const bool up = !typing && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up());
  const bool down = !typing && (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down());
  const bool left = !typing && (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left());
  const bool right = !typing && (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right());
  const bool accept = !typing && UiPad::AcceptPressed();
  const bool back = !typing && UiPad::BackPressed();

  const auto heading = [&](const char *title, const char *text) {
    ImGui::Dummy(ImVec2(0, 10.0f * s));
    ImGui::PushFont(NobleStyle::TitleFont());
    ImGui::TextUnformatted(title);
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 6.0f * s));
    ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 14.0f * s));
  };
  int rows = 2;
  const auto option = [&](int index, const char *label) {
    const bool focused = s_focus == index;
    ImGui::PushStyleColor(ImGuiCol_Button,
                          focused ? ImVec4(0.20f, 0.45f, 0.70f, 0.95f)
                                  : ImVec4(0.08f, 0.12f, 0.16f, 0.9f));
    const bool clicked = ImGui::Button(
        label, ImVec2(ImGui::GetContentRegionAvail().x, 44.0f * s));
    ImGui::PopStyleColor();
    return clicked || (accept && focused);
  };
  const auto nextStep = [&](int step) {
    s_step = step;
    s_focus = 0;
    MenuSounds::Click();
  };

  if (s_step == 0) {
    heading("NEW TO HALO: REACH?", "This is only asked once. You can run the "
                                   "setup again from Settings > System.");
    if (option(0, "NEW PLAYER"))
      nextStep(1);
    if (option(1, "REGULAR OR RETURNING PLAYER"))
      finish(false);
  } else if (s_step == 1) {
    heading("HOW WILL YOU PLAY?",
            "Both always work, and Noble switches to whichever you touched "
            "last. This picks which look speed to set up next.");
    const auto choose = [&](bool controller) {
      s_controller = controller;
      s_sensitivity = controller ? saved.padLookSpeedX : saved.mouseSensitivity;
      nextStep(2);
    };
    if (option(0, "KEYBOARD AND MOUSE"))
      choose(false);
    if (option(1, "CONTROLLER"))
      choose(true);
  } else if (s_step == 2) {
    rows = 1;
    heading("YOUR NAME",
            "Shown on scoreboards and in chat. Typing needs a keyboard; on a "
            "controller, CONTINUE keeps what's here.");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
    const bool nameDone = ImGui::InputTextWithHint(
        "Name", "Up to 15 characters", s_name, sizeof(s_name),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
    const bool tagDone = ImGui::InputTextWithHint(
        "Service tag", "4 letters or digits", s_tag, sizeof(s_tag),
        ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_CharsUppercase |
            ImGuiInputTextFlags_CharsNoBlank);
    ImGui::Dummy(ImVec2(0, 10.0f * s));
    if (option(0, "CONTINUE") || nameDone || tagDone)
      nextStep(3);
  } else {
    rows = 1;
    heading("LOOK SPEED", s_controller
                              ? "Left and right change how fast you turn. "
                                "2.00x is Reach's own speed."
                              : "Left and right change how far you turn per "
                                "mouse movement. 1.00x is Noble's default.");
    const float lo = s_controller ? 1.0f : 0.1f,
                hi = s_controller ? 10.0f : 5.0f,
                step = s_controller ? 0.25f : 0.05f;
    if (left)
      s_sensitivity = std::clamp(s_sensitivity - step, lo, hi);
    if (right)
      s_sensitivity = std::clamp(s_sensitivity + step, lo, hi);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::SliderFloat("##firststartlook", &s_sensitivity, lo, hi, "%.2fx");
    ImGui::Dummy(ImVec2(0, 10.0f * s));
    if (option(0, "FINISH"))
      finish(true);
  }
  if (up)
    s_focus = (s_focus + rows - 1) % rows;
  if (down)
    s_focus = (s_focus + 1) % rows;
  if (back && s_step > 0) {
    s_focus = 0;
    --s_step;
    MenuSounds::Back();
  }
  ImGui::End();
  return true;
}

static bool s_forcedChangelogDismissed = false;
static bool s_autoChangelogDismissedThisBoot = false;

bool ShouldShowChangelogModal(bool manualOpen, bool firstStartActive) {
  if (firstStartActive)
    return false;
  if (manualOpen)
    return true;
  static const bool s_forcedChangelog =
      wcsstr(GetCommandLineW(), L"-changelog") != nullptr;
  static const bool s_suppressAutoChangelog =
      wcsstr(GetCommandLineW(), L"-auto-play-map") != nullptr ||
      wcsstr(GetCommandLineW(), L"-menu-capture") != nullptr ||
      wcsstr(GetCommandLineW(), L"-social-friends-demo") != nullptr ||
      wcsstr(GetCommandLineW(), L"-armory-species") != nullptr ||
      wcsstr(GetCommandLineW(), L"-armory-cycle") != nullptr ||
      wcsstr(GetCommandLineW(), L"-pad-script") != nullptr ||
      wcsstr(GetCommandLineW(), L"-test-map-gametypes") != nullptr ||
      wcsstr(GetCommandLineW(), L"-first-start") != nullptr;
  if (s_forcedChangelog && !s_forcedChangelogDismissed)
    return true;
  if (s_suppressAutoChangelog || s_autoChangelogDismissedThisBoot)
    return false;
  const SettingsData saved = Settings::Get().Snapshot();
  return saved.lastSeenVersion != NobleChangelog::CurrentChangelogVersionKey();
}

void DrawChangelogModal(const Ui &ui, bool &ioManualOpen) {
  static bool s_wasOpen = false;
  const bool opening = !s_wasOpen;
  s_wasOpen = true;

  // Full-screen dim scrim under the changelog dialog.
  ui.dl->AddRectFilled(ui.origin,
                       ImVec2(ui.origin.x + ui.size.x, ui.origin.y + ui.size.y),
                       IM_COL32(2, 6, 12, 192));

  const float s = ui.s;
  const std::string currentVer = NobleChangelog::CurrentChangelogVersionKey();
  static const std::vector<NobleChangelog::ReleaseNotes> kReleases =
      NobleChangelog::BuildReleaseNotes();

  const ImVec2 winSize(std::min(ui.size.x * 0.84f, 900.0f * s),
                       std::min(ui.size.y * 0.84f, 650.0f * s));
  const ImVec2 winPos(ui.origin.x + (ui.size.x - winSize.x) * 0.5f,
                      ui.origin.y + (ui.size.y - winSize.y) * 0.5f);
  ImGui::SetNextWindowPos(winPos);
  ImGui::SetNextWindowSize(winSize);
  ImGui::SetNextWindowFocus();
  ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(9, 15, 24, 248));
  ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(95, 175, 232, 165));
  ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, IM_COL32(8, 13, 20, 170));
  ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, IM_COL32(68, 125, 178, 190));
  ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered,
                        IM_COL32(115, 188, 245, 235));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(26.0f * s, 22.0f * s));

  ImGui::Begin("##noble_changelog", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus);

  ImDrawList *dl = ImGui::GetWindowDrawList();
  const ImVec2 w0 = ImGui::GetWindowPos();
  const ImVec2 w1(w0.x + ImGui::GetWindowSize().x,
                  w0.y + ImGui::GetWindowSize().y);
  // Crisp Reach top accent bar & corner brackets.
  dl->AddRectFilled(w0, ImVec2(w1.x, w0.y + 4.0f * s), kAccent);
  NobleUi::DrawTacticalFrame(dl, w0, w1, s, false, true, false, false);

  ImFont *titleFont = Font(NobleStyle::TitleFont());
  ImFont *smallFont = Font(NobleStyle::SmallFont());
  ImFont *bodyFont = Font(nullptr);

  // Header block
  {
    const ImVec2 hPos = ImGui::GetCursorScreenPos();
    const float availW = ImGui::GetContentRegionAvail().x;
    dl->AddText(smallFont, 12.5f * s, hPos, IM_COL32(138, 212, 255, 235),
                "UPDATE BRIEFING  //  PROJECT NOBLE");

    // Version badge pill top-right
    const std::string verUpper = Upper(currentVer);
    const float verW =
        smallFont->CalcTextSizeA(13.5f * s, FLT_MAX, 0.0f, verUpper.c_str()).x;
    const float pillW = verW + 24.0f * s;
    const float pillH = 26.0f * s;
    const ImVec2 vp0(hPos.x + availW - pillW, hPos.y);
    const ImVec2 vp1(vp0.x + pillW, vp0.y + pillH);
    dl->AddRectFilled(vp0, vp1, IM_COL32(18, 44, 68, 230), 2.0f * s);
    dl->AddRect(vp0, vp1, IM_COL32(120, 200, 255, 190), 2.0f * s, 0, 1.0f);
    dl->AddText(smallFont, 13.5f * s,
                ImVec2(vp0.x + 12.0f * s, vp0.y + 5.5f * s), kWhite,
                verUpper.c_str());

    dl->AddText(titleFont, 30.0f * s, ImVec2(hPos.x, hPos.y + 18.0f * s),
                kWhite, "WHAT'S NEW");
    dl->AddText(
        bodyFont, 15.0f * s, ImVec2(hPos.x, hPos.y + 54.0f * s), kDim,
        "Highlights from your newest update and recent tactical overhauls.");

    const float ruleY = hPos.y + 82.0f * s;
    dl->AddLine(ImVec2(hPos.x, ruleY), ImVec2(hPos.x + availW, ruleY),
                IM_COL32(120, 175, 225, 95), 1.0f);
    ImGui::Dummy(ImVec2(availW, 92.0f * s));
  }

  // Scrollable release-note cards
  const float footerH = 54.0f * s;
  const float scrollH =
      std::max(140.0f * s, ImGui::GetContentRegionAvail().y - footerH);
  if (opening) {
    ImGui::SetNextWindowScroll(ImVec2(0.0f, 0.0f));
  }
  ImGui::BeginChild("##changelog_list", ImVec2(0.0f, scrollH),
                    ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);
  UiPad::ScrollWindow(true, true);

  const float cardW =
      std::max(260.0f * s, ImGui::GetContentRegionAvail().x - 10.0f * s);
  const float innerPadX = 18.0f * s;
  const float innerPadY = 14.0f * s;
  const float badgeW = 92.0f * s;
  const float detailOffset = badgeW + 12.0f * s;
  const float detailW =
      std::max(140.0f * s, cardW - innerPadX * 2.0f - detailOffset);

  for (size_t ri = 0; ri < kReleases.size(); ++ri) {
    const NobleChangelog::ReleaseNotes &rel = kReleases[ri];
    const bool isLatest = (ri == 0);

    // Pre-measure card height deterministically so frames never clip or
    // overlap.
    float itemsH = 0.0f;
    for (size_t ii = 0; ii < rel.items.size(); ++ii) {
      const auto &item = rel.items[ii];
      const float headH =
          titleFont->CalcTextSizeA(16.5f * s, FLT_MAX, detailW, item.headline)
              .y;
      const float bodyH =
          bodyFont->CalcTextSizeA(14.8f * s, FLT_MAX, detailW, item.detail).y;
      itemsH += std::max(22.0f * s, headH) + bodyH + 4.0f * s;
      if (ii + 1 < rel.items.size())
        itemsH += 10.0f * s;
    }
    const float headerBandH = 48.0f * s;
    const float cardH = innerPadY * 2.0f + headerBandH + itemsH + 4.0f * s;

    const ImVec2 c0 = ImGui::GetCursorScreenPos();
    const ImVec2 c1(c0.x + cardW, c0.y + cardH);
    ImDrawList *cdl = ImGui::GetWindowDrawList();

    cdl->AddRectFilled(c0, c1,
                       isLatest ? IM_COL32(15, 27, 42, 232)
                                : IM_COL32(12, 20, 31, 205),
                       2.0f * s);
    NobleUi::DrawTacticalFrame(cdl, c0, c1, s, isLatest, isLatest, false,
                               false);
    if (isLatest) {
      cdl->AddRectFilled(c0, ImVec2(c0.x + 3.5f * s, c1.y), kAccent);
    }

    // Release card header
    const std::string verTag = Upper(rel.version);
    const float vtW =
        smallFont->CalcTextSizeA(12.5f * s, FLT_MAX, 0.0f, verTag.c_str()).x +
        16.0f * s;
    const ImVec2 tv0(c0.x + innerPadX, c0.y + innerPadY);
    const ImVec2 tv1(tv0.x + vtW, tv0.y + 22.0f * s);
    cdl->AddRectFilled(tv0, tv1,
                       isLatest ? IM_COL32(34, 92, 138, 240)
                                : IM_COL32(28, 44, 62, 210),
                       2.0f * s);
    cdl->AddRect(tv0, tv1,
                 isLatest ? IM_COL32(130, 208, 255, 220)
                          : IM_COL32(95, 135, 175, 140),
                 2.0f * s, 0, 1.0f);
    cdl->AddText(smallFont, 12.5f * s,
                 ImVec2(tv0.x + 8.0f * s, tv0.y + 4.0f * s), kWhite,
                 verTag.c_str());

    cdl->AddText(
        titleFont, 18.0f * s, ImVec2(tv1.x + 12.0f * s, tv0.y + 1.5f * s),
        isLatest ? kWhite : IM_COL32(215, 226, 238, 235), rel.title.c_str());

    if (!rel.subtitle.empty()) {
      cdl->AddText(smallFont, 12.5f * s,
                   ImVec2(c0.x + innerPadX, tv1.y + 5.0f * s),
                   IM_COL32(148, 175, 200, 210), rel.subtitle.c_str());
    }

    float curY = c0.y + innerPadY + headerBandH;
    cdl->AddLine(ImVec2(c0.x + innerPadX, curY - 6.0f * s),
                 ImVec2(c1.x - innerPadX, curY - 6.0f * s),
                 IM_COL32(110, 155, 195, 65), 1.0f);

    for (size_t ii = 0; ii < rel.items.size(); ++ii) {
      const auto &item = rel.items[ii];
      ImU32 catCol = IM_COL32(110, 198, 255, 255);
      ImU32 catBg = IM_COL32(20, 55, 82, 220);
      if (std::strcmp(item.category, "CONTROLLER") == 0) {
        catCol = IM_COL32(125, 232, 168, 255);
        catBg = IM_COL32(20, 62, 42, 220);
      } else if (std::strcmp(item.category, "NETCODE") == 0) {
        catCol = IM_COL32(255, 198, 98, 255);
        catBg = IM_COL32(70, 50, 18, 220);
      } else if (std::strcmp(item.category, "FEATURE") == 0) {
        catCol = IM_COL32(182, 164, 255, 255);
        catBg = IM_COL32(46, 36, 78, 220);
      } else if (std::strcmp(item.category, "FIX") == 0) {
        catCol = IM_COL32(255, 146, 125, 255);
        catBg = IM_COL32(72, 32, 28, 220);
      }

      const ImVec2 b0(c0.x + innerPadX, curY + 2.0f * s);
      const ImVec2 b1(b0.x + badgeW, b0.y + 20.0f * s);
      cdl->AddRectFilled(b0, b1, catBg, 2.0f * s);
      cdl->AddRect(b0, b1, catCol, 2.0f * s, 0, 1.0f);
      const float catTw =
          smallFont->CalcTextSizeA(11.0f * s, FLT_MAX, 0.0f, item.category).x;
      cdl->AddText(smallFont, 11.0f * s,
                   ImVec2(b0.x + std::max(4.0f * s, (badgeW - catTw) * 0.5f),
                          b0.y + 3.5f * s),
                   catCol, item.category);

      const float tx = c0.x + innerPadX + detailOffset;
      cdl->AddText(titleFont, 16.5f * s, ImVec2(tx, curY), kWhite,
                   item.headline, nullptr, detailW);
      const float headH = std::max(
          22.0f * s,
          titleFont->CalcTextSizeA(16.5f * s, FLT_MAX, detailW, item.headline)
              .y);
      const float dy = curY + headH + 2.0f * s;
      cdl->AddText(bodyFont, 14.8f * s, ImVec2(tx, dy),
                   IM_COL32(198, 212, 226, 232), item.detail, nullptr, detailW);
      const float bodyH =
          bodyFont->CalcTextSizeA(14.8f * s, FLT_MAX, detailW, item.detail).y;

      curY = dy + bodyH + (ii + 1 < rel.items.size() ? 10.0f * s : 0.0f);
    }

    ImGui::Dummy(ImVec2(cardW, cardH));
    ImGui::Dummy(ImVec2(0.0f, 12.0f * s));
  }

  ImGui::EndChild();

  // Footer: scroll / dismiss hints on left, focused CONTINUE button on right
  {
    const float availW = ImGui::GetContentRegionAvail().x;
    const ImVec2 fPos = ImGui::GetCursorScreenPos();
    dl->AddLine(ImVec2(fPos.x, fPos.y + 4.0f * s),
                ImVec2(fPos.x + availW, fPos.y + 4.0f * s),
                IM_COL32(120, 175, 225, 85), 1.0f);

    const float rowY = fPos.y + 18.0f * s;
    const bool pad = InputPrompts::Current() == InputPrompts::Device::Gamepad;
    float hx = fPos.x;
    hx += (pad ? InputPrompts::DrawButton(dl, smallFont, ImVec2(hx, rowY),
                                          20.0f * s,
                                          InputPrompts::Button::DpadDown)
               : InputPrompts::DrawKey(dl, smallFont, ImVec2(hx, rowY),
                                       20.0f * s, "UP/DN")) +
          7.0f * s;
    dl->AddText(smallFont, 14.0f * s, ImVec2(hx, rowY + 2.0f * s), kDim,
                "Scroll");
    hx += smallFont->CalcTextSizeA(14.0f * s, FLT_MAX, 0.0f, "Scroll").x +
          20.0f * s;
    hx += (pad ? InputPrompts::DrawButton(dl, smallFont, ImVec2(hx, rowY),
                                          20.0f * s, InputPrompts::Button::B)
               : InputPrompts::DrawKey(dl, smallFont, ImVec2(hx, rowY),
                                       20.0f * s, "ESC")) +
          7.0f * s;
    dl->AddText(smallFont, 14.0f * s, ImVec2(hx, rowY + 2.0f * s), kDim,
                "Close");

    const float btnW = 190.0f * s, btnH = 34.0f * s;
    const ImVec2 b0(fPos.x + availW - btnW, fPos.y + 12.0f * s);
    const ImVec2 b1(b0.x + btnW, b0.y + btnH);
    ImGui::SetCursorScreenPos(b0);
    const bool clickedContinue =
        ImGui::InvisibleButton("##changelog_continue", ImVec2(btnW, btnH));
    const bool hovered = ImGui::IsItemHovered();
    dl->AddRectFilled(b0, b1,
                      hovered ? IM_COL32(72, 148, 200, 255)
                              : IM_COL32(46, 108, 152, 240),
                      2.0f * s);
    NobleUi::DrawTacticalFrame(dl, b0, b1, s, true, false, hovered, true);

    const char *lbl = "CONTINUE";
    const float lw = titleFont->CalcTextSizeA(18.0f * s, FLT_MAX, 0.0f, lbl).x;
    const float gw =
        pad ? InputPrompts::ButtonWidth(smallFont, 18.0f * s,
                                        InputPrompts::Button::A)
            : InputPrompts::KeyWidth(smallFont, 18.0f * s, "ENTER");
    const float totalW = gw + 8.0f * s + lw;
    float cx = b0.x + (btnW - totalW) * 0.5f;
    if (pad)
      cx += InputPrompts::DrawButton(
                dl, smallFont, ImVec2(cx, b0.y + (btnH - 18.0f * s) * 0.5f),
                18.0f * s, InputPrompts::Button::A) +
            8.0f * s;
    else
      cx += InputPrompts::DrawKey(dl, smallFont,
                                  ImVec2(cx, b0.y + (btnH - 18.0f * s) * 0.5f),
                                  18.0f * s, "ENTER") +
            8.0f * s;
    dl->AddText(titleFont, 18.0f * s, ImVec2(cx, b0.y + 7.5f * s), kWhite, lbl);

    const bool acceptPressed = UiPad::AcceptPressed() ||
                               ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                               ImGui::IsKeyPressed(ImGuiKey_Space, false);
    const bool backPressed =
        UiPad::BackPressed() || ImGui::IsKeyPressed(ImGuiKey_Escape, false);
    if (clickedContinue || acceptPressed || backPressed) {
      UiPad::ConsumeAccept();
      UiPad::ConsumeBack();
      ImGui::GetIO().ClearInputKeys();
      const SettingsData snap = Settings::Get().Snapshot();
      if (snap.lastSeenVersion != currentVer) {
        SettingsData updated = snap;
        updated.lastSeenVersion = currentVer;
        Settings::Get().Update(updated);
      }
      s_forcedChangelogDismissed = true;
      s_autoChangelogDismissedThisBoot = true;
      ioManualOpen = false;
      s_wasOpen = false;
      if (backPressed && !clickedContinue && !acceptPressed)
        MenuSounds::Back();
      else
        MenuSounds::Click();
    }
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(5);
}

// Menu music player: a compact now-playing strip (track, prev / play-pause /
// next, mute, shuffle) in the bottom-right of the main menu. Every action takes
// effect immediately and is saved to settings.json, the same rule the Settings
// > Audio page uses for the music track.
void DrawMusicPlayer(const Ui &ui) {
  const SettingsData sd = Settings::Get().Snapshot();
  // MusicTracks() builds the whole list, and this runs every frame on the main
  // menu: build it once.
  static const std::vector<ReachAudio::TrackInfo> tracks =
      ReachAudio::MusicTracks();
  if (tracks.empty())
    return;

  const std::string id = sd.menuMusicTrack;
  int index = -1;
  for (size_t i = 0; i < tracks.size(); ++i) {
    if (tracks[i].id == id) {
      index = static_cast<int>(i);
      break;
    }
  }

  std::string label;
  if (id == "all")
    label = "Shuffle \xC2\xB7 every game";
  else if (id == "off")
    label = "Music off";
  else if (index >= 0)
    label = tracks[index].game + " \xC2\xB7 " + tracks[index].label;
  else
    label = ReachAudio::TrackLabel(id);
  if (ReachAudio::IsMusicPaused())
    label += "   (paused)";

  auto saveTrack = [&](const std::string &next) {
    SettingsData d = Settings::Get().Snapshot();
    d.menuMusicTrack = next;
    Settings::Get().Update(d);
    ReachAudio::PlayMenuMusic(next);
  };
  auto step = [&](int dir) {
    if (tracks.empty())
      return;
    int from = index >= 0 ? index : 0;
    from = (from + dir + static_cast<int>(tracks.size())) %
           static_cast<int>(tracks.size());
    saveTrack(tracks[from].id);
  };

  const float w = 470.0f * ui.s, h = 30.0f * ui.s;
  const ImVec2 p0(ui.origin.x + ui.size.x - w - 40.0f * ui.s,
                  ui.origin.y + ui.size.y - 140.0f * ui.s);
  const ImVec2 p1(p0.x + w, p0.y + h);
  ui.dl->AddRectFilled(p0, p1, IM_COL32(8, 12, 18, 190));
  ui.dl->AddRect(p0, p1, IM_COL32(90, 120, 150, 90), 0.0f, 0, 1.0f);

  // A small note glyph, then the track name (clipped if the file names are
  // long).
  const ImVec2 note(p0.x + 14.0f * ui.s, p0.y + h * 0.5f);
  ui.dl->AddCircleFilled(ImVec2(note.x - 2.0f * ui.s, note.y + 4.0f * ui.s),
                         3.5f * ui.s, kAccent);
  ui.dl->AddLine(ImVec2(note.x + 1.5f * ui.s, note.y + 4.0f * ui.s),
                 ImVec2(note.x + 1.5f * ui.s, note.y - 7.0f * ui.s), kAccent,
                 1.6f * ui.s);
  ui.dl->AddLine(ImVec2(note.x + 1.5f * ui.s, note.y - 7.0f * ui.s),
                 ImVec2(note.x + 9.0f * ui.s, note.y - 9.0f * ui.s), kAccent,
                 1.6f * ui.s);

  const float buttonsW = 186.0f * ui.s; // five 34 s buttons with 4 s gaps
  const float textRight = p1.x - buttonsW - 10.0f * ui.s;
  const std::string shown = Upper(label);
  ui.dl->PushClipRect(ImVec2(p0.x + 30.0f * ui.s, p0.y),
                      ImVec2(textRight, p1.y), true);
  Text(ui, NobleStyle::SmallFont(), 12.5f,
       ImVec2(p0.x + 30.0f * ui.s, p0.y + 8.0f * ui.s),
       ReachAudio::IsMusicPaused() ? kDim : IM_COL32(205, 220, 235, 235),
       shown.c_str());
  ui.dl->PopClipRect();

  const char *glyphs[5] = {"|<<", "||>", ">>|", "VOL",
                           "SHF"}; // prev, play/pause, next, mute, shuffle
  if (ReachAudio::IsMusicPaused())
    glyphs[1] = ">  ";
  if (g_menuMusicMuted)
    glyphs[3] = "MUT";
  float bx = textRight + 6.0f * ui.s;
  const float bw = 34.0f * ui.s;
  for (int i = 0; i < 5; ++i) {
    const ImVec2 a(bx + i * (bw + 4.0f * ui.s), p0.y + 4.0f * ui.s);
    const ImVec2 b(a.x + bw, p0.y + h - 4.0f * ui.s);
    ImGui::SetCursorScreenPos(a);
    ImGui::PushID(3000 + i);
    const bool pressed =
        ImGui::InvisibleButton("##music", ImVec2(bw, h - 8.0f * ui.s));
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    const bool active = (i == 1 && ReachAudio::IsMusicPaused()) ||
                        (i == 3 && g_menuMusicMuted) || (i == 4 && id == "all");
    ui.dl->AddRectFilled(a, b,
                         active ? IM_COL32(52, 112, 156, 230)
                                : (hovered ? IM_COL32(40, 56, 72, 230)
                                           : IM_COL32(18, 24, 32, 220)));
    ui.dl->AddRect(a, b,
                   hovered ? IM_COL32(170, 220, 255, 180)
                           : IM_COL32(80, 110, 140, 90),
                   0.0f, 0, 1.0f);
    const float gw =
        TextWidth(NobleStyle::SmallFont(), 11.0f * ui.s, glyphs[i]);
    Text(ui, NobleStyle::SmallFont(), 11.0f,
         ImVec2(a.x + (bw - gw) * 0.5f, a.y + 4.0f * ui.s), kWhite, glyphs[i]);
    if (!pressed)
      continue;
    MenuSounds::Click();
    switch (i) {
    case 0:
      step(-1);
      break;
    case 1:
      if (ReachAudio::IsMusicPaused())
        ReachAudio::ResumeMusic();
      else if (!ReachAudio::PauseMusic() || id == "off")
        saveTrack(index >= 0 ? tracks[index].id : "reach:menu");
      break;
    case 2:
      step(1);
      break;
    case 3:
      g_menuMusicMuted = !g_menuMusicMuted;
      ReachAudio::SetMuted(g_menuMusicMuted);
      break;
    case 4:
      saveTrack(id == "all" ? (index >= 0 ? tracks[index].id : "reach:menu")
                            : "all");
      break;
    default:
      break;
    }
  }
}

// A friend's game invite is shown on every menu screen as a small panel with
// JOIN / DISMISS, unless the player turned invites off in Settings > Online
// services. The invite carries the host endpoint, so JOIN is the same relaunch
// the server browser uses for a direct connect (2026-10-30).
void DrawInviteToast(const Ui &ui, MenuState &st) {
  if (st.confirmQuit || g_showAccountModal || g_showDevMenu ||
      ModDownloadPrompt::Get().IsActive())
    return;
  if (!Settings::Get().Snapshot().inviteNotifications)
    return;
  if (!AuthClient::Get().IsLoggedIn())
    return;

  const std::vector<GameInvite> invites = SocialClient::Get().GetInvites();
  if (invites.empty())
    return;
  const GameInvite *newest = &invites.front();
  for (const GameInvite &inv : invites) {
    if (inv.timestamp >= newest->timestamp)
      newest = &inv;
  }

  static std::string
      s_answered; // "sender|timestamp" already answered this session
  char key[160];
  std::snprintf(key, sizeof(key), "%s|%llu", newest->senderUsername.c_str(),
                static_cast<unsigned long long>(newest->timestamp));
  if (s_answered == key)
    return;

  const float w = 470.0f * ui.s, h = 96.0f * ui.s;
  const ImVec2 p0(ui.origin.x + ui.size.x - w - 40.0f * ui.s,
                  ui.origin.y + ui.size.y - h - 150.0f * ui.s);
  const ImVec2 p1(p0.x + w, p0.y + h);
  ui.dl->AddRectFilled(p0, p1, IM_COL32(12, 20, 30, 245));
  ui.dl->AddRect(p0, p1, IM_COL32(143, 216, 255, 170), 0.0f, 0, 1.2f);
  ui.dl->AddLine(ImVec2(p0.x, p0.y + 3.0f * ui.s),
                 ImVec2(p1.x, p0.y + 3.0f * ui.s), kAccent, 3.0f * ui.s);

  Text(ui, NobleStyle::SmallFont(), 12.0f,
       ImVec2(p0.x + 16.0f * ui.s, p0.y + 11.0f * ui.s), kAccent,
       "GAME INVITE");
  Text(ui, NobleStyle::TitleFont(), 19.0f,
       ImVec2(p0.x + 16.0f * ui.s, p0.y + 30.0f * ui.s), kWhite,
       newest->senderUsername.c_str());
  const float nameW = TextWidth(NobleStyle::TitleFont(), 19.0f * ui.s,
                                newest->senderUsername.c_str());
  const std::string inviteDesc =
      newest->mode.empty()
          ? "is inviting you to a match"
          : ("is inviting you to a " + newest->mode + " match");
  Text(ui, nullptr, 15.0f,
       ImVec2(p0.x + 16.0f * ui.s + nameW + 8.0f * ui.s, p0.y + 36.0f * ui.s),
       kDim, inviteDesc.c_str());

  const float bw = 160.0f * ui.s, bh = 30.0f * ui.s;
  const ImVec2 j0(p0.x + w - bw * 2.0f - 26.0f * ui.s,
                  p1.y - bh - 12.0f * ui.s);
  const ImVec2 d0(j0.x + bw + 10.0f * ui.s, j0.y);
  auto button = [&](const char *id, ImVec2 a, const char *label, bool primary) {
    ImGui::SetCursorScreenPos(a);
    ImGui::PushID(id);
    const bool pressed = ImGui::InvisibleButton("##b", ImVec2(bw, bh));
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    const ImVec2 b(a.x + bw, a.y + bh);
    ui.dl->AddRectFilled(a, b,
                         primary ? (hovered ? IM_COL32(76, 150, 200, 255)
                                            : IM_COL32(52, 112, 156, 240))
                                 : (hovered ? IM_COL32(60, 74, 88, 240)
                                            : IM_COL32(26, 32, 40, 230)));
    ui.dl->AddRect(a, b,
                   primary ? IM_COL32(170, 220, 255, hovered ? 220 : 90)
                           : IM_COL32(120, 140, 160, 90));
    const float tw = TextWidth(NobleStyle::TitleFont(), 16.0f * ui.s, label);
    Text(ui, NobleStyle::TitleFont(), 16.0f,
         ImVec2(a.x + (bw - tw) * 0.5f, a.y + 5.0f * ui.s), kWhite, label);
    if (pressed)
      MenuSounds::Click();
    return pressed;
  };

  if (button("invite_join", j0, newest->hostIp.empty() ? "WAITING..." : "JOIN",
             true)) {
    if (newest->hostIp.empty()) {
      // The server sent an invite without an endpoint (older server build): do
      // not relaunch with "-connect :"
      NOBLE_WARN(
          "FrontEndMenu: invite from {} has no host address; ignoring JOIN",
          newest->senderUsername);
    } else {
      s_answered = key;
      SocialClient::Get().ClearInviteAsync(newest->senderUsername);
      SocialClient::Get().RefreshFriendsAsync();
      NOBLE_INFO(
          "FrontEndMenu: accepting invite into lobby from {} ({}, mode='{}')",
          newest->senderUsername, newest->hostIp, newest->mode);
      ServerEntry s;
      s.id = newest->sessionId;
      s.name = newest->senderUsername + "'s Game";
      s.hostAddress = newest->hostIp;
      s.port = newest->hostPort;
      s.mapName = newest->map.empty() ? "20_sword_slayer" : newest->map;
      s.gameMode = newest->mode.empty() ? "multiplayer" : newest->mode;
      s.maxPlayers = 16;
      EnterLobby(GetOverlayCtx(), s);
      MenuSounds::Click();
    }
  }
  if (button("invite_dismiss", d0, "DISMISS", false)) {
    s_answered = key;
    SocialClient::Get().ClearInviteAsync(newest->senderUsername);
    SocialClient::Get().RefreshFriendsAsync();
  }
}


} // namespace noble::fe
