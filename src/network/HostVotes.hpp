#pragma once

// In-match votes (multiplayer and Firefight): vote kick, vote end round, vote
// end game. One vote at a time, counted on the host.
//
// Messages (NBCJ control JSON, routed by NetworkTransport::HandleControlJson):
//   vote_request {kind, target, name, from}            player -> host
//   vote_start   {id, kind, target, name, starter, voters, need, seconds}
//                                                       host -> everyone
//   vote_cast    {id, yes, from}                        player -> host
//   vote_state   {id, yes, no}                          host -> everyone
//   vote_end     {id, passed, vetoed, yes, no}          host -> everyone
//
// Who votes: vote kick = the target's team without the target (free for all:
// everyone but the target); end round / end game = everyone. Passing needs
// PassPercent() of the voters to say yes. The host may veto when HostVeto() is
// on. After a vote ends no new one can start for CooldownSeconds().

#include <cstdint>
#include <string>
#include <vector>

namespace noble::HostVotes {

enum class Kind { Kick = 0, EndRound = 1, EndGame = 2 };
const char *KindText(Kind kind); // "KICK", "END ROUND", "END GAME"

struct Voter {
  uint64_t machineId = 0; // match machine id (the host's own included)
  std::string name;
  int team = 0; // 0 = no teams
};

struct View {
  bool active = false;
  Kind kind = Kind::Kick;
  std::string targetName;
  std::string starterName;
  int yes = 0, no = 0, need = 1, voters = 0;
  int secondsLeft = 0;
  bool canVote = false; // this player is a voter
  int myChoice = -1;    // -1 not voted, 0 no, 1 yes
  // The last result, shown for a few seconds after a vote ends.
  bool showResult = false;
  bool passed = false;
  bool vetoed = false;
};

// Host settings (saved in data\userdata\host_votes.json).
inline constexpr int kPassChoices[] = {50, 60, 66, 75};
inline constexpr int kCooldownChoices[] = {30, 60, 120, 300};
int PassPercent();
void SetPassPercent(int percent);
bool HostVeto();
void SetHostVeto(bool on);
int CooldownSeconds();
void SetCooldownSeconds(int seconds);
bool VotesEnabled();
void SetVotesEnabled(bool on);

// Any player: ask for a vote (the host starts it straight away). Returns a
// message for the player when the vote can't start ("" when it was sent).
std::string Request(Kind kind, uint64_t targetMachine,
                    const std::string &targetName);
void Cast(bool yes);
void Veto(); // host only, when HostVeto() is on

// Host: every frame in the match, with everyone in it (host included).
void HostTick(const std::vector<Voter> &players);
// A passed vote the match code must carry out. Host only.
bool TakeResult(Kind &kind, uint64_t &targetMachine, std::string &targetName);

View Current();

// NetworkTransport calls this for every vote_* message.
void OnMessage(const std::string &type, const std::string &json,
               uint64_t senderMachine, bool isHost);

} // namespace noble::HostVotes
