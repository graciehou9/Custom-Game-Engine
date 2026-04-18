//
//  Rigidbody.h
//  game_engine
//
//  Created by Gracie Hou on 3/18/26.
//

#ifndef Rigidbody_h
#define Rigidbody_h

#pragma once

#include <string>
#include "Actor.h"
// #include "Engine.h"
#include "../src/box2d/include/box2d/box2d.h"
#include "../src/glm/glm.hpp"

class Engine;
extern Engine* engine_ptr;

class Rigidbody {
public:
    std::string key = "";
    std::string type = "Rigidbody";
    bool enabled = true;
    Actor* actor = nullptr;

    float x = 0.0f;
    float y = 0.0f;
    std::string body_type = "dynamic";
    bool precise = true;
    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;
    bool has_collider = true;
    bool has_trigger = true;
    float width = 1.0f;
    float height = 1.0f;
    std::string collider_type = "box"; 
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;

    b2Body* body = nullptr;

    Rigidbody() {}

    Rigidbody(const Rigidbody& other) = default; 

    void OnStart();
    void OnDestroy();

    void AddForce(b2Vec2 force) {
        if (body) body->ApplyForceToCenter(force, true);
    }
    void SetVelocity(b2Vec2 vel) {
        if (body) body->SetLinearVelocity(vel);
    }
    void SetPosition(b2Vec2 pos) {
        if (body) body->SetTransform(pos, body->GetAngle());
        else { x = pos.x; y = pos.y; }
    }
    void SetRotation(float degrees_clockwise) {
        if (body) body->SetTransform(body->GetPosition(), degrees_clockwise * (b2_pi / 180.0f));
        else rotation = degrees_clockwise;
    }
    void SetAngularVelocity(float degrees_clockwise) {
        if (body) body->SetAngularVelocity(degrees_clockwise * (b2_pi / 180.0f));
    }
    void SetGravityScale(float scale) {
        if (body) body->SetGravityScale(scale);
        else gravity_scale = scale;
    }
    void SetUpDirection(b2Vec2 dir) {
        dir.Normalize();
        float angle = glm::atan(dir.x, -dir.y);
        if (body) body->SetTransform(body->GetPosition(), angle);
        else rotation = angle * (180.0f / b2_pi);
    }
    void SetRightDirection(b2Vec2 dir) {
        dir.Normalize();
        float angle = glm::atan(dir.x, -dir.y) - (b2_pi / 2.0f);
        if (body) body->SetTransform(body->GetPosition(), angle);
        else rotation = angle * (180.0f / b2_pi);
    }

    b2Vec2 GetVelocity() {
        return body ? body->GetLinearVelocity() : b2Vec2(0.0f, 0.0f);
    }
    float GetAngularVelocity() {
        return body ? body->GetAngularVelocity() * (180.0f / b2_pi) : 0.0f;
    }
    float GetGravityScale() {
        return body ? body->GetGravityScale() : gravity_scale;
    }
    b2Vec2 GetPosition() {
        if (body) return body->GetPosition();
        return b2Vec2(x, y);
    }
    float GetRotation() {
        if (body) return body->GetAngle() * (180.0f / b2_pi);
        return rotation;
    }
    b2Vec2 GetUpDirection() {
        float angle = body ? body->GetAngle() : rotation * (b2_pi / 180.0f);
        b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
        result.Normalize();
        return result;
    }
    b2Vec2 GetRightDirection() {
        float angle = body ? body->GetAngle() : rotation * (b2_pi / 180.0f);
        b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
        result.Normalize();
        return result;
    }
};

#include "Engine.h"

inline void Rigidbody::OnStart() {
    if (engine_ptr->physics_world == nullptr) {
        engine_ptr->physics_world = new b2World(b2Vec2(0.0f, 9.8f));
        engine_ptr->physics_world->SetContactListener(&engine_ptr->contact_listener);
    }

    b2BodyDef bodyDef;
    if (body_type == "dynamic") bodyDef.type = b2_dynamicBody;
    else if (body_type == "static") bodyDef.type = b2_staticBody;
    else if (body_type == "kinematic") bodyDef.type = b2_kinematicBody;

    bodyDef.position.Set(x, y);
    bodyDef.angle = rotation * (b2_pi / 180.0f);
    bodyDef.bullet = precise;
    bodyDef.gravityScale = gravity_scale;
    bodyDef.angularDamping = angular_friction;

    body = engine_ptr->physics_world->CreateBody(&bodyDef);

    
    if (!has_collider && !has_trigger) {
        b2PolygonShape phantom_shape;
        phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);
        
        b2FixtureDef phantom_fixture_def;
        phantom_fixture_def.isSensor = true;
        phantom_fixture_def.userData.pointer = 0; //reinterpret_cast<uintptr_t>(actor);
        phantom_fixture_def.shape = &phantom_shape;
        phantom_fixture_def.density = density;
        
        body->CreateFixture(&phantom_fixture_def);
    }
    
    if (has_collider) {
        b2FixtureDef collider_def;
        collider_def.isSensor = false;
        collider_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
        collider_def.density = density;
        collider_def.friction = friction;
        collider_def.restitution = bounciness;
        
        b2PolygonShape box_shape;
        b2CircleShape circle_shape;

        if (collider_type == "box") {
            box_shape.SetAsBox(width * 0.5f, height * 0.5f); 
            collider_def.shape = &box_shape;
            body->CreateFixture(&collider_def);
        } 
        else if (collider_type == "circle") {
            circle_shape.m_radius = radius;
            collider_def.shape = &circle_shape;
            body->CreateFixture(&collider_def);
        }
    }

    if (has_trigger) {
        b2FixtureDef trigger_def;
        trigger_def.isSensor = true;
        trigger_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
        trigger_def.density = density;
        
        b2PolygonShape box_shape;
        b2CircleShape circle_shape;

        if (trigger_type == "box") {
            box_shape.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f); 
            trigger_def.shape = &box_shape;
            body->CreateFixture(&trigger_def);
        } 
        else if (trigger_type == "circle") {
            circle_shape.m_radius = trigger_radius;
            trigger_def.shape = &circle_shape;
            body->CreateFixture(&trigger_def);
        }
    }
}

inline void Rigidbody::OnDestroy() {
    if (body) {
        engine_ptr->physics_world->DestroyBody(body);
        body = nullptr;
    }
}


#endif /* Rigidbody_h */
