//
//  EventBus.h
//  game_engine
//
//  Created by Gracie Hou on 3/22/26.
//

#ifndef EventBus_h
#define EventBus_h

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <iostream>
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"

class EventBus {
public:
    static inline std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> subscriptions;
    static inline std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> pending_subscriptions;
    static inline std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> pending_unsubscriptions;

    static void Publish(std::string event_type, luabridge::LuaRef event_object) {
        if (subscriptions.find(event_type) != subscriptions.end()) {
            for (auto& sub : subscriptions[event_type]) {
                if (sub.second.isFunction()) {
                    try {
                        sub.second(sub.first, event_object);
                    } catch (const luabridge::LuaException& e) {
                        std::string error_message = e.what();
                        std::replace(error_message.begin(), error_message.end(), '\\', '/');
                        std::cout << "\033[31mEventBus Error : " << error_message << "\033[0m\n";
                    }
                }
            }
        }
    }

    static void Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
        pending_subscriptions.push_back({event_type, component, function});
    }

    static void Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
        pending_unsubscriptions.push_back({event_type, component, function});
    }

    static void ProcessDeferred() {
        for (auto& sub : pending_subscriptions) {
            subscriptions[std::get<0>(sub)].push_back({std::get<1>(sub), std::get<2>(sub)});
        }
        pending_subscriptions.clear();

        for (auto& unsub : pending_unsubscriptions) {
            std::string type = std::get<0>(unsub);
            auto comp = std::get<1>(unsub);
            auto func = std::get<2>(unsub);
            
            if (subscriptions.find(type) != subscriptions.end()) {
                auto& vec = subscriptions[type];
                vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const std::pair<luabridge::LuaRef, luabridge::LuaRef>& p) {
                    return p.first == comp && p.second == func;
                }), vec.end());
            }
        }
        pending_unsubscriptions.clear();
    }
};

#endif /* EventBus_h */
