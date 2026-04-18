//
//  Engine.cpp
//  game_engine
//
//  Created by Gracie Hou on 1/26/26.
//

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../src/rapidjson/filereadstream.h"
#include "../src/rapidjson/document.h"
#include "Engine.h"
#include "SceneDB.h"
#include "EngineUtils.h"
#include "ImageDB.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "Input.h"
#include "ActorTemplateDB.h"
#include "Rigidbody.h"
#include "Raycast.h"
#include "EventBus.h"
#include "ParticleSystem.h"
using namespace std;

static vector<Actor*>* helper_actors = nullptr;
extern int global_actor_id_counter;
Engine* engine_ptr = nullptr;


static uint64_t string_hash(int x, int y) {
    uint32_t ux = static_cast<uint32_t>(x);
    uint32_t uy = static_cast<uint32_t>(y);
    return (static_cast<uint64_t>(ux) << 32) | uy;
}

static luabridge::LuaRef FindActor(string name, lua_State* state){
    if (engine_ptr) {
        Actor* found = engine_ptr->Find(name); 
        if (found) {
            return luabridge::LuaRef(state, found);
        }
    }
    return luabridge::LuaRef(state);
}

static luabridge::LuaRef FindAllActor(string name, lua_State* state){
    luabridge::LuaRef table = luabridge::newTable(state);
    if (engine_ptr) {
        vector<Actor*> found_actors = engine_ptr->FindAll(name);
        int i = 1;
        for (Actor* actor : found_actors) {
            table[i] = actor;
            i++;
        }
    }
    return table;
}

void CppLog(std::string message) {
    std::cout << message << std::endl;
}
void CppLogError(std::string message) {
    std::cout << message << std::endl;
}

void AppQuit() {
    exit(0);
}
void AppSleep(int ms_time){
    this_thread::sleep_for(std::chrono::milliseconds(ms_time));
}
static int AppGetFrame(){
    return Helper::GetFrameNumber();
}
static void AppOpenURL(string url){
    #ifdef _WIN32
        system(("start " + url).c_str());
    #elif __APPLE__
        system(("open " + url).c_str());
    #else
        system(("xdg-open "+url).c_str());
    #endif
}

static Actor* InstantiateActor(string name) {
    return engine_ptr->Instantiate(name);
}
static void DestroyActor(Actor* actor) {
    engine_ptr->Destroy(actor);
}

static void TextDraw(string content, float x, float y, string font_name, float font_size, float r, float g, float b, float a) {
    TextDrawRequest req;
    req.content = content;
    req.x = (int)x;
    req.y = (int)y;
    req.font_name = font_name;
    req.font_size = (int)font_size;
    req.r = (int)r;
    req.g = (int)g;
    req.b = (int)b;
    req.a = (int)a;
    engine_ptr->text_draw_queue.push(req);
}

static void AudioPlay(int channel, std::string clip_name, bool does_loop) {
    if (does_loop){
        AudioHelper::Mix_PlayChannel(channel, AudioDB::LoadSound(clip_name), -1);
    } else {
        AudioHelper::Mix_PlayChannel(channel, AudioDB::LoadSound(clip_name), 0);
    }
}
static void AudioHalt(int channel) {
    AudioHelper::Mix_HaltChannel(channel);
}
static void AudioVolume(int channel, float volume) {
    AudioHelper::Mix_Volume(channel, static_cast<int>(volume));
}

