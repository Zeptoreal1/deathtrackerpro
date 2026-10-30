#include "LevelStats.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace dtp {
    int LevelStats::deathsInRange(int from, int to) const {
        int total = 0;
        for (int i = std::max(0, from); i < std::min(kBuckets, to); ++i) total += deathsAt[i];
        return total;
    }

    double LevelStats::averageDeathPercent(size_t lastN) const {
        std::vector<int> d;
        for (auto v : history) if (v < 100) d.push_back(v);
        if (d.empty()) return -1.0;
        size_t begin = (lastN > 0 && d.size() > lastN) ? d.size() - lastN : 0;
        double sum = 0;
        for (size_t i = begin; i < d.size(); ++i) sum += d[i];
        return sum / static_cast<double>(d.size() - begin);
    }

    int LevelStats::deadliestWindow(int width, int& count) const {
        width = std::clamp(width, 1, kBuckets);
        int bestStart = 0;
        count = 0;
        for (int s = 0; s + width <= kBuckets; ++s) {
            int c = deathsInRange(s, s + width);
            if (c > count) { count = c; bestStart = s; }
        }
        return bestStart;
    }

    bool LevelStats::improvement(double& delta) const {
        if (history.size() < 20) return false;
        auto avg = [&](size_t from, size_t to) {
            double s = 0;
            for (size_t i = from; i < to; ++i) s += history[i];
            return s / static_cast<double>(to - from);
        };
        size_t n = history.size();
        delta = avg(n - 10, n) - avg(n - 20, n - 10);
        return true;
    }

    matjson::Value LevelStats::toJson() const {
        std::vector<matjson::Value> da, hist, sess;
        for (int v : deathsAt) da.emplace_back(v);
        for (auto v : history) hist.emplace_back(static_cast<int>(v));
        for (auto const& s : sessions) {
            sess.push_back(matjson::makeObject({
                {"start", s.start}, {"duration", s.durationSec}, {"attempts", s.attempts},
                {"deaths", s.deaths}, {"best", s.bestPercent},
            }));
        }
        return matjson::makeObject({
            {"key", key}, {"name", name}, {"id", levelID},
            {"attempts", attempts}, {"deaths", deaths}, {"completions", completions},
            {"best", bestPercent}, {"first", firstPlayed}, {"last", lastPlayed},
            {"seconds", totalSeconds},
            {"deathsAt", da}, {"history", hist}, {"sessions", sess},
        });
    }

    LevelStats LevelStats::fromJson(matjson::Value const& j) {
        LevelStats s;
        s.key = j["key"].asString().unwrapOr(std::string());
        s.name = j["name"].asString().unwrapOr(std::string());
        s.levelID = j["id"].as<int>().unwrapOr(0);
        s.attempts = j["attempts"].as<int>().unwrapOr(0);
        s.deaths = j["deaths"].as<int>().unwrapOr(0);
        s.completions = j["completions"].as<int>().unwrapOr(0);
        s.bestPercent = std::clamp(j["best"].as<int>().unwrapOr(0), 0, 100);
        s.firstPlayed = j["first"].as<int64_t>().unwrapOr(0);
        s.lastPlayed = j["last"].as<int64_t>().unwrapOr(0);
        s.totalSeconds = j["seconds"].as<int>().unwrapOr(0);

        if (auto r = j["deathsAt"].asArray()) {
            auto const& arr = r.unwrap();
            for (size_t i = 0; i < arr.size() && i < static_cast<size_t>(kBuckets); ++i)
                s.deathsAt[i] = std::max(0, arr[i].as<int>().unwrapOr(0));
        }
        if (auto r = j["history"].asArray()) {
            for (auto const& v : r.unwrap())
                s.history.push_back(static_cast<uint8_t>(std::clamp(v.as<int>().unwrapOr(0), 0, 100)));
            if (s.history.size() > kHistoryCap)
                s.history.erase(s.history.begin(), s.history.end() - kHistoryCap);
        }
        if (auto r = j["sessions"].asArray()) {
            for (auto const& v : r.unwrap()) {
                SessionRecord rec;
                rec.start = v["start"].as<int64_t>().unwrapOr(0);
                rec.durationSec = v["duration"].as<int>().unwrapOr(0);
                rec.attempts = v["attempts"].as<int>().unwrapOr(0);
                rec.deaths = v["deaths"].as<int>().unwrapOr(0);
                rec.bestPercent = v["best"].as<int>().unwrapOr(0);
                s.sessions.push_back(rec);
            }
            if (s.sessions.size() > kSessionCap)
                s.sessions.erase(s.sessions.begin(), s.sessions.end() - kSessionCap);
        }
        return s;
    }
}
