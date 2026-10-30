#pragma once
#include "data/LevelStats.hpp"
#include <Geode/Geode.hpp>

namespace dtp {
    // Bar chart: how many deaths happened at each part of the level.
    class DeathMapChart : public cocos2d::CCNode {
    public:
        static DeathMapChart* create(cocos2d::CCSize size, LevelStats const& stats, int bucketPercent);
    protected:
        bool init(cocos2d::CCSize size, LevelStats const& stats, int bucketPercent);
    };

    // Line chart of values in the range 0-100 over time (attempts or sessions).
    class ProgressChart : public cocos2d::CCNode {
    public:
        // `runningBest` additionally draws a "best so far" line.
        static ProgressChart* create(cocos2d::CCSize size, std::vector<int> const& values, bool runningBest);
    protected:
        bool init(cocos2d::CCSize size, std::vector<int> const& values, bool runningBest);
    };
}
