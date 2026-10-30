#pragma once
#include <Geode/Geode.hpp>
#include <array>
#include <string>
#include <vector>

namespace dtp {
    constexpr int kBuckets = 100;        // death map resolution: one slot per percent (0-99)
    constexpr size_t kHistoryCap = 300;  // attempts kept for the progress graph
    constexpr size_t kSessionCap = 60;   // sessions kept per level

    // One play session of a level (from entering it until leaving it).
    struct SessionRecord {
        int64_t start = 0;
        int durationSec = 0;
        int attempts = 0;
        int deaths = 0;
        int bestPercent = 0;
    };

    // Everything persisted for a single level.
    struct LevelStats {
        std::string key;   // stable identifier, see StatsManager::keyFor
        std::string name;
        int levelID = 0;

        int attempts = 0;
        int deaths = 0;
        int completions = 0;
        int bestPercent = 0;        // normal mode only
        int64_t firstPlayed = 0;
        int64_t lastPlayed = 0;
        int totalSeconds = 0;

        std::array<int, kBuckets> deathsAt{};   // deaths per percent (index = percent)
        std::vector<uint8_t> history;           // end percent of each recent attempt (100 = completed)
        std::vector<SessionRecord> sessions;    // oldest first

        // --- derived values (pure functions of the data above) ---
        int deathsInRange(int fromPercent, int toPercentExclusive) const;
        // Average percent at which the last `lastN` deaths happened (0 = all). Returns -1 if none.
        double averageDeathPercent(size_t lastN = 0) const;
        // Most deadly window of `width` percent points; returns the start and fills `count`.
        int deadliestWindow(int width, int& count) const;
        // Average of the latest 10 attempts minus the 10 before them. Returns false if < 20 attempts.
        bool improvement(double& delta) const;

        matjson::Value toJson() const;
        static LevelStats fromJson(matjson::Value const& json);
    };
}
