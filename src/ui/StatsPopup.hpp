#pragma once
#include "data/LevelStats.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

namespace dtp {
    // Detailed statistics window for one level. Opened from the pause menu and the level page.
    class StatsPopup : public geode::Popup {
    public:
        static StatsPopup* create(GJGameLevel* level);

    protected:
        static constexpr int kTabCount = 4;

        bool init(GJGameLevel* level);

        // one builder per tab, so adding a tab = one new function + one entry in init()
        void buildOverview(cocos2d::CCNode* tab);
        void buildDeathMap(cocos2d::CCNode* tab);
        void buildProgress(cocos2d::CCNode* tab);
        void buildSessions(cocos2d::CCNode* tab);

        void selectTab(int index);
        void onTab(cocos2d::CCObject* sender);
        void onReset(cocos2d::CCObject* sender);

        std::string m_key;
        LevelStats m_stats;
        cocos2d::CCNode* m_tabs[kTabCount] = {};
        ButtonSprite* m_tabButtons[kTabCount] = {};
        int m_current = 0;
    };
}
