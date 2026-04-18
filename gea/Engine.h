//
//  Engine.hpp
//  game_engine
//
//  Created by Gracie Hou on 1/26/26.
//

#ifndef Engine_h
#define Engine_h

#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <queue>
#include <unordered_map>
#include "../src/glm/glm.hpp"
#include "../src/rapidjson/document.h"
#include "Actor.h"
#include "Collision.h"
#include "../Helper.h"
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"
#include "../src/box2d/include/box2d/box2d.h"

struct TextDrawRequest {
    std::string content;
    int x, y;
    int font_size;
    int r, g, b, a;
    std::string font_name;
};
struct PixelDrawRequest {
    float x, y;
    int r, g, b, a;
};

class Engine {
public:
    Engine();
    void GameLoop();
    void Dialogue(Actor& actor, std::string current_dialogue, bool render_text, int curr_actor_index);
    void Update();
    void Render();

    Actor* Instantiate(std::string actor_template_name);
    void Destroy(Actor* actor);
    Actor* Find(const std::string& name);
    std::vector<Actor*> FindAll(const std::string& name);

    std::queue<TextDrawRequest> text_draw_queue;
    std::queue<PixelDrawRequest> pixel_draw_queue;
    float zoom_factor = 1.0f;
    glm::vec2 camera_pos = {0.0f, 0.0f};
    std::string current_scene = "";
    std::string next_scene = "";
    b2World* physics_world = nullptr;
    ContactListener contact_listener;

private:
    lua_State* lua_state = nullptr;
    std::vector<Actor*> actors_to_add;
    std::vector<Actor*> actors_to_remove;

    Actor* player_ptr = nullptr;
    std::vector<const Actor*> sorted_actors;
    
    float COLLIDER_CELL_SIZE = 1.0f;
    float TRIGGER_CELL_SIZE = 1.0f;
    float max_trigger_dim = 1.0f;
    float render_buffer = 2.0f;
    std::unordered_multimap<uint64_t, Actor*> collider_map;
    std::unordered_multimap<uint64_t, Actor*> trigger_map;
    bool running;
    
    std::string game_start_message;
    std::string game_over_bad_message;
    std::string game_over_good_message;
    bool game_over_audio_played = false;

    // std::unordered_multimap<uint64_t, Actor*> actor_map;
    std::vector<Actor*> actors;
    std::vector<int> actor_points;
    std::vector<std::pair<int, std::string>> pending_dialogue;
    std::vector<std::string> intro_images;
    std::vector<std::string> intro_texts;
    std::string game_font = "";
    std::string intro_bgm = "";
    bool audio_started = false;
    int last_damage = -9999;
    float cam_ease_factor = 1.0f;

    int image_index = 0;

    void RenderMap();

    int camera_width = 13;
    int camera_height = 9;
    int camera_half_width = 6;
    int camera_half_height = 4;
    float camera_offset_x = 0.0f;
    float camera_offset_y = 0.0f;
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int clear_r = 255;
    int clear_g = 255;
    int clear_b = 255;

    int window_width = 640;
    int window_height = 360;
};

#endif /* Engine_h */
