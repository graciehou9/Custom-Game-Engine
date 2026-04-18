//
//  Collision.h
//  game_engine
//
//  Created by Gracie Hou on 3/20/26.
//

#ifndef Collision_h
#define Collision_h

#pragma once
#include "../src/box2d/include/box2d/box2d.h"
#include "Actor.h"
#include <iostream>
#include <algorithm>
#include <map>

struct Collision {
    Actor* other;
    b2Vec2 point;
    b2Vec2 relative_velocity;
    b2Vec2 normal;
};

class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override{
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();

        Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
        Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
        if (!actorA || !actorB) return;

        bool sensorA = fixtureA->IsSensor();
        bool sensorB = fixtureB->IsSensor();
        if (sensorA != sensorB) return;

        b2Vec2 point;
        b2Vec2 normal;
        if (sensorA && sensorB) {
            point = b2Vec2(-999.0f, -999.0f);
            normal = b2Vec2(-999.0f, -999.0f);
        } else {
            b2WorldManifold worldManifold;
            contact->GetWorldManifold(&worldManifold);
            point = worldManifold.points[0];
            normal = worldManifold.normal;
        }

        b2Vec2 relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
        Collision colA = {actorB, point, relative_velocity, normal};
        Collision colB = {actorA, point, relative_velocity, normal};

        if (sensorA && sensorB) {
            CallLifecycle(actorA, "OnTriggerEnter", colA);
            CallLifecycle(actorB, "OnTriggerEnter", colB);
        } else {
            CallLifecycle(actorA, "OnCollisionEnter", colA);
            CallLifecycle(actorB, "OnCollisionEnter", colB);
        }
    }

    void EndContact(b2Contact* contact) override{
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();
        Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
        Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
        if (!actorA || !actorB) return;

        bool sensorA = fixtureA->IsSensor();
        bool sensorB = fixtureB->IsSensor();
        if (sensorA != sensorB) return;

        b2Vec2 point(-999.0f, -999.0f);
        b2Vec2 normal(-999.0f, -999.0f);
        b2Vec2 relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
        Collision colA = {actorB, point, relative_velocity, normal};
        Collision colB = {actorA, point, relative_velocity, normal};

        if (sensorA && sensorB) {
            CallLifecycle(actorA, "OnTriggerExit", colA);
            CallLifecycle(actorB, "OnTriggerExit", colB);
        } else {
            CallLifecycle(actorA, "OnCollisionExit", colA);
            CallLifecycle(actorB, "OnCollisionExit", colB);
        }
    }

private:
    void CallLifecycle(Actor* actor, const std::string& func_name, Collision& col) {
        for (auto& [key, comp] : actor->components) {
            if (!comp["enabled"]) continue;
            luabridge::LuaRef func = comp[func_name];
            if (func.isFunction()) {
                try { func(comp, col); }
                catch (const luabridge::LuaException& e) { 
                    // ReportError(actor->GetName(), e);
                    std::string error_message = e.what();
                    std::replace(error_message.begin(), error_message.end(), '\\', '/');
                    std::cout << "\033[31m" << actor->GetName() << " : " << error_message << "\033[0m" << std::endl;
                }
            }
        }
    }
};

#endif /* Collision_h */