static void DrawUI(string image_name, float x, float y){
    ImageDrawRequest req;
    req.image_name = image_name;
    req.x = static_cast<int>(x);
    req.y = static_cast<int>(y);
    req.rotation_degrees = 0;
    req.scale_x = 1.0f;
    req.scale_y = 1.0f;
    req.pivot_x = 0.5f;
    req.pivot_y = 0.5f;
    req.r = 255;
    req.g = 255;
    req.b = 255;
    req.a = 255;
    req.sorting_order = 0;
    req.is_ui = true;
    ImageDB::image_draw_queue.push_back(req);
}
static void DrawUIEx(string image_name, float x, float y, float r, float g, float b, float a, float sorting_order){
    ImageDrawRequest req;
    req.image_name = image_name;
    req.x = static_cast<int>(x);
    req.y = static_cast<int>(y);
    req.rotation_degrees = 0;
    req.scale_x = 1.0f;
    req.scale_y = 1.0f;
    req.pivot_x = 0.5f;
    req.pivot_y = 0.5f;
    req.r = static_cast<int>(r);
    req.g = static_cast<int>(g);
    req.b = static_cast<int>(b);
    req.a = static_cast<int>(a);
    req.sorting_order = static_cast<int>(sorting_order);
    req.is_ui = true;
    ImageDB::image_draw_queue.push_back(req);
}
static void ImageDraw(string image_name, float x, float y) {
    ImageDrawRequest req;
    req.image_name = image_name;
    req.x = x;
    req.y = y;
    req.rotation_degrees = 0;
    req.scale_x = 1.0f;
    req.scale_y = 1.0f;
    req.pivot_x = 0.5f;
    req.pivot_y = 0.5f;
    req.r = 255;
    req.g = 255;
    req.b = 255;
    req.a = 255;
    req.is_ui = false;
    ImageDB::image_draw_queue.push_back(req);
}
static void ImageDrawEx(string image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
    ImageDrawRequest req;
    req.image_name = image_name;
    req.x = x;
    req.y = y;
    req.rotation_degrees = static_cast<int>(rotation_degrees);
    req.scale_x = scale_x;
    req.scale_y = scale_y;
    req.pivot_x = pivot_x;
    req.pivot_y = pivot_y;
    req.r = static_cast<int>(r);
    req.g = static_cast<int>(g);
    req.b = static_cast<int>(b);
    req.a = static_cast<int>(a);
    req.sorting_order = static_cast<int>(sorting_order);
    req.is_ui = false;
    ImageDB::image_draw_queue.push_back(req);
}

static void ImageDrawPixel(float x, float y, float r, float g, float b, float a){
    PixelDrawRequest req;
    req.x = static_cast<int>(x);
    req.y = static_cast<int>(y);
    req.r = static_cast<int>(r);
    req.g = static_cast<int>(g);
    req.b = static_cast<int>(b);
    req.a = static_cast<int>(a);
    engine_ptr->pixel_draw_queue.push(req);
}

static void CameraSetPosition(float x, float y){
    engine_ptr->camera_pos.x = x;
    engine_ptr->camera_pos.y = y;
}
static float CameraGetPositionX(){
    return engine_ptr->camera_pos.x;
}
static float CameraGetPositionY(){
    return engine_ptr->camera_pos.y;
}
static void CameraSetZoom(float zoom_factor){
    engine_ptr->zoom_factor = zoom_factor;
}
static float CameraGetZoom(){
    return engine_ptr->zoom_factor;
}

static void SceneLoad(string scene_name){
    engine_ptr->next_scene= scene_name;
}
static string SceneGetCurrent(){
    return engine_ptr->current_scene;
}
static void SceneDontDestroy(luabridge::LuaRef actor_ref){
    Actor* actor = actor_ref.cast<Actor*>();
    actor->destroy_on_load = false;
}

