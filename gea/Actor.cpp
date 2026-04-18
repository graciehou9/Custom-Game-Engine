//
//  Actor.cpp
//  game_engine
//
//  Created by Gracie Hou on 3/3/26.
//

#include <stdio.h>
#include "Actor.h"
#include "EngineUtils.h"
#include "Engine.h"

using namespace std;

static int runtime_added_counter = 0;
int global_actor_id_counter = 0;
extern Engine* engine_ptr;


luabridge::LuaRef Actor::GetComponentByKey(std::string key, lua_State* state){
    if (components.count(key)){
        if (!components.at(key)["enabled"]){
            return luabridge::LuaRef(state);
        }
        return components.at(key);
    }
    return luabridge::LuaRef(state);
}

luabridge::LuaRef Actor::GetComponent(std::string type_name, lua_State* state){
    for (auto& [key, component]:components){
        if (!component["enabled"]) continue;
        luabridge::LuaRef type = component["type"];
        if (type.isString() && type.cast<std::string>() == type_name){
            return component;
        }
    }
    return luabridge::LuaRef(state);
}
luabridge::LuaRef Actor::GetComponents(std::string type_name, lua_State* state){
    luabridge::LuaRef table = luabridge::newTable(state);
    int i = 1;
    for (auto& [key, component]:components){
        if (!component["enabled"]) continue; 
        luabridge::LuaRef type = component["type"];
        if (type.isString() && type.cast<std::string>() == type_name){
            table[i] = component;
            i++;
        }
    }
    return table;
}

void Actor::InjectReferences(lua_State* L){
    for (auto& [key, component]:components){
        component["actor"]=this;
        component["key"]=key;
    }
}

luabridge::LuaRef Actor::AddComponent(string type_name){
    string key = "r" + to_string(runtime_added_counter++);
    addLuaComponent(*this, key, type_name, lua_state);
    luabridge::LuaRef& component = components.at(key);
    component["actor"] = this;
    component["key"]=key;
    component["enabled"] = true;
    components_to_add.push_back(key);
    return component;
}

void Actor::RemoveComponent(luabridge::LuaRef component_ref){
    component_ref["enabled"]=false;
    string key = component_ref["key"];
    components_to_remove.push_back(key);
}