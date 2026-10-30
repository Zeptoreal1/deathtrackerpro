#include "ui/StatsPopup.hpp"
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// Adds a "Stats" button to the pause menu that opens the stats window for the current level.
class $modify(DtpPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto mod = Mod::get();
        auto win = CCDirector::get()->getWinSize();
        auto corner = mod->getSettingValue<std::string>("pause-button-corner");
        bool right = corner.find("Right") != std::string::npos;
        bool bottom = corner.find("Bottom") != std::string::npos;

        auto spr = ButtonSprite::create("Stats", 60, true, "goldFont.fnt", "GJ_button_01.png", 34.f, 0.8f);
        spr->setScale(static_cast<float>(mod->getSettingValue<double>("pause-button-scale")));
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DtpPauseLayer::onStats));

        auto menu = CCMenu::create();
        menu->setPosition({right ? win.width - 55.f : 55.f, bottom ? 35.f : win.height - 35.f});
        menu->setZOrder(100);
        btn->setPosition({0, 0});
        menu->addChild(btn);
        this->addChild(menu);
    }

    void onStats(CCObject*) {
        auto pl = PlayLayer::get();
        if (!pl) return;
        if (auto popup = dtp::StatsPopup::create(pl->m_level)) popup->show();
    }
};
