#include "SessionHud.hpp"
#include "Theme.hpp"
#include "data/StatsManager.hpp"

using namespace geode::prelude;

namespace dtp {
    namespace {
        Ref<CCNode> s_root;
        Ref<CCLabelBMFont> s_label;
        Ref<CCScale9Sprite> s_bg;
    }

    void SessionHud::attach(PlayLayer* layer) {
        s_root = nullptr; s_label = nullptr; s_bg = nullptr;
        if (!Mod::get()->getSettingValue<bool>("hud-enabled")) return;

        auto root = CCNode::create();
        root->setZOrder(100);
        auto bg = CCScale9Sprite::create("square02_001.png");
        auto lbl = CCLabelBMFont::create("", "chatFont.fnt");
        root->addChild(bg);
        root->addChild(lbl);
        layer->addChild(root);

        s_root = root; s_bg = bg; s_label = lbl;
        refresh();
    }

    void SessionHud::refresh() {
        if (!s_root || !s_label || !s_bg) return;
        auto mod = Mod::get();
        auto const& sess = StatsManager::get().session();

        std::vector<std::string> parts;
        if (mod->getSettingValue<bool>("hud-show-attempts")) parts.push_back(fmt::format("Attempts {}", sess.attempts));
        if (mod->getSettingValue<bool>("hud-show-deaths")) parts.push_back(fmt::format("Deaths {}", sess.deaths));
        if (mod->getSettingValue<bool>("hud-show-best")) parts.push_back(fmt::format("Best {}%", sess.bestPercent));
        std::string text;
        for (size_t i = 0; i < parts.size(); ++i) text += (i ? "   |   " : "") + parts[i];

        float opacity = std::clamp<int64_t>(mod->getSettingValue<int64_t>("hud-opacity"), 10, 100) / 100.f;
        s_label->setString(text.c_str());
        s_label->setColor(theme::accent());
        s_label->setOpacity(static_cast<GLubyte>(255 * opacity));

        auto size = s_label->getContentSize();
        s_bg->setContentSize({size.width + 14.f, size.height + 8.f});
        s_bg->setOpacity(static_cast<GLubyte>(120 * opacity));
        s_label->setPosition({0, 0});
        s_bg->setPosition({0, 0});
        s_root->setVisible(!parts.empty());
        s_root->setScale(static_cast<float>(mod->getSettingValue<double>("hud-scale")));

        // anchor the panel to the chosen corner
        auto win = CCDirector::get()->getWinSize();
        float halfW = (size.width + 14.f) * 0.5f * s_root->getScale();
        float halfH = (size.height + 8.f) * 0.5f * s_root->getScale();
        constexpr float margin = 6.f;
        auto corner = mod->getSettingValue<std::string>("hud-corner");
        bool right = corner.find("Right") != std::string::npos;
        bool bottom = corner.find("Bottom") != std::string::npos;
        s_root->setPosition({right ? win.width - margin - halfW : margin + halfW,
                             bottom ? margin + halfH : win.height - margin - halfH});
    }

    void SessionHud::detach() {
        if (s_root) s_root->removeFromParent();
        s_root = nullptr; s_label = nullptr; s_bg = nullptr;
    }
}
