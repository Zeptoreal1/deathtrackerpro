#pragma once
#include <Geode/Geode.hpp>

// Central place for colors so the whole UI follows the user's accent color setting.
namespace dtp::theme {
    inline cocos2d::ccColor3B accent() {
        return geode::Mod::get()->getSettingValue<cocos2d::ccColor3B>("accent-color");
    }

    inline cocos2d::ccColor4F toF(cocos2d::ccColor3B c, float a = 1.f) {
        return {c.r / 255.f, c.g / 255.f, c.b / 255.f, a};
    }

    // 0 = accent color, 1 = hot red. Used to color death-map bars by how deadly they are.
    inline cocos2d::ccColor4F heat(float t, float a = 1.f) {
        auto c = toF(accent());
        t = std::clamp(t, 0.f, 1.f);
        return {c.r + (1.0f - c.r) * t, c.g + (0.30f - c.g) * t, c.b + (0.25f - c.b) * t, a};
    }

    inline cocos2d::ccColor4F panel() { return {0.f, 0.f, 0.f, 0.35f}; }
    inline cocos2d::ccColor4F border() { return {1.f, 1.f, 1.f, 0.12f}; }
    inline cocos2d::ccColor4F grid() { return {1.f, 1.f, 1.f, 0.08f}; }
    inline cocos2d::ccColor4F dim() { return {1.f, 1.f, 1.f, 0.35f}; }
}
