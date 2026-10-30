#include "StatsManager.hpp"
#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace dtp {
    StatsManager& StatsManager::get() {
        static StatsManager instance;
        return instance;
    }

    std::string StatsManager::keyFor(GJGameLevel* level) {
        int id = level->m_levelID.value();
        if (id > 0) return fmt::format("id:{}", id);
        return fmt::format("local:{}", std::string(level->m_levelName));
    }

    std::filesystem::path StatsManager::filePath() const {
        return Mod::get()->getSaveDir() / "levels.json";
    }

    void StatsManager::ensureLoaded() {
        if (m_loaded) return;
        m_loaded = true;
        auto res = utils::file::readJson(this->filePath());
        if (!res) return; // first run or unreadable file: start empty
        auto const& root = res.unwrap();
        if (auto arr = root["levels"].asArray()) {
            for (auto const& entry : arr.unwrap()) {
                auto stats = LevelStats::fromJson(entry);
                if (!stats.key.empty()) m_levels[stats.key] = std::move(stats);
            }
        }
    }

    void StatsManager::save() {
        if (!m_loaded) return;
        std::vector<matjson::Value> levels;
        for (auto const& [key, stats] : m_levels) levels.push_back(stats.toJson());
        auto root = matjson::makeObject({{"version", 1}, {"levels", levels}});
        auto res = utils::file::writeStringSafe(this->filePath(), root.dump(matjson::NO_INDENTATION));
        if (!res) log::warn("Could not save death stats: {}", res.unwrapErr());
        m_unsavedDeaths = 0;
    }

    LevelStats const* StatsManager::find(std::string const& key) {
        this->ensureLoaded();
        auto it = m_levels.find(key);
        return it == m_levels.end() ? nullptr : &it->second;
    }

    LevelStats const* StatsManager::find(GJGameLevel* level) {
        return level ? this->find(keyFor(level)) : nullptr;
    }

    LevelStats& StatsManager::statsFor(GJGameLevel* level) {
        this->ensureLoaded();
        auto key = keyFor(level);
        auto& s = m_levels[key];
        if (s.key.empty()) {
            s.key = key;
            s.levelID = level->m_levelID.value();
            s.firstPlayed = std::time(nullptr);
        }
        s.name = std::string(level->m_levelName); // keep the name fresh
        return s;
    }

    void StatsManager::beginSession(GJGameLevel* level) {
        if (m_session.active) this->endSession();
        auto& s = this->statsFor(level);
        m_key = s.key;
        m_session = Session{};
        m_session.active = true;
        m_session.start = std::time(nullptr);
        m_session.startBest = s.bestPercent;
        m_session.bestPercent = 0;
    }

    void StatsManager::recordAttempt() {
        if (!m_session.active) return;
        auto& s = m_levels[m_key];
        ++s.attempts;
        ++m_session.attempts;
        s.lastPlayed = std::time(nullptr);
    }

    DeathResult StatsManager::recordDeath(int percent, bool practice) {
        DeathResult result;
        if (!m_session.active) return result;
        percent = std::clamp(percent, 0, 99);
        auto& s = m_levels[m_key];
        ++s.deaths;
        ++m_session.deaths;
        ++s.deathsAt[percent];
        s.history.push_back(static_cast<uint8_t>(percent));
        if (s.history.size() > kHistoryCap) s.history.erase(s.history.begin());
        if (!practice) {
            if (percent > s.bestPercent) { s.bestPercent = percent; result.newBest = true; }
            m_session.bestPercent = std::max(m_session.bestPercent, percent);
        }
        if (++m_unsavedDeaths >= 25) this->save();
        return result;
    }

    void StatsManager::recordCompletion(bool practice) {
        if (!m_session.active) return;
        auto& s = m_levels[m_key];
        ++s.completions;
        s.history.push_back(100);
        if (s.history.size() > kHistoryCap) s.history.erase(s.history.begin());
        if (!practice) {
            s.bestPercent = 100;
            m_session.bestPercent = 100;
        }
        this->save();
    }

    void StatsManager::endSession() {
        if (!m_session.active) return;
        auto& s = m_levels[m_key];
        int64_t now = std::time(nullptr);
        int duration = static_cast<int>(std::max<int64_t>(0, now - m_session.start));
        s.totalSeconds += duration;
        if (m_session.attempts > 0) {
            SessionRecord rec;
            rec.start = m_session.start;
            rec.durationSec = duration;
            rec.attempts = m_session.attempts;
            rec.deaths = m_session.deaths;
            rec.bestPercent = m_session.bestPercent;
            s.sessions.push_back(rec);
            if (s.sessions.size() > kSessionCap) s.sessions.erase(s.sessions.begin());
        }
        m_session.active = false;
        this->save();
    }

    void StatsManager::reset(std::string const& key) {
        this->ensureLoaded();
        std::string name;
        int id = 0;
        if (auto it = m_levels.find(key); it != m_levels.end()) { name = it->second.name; id = it->second.levelID; }
        m_levels.erase(key);
        if (m_session.active && m_key == key) {
            // keep tracking the level that is being played, just from zero
            auto& s = m_levels[key];
            s.key = key; s.name = name; s.levelID = id;
            s.firstPlayed = std::time(nullptr);
            m_session.attempts = 0; m_session.deaths = 0;
            m_session.bestPercent = 0; m_session.startBest = 0;
            m_session.start = std::time(nullptr);
        }
        this->save();
    }
}

$on_mod(DataSaved) {
    dtp::StatsManager::get().save();
}