Actor* Engine::Find(const string& name) {
    for (Actor* actor : actors) {
        if (!actor->destroyed && actor->GetName() == name) return actor;
    }
    for (Actor* actor : actors_to_add) {
        if (!actor->destroyed && actor->GetName() == name) return actor;
    }
    return nullptr;
}
vector<Actor*> Engine::FindAll(const string& name) {
    vector<Actor*> result;
    for (Actor* actor : actors) {
        if (!actor->destroyed && actor->GetName() == name) result.push_back(actor);
    }
    for (Actor* actor : actors_to_add) {
        if (!actor->destroyed && actor->GetName() == name) result.push_back(actor);
    }
    return result;
}
Actor* Engine::Instantiate(string actor_template_name){
    Actor* new_actor = new Actor();
    new_actor->id = global_actor_id_counter++;
    new_actor->lua_state = lua_state;
    ActorTemplateDB::LoadTemplate(actor_template_name, *new_actor, lua_state);
    new_actor->InjectReferences(lua_state);
    actors_to_add.push_back(new_actor);
    return new_actor;
}
void Engine::Destroy(Actor* actor) {
    if (actor == player_ptr) player_ptr = nullptr;
    actor->destroyed = true;
    for (auto& [key, comp] : actor->components) {
        comp["enabled"] = false;
    }
    actors_to_remove.push_back(actor);

    actors_to_add.erase(remove(actors_to_add.begin(), actors_to_add.end(), actor), actors_to_add.end());
}

