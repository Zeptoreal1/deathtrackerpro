#include "data/StatsManager.hpp"
#include "ui/SessionHud.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Feeds gameplay events (attempts, deaths, completions) into the StatsManager.
class $modify(DtpPlayLayer, PlayLayer) {
    struct Fields {
        bool attemptOpen = false;   // an attempt has been counted and not finished by a death yet
        bool deathCounted = false;
        bool completed = false;
    };

    bool tracking() {
        if (m_isTestMode) return false;
        return !m_isPracticeMode || Mod::get()->getSettingValue<bool>("track-practice");
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        // begin first: the very first resetLevel can already happen inside init
        dtp::StatsManager::get().beginSession(level);
        return PlayLayer::init(level, useReplay, dontCreateObjects);
    }

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        dtp::SessionHud::attach(this);
    }

    void resetLevel() {
        float percentBefore = this->getCurrentPercent();
        PlayLayer::resetLevel();
        if (!this->tracking()) return;

        auto f = m_fields.self();
        // ignore back-to-back resets at 0% that no death caused (the game resets more than once on load)
        bool realAttempt = !f->attemptOpen || f->deathCounted || percentBefore > 0.5f;
        if (!realAttempt) return;

        dtp::StatsManager::get().recordAttempt();
        f->attemptOpen = true;
        f->deathCounted = false;
        f->completed = false;
        dtp::SessionHud::refresh();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        float percent = this->getCurrentPercent();
        PlayLayer::destroyPlayer(player, object);

        auto f = m_fields.self();
        if (f->deathCounted || !this->tracking()) return;
        if (!player || !player->m_isDead) return; // something (e.g. noclip) prevented the death

        f->deathCounted = true;
        dtp::StatsManager::get().recordDeath(static_cast<int>(percent), m_isPracticeMode);
        dtp::SessionHud::refresh();
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        auto f = m_fields.self();
        if (f->completed || !this->tracking()) return;
        f->completed = true;
        dtp::StatsManager::get().recordCompletion(m_isPracticeMode);
        dtp::SessionHud::refresh();
    }

    void onQuit() {
        dtp::SessionHud::detach();
        dtp::StatsManager::get().endSession();
        PlayLayer::onQuit();
    }
};
