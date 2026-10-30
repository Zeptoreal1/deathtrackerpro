#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>

namespace dtp {
    // Small on-screen panel with the numbers of the current session.
    class SessionHud {
    public:
        static void attach(PlayLayer* layer);
        static void refresh();
        static void detach();
    };
}
