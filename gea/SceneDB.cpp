//
//  SceneDB.cpp
//  game_engine
//
//  Created by Gracie Hou on 1/29/26.
//

#include <filesystem>
#include <iostream>
#include <fstream>
#include "../src/rapidjson/document.h"
#include "SceneDB.h"
#include "ActorTemplateDB.h"
#include "EngineUtils.h"

using namespace std;
extern int global_actor_id_counter;

void SceneDB::LoadScene(const string& sceneName, vector<Actor*>& actors, vector<Actor*>& actors_to_add, lua_State* state){

    string path = "resources/scenes/"+sceneName+".scene";
    
    if (!filesystem::exists(path)) {
        cout << "error: scene " << sceneName << " is missing";
        exit(0);
    }

    rapidjson::Document doc;
    ReadJsonFile(path, doc);

    // for (auto* actor : actors) { 
    //     delete actor;
    // }
    // actors.clear();

    if (doc.HasMember("actors") && doc["actors"].IsArray()) {        
        for (auto& actor : doc["actors"].GetArray()) {
            Actor *newActor=new Actor();
            newActor->lua_state = state;

            //check if template available
            if (actor.HasMember("template")) {
                string templateName = actor["template"].GetString();
                ActorTemplateDB::LoadTemplate(templateName, *newActor, state); 
            }

            newActor->id = global_actor_id_counter;
            global_actor_id_counter++;

            if (actor.HasMember("components") && actor["components"].IsObject()) {
                const rapidjson::Value& component = actor["components"];
                
                for (auto it = component.MemberBegin(); it!=component.MemberEnd(); it++){
                    string key = it->name.GetString();

                    if (newActor->components.count(key)){
                        overrideLuaComponent(newActor->components.at(key), it->value);
                    } else if (it->value.HasMember("type")){
                        string type = it->value["type"].GetString();
                        addLuaComponent(*newActor, key, type, state);
                        overrideLuaComponent(newActor->components.at(key), it->value);
                    }
                }
            }

            ApplyJsonToActor(*newActor, actor);
            // actors.push_back(newActor);
            newActor->InjectReferences(state);
            actors_to_add.push_back(newActor);
        }

        // for (auto &actor : actors){
        //     actor->InjectReferences(state);
        //     actors_to_add.push_back(actor);
        // }
    }

}
