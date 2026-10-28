#pragma once

namespace noble {

// SERVICE RECORD screen (front-end menu): the player's career from career/Career.cpp in three tabs,
// SERVICE RECORD (rank, totals, campaign progress, recent games), CHALLENGES (daily and weekly) and
// COMMENDATIONS (lifetime tracks with tiers) and RANKS (all 50 ranks with their icons). Read-only; everything is earned by finishing matches.
class CareerPanel {
public:
    void Render();
    void SetTab(int tab) { m_tab = tab % 4; }   // test option -menu-tab

private:
    void RenderRecord();
    void RenderChallenges();
    void RenderCommendations();
    void RenderRanks();

    int m_tab = 0;
};

} // namespace noble
