#!/usr/bin/env python3
"""Builds src/career/ReachChallenges.inc from docs/career/challenges.json.

Halo: Reach's real daily and weekly challenges (Sep 2010 - Feb 2012) are kept when Noble can track
them from what it knows about a finished game: kills, assists, deaths, score, minutes, mode, map,
difficulty and skulls, and the challenges the player completed. Matchmaking / Xbox Live wording is
dropped ("in Firefight Matchmaking" -> "in Firefight"). Challenges that need stats Noble can't read
yet (headshots, weapons, medals, enemy types, waves, wins, co-op, ...) are left out; this script
prints how many were kept.

Usage:  python tools/career/gen_reach_challenges.py   (from the repo root)
"""
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = os.path.join(ROOT, 'docs', 'career', 'challenges.json')
OUT = os.path.join(ROOT, 'src', 'career', 'ReachChallenges.inc')

DIFF = {'easy': 0, 'normal': 1, 'heroic': 2, 'legendary': 3}
SKULL_BITS = {  # MCC skull numbering (launch block +0x48), as in FrontEndMenu.cpp kSkulls
    'iron': 18, 'black eye': 3, 'tough luck': 36, 'touch luck': 36, 'catch': 7, 'cloud': 13,
    'famine': 11, 'thunderstorm': 34, 'tilt': 35, 'mythic': 23,
}
ALL_SKULLS = ['iron', 'black eye', 'tough luck', 'catch', 'cloud', 'famine', 'thunderstorm', 'tilt', 'mythic']
MISSIONS = {  # names used in the challenge texts -> scenario
    'winter contingency': 'm10', 'winter': 'm10', 'oni': 'm20', 'oni: sword base': 'm20', 'sword base': 'm20',
    'nightfall': 'm30', 'tip of the spear': 'm35', 'long night of solace': 'm45', 'the long night of solace': 'm45',
    'exodus': 'm50', 'new alexandria': 'm52', 'alexandria': 'm52', 'the package': 'm60',
    'the pillar of autumn': 'm70', 'pillar of autumn': 'm70',
}
MODES = {'campaign': 0, 'firefight': 1, 'multiplayer': 2, 'any': 3}


def number(s):
    return int(s.replace(',', ''))


def reword(text):
    t = text.replace(' Matchmaking game', ' game').replace(' matchmaking game', ' game')
    t = t.replace(' in multiplayer Matchmaking', ' in multiplayer').replace(' in Multiplayer Matchmaking', ' in multiplayer')
    t = t.replace(' in Firefight Matchmaking', ' in Firefight').replace(' in Halo: Reach Matchmaking', '')
    t = t.replace(' in any game mode in Reach', ' in any game mode').replace('Touch Luck', 'Tough Luck')
    t = t.replace(' in the Reach Campaign', ' in the Campaign')
    return t


def skull_mask(names):
    mask = 0
    for n in names:
        n = n.strip().lower().replace(' skull', '')
        if n not in SKULL_BITS:
            return None
        mask |= 1 << SKULL_BITS[n]
    return mask


def split_list(s):
    s = s.replace(', and ', ', ').replace(' and ', ', ')
    return [p for p in (x.strip() for x in s.split(',')) if p]


