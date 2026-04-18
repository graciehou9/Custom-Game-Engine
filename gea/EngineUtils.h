//
//  EngineUtils.hpp
//  game_engine
//
//  Created by Gracie Hou on 2/4/26.
//

#ifndef EngineUtils_h
#define EngineUtils_h

#include <stdio.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "../src/rapidjson/document.h"
#include "../src/rapidjson/filereadstream.h"
#include "Actor.h"
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"
#include "Rigidbody.h"
#include "ParticleSystem.h"


static void ReadJsonFile(const std::string& path, rapidjson::Document & out_document)
{
    FILE* file_pointer = nullptr;
#ifdef _WIN32
    fopen_s(&file_pointer, path.c_str(), "rb");
#else
    file_pointer = fopen(path.c_str(), "rb");
#endif
    char buffer[65536];
    rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
    out_document.ParseStream(stream);
    std::fclose(file_pointer);

    if (out_document.HasParseError()) {
        rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
        std::cout << "error parsing json at [" << path << "]" << std::endl;
        exit(0);
    }
}

static float GetFloatFunc(const rapidjson::Value& val) {
    if (val.IsDouble()) return (float)val.GetDouble();
    if (val.IsInt()) return (float)val.GetInt();
    return 0.0f;
}

static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase) {
    size_t pos = input.find(phrase);
    if (pos == std::string::npos) {
        return "";
    }
    pos += phrase.length();
    while (pos < input.size() && isspace(input[pos])) {
        pos++;
    }

    if (pos == input.size()) {
        return "";
    }

    size_t endPos = pos;
    while (endPos < input.size() && !isspace(input[endPos])) {
        endPos++;
    }
    return input.substr(pos, endPos - pos);
}

static void ApplyJsonToActor(Actor& actor, const rapidjson::Value& jsonVal) {
    if (jsonVal.HasMember("name")) actor.actor_name = jsonVal["name"].GetString();
    if (jsonVal.HasMember("view_image")) {
        actor.view_image = jsonVal["view_image"].GetString();
        actor.current_view_image = actor.view_image;
    }
    if (jsonVal.HasMember("view_image_back")) actor.view_image_back = jsonVal["view_image_back"].GetString();
    if (jsonVal.HasMember("view")) actor.view = jsonVal["view"].GetString()[0];
    

    if (jsonVal.HasMember("x")) actor.position.x = GetFloatFunc(jsonVal["x"]);
    if (jsonVal.HasMember("y")) actor.position.y = GetFloatFunc(jsonVal["y"]);
    if (jsonVal.HasMember("transform_position_x")) actor.position.x = GetFloatFunc(jsonVal["transform_position_x"]);
    if (jsonVal.HasMember("transform_position_y")) actor.position.y = GetFloatFunc(jsonVal["transform_position_y"]);

    if (jsonVal.HasMember("transform_scale_x")) actor.transform_scale.x = GetFloatFunc(jsonVal["transform_scale_x"]);
    if (jsonVal.HasMember("transform_scale_y")) actor.transform_scale.y = GetFloatFunc(jsonVal["transform_scale_y"]);
    if (jsonVal.HasMember("transform_rotation_degrees")) actor.transform_rotation_degrees = GetFloatFunc(jsonVal["transform_rotation_degrees"]);

    if (jsonVal.HasMember("view_pivot_offset_x")) actor.view_pivot_offset_x = GetFloatFunc(jsonVal["view_pivot_offset_x"]);
    if (jsonVal.HasMember("view_pivot_offset_y")) actor.view_pivot_offset_y = GetFloatFunc(jsonVal["view_pivot_offset_y"]);

    if (jsonVal.HasMember("box_collider_width")){
        if (jsonVal["box_collider_width"].IsNull()) actor.box_collider_width.reset();
        else actor.box_collider_width = GetFloatFunc(jsonVal["box_collider_width"]);
    }
    if (jsonVal.HasMember("box_collider_height")){
        if (jsonVal["box_collider_height"].IsNull()) actor.box_collider_height.reset();
        else actor.box_collider_height = GetFloatFunc(jsonVal["box_collider_height"]);
    }
    if (jsonVal.HasMember("box_trigger_width")){
        if (jsonVal["box_trigger_width"].IsNull()) actor.box_trigger_width.reset();
        else actor.box_trigger_width = GetFloatFunc(jsonVal["box_trigger_width"]);
    }
    if (jsonVal.HasMember("box_trigger_height")){
        if (jsonVal["box_trigger_height"].IsNull()) actor.box_trigger_height.reset();
        else actor.box_trigger_height = GetFloatFunc(jsonVal["box_trigger_height"]);
    }
    
    if (jsonVal.HasMember("view_image_damage")) actor.view_image_damage = jsonVal["view_image_damage"].GetString();
    if (jsonVal.HasMember("view_image_attack")) actor.view_image_attack = jsonVal["view_image_attack"].GetString();
    
    if (jsonVal.HasMember("vel_x")) actor.velocity.x = GetFloatFunc(jsonVal["vel_x"]);
    if (jsonVal.HasMember("vel_y")) actor.velocity.y = GetFloatFunc(jsonVal["vel_y"]);

    actor.blocking = (actor.box_collider_width.has_value() && actor.box_collider_height.has_value());
    if (jsonVal.HasMember("movement_bounce_enabled")) actor.movement_bounce_enabled = jsonVal["movement_bounce_enabled"].GetBool();
   
    if (jsonVal.HasMember("render_order")) actor.render_order = jsonVal["render_order"].GetInt();

    if (jsonVal.HasMember("nearby_dialogue_sfx")) actor.nearby_dialogue_sfx = jsonVal["nearby_dialogue_sfx"].GetString();
}