Engine::Engine() {
    engine_ptr = this;
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    helper_actors = &actors;

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*)(float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .addStaticFunction("Distance", &b2Distance)
        .addStaticFunction("Dot", static_cast<float(*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .endClass();

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Rigidbody>("Rigidbody")
        .addConstructor<void(*)()>()
        .addProperty("key", &Rigidbody::key)
        .addProperty("type", &Rigidbody::type)
        .addProperty("enabled", &Rigidbody::enabled)
        .addProperty("actor", &Rigidbody::actor)
        .addProperty("x", &Rigidbody::x)
        .addProperty("y", &Rigidbody::y)
        .addProperty("body_type", &Rigidbody::body_type)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravity_scale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angular_friction)
        .addProperty("rotation", &Rigidbody::rotation)
        .addProperty("width", &Rigidbody::width)
        .addProperty("height", &Rigidbody::height)
        .addProperty("has_collider", &Rigidbody::has_collider)
        .addProperty("has_trigger", &Rigidbody::has_trigger)
        .addProperty("collider_type", &Rigidbody::collider_type)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .addProperty("trigger_type", &Rigidbody::trigger_type)
        .addProperty("trigger_width", &Rigidbody::trigger_width)
        .addProperty("trigger_height", &Rigidbody::trigger_height)
        .addProperty("trigger_radius", &Rigidbody::trigger_radius)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("OnStart", &Rigidbody::OnStart)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .endClass();

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Collision>("Collision")
        .addProperty("other", &Collision::other)
        .addProperty("point", &Collision::point)
        .addProperty("relative_velocity", &Collision::relative_velocity)
        .addProperty("normal", &Collision::normal)
        .endClass();

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("normal", &HitResult::normal)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .endClass();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Physics")
        .addFunction("Raycast", Physics_Raycast)
        .addFunction("RaycastAll", Physics_RaycastAll)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Event")
        .addFunction("Publish", EventBus::Publish)
        .addFunction("Subscribe", EventBus::Subscribe)
        .addFunction("Unsubscribe", EventBus::Unsubscribe)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addFunction("AddComponent", &Actor::AddComponent)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Actor")
        .addFunction("Find", FindActor)
        .addFunction("FindAll", FindAllActor)
        .addFunction("Instantiate", InstantiateActor)
        .addFunction("Destroy", DestroyActor)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Debug")
        .addFunction("Log", CppLog)
        .addFunction("LogError", CppLogError)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Application")
        .addFunction("Quit", AppQuit)
        .addFunction("Sleep", AppSleep)
        .addFunction("GetFrame", AppGetFrame)
        .addFunction("OpenURL", AppOpenURL)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Input")
        .addFunction("GetKey", static_cast<bool(*)(std::string)>(Input::GetKey))
        .addFunction("GetKeyDown", static_cast<bool(*)(std::string)>(Input::GetKeyDown))
        .addFunction("GetKeyUp", static_cast<bool(*)(std::string)>(Input::GetKeyUp))
        .addFunction("GetMousePosition", Input::GetMousePosition)
        .addFunction("GetMouseButton", Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", Input::GetMouseButtonUp)
        .addFunction("GetMouseScrollDelta", Input::GetMouseScrollDelta)
        .addFunction("HideCursor", Input::HideCursor)
        .addFunction("ShowCursor", Input::ShowCursor)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Text")
        .addFunction("Draw", TextDraw)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Audio")
        .addFunction("Play", AudioPlay)
        .addFunction("Halt", AudioHalt)
        .addFunction("SetVolume", AudioVolume)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Image")
        .addFunction("Draw", ImageDraw)
        .addFunction("DrawEx", ImageDrawEx)
        .addFunction("DrawUI", DrawUI)
        .addFunction("DrawUIEx", DrawUIEx)
        .addFunction("DrawPixel", ImageDrawPixel)
        .endNamespace();
    
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Camera")
        .addFunction("SetPosition", CameraSetPosition)
        .addFunction("GetPositionX", CameraGetPositionX)
        .addFunction("GetPositionY", CameraGetPositionY)
        .addFunction("SetZoom", CameraSetZoom)
        .addFunction("GetZoom", CameraGetZoom)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Scene")
        .addFunction("Load", SceneLoad)
        .addFunction("GetCurrent", SceneGetCurrent)
        .addFunction("DontDestroy", SceneDontDestroy)
        .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
        .beginClass<ParticleSystem>("ParticleSystem")
        .addConstructor<void(*)()>()
        .addProperty("key", &ParticleSystem::key)
        .addProperty("type", &ParticleSystem::type)
        .addProperty("enabled", &ParticleSystem::enabled)
        .addProperty("actor", &ParticleSystem::actor)
        .addProperty("x", &ParticleSystem::x)
        .addProperty("y", &ParticleSystem::y)
        .addProperty("frames_between_bursts", &ParticleSystem::frames_between_bursts)
        .addProperty("burst_quantity", &ParticleSystem::burst_quantity)
        .addProperty("start_scale_min", &ParticleSystem::start_scale_min)
        .addProperty("start_scale_max", &ParticleSystem::start_scale_max)
        .addProperty("rotation_min", &ParticleSystem::rotation_min)
        .addProperty("rotation_max", &ParticleSystem::rotation_max)
        .addProperty("start_color_r", &ParticleSystem::start_color_r)
        .addProperty("start_color_g", &ParticleSystem::start_color_g)
        .addProperty("start_color_b", &ParticleSystem::start_color_b)
        .addProperty("start_color_a", &ParticleSystem::start_color_a)
        .addProperty("emit_radius_min", &ParticleSystem::emit_radius_min)
        .addProperty("emit_radius_max", &ParticleSystem::emit_radius_max)
        .addProperty("emit_angle_min", &ParticleSystem::emit_angle_min)
        .addProperty("emit_angle_max", &ParticleSystem::emit_angle_max)
        .addProperty("image", &ParticleSystem::image)
        .addProperty("sorting_order", &ParticleSystem::sorting_order)
        .addProperty("duration_frames", &ParticleSystem::duration_frames)
        .addProperty("start_speed_min", &ParticleSystem::start_speed_min)
        .addProperty("start_speed_max", &ParticleSystem::start_speed_max)
        .addProperty("rotation_speed_min", &ParticleSystem::rotation_speed_min)
        .addProperty("rotation_speed_max", &ParticleSystem::rotation_speed_max)
        .addProperty("gravity_scale_x", &ParticleSystem::gravity_scale_x)
        .addProperty("gravity_scale_y", &ParticleSystem::gravity_scale_y)
        .addProperty("drag_factor", &ParticleSystem::drag_factor)
        .addProperty("angular_drag_factor", &ParticleSystem::angular_drag_factor)
        .addProperty("end_scale", &ParticleSystem::end_scale)
        .addProperty("end_color_r", &ParticleSystem::end_color_r)
        .addProperty("end_color_g", &ParticleSystem::end_color_g)
        .addProperty("end_color_b", &ParticleSystem::end_color_b)
        .addProperty("end_color_a", &ParticleSystem::end_color_a)
        .addFunction("OnStart", &ParticleSystem::OnStart)
        .addFunction("OnUpdate", &ParticleSystem::OnUpdate)
        .addFunction("OnDestroy", &ParticleSystem::OnDestroy)
        .addFunction("Play", &ParticleSystem::Play)
        .addFunction("Stop", &ParticleSystem::Stop)
        .addFunction("Burst", &ParticleSystem::Burst)
        .endClass();

    running = true;

    string gameTitle = "";
   
    if (!std::filesystem::exists("resources/")) {
        cout << "error: resources/ missing";
        exit(0);
    }
    string gameConfig = "resources/game.config";
    if (!std::filesystem::exists(gameConfig)) {
        cout << "error: resources/game.config missing";
        exit(0);
    }
    //read game config
    rapidjson::Document gameDoc;
    ReadJsonFile(gameConfig, gameDoc);
    if (gameDoc.HasMember("game_start_message")) game_start_message = gameDoc["game_start_message"].GetString();
    if (gameDoc.HasMember("game_over_bad_message")) game_over_bad_message = gameDoc["game_over_bad_message"].GetString();
    if (gameDoc.HasMember("game_over_good_message")) game_over_good_message = gameDoc["game_over_good_message"].GetString();
    if (gameDoc.HasMember("game_title")) gameTitle = gameDoc["game_title"].GetString();
    if (gameDoc.HasMember("intro_image")) {
        if (gameDoc["intro_image"].IsArray()) {
            const rapidjson::Value& image = gameDoc["intro_image"];
            for (rapidjson::SizeType i = 0; i < image.Size(); i++) {
                intro_images.push_back(image[i].GetString());
            }
        }
    } 
    if (gameDoc.HasMember("intro_text")) {
        if (gameDoc["intro_text"].IsArray()) {
            const rapidjson::Value& txt = gameDoc["intro_text"];
            for (rapidjson::SizeType i = 0; i < txt.Size(); i++) {
                intro_texts.push_back(txt[i].GetString());
            }
        }
    }
    if (gameDoc.HasMember("font")) game_font = gameDoc["font"].GetString();
    if (gameDoc.HasMember("intro_bgm")) intro_bgm = gameDoc["intro_bgm"].GetString();

    AudioDB::Init();
    if (!intro_bgm.empty()) {
        AudioDB::PlaySound(intro_bgm);
    }

    //read rendering config
    string renderConfig = "resources/rendering.config";
    if (std::filesystem::exists(renderConfig)) {
        rapidjson::Document renderDoc;
        ReadJsonFile(renderConfig, renderDoc);
        if (renderDoc.HasMember("x_resolution")) window_width = renderDoc["x_resolution"].GetInt();
        if (renderDoc.HasMember("y_resolution")) window_height = renderDoc["y_resolution"].GetInt();
        if (renderDoc.HasMember("clear_color_r")) clear_r = renderDoc["clear_color_r"].GetInt();
        if (renderDoc.HasMember("clear_color_g")) clear_g = renderDoc["clear_color_g"].GetInt();
        if (renderDoc.HasMember("clear_color_b")) clear_b = renderDoc["clear_color_b"].GetInt();
        if (renderDoc.HasMember("cam_offset_x")) camera_offset_x = renderDoc["cam_offset_x"].GetFloat();
        if (renderDoc.HasMember("cam_offset_y")) camera_offset_y = renderDoc["cam_offset_y"].GetFloat();
        if (renderDoc.HasMember("zoom_factor")) zoom_factor = renderDoc["zoom_factor"].GetFloat();
        if (renderDoc.HasMember("cam_ease_factor")) cam_ease_factor = renderDoc["cam_ease_factor"].GetFloat();
    }

    window = Helper::SDL_CreateWindow(gameTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN);
    renderer = Helper::SDL_CreateRenderer(window,-1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    ImageDB::CreateDefaultParticleTextureWithName("default_p", renderer);
    for (const string& img : intro_images) {
        ImageDB::LoadImage(img, renderer);
    }
    if (!intro_texts.empty() && game_font.empty()) {
        cout << "error: text render failed. No font configured";
        exit(0);
    }
    
    TextDB::Init();
    TextDB::setRenderer(renderer);
    if (!game_font.empty()) {
        TextDB::LoadText(game_font, 16);
    }
    
    string initialScene = "";
    if (gameDoc.HasMember("initial_scene")) {
        initialScene = gameDoc["initial_scene"].GetString();
    }
    if (initialScene == "") {
        cout << "error: initial_scene unspecified";
        exit(0);
    }

    current_scene = initialScene;
    SceneDB::LoadScene(initialScene, actors, actors_to_add, lua_state);

    //set cell grid size
    COLLIDER_CELL_SIZE = 1.0f;
    TRIGGER_CELL_SIZE = 2.0f;
    for (auto& a : actors) {
        float current_w = a->box_collider_width.value_or(0.0f)* glm::abs(a->transform_scale.x);
        float current_h = a->box_collider_height.value_or(0.0f)* glm::abs(a->transform_scale.y);
        if (current_w > COLLIDER_CELL_SIZE) COLLIDER_CELL_SIZE = current_w;
        if (current_h > COLLIDER_CELL_SIZE) COLLIDER_CELL_SIZE = current_h;
    }
    COLLIDER_CELL_SIZE += 0.1f;
    max_trigger_dim = 1.0f;
    for (auto& a : actors) {
        float trigger_w = a->box_trigger_width.value_or(0.0f) * glm::abs(a->transform_scale.x);
        float trigger_h = a->box_trigger_height.value_or(0.0f) * glm::abs(a->transform_scale.y);
        if (trigger_w > max_trigger_dim) max_trigger_dim = trigger_w;
        if (trigger_h > max_trigger_dim) max_trigger_dim = trigger_h;
    }

    bool has_player = false;
    for (const auto& a : actors) {
        if (a->actor_name == "player") {
            camera_pos = a->position;
            camera_pos.x += camera_offset_x;
            camera_pos.y += camera_offset_y;
            has_player = true;
            break;
        }
    }
    
    RenderMap();
    actor_points.resize(actors.size(), 0);
    Input::Init();
}

void Engine::RenderMap() {
    collider_map.clear();
    trigger_map.clear();
    render_buffer = 2.0f;
    for (Actor* actor : actors) {
        int cx = static_cast<int>(floor(actor->position.x/COLLIDER_CELL_SIZE));
        int cy = static_cast<int>(floor(actor->position.y/COLLIDER_CELL_SIZE));
        uint64_t key = string_hash(cx, cy);
        collider_map.insert({key, actor});

        int tx = static_cast<int>(floor(actor->position.x/TRIGGER_CELL_SIZE));
        int ty = static_cast<int>(floor(actor->position.y/TRIGGER_CELL_SIZE));
        key = string_hash(tx, ty);
        trigger_map.insert({key, actor});

        //rendering actors buffer limit init
        if (actor->view_image.empty()) continue;
        SDL_Texture* texture = ImageDB::LoadImage(actor->view_image, renderer);
        float w, h;
        Helper::SDL_QueryTexture(texture, &w, &h);
        float half_actor_w = (w * glm::abs(actor->transform_scale.x))/100.0f;
        float half_actor_h = (h * glm::abs(actor->transform_scale.y))/100.0f;
        render_buffer = glm::max(render_buffer, glm::max(half_actor_w, half_actor_h));
    }
}

void Engine::Update() {
    if (!actors_to_add.empty()) {
        for (Actor* actor : actors_to_add) {
            actors.push_back(actor);
        }
        actors_to_add.clear();
    }

    for (Actor* actor : actors) {
        if (actor->destroyed) continue;
        for (auto& [key, component] : actor->components) {
            if (actor->component_started[key]) continue;
            if (!component["enabled"]) continue;
            if (!actor->components_to_add.empty()) {
                if (find(actor->components_to_add.begin(), actor->components_to_add.end(), key) != actor->components_to_add.end()) {
                    continue;
                }
            }

            if (!actor->component_started[key]) {
                luabridge::LuaRef started = component["started"];
                if (started.isNil()) {
                    // component["started"] = true;
                    luabridge::LuaRef type_ref = component["type"];
                    if (type_ref.isString() && type_ref.cast<std::string>() != "Rigidbody" && type_ref.cast<std::string>() != "ParticleSystem") {
                        component["started"] = true;
                    }

                    luabridge::LuaRef OnStart = component["OnStart"];
                    if (OnStart.isFunction()) {
                        try { OnStart(component); }
                        catch (const luabridge::LuaException& e) { ReportError(actor->GetName(), e); }
                    }
                }
                actor->component_started[key] = true;
                actor->components_with_update[key] = component["OnUpdate"].isFunction();
                actor->components_with_late_update[key] = component["OnLateUpdate"].isFunction();
            }
        }
    }
    
    for (Actor* actor : actors) {
        if (actor->destroyed) continue;
        for (auto& [key, component] : actor->components) {
            if (!actor->component_started[key]) continue;
            if (!actor->components_with_update[key]) continue;
            if (!component["enabled"]) continue;
            
            luabridge::LuaRef OnUpdate = component["OnUpdate"];
            if (OnUpdate.isFunction()){
                try{ OnUpdate(component); }
                catch (const luabridge::LuaException& e){ ReportError(actor->GetName(), e); }
            }
        }
    }

    for (Actor* actor : actors) {
        if (actor->destroyed) continue;
        for (auto& [key, component] : actor->components) {
            if (!actor->component_started[key]) continue;
            if (!actor->components_with_late_update[key]) continue;
            if (!component["enabled"]) continue;
            
            luabridge::LuaRef OnLateUpdate = component["OnLateUpdate"];
            if (OnLateUpdate.isFunction()){
                try{ OnLateUpdate(component); }
                catch (const luabridge::LuaException& e){ ReportError(actor->GetName(), e); }
            }
        }
    }

    //removing actors
    for (auto* actor : actors) {
        if (actor->destroyed) continue;
        if (!actor->components_to_remove.empty()) {
            vector<string> keys_to_remove = actor->components_to_remove;
            sort(keys_to_remove.begin(), keys_to_remove.end());

            for (auto& key : keys_to_remove) {
                if (actor->components.find(key) != actor->components.end()) {
                    luabridge::LuaRef comp = actor->components.at(key);
                    luabridge::LuaRef OnDestroy = comp["OnDestroy"];
                    if (OnDestroy.isFunction()) {
                        try { OnDestroy(comp); }
                        catch (const luabridge::LuaException& e) {
                            ReportError(actor->GetName(), e);
                        }
                    }
                    actor->components.erase(key);
                    actor->component_started.erase(key);
                    actor->components_with_update.erase(key);
                    actor->components_with_late_update.erase(key);
                }
            }
            actor->components_to_remove.clear();
        }
        actor->components_to_add.clear();
    }
    
    for (Actor* actor : actors_to_remove) {
        // if (!actor->components_to_remove.empty()) {
        //     for (auto& key : actor->components_to_remove) {
        //         actor->components.erase(key);
        //         actor->component_started.erase(key);
        //         actor->components_with_update.erase(key);
        //         actor->components_with_late_update.erase(key);
        //     }
        //     actor->components_to_remove.clear();
        // }
        // actor->components_to_add.clear();
        if (actor->destroyed) {
            map<string, luabridge::LuaRef> sorted_comps(actor->components.begin(), actor->components.end());
            for (auto& [key, comp] : sorted_comps) {
                luabridge::LuaRef OnDestroy = comp["OnDestroy"];
                if (OnDestroy.isFunction()) {
                    try { OnDestroy(comp); }
                    catch (const luabridge::LuaException& e) { ReportError(actor->GetName(), e); }
                }
            }
            actor->components.clear();
        }
    }

    actors.erase(remove_if(actors.begin(), actors.end(), [](Actor* a) {
        if (a->destroyed) {
            delete a;
            return true;
        }
        return false;
    }), actors.end());
    actors_to_remove.clear();   
}

void Engine::Render() {
    SDL_RenderSetScale(renderer, 1.0f, 1.0f);

    ImageDB::RenderAndClearAllImages(renderer, camera_pos, zoom_factor, window_width, window_height);

    while (!text_draw_queue.empty()){
        TextDrawRequest& req = text_draw_queue.front();
        SDL_Color color = {
            static_cast<Uint8>(req.r),
            static_cast<Uint8>(req.g),
            static_cast<Uint8>(req.b),
            static_cast<Uint8>(req.a)
        };
        TextDB::DrawText(req.content, req.font_name, req.font_size, color, req.x, req.y);
        text_draw_queue.pop();
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    while (!pixel_draw_queue.empty()) {
        PixelDrawRequest& req = pixel_draw_queue.front();
        SDL_SetRenderDrawColor(renderer, req.r, req.g, req.b, req.a);
        SDL_RenderDrawPoint(renderer, req.x, req.y);
        pixel_draw_queue.pop();
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Engine::GameLoop() {
	if (!game_start_message.empty()) {
        cout << game_start_message << endl;
    }

	while (running) {
        if (!next_scene.empty()) {
            vector<Actor*> kept_scene_actors;
            for (auto* actor : actors){
                if (!actor->destroy_on_load && !actor->destroyed){
                    map<string, luabridge::LuaRef> sorted_comps(actor->components.begin(), actor->components.end());
                    for (auto& [key, comp] : sorted_comps) {
                        luabridge::LuaRef OnDestroy = comp["OnDestroy"];
                        if (OnDestroy.isFunction()) {
                            try { OnDestroy(comp); }
                            catch (const luabridge::LuaException& e) {
                                ReportError(actor->GetName(), e);
                            }
                        }
                    }
                    delete actor;
                }
            }
            actors.clear();
            for (auto* actor : actors_to_add){
                if (!actor->destroy_on_load && !actor->destroyed){
                    map<string, luabridge::LuaRef> sorted_comps(actor->components.begin(), actor->components.end());
                    for (auto& [key, comp] : sorted_comps) {
                        luabridge::LuaRef OnDestroy = comp["OnDestroy"];
                        if (OnDestroy.isFunction()) {
                            try { OnDestroy(comp); }
                            catch (const luabridge::LuaException& e) { ReportError(actor->GetName(), e); }
                        }
                    }
                    delete actor;
                }
                else kept_scene_actors.push_back(actor);
            }
            actors_to_add.clear();
            actors_to_remove.clear();
            collider_map.clear();
            trigger_map.clear();

            player_ptr = nullptr;

            for (auto* actor : kept_scene_actors) {
                actors.push_back(actor);
            }

            SceneDB::LoadScene(next_scene, actors, actors_to_add, lua_state);
            current_scene = next_scene;
            next_scene = "";
            while (!text_draw_queue.empty()) text_draw_queue.pop();
            while (!pixel_draw_queue.empty()) pixel_draw_queue.pop();
            ImageDB::image_draw_queue.clear();
        }
        
        //game event loop
        SDL_Event nextEvent;
        while (Helper::SDL_PollEvent(&nextEvent)) {
            if (nextEvent.type == SDL_QUIT) {
                running = false;
            }
            Input::ProcessEvent(nextEvent); 
        }

        //event loop ends
        SDL_SetRenderDrawColor(renderer, clear_r, clear_g, clear_b, 255);
        SDL_RenderClear(renderer);

        Update();

        EventBus::ProcessDeferred();

        if (physics_world != nullptr) {
            physics_world->Step(1.0f / 60.0f, 8, 3);
        }
        
        Render(); 

        Helper::SDL_RenderPresent(renderer);
        Input::LateUpdate();

        #ifdef __EMSCRIPTEN__
        emscripten_sleep(0);
        #endif
    }
}