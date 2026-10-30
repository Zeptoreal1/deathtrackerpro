#pragma once
#include "LevelStats.hpp"
#include <map>

namespace dtp {
    // Live numbers for the level currently being played.
    struct Session {
        bool active = false;
        int64_t start = 0;
        int attempts = 0;
        int deaths = 0;
        int bestPercent = 0;
        int startBest = 0;   // best % when the session began (to show improvement)
    };

    struct DeathResult {
        bool newBest = false;
    };

    // Single source of truth for all tracked data. Hooks feed it, UI reads from it.
    class StatsManager {
    public:
        static StatsManager& get();

        static std::string keyFor(GJGameLevel* level);

        // lookup
        LevelStats const* find(std::string const& key);
        LevelStats const* find(GJGameLevel* level);
        LevelStats& statsFor(GJGameLevel* level);
        Session const& session() const { return m_session; }
        std::string const& sessionKey() const { return m_key; }

        // recording (called from hooks)
        void beginSession(GJGameLevel* level);
        void recordAttempt();
        DeathResult recordDeath(int percent, bool practice);
        void recordCompletion(bool practice);
        void endSession();

        // maintenance
        void reset(std::string const& key);
        void save();

    private:
        StatsManager() = default;
        void ensureLoaded();
        std::filesystem::path filePath() const;

        std::map<std::string, LevelStats> m_levels;
        bool m_loaded = false;
        Session m_session;
        std::string m_key;
        int m_unsavedDeaths = 0;
    };
}
