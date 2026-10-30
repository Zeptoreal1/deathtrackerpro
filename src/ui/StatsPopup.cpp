#include "StatsPopup.hpp"
#include "Charts.hpp"
#include "Theme.hpp"
#include "data/StatsManager.hpp"
#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace dtp {
    namespace {
        constexpr char const* kTabNames[] = {"Overview", "Death Map", "Progress", "Sessions"};

        std::string formatDuration(int seconds) {
            int h = seconds / 3600, m = (seconds % 3600) / 60;
            if (h > 0) return fmt::format("{}h {}m", h, m);
            if (m > 0) return fmt::format("{}m", m);
            return fmt::format("{}s", seconds);
        }

        std::string formatDate(int64_t t) {
            if (t <= 0) return "-";
            std::time_t tt = static_cast<std::time_t>(t);
            char buf[32];
            std::tm tmv{};
#ifdef GEODE_IS_WINDOWS
            localtime_s(&tmv, &tt);
#else
            localtime_r(&tt, &tmv);
#endif
            std::strftime(buf, sizeof(buf), "%d %b %H:%M", &tmv);
            return buf;
        }

        CCLabelBMFont* label(CCNode* parent, std::string const& text, char const* font, float scale, CCPoint pos,
                             CCPoint anchor = {0.5f, 0.5f}, ccColor3B color = {255, 255, 255}, GLubyte opacity = 255) {
            auto l = CCLabelBMFont::create(text.c_str(), font);
            l->setScale(scale);
            l->setAnchorPoint(anchor);
            l->setPosition(pos);
            l->setColor(color);
            l->setOpacity(opacity);
            parent->addChild(l);
            return l;
        }

        void card(CCNode* parent, CCPoint center, std::string const& title, std::string const& value,
                  ccColor3B valueColor = {255, 255, 255}) {
            constexpr float w = 84.f, h = 58.f;
            auto bg = CCScale9Sprite::create("square02_001.png");
            bg->setContentSize({w, h});
            bg->setOpacity(70);
            bg->setPosition(center);
            parent->addChild(bg);
            auto v = label(parent, value, "bigFont.fnt", 0.5f, {center.x, center.y + 7.f}, {0.5f, 0.5f}, valueColor);
            v->limitLabelWidth(w - 10.f, 0.5f, 0.2f);
            label(parent, title, "chatFont.fnt", 0.5f, {center.x, center.y - 17.f}, {0.5f, 0.5f}, {255, 255, 255}, 170);
        }
    }

    StatsPopup* StatsPopup::create(GJGameLevel* level) {
        auto ret = new StatsPopup();
        if (ret->init(level)) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }

    bool StatsPopup::init(GJGameLevel* level) {
        if (!Popup::init(430.f, 290.f)) return false;

        m_key = StatsManager::keyFor(level);
        if (auto found = StatsManager::get().find(m_key)) m_stats = *found;
        std::string name = std::string(level->m_levelName);
        if (name.size() > 26) name = name.substr(0, 25) + "...";
        this->setTitle(name.c_str());

        // tabs
        for (int i = 0; i < kTabCount; ++i) {
            auto spr = ButtonSprite::create(kTabNames[i], 84, true, "bigFont.fnt", "GJ_button_04.png", 24.f, 0.5f);
            auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(StatsPopup::onTab));
            btn->setTag(i);
            btn->setPosition({62.f + i * 102.f, 238.f});
            m_buttonMenu->addChild(btn);
            m_tabButtons[i] = spr;

            auto node = CCNode::create();
            node->setContentSize({360.f, 170.f});
            node->setAnchorPoint({0.f, 0.f});
            node->setPosition({35.f, 48.f});
            m_mainLayer->addChild(node);
            m_tabs[i] = node;
        }
        this->buildOverview(m_tabs[0]);
        this->buildDeathMap(m_tabs[1]);
        this->buildProgress(m_tabs[2]);
        this->buildSessions(m_tabs[3]);
        this->selectTab(0);

        // footer
        label(m_mainLayer, fmt::format("Last played: {}", formatDate(m_stats.lastPlayed)), "chatFont.fnt", 0.5f,
              {35.f, 28.f}, {0.f, 0.5f}, {255, 255, 255}, 150);
        auto resetSpr = ButtonSprite::create("Reset", 50, true, "bigFont.fnt", "GJ_button_06.png", 22.f, 0.45f);
        auto resetBtn = CCMenuItemSpriteExtra::create(resetSpr, this, menu_selector(StatsPopup::onReset));
        resetBtn->setPosition({385.f, 28.f});
        m_buttonMenu->addChild(resetBtn);
        return true;
    }

    void StatsPopup::selectTab(int index) {
        m_current = index;
        for (int i = 0; i < kTabCount; ++i) {
            m_tabs[i]->setVisible(i == index);
            m_tabButtons[i]->updateBGImage(i == index ? "GJ_button_01.png" : "GJ_button_04.png");
        }
    }

    void StatsPopup::onTab(CCObject* sender) {
        this->selectTab(static_cast<CCNode*>(sender)->getTag());
    }

    void StatsPopup::onReset(CCObject*) {
        Ref<StatsPopup> self = this;
        std::string key = m_key;
        createQuickPopup("Reset stats", "Delete <cr>all</c> tracked stats for this level?\nThis cannot be undone.",
            "Cancel", "Reset", [self, key](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                StatsManager::get().reset(key);
                self->onClose(nullptr);
            });
    }

    // ---------------------------------------------------------------- tabs
    void StatsPopup::buildOverview(CCNode* tab) {
        auto const& s = m_stats;
        auto accent = theme::accent();
        float xs[4] = {42.f, 134.f, 226.f, 318.f};

        card(tab, {xs[0], 132.f}, "Attempts", std::to_string(s.attempts));
        card(tab, {xs[1], 132.f}, "Deaths", std::to_string(s.deaths));
        card(tab, {xs[2], 132.f}, "Best", fmt::format("{}%", s.bestPercent), accent);
        card(tab, {xs[3], 132.f}, "Completions", std::to_string(s.completions));

        double avg = s.averageDeathPercent();
        int count = 0;
        int start = s.deadliestWindow(3, count);
        double delta = 0;
        bool hasDelta = s.improvement(delta);

        card(tab, {xs[0], 68.f}, "Avg death", avg < 0 ? "-" : fmt::format("{:.1f}%", avg));
        card(tab, {xs[1], 68.f}, "Deadliest", count == 0 ? "-" : fmt::format("{}-{}%", start, start + 3));
        card(tab, {xs[2], 68.f}, "Play time", formatDuration(s.totalSeconds));
        card(tab, {xs[3], 68.f}, "Trend (last 10)", hasDelta ? fmt::format("{:+.1f}%", delta) : "n/a",
             !hasDelta ? ccColor3B{255, 255, 255} : (delta >= 0 ? ccColor3B{120, 255, 140} : ccColor3B{255, 120, 110}));

        auto const& sess = StatsManager::get().session();
        std::string line = "No active session";
        if (sess.active && StatsManager::get().sessionKey() == m_key) {
            line = fmt::format("This session: {} attempts, {} deaths, best {}%", sess.attempts, sess.deaths, sess.bestPercent);
            if (sess.bestPercent > sess.startBest) line += fmt::format(" (was {}%)", sess.startBest);
        }
        label(tab, line, "chatFont.fnt", 0.6f, {180.f, 14.f}, {0.5f, 0.5f}, accent);
    }

    void StatsPopup::buildDeathMap(CCNode* tab) {
        int bucket = static_cast<int>(Mod::get()->getSettingValue<int64_t>("death-map-bucket"));
        auto chart = DeathMapChart::create({340.f, 118.f}, m_stats, bucket);
        chart->setPosition({10.f, 38.f});
        tab->addChild(chart);

        int count = 0;
        int start = m_stats.deadliestWindow(std::max(bucket, 3), count);
        int w = std::max(bucket, 3);
        std::string line = count == 0 ? "Die a few times and the hot spots will show up here"
            : fmt::format("Deadliest stretch: {}-{}% ({} deaths, {:.0f}% of all)", start, start + w, count,
                          100.0 * count / std::max(1, m_stats.deaths));
        label(tab, line, "chatFont.fnt", 0.6f, {180.f, 6.f}, {0.5f, 0.5f}, theme::accent());
    }

    void StatsPopup::buildProgress(CCNode* tab) {
        std::vector<int> values(m_stats.history.begin(), m_stats.history.end());
        auto chart = ProgressChart::create({322.f, 118.f}, values, true);
        chart->setPosition({28.f, 38.f});
        tab->addChild(chart);
        label(tab, fmt::format("Last {} attempts. Bright line = best so far.", values.size()), "chatFont.fnt", 0.6f,
              {180.f, 6.f}, {0.5f, 0.5f}, theme::accent());
    }

    void StatsPopup::buildSessions(CCNode* tab) {
        auto const& sessions = m_stats.sessions;
        std::vector<int> bests;
        size_t from = sessions.size() > 30 ? sessions.size() - 30 : 0;
        for (size_t i = from; i < sessions.size(); ++i) bests.push_back(sessions[i].bestPercent);

        auto chart = ProgressChart::create({322.f, 66.f}, bests, false);
        chart->setPosition({28.f, 100.f});
        tab->addChild(chart);

        if (sessions.empty()) {
            label(tab, "Finished sessions will be listed here", "chatFont.fnt", 0.6f, {180.f, 60.f});
            return;
        }
        // newest first, 5 rows
        int row = 0;
        for (size_t i = sessions.size(); i-- > 0 && row < 5; ++row) {
            auto const& r = sessions[i];
            std::string delta;
            if (i > 0) delta = fmt::format("  ({:+d}%)", r.bestPercent - sessions[i - 1].bestPercent);
            label(tab, fmt::format("{}   {} att   {} deaths   best {}%{}", formatDate(r.start), r.attempts, r.deaths, r.bestPercent, delta),
                  "chatFont.fnt", 0.55f, {10.f, 84.f - row * 15.f}, {0.f, 0.5f}, {255, 255, 255}, 200);
        }
    }
}
