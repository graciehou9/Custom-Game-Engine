//
//  Actor.hpp
//  game_engine
//
//  Created by Gracie Hou on 1/30/26.
//

#ifndef Actor_h
#define Actor_h

#include <stdio.h>
#include <string>
#include <optional>
#include <vector>
#include <map>
#include "../src/glm/glm.hpp"
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"

// static int global_actor_id_counter = 0;

struct Actor
{
public:
    int id=0;
	std::string actor_name="";
	std::map<std::string, luabridge::LuaRef> components;
	std::vector<std::string> components_to_add;
	std::vector<std::string> components_to_remove;
	bool destroyed = false;
	bool destroy_on_load = true;

	std::map<std::string, bool> component_started;
    std::map<std::string, bool> components_with_update;
    std::map<std::string, bool> components_with_late_update;

	lua_State* lua_state = nullptr; //to same lua_state as engine.h

	char view='?';
	std::string view_image="";
	std::string view_image_back = "";
	std::string current_view_image = "";
	bool movement_bounce_enabled = false;

	glm::vec2 position={0,0};
	glm::vec2 velocity={0,0};
	bool blocking=false;
	
	
	glm::vec2 transform_scale={1.0f, 1.0f};
	float transform_rotation_degrees=0.0f;

	std::optional<float> view_pivot_offset_x;
    std::optional<float> view_pivot_offset_y;
	std::optional<float> box_collider_width;
	std::optional<float> box_collider_height;
	std::optional<float> box_trigger_width;
	std::optional<float> box_trigger_height;

	bool player_in_trigger = false;
	bool player_in_contact = false;
	bool inverted_this_frame = false;

    std::string nearby_dialogue_sfx = "";
    bool has_played_dialogue_sfx = false;

	std::string view_image_damage = "";
    std::string view_image_attack = "";

	std::vector<Actor*> colliding_actors_this_frame;
    int attack_frame_timer = 0;

	std::optional<int> render_order;

	std::string GetName() const {
		return actor_name;
	}

	int GetID() const {
		return id;
	}

	luabridge::LuaRef GetComponentByKey(std::string key, lua_State* state);
	luabridge::LuaRef GetComponent(std::string type, lua_State* state);
	luabridge::LuaRef GetComponents(std::string type, lua_State* state);

	void InjectReferences(lua_State* L);
	

	luabridge::LuaRef AddComponent(std::string type_name);
	void RemoveComponent(luabridge::LuaRef component_ref);


	Actor(std::string actor_name, char view, glm::vec2 position, glm::vec2 initial_velocity,
		bool blocking, std::string nearby_dialogue, std::string contact_dialogue)
		: id(0), actor_name(actor_name), view(view), position(position), velocity(initial_velocity), blocking(blocking) {
            //TODO: do something like id = g_uuid, g_uuid++ (where g_uuid is a global variable)
			// add to cpp? or just keep here? or just keep in the scenedb
			// id = global_actor_id_counter;
			// global_actor_id_counter++;
    }

	Actor() : id(0) {}
	// Actor();
};

#endif /* Actor_h */