static bool CheckCollisions(glm::vec2 pos_a, const Actor&a, glm::vec2 pos_b, const Actor& b, bool is_trigger){
    float wA, wB, hA, hB;

    if (is_trigger){
        if (!b.box_trigger_height.has_value() || !b.box_trigger_width.has_value()){
            return false;
        }
        wB = b.box_trigger_width.value() * glm::abs(b.transform_scale.x);
        hB = b.box_trigger_height.value() * glm::abs(b.transform_scale.y);
        float b_left = pos_b.x - (wB*0.5f);
        float b_right = pos_b.x + (wB*0.5f);
        float b_top = pos_b.y - (hB*0.5f);
        float b_bottom = pos_b.y + (hB*0.5f);

        if (a.box_trigger_width.has_value() && a.box_trigger_height.has_value()) {
            wA = a.box_trigger_width.value() * glm::abs(a.transform_scale.x);
            hA = a.box_trigger_height.value() * glm::abs(a.transform_scale.y);
            float a_left = pos_a.x - (wA*0.5f);
            float a_right = pos_a.x + (wA*0.5f);
            float a_top = pos_a.y - (hA*0.5f);
            float a_bottom = pos_a.y + (hA*0.5f);
            if (a_left >= b_right || a_right <= b_left || a_top >= b_bottom || a_bottom <= b_top) return false;
            return true;
        }
        else {
            if (pos_a.x > b_left && pos_a.x < b_right && pos_a.y > b_top && pos_a.y < b_bottom){
                return true;
            } else {
                return false;
            }
        }
    } else {
        if (a.box_collider_height.has_value() && a.box_collider_width.has_value() && 
            b.box_collider_height.has_value() && b.box_collider_width.has_value()){
            wA = a.box_collider_width.value()* glm::abs(a.transform_scale.x);
            hA = a.box_collider_height.value()* glm::abs(a.transform_scale.y);
            wB = b.box_collider_width.value()* glm::abs(b.transform_scale.x);
            hB = b.box_collider_height.value()* glm::abs(b.transform_scale.y);

            float a_left = pos_a.x - (wA*0.5f);
            float a_right = pos_a.x + (wA*0.5f);
            float a_top = pos_a.y - (hA*0.5f);
            float a_bottom = pos_a.y + (hA*0.5f);

            float b_left = pos_b.x - (wB*0.5f);
            float b_right = pos_b.x + (wB*0.5f);
            float b_top = pos_b.y - (hB*0.5f);
            float b_bottom = pos_b.y + (hB*0.5f);

            if (a_left >= b_right || a_right <= b_left || a_top >= b_bottom || a_bottom <= b_top) {
                return false;
            }
            return true;

        } else {
            return false;
        }
    }
}

inline void EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table, lua_State* L) {
    luabridge::LuaRef new_metatable = luabridge::newTable(L);
    new_metatable["__index"] = parent_table;
    instance_table.push(L);
    new_metatable.push(L);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

inline void addLuaComponent(Actor& actor, const std::string& key, const std::string& type, lua_State* state){
    static std::unordered_map<std::string, luabridge::LuaRef> component_cache;

    if (type == "Rigidbody") {
        Rigidbody* new_component = new Rigidbody(); 
        luabridge::LuaRef instance_table(state, new_component); 
        
        instance_table["key"] = key;
        instance_table["enabled"] = true;
        instance_table["type"] = type;
        
        actor.components.insert({key, instance_table});
        return;
    }
    else if (type == "ParticleSystem") {
        ParticleSystem* new_component = new ParticleSystem(); 
        luabridge::LuaRef instance_table(state, new_component); 
        
        instance_table["key"] = key;
        instance_table["enabled"] = true;
        instance_table["type"] = type;
        
        actor.components.insert({key, instance_table});
        return;
    }
    
    auto cache_find = component_cache.find(type);
    if (cache_find == component_cache.end()) {
        std::string path = "resources/component_types/" + type + ".lua";
        if (!std::filesystem::exists(path)){
            std::cout << "error: failed to locate component " << type;
            exit(0);
        }
        if (luaL_dofile(state, path.c_str())!=LUA_OK){
            std::cout << "problem with lua file " << type;
            exit(0);
        }
        component_cache.emplace(type, luabridge::getGlobal(state, type.c_str()));
    }

    luabridge::LuaRef class_table = luabridge::getGlobal(state, type.c_str());
    luabridge::LuaRef instance_table = luabridge::newTable(state);
    EstablishInheritance(instance_table, class_table, state);
    instance_table["key"] = key;
    instance_table["enabled"] = true;
    instance_table["type"] = type;

    actor.components.insert({key, instance_table});
}

inline void overrideLuaComponent(luabridge::LuaRef& component, const rapidjson::Value& json){
    for (auto& c : json.GetObject()){
        std::string name = c.name.GetString();
        if (name == "type") continue;

        if (c.value.IsString()){
            component[name] = c.value.GetString();
        } else if (c.value.IsBool()){
            component[name] = c.value.GetBool();
        } else if (c.value.IsInt()){
            component[name] = c.value.GetInt();
        } else if (c.value.IsDouble()){
            component[name] = c.value.GetDouble();
        }
    }
}

static void ReportError(const std::string& actor_name, const luabridge::LuaException& e) {
    std::string error_message = e.what();
    
    std::replace(error_message.begin(), error_message.end(), '\\', '/');
    
    std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
}

#endif /* EngineUtils_h */
