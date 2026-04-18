//
//  Raycast.h
//  game_engine
//
//  Created by Gracie Hou on 3/21/26.
//

#ifndef Raycast_h
#define Raycast_h

#include "../src/box2d/include/box2d/box2d.h"
#include "Actor.h"
#include "Engine.h"
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"
#include <vector>
#include <algorithm>

extern Engine* engine_ptr;

struct HitResult {
    Actor* actor;
    b2Vec2 point;
    b2Vec2 normal;
    bool is_trigger;
};

struct RaycastHitData {
    HitResult result;
    float fraction;
};

class ClosestRayCastCallback : public b2RayCastCallback {
public:
    bool hit = false;
    HitResult result;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
        if (actor == nullptr) return -1.0f; 

        hit = true;
        result.actor = actor;
        result.point = point;
        result.normal = normal;
        result.is_trigger = fixture->IsSensor();
        return fraction; 
    }
};

class AllRayCastCallback : public b2RayCastCallback {
public:
    std::vector<RaycastHitData> hits;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
        if (actor == nullptr) return -1.0f;

        HitResult res;
        res.actor = actor;
        res.point = point;
        res.normal = normal;
        res.is_trigger = fixture->IsSensor();
        
        hits.push_back({res, fraction});
        return 1.0f; 
    }
};

inline luabridge::LuaRef Physics_Raycast(b2Vec2 pos, b2Vec2 dir, float dist, lua_State* L) {
    if (dist <= 0.0f || engine_ptr->physics_world == nullptr) {
        return luabridge::LuaRef(L); 
    }

    dir.Normalize();
    b2Vec2 p2 = pos + (dist * dir);

    ClosestRayCastCallback callback;
    engine_ptr->physics_world->RayCast(&callback, pos, p2);

    if (callback.hit) {
        return luabridge::LuaRef(L, callback.result);
    }
    return luabridge::LuaRef(L);
}

inline luabridge::LuaRef Physics_RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist, lua_State* L) {
    luabridge::LuaRef table = luabridge::newTable(L);
    if (dist <= 0.0f || engine_ptr->physics_world == nullptr) {
        return table;
    }

    dir.Normalize();
    b2Vec2 p2 = pos + (dist * dir);

    AllRayCastCallback callback;
    engine_ptr->physics_world->RayCast(&callback, pos, p2);

    std::sort(callback.hits.begin(), callback.hits.end(), [](const RaycastHitData& a, const RaycastHitData& b) {
        return a.fraction < b.fraction;
    });

    int index = 1;
    for (const auto& hitData : callback.hits) {
        table[index++] = hitData.result;
    }
    return table;
}

#endif /* Raycast_h */
