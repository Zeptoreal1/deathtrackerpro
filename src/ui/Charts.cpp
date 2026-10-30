#include "Charts.hpp"
#include "Theme.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace dtp {
    namespace {
        CCLabelBMFont* makeLabel(char const* text, float scale, CCPoint pos, CCPoint anchor = {0.5f, 0.5f}, GLubyte opacity = 150) {
            auto l = CCLabelBMFont::create(text, "chatFont.fnt");
            l->setScale(scale);
            l->setAnchorPoint(anchor);
            l->setPosition(pos);
            l->setOpacity(opacity);
            return l;
        }

        void drawFrame(CCDrawNode* draw, CCSize size) {
            draw->drawRect({0, 0}, {size.width, size.height}, theme::panel(), 1.f, theme::border());
        }
    }

    // ---------------------------------------------------------------- death map
    DeathMapChart* DeathMapChart::create(CCSize size, LevelStats const& stats, int bucket) {
        auto ret = new DeathMapChart();
        if (ret->init(size, stats, bucket)) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }

    bool DeathMapChart::init(CCSize size, LevelStats const& stats, int bucket) {
        if (!CCNode::init()) return false;
        this->setContentSize(size);
        bucket = std::clamp(bucket, 1, 10);

        auto draw = CCDrawNode::create();
        this->addChild(draw);
        drawFrame(draw, size);

        for (int p : {25, 50, 75}) {
            float x = size.width * p / 100.f;
            draw->drawSegment({x, 0}, {x, size.height}, 0.4f, theme::grid());
        }
        for (int p : {0, 25, 50, 75, 100}) {
            this->addChild(makeLabel(fmt::format("{}%", p).c_str(), 0.4f, {size.width * p / 100.f, -7.f}));
        }

        int n = (kBuckets + bucket - 1) / bucket;
        std::vector<int> counts(n, 0);
        int maxCount = 0, total = 0;
        for (int i = 0; i < n; ++i) {
            counts[i] = stats.deathsInRange(i * bucket, (i + 1) * bucket);
            maxCount = std::max(maxCount, counts[i]);
            total += counts[i];
        }

        if (total == 0) {
            this->addChild(makeLabel("No deaths recorded yet", 0.6f, {size.width / 2, size.height / 2}));
            return true;
        }

        float slot = size.width / static_cast<float>(n);
        float usable = size.height - 14.f;
        for (int i = 0; i < n; ++i) {
            if (counts[i] == 0) continue;
            float t = static_cast<float>(counts[i]) / static_cast<float>(maxCount);
            float x0 = i * slot + 0.5f, x1 = (i + 1) * slot - 0.5f;
            float h = std::max(2.f, t * usable);
            draw->drawRect({x0, 1.f}, {x1, 1.f + h}, theme::heat(t, 0.95f), 0.f, {0, 0, 0, 0});
        }

        // marker for the best run
        if (stats.bestPercent > 0 && stats.bestPercent < 100) {
            float x = size.width * stats.bestPercent / 100.f;
            draw->drawSegment({x, 0}, {x, size.height}, 0.8f, theme::toF(theme::accent(), 0.9f));
            this->addChild(makeLabel(fmt::format("Best {}%", stats.bestPercent).c_str(), 0.45f,
                {std::min(x + 3.f, size.width - 2.f), size.height - 6.f}, {x > size.width - 45.f ? 1.f : 0.f, 0.5f}, 230));
        }
        return true;
    }

    // ---------------------------------------------------------------- progress chart
    ProgressChart* ProgressChart::create(CCSize size, std::vector<int> const& values, bool runningBest) {
        auto ret = new ProgressChart();
        if (ret->init(size, values, runningBest)) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }

    bool ProgressChart::init(CCSize size, std::vector<int> const& values, bool runningBest) {
        if (!CCNode::init()) return false;
        this->setContentSize(size);

        auto draw = CCDrawNode::create();
        this->addChild(draw);
        drawFrame(draw, size);

        for (int p : {0, 50, 100}) {
            float y = 4.f + (size.height - 8.f) * p / 100.f;
            if (p != 0) draw->drawSegment({0, y}, {size.width, y}, 0.4f, theme::grid());
            this->addChild(makeLabel(fmt::format("{}%", p).c_str(), 0.4f, {-3.f, y}, {1.f, 0.5f}));
        }

        if (values.size() < 2) {
            this->addChild(makeLabel("Play a few more attempts to see a graph", 0.6f, {size.width / 2, size.height / 2}));
            return true;
        }

        float pad = 6.f;
        float stepX = (size.width - pad * 2) / static_cast<float>(values.size() - 1);
        auto pointAt = [&](size_t i, int v) {
            return CCPoint{pad + stepX * i, 4.f + (size.height - 8.f) * std::clamp(v, 0, 100) / 100.f};
        };

        // raw values (dim), then running best on top
        for (size_t i = 0; i + 1 < values.size(); ++i)
            draw->drawSegment(pointAt(i, values[i]), pointAt(i + 1, values[i + 1]), 0.5f, theme::dim());
        if (values.size() <= 60)
            for (size_t i = 0; i < values.size(); ++i)
                draw->drawDot(pointAt(i, values[i]), 1.6f, theme::dim());

        if (runningBest) {
            int best = 0;
            std::vector<int> rb;
            for (int v : values) { best = std::max(best, v); rb.push_back(best); }
            for (size_t i = 0; i + 1 < rb.size(); ++i)
                draw->drawSegment(pointAt(i, rb[i]), pointAt(i + 1, rb[i + 1]), 1.1f, theme::toF(theme::accent()));
        }
        draw->drawDot(pointAt(values.size() - 1, values.back()), 3.f, theme::toF(theme::accent()));
        return true;
    }
}