def parse(c):
    """Returns a dict of the challenge's tracking fields, or None when Noble can't track it."""
    r = c['requirement'].strip().rstrip('.')
    f = dict(metric=None, target=1, per_game=False, min_diff=-1, exact=False, skulls=0, mission='', no_deaths=False,
             max_minutes=0, mode=c['mode'])
    low = r.lower()
    if any(w in low for w in ('co-op', 'coop', 'bungie.net', 'file share', 'clip', 'gamertag', 'win ', 'waves',
                              'rounds', 'round in', 'sets on')):
        return None

    def diff_from(text):
        m = re.search(r'on (easy|normal|heroic|legendary)( or harder)?', text)
        if m:
            f['min_diff'] = DIFF[m.group(1)]
            f['exact'] = m.group(2) is None
        if re.search(r'\blegendary\b', text) and f['min_diff'] < 0:
            f['min_diff'] = 3

    # Kills / assists / games, totals over the period.
    m = re.match(r'^(?:Kill|Earn) ([\d,]+) (enemies|kills|assists)(?: today)? in (any game mode in Reach|Firefight Matchmaking|multiplayer Matchmaking|Multiplayer Matchmaking|the Campaign)(?: this week| today)?(?: on (Easy|Normal|Heroic|Legendary)(?: or harder)?)?$', r)
    if m:
        f['metric'] = 'Assists' if m.group(2) == 'assists' else 'Kills'
        f['target'] = number(m.group(1))
        diff_from(low)
        return f
    m = re.match(r'^Complete ([\d,]+) games in (multiplayer|Firefight) Matchmaking(?: this week)?$', r)
    if m:
        f['metric'] = 'Games'; f['target'] = number(m.group(1))
        return f
    # Best single game.
    m = re.match(r'^(?:Earn at least|Finish a multiplayer matchmaking game with at least) ([\d,]+) kills(?: in a multiplayer Matchmaking game)?$', r)
    if m:
        f['metric'] = 'Kills'; f['target'] = number(m.group(1)); f['per_game'] = True; f['mode'] = 'multiplayer'
        return f
    m = re.match(r'^Earn ([\d,]+) assists in a (multiplayer|Firefight) Matchmaking game$', r)
    if m:
        f['metric'] = 'Assists'; f['target'] = number(m.group(1)); f['per_game'] = True
        return f
    m = re.match(r'^Score ([\d,]+) points in a Firefight Matchmaking game$', r)
    if m:
        f['metric'] = 'Score'; f['target'] = number(m.group(1)); f['per_game'] = True
        return f
    m = re.match(r'^Complete a Firefight Matchmaking game without dying$', r)
    if m:
        f['metric'] = 'Games'; f['no_deaths'] = True
        return f
    # Daily challenges themselves.
    m = re.match(r'^Complete ([\d,]+) Daily Challenges this week$', r)
    if m:
        f['metric'] = 'DailyChallenges'; f['target'] = number(m.group(1))
        return f
    if r == 'Complete all of the Daily Challenges for a given day':
        f['metric'] = 'AllDailies'
        return f
    # Campaign missions: count on a difficulty.
    m = re.match(r'^Complete ([\d,]+) Campaign missions on (Easy|Normal|Heroic|Legendary)( or harder)?$', r)
    if m:
        f['metric'] = 'Missions'; f['target'] = number(m.group(1))
        f['min_diff'] = DIFF[m.group(2).lower()]; f['exact'] = m.group(3) is None
        return f
    # LASO: "Complete <mission>, Legendary, All Skulls On (LASO)".
    m = re.match(r'^Complete (.+?), Legendary, All Skulls On \(LASO\)$', r)
    if m:
        mission = MISSIONS.get(m.group(1).lower())
        if not mission:
            return None
        f.update(metric='Missions', mission=mission, min_diff=3, skulls=skull_mask(ALL_SKULLS))
        return f
    # Single campaign completions with conditions.
    m = re.match(r'^(?:Complete|Clear) (.+)$', r)
    if m:
        body = m.group(1).lower()
        mission = None
        for name in sorted(['any campaign mission', 'a campaign mission'] + list(MISSIONS), key=len, reverse=True):
            if body == name or body.startswith(name + ' ') or body.startswith(name + ','):
                mission = '' if 'campaign mission' in name else MISSIONS[name]
                rest = body[len(name):]
                break
        if mission is None:
            return None
        f.update(metric='Missions', mission=mission)
        diff_from(rest)
        if 'without dying' in rest:
            f['no_deaths'] = True
        tm = re.search(r'in ([\d,]+) minutes', rest)
        if tm:
            f['max_minutes'] = number(tm.group(1))
        sk = re.search(r'with (?:the )?(.+?) (?:skull )?on$', rest)
        if sk:
            mask = skull_mask(split_list(sk.group(1)))
            if mask is None:
                return None
            f['skulls'] = mask
        leftovers = re.sub(r'(on (easy|normal|heroic|legendary)( or harder)?|without dying|in [\d,]+ minutes( or less)?|with (the )?.+? (skull )?on$|alone|[ ,])', '', rest)
        if leftovers:
            return None
        f['mode'] = 'campaign'
        return f
    return None


def cstr(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'


def main():
    data = json.load(open(SRC, encoding='utf-8'))
    seen = set()
    rows = []
    for c in data:
        f = parse(c)
        if not f:
            continue
        text = reword(c['requirement'].strip())
        key = (c['kind'], c['name'], text, c['credits'])
        if key in seen:
            continue
        seen.add(key)
        rows.append((c, f, text))
    out = ['// Generated by tools/career/gen_reach_challenges.py from docs/career/challenges.json - do not edit.',
           '// Halo: Reach\'s own daily / weekly challenges (2010-2012) that Noble can track; Matchmaking wording dropped.',
           '// { weekly, mode (0 campaign, 1 Firefight, 2 multiplayer, 3 any), name, requirement, cR, metric, target,',
           '//   best single game, min difficulty (-1 any), exact difficulty, skulls (MCC bits, all needed), mission,',
           '//   without dying, max minutes (0 = no limit) }']
    for c, f, text in rows:
        out.append('    { %s, %d, %s, %s, %d, ChallengeMetric::%s, %d, %s, %d, %s, 0x%Xull, %s, %s, %d },' % (
            'true' if c['kind'] == 'weekly' else 'false', MODES[f['mode']], cstr(c['name']), cstr(text), c['credits'],
            f['metric'], f['target'], 'true' if f['per_game'] else 'false', f['min_diff'],
            'true' if f['exact'] else 'false', f['skulls'], cstr(f['mission']), 'true' if f['no_deaths'] else 'false',
            f['max_minutes']))
    with open(OUT, 'w', encoding='utf-8', newline='\r\n') as fh:
        fh.write('\n'.join(out) + '\n')
    daily = sum(1 for c, _, _ in rows if c['kind'] == 'daily')
    print(f'{len(rows)} challenges kept ({daily} daily, {len(rows) - daily} weekly) of {len(data)} -> {OUT}')


if __name__ == '__main__':
    sys.exit(main())
