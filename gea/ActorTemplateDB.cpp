//
//  ActorTemplateDB.cpp
//  game_engine
//
//  Created by Gracie Hou on 1/30/26.
//

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include "Actor.h"
#include "ActorTemplateDB.h"
#include "../src/rapidjson/filereadstream.h"
#include "../src/rapidjson/document.h"
#include "EngineUtils.h"

using namespace std;
static unordered_map<string, rapidjson::Document> template_cache;

void ActorTemplateDB::LoadTemplate(const std::string& name, Actor& actor, lua_State* state){
    
    if (template_cache.find(name) == template_cache.end()) {
        string path = "resources/actor_templates/"+name+".template";
        
        if (!filesystem::exists(path)) {
            cout << "error: template " << name << " is missing";
            exit(0);
        }

        rapidjson::Document doc;
        ReadJsonFile(path, doc);
        template_cache.emplace(name, std::move(doc));
    }

    const rapidjson::Document& doc = template_cache[name];

    if (doc.HasMember("components")) {
        for (auto& component : doc["components"].GetObject()) {
            string key = component.name.GetString();

            if (component.value.HasMember("type")){
                string type = component.value["type"].GetString();
                addLuaComponent(actor, key, type, state);

                if (actor.components.count(key)){
                    overrideLuaComponent(actor.components.at(key), component.value);
                }
            }
            
        }
    }

    ApplyJsonToActor(actor, doc);
}