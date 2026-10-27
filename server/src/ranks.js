/**
 * Project Noble - Halo: Reach Career Ranks & Credit Calculations
 * Directly matches the 50 Bungie Reach ranks and Noble formulas from Career.cpp.
 */

const RANKS = [
    { name: "Recruit", credits: 0 },
    { name: "Private", credits: 7500 },
    { name: "Corporal", credits: 10000 },
    { name: "Corporal Grade 1", credits: 15000 },
    { name: "Sergeant", credits: 20000 },
    { name: "Sergeant Grade 1", credits: 26250 },
    { name: "Sergeant Grade 2", credits: 32500 },
    { name: "Warrant Officer", credits: 45000 },
    { name: "Warrant Officer Grade 1", credits: 78000 },
    { name: "Warrant Officer Grade 2", credits: 111000 },
    { name: "Warrant Officer Grade 3", credits: 144000 },
    { name: "Captain", credits: 210000 },
    { name: "Captain Grade 1", credits: 233000 },
    { name: "Captain Grade 2", credits: 256000 },
    { name: "Captain Grade 3", credits: 279000 },
    { name: "Major", credits: 325000 },
    { name: "Major Grade 1", credits: 350000 },
    { name: "Major Grade 2", credits: 370000 },
    { name: "Major Grade 3", credits: 400000 },
    { name: "Lt. Colonel", credits: 450000 },
    { name: "Lt. Colonel Grade 1", credits: 480000 },
    { name: "Lt. Colonel Grade 2", credits: 510000 },
    { name: "Lt. Colonel Grade 3", credits: 540000 },
    { name: "Commander", credits: 600000 },
    { name: "Commander Grade 1", credits: 650000 },
    { name: "Commander Grade 2", credits: 700000 },
    { name: "Commander Grade 3", credits: 750000 },
    { name: "Colonel", credits: 850000 },
    { name: "Colonel Grade 1", credits: 960000 },
    { name: "Colonel Grade 2", credits: 1070000 },
    { name: "Colonel Grade 3", credits: 1180000 },
    { name: "Brigadier", credits: 1400000 },
    { name: "Brigadier Grade 1", credits: 1520000 },
    { name: "Brigadier Grade 2", credits: 1640000 },
    { name: "Brigadier Grade 3", credits: 1760000 },
    { name: "General", credits: 2000000 },
    { name: "General Grade 1", credits: 2200000 },
    { name: "General Grade 2", credits: 2350000 },
    { name: "General Grade 3", credits: 2500000 },
    { name: "General Grade 4", credits: 2650000 },
    { name: "Field Marshall", credits: 3000000 },
    { name: "Hero", credits: 3700000 },
    { name: "Legend", credits: 4600000 },
    { name: "Mythic", credits: 5650000 },
    { name: "Noble", credits: 7000000 },
    { name: "Eclipse", credits: 8500000 },
    { name: "Nova", credits: 11000000 },
    { name: "Forerunner", credits: 13000000 },
    { name: "Reclaimer", credits: 16500000 },
    { name: "Inheritor", credits: 20000000 },
];

/**
 * Find the rank for a given credit total.
 * @param {number} totalCredits
 * @returns {{ index: number, name: string, creditsNeeded: number, nextRankName: string|null, nextRankCredits: number|null, progressPercent: number }}
 */
function getRank(totalCredits) {
    const credits = Math.max(0, Number(totalCredits) || 0);
    let index = 0;
    for (let i = RANKS.length - 1; i >= 0; i--) {
        if (credits >= RANKS[i].credits) {
            index = i;
            break;
        }
    }

    const currentRank = RANKS[index];
    const nextRank = index + 1 < RANKS.length ? RANKS[index + 1] : null;

    let progressPercent = 100;
    if (nextRank) {
        const span = nextRank.credits - currentRank.credits;
        const gained = credits - currentRank.credits;
        progressPercent = Math.min(100, Math.max(0, Math.floor((gained / span) * 100)));
    }

    return {
        index,
        name: currentRank.name,
        creditsNeeded: currentRank.credits,
        nextRankName: nextRank ? nextRank.name : null,
        nextRankCredits: nextRank ? nextRank.credits : null,
        progressPercent,
    };
}

/**
 * Calculate match credit award breakdown based on Noble Career parameters:
 * - Pay per minute (scales with rank)
 * - Kills, assists, medals
 * - Performance bonus & random jackpot chance
 */
function calculateCredits({
    minutesPlayed = 0,
    kills = 0,
    assists = 0,
    medals = 0,
    currentCredits = 0,
    isWinner = false,
}) {
    const rank = getRank(currentCredits);
    const clampedMinutes = Math.min(90.0, Math.max(0, Number(minutesPlayed) || 0));

    // Base formula from Career.cpp:
    // kPayPerMinuteBase (60.0) + rankIndex * kPayPerMinutePerRank (6.0)
    const payRate = 60.0 + (rank.index * 6.0);
    const timeCredits = Math.floor(clampedMinutes * payRate);

    const killCredits = (Number(kills) || 0) * 10;
    const assistCredits = (Number(assists) || 0) * 5;
    const medalCredits = (Number(medals) || 0) * 25;
    const winBonus = isWinner ? 250 : 0;

    // Random roll for bonus
    let randomBonus = 0;
    const roll = Math.floor(Math.random() * 8); // 1 in 8 chance
    if (roll === 0) {
        const jackpotRoll = Math.floor(Math.random() * 200); // 1 in 200 chance
        if (jackpotRoll === 77) {
            randomBonus = 7777; // Lucky 7s Jackpot
        } else {
            randomBonus = 100 + Math.floor(Math.random() * 900);
        }
    }

    const total = timeCredits + killCredits + assistCredits + medalCredits + winBonus + randomBonus;

    return {
        total,
        breakdown: {
            time: timeCredits,
            kills: killCredits,
            assists: assistCredits,
            medals: medalCredits,
            winBonus,
            randomBonus,
        },
        creditsBefore: currentCredits,
        creditsAfter: currentCredits + total,
        rankBefore: rank.index,
        rankAfter: getRank(currentCredits + total).index,
    };
}

module.exports = {
    RANKS,
    getRank,
    calculateCredits,
};
