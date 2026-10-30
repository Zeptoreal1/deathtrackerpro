#include "ui/StatsPopup.hpp"
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

// Lets you open the stats of any level from its level page, without starting it.
class $modify(DtpLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto spr = ButtonSprite::create("Stats", 50, true, "goldFont.fnt", "GJ_button_01.png", 28.f, 0.7f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DtpLevelInfoLayer::onStats));
        auto menu = CCMenu::create();
        menu->setPosition({45.f, 28.f});
        btn->setPosition({0, 0});
        menu->addChild(btn);
        this->addChild(menu, 10);
        return true;
    }

    void onStats(CCObject*) {
        if (auto popup = dtp::StatsPopup::create(m_level)) popup->show();
    }
};
