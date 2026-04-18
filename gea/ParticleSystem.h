//
//  ParticleSystem.hpp
//  game_engine
//
//  Created by Gracie Hou on 3/27/26.
//

#ifndef ParticleSystem_h
#define ParticleSystem_h

#include <stdio.h>
#include <string>
#include <vector>
#include <queue>
#include "Actor.h"
#include "../src/box2d/include/box2d/box2d.h"
#include "../src/glm/glm.hpp"
#include "../Helper.h"

class ParticleSystem {
public:
    std::string key = "";
    std::string type = "ParticleSystem";
    bool enabled = true;
    Actor* actor = nullptr;

    float x = 0.0f;
    float y = 0.0f;
    int frames_between_bursts = 1;
    int burst_quantity = 1;
    float start_scale_min = 1.0f;
    float start_scale_max = 1.0f;
    float rotation_min = 0.0f;
    float rotation_max = 0.0f;
    int start_color_r = 255;
    int start_color_g = 255;
    int start_color_b = 255;
    int start_color_a = 255;
    float emit_radius_min = 0.0f;
    float emit_radius_max = 0.5f;
    float emit_angle_min = 0.0f;
    float emit_angle_max = 360.0f;
    std::string image = "";
    int sorting_order = 9999;

    //animated properties
    int duration_frames = 300;
    float start_speed_min = 0.0f, start_speed_max = 0.0f;
    float rotation_speed_min = 0.0f, rotation_speed_max = 0.0f;
    float gravity_scale_x = 0.0f, gravity_scale_y = 0.0f;
    float drag_factor = 1.0f, angular_drag_factor = 1.0f;
    float end_scale = -1.0f;
    int end_color_r = -1, end_color_g = -1, end_color_b = -1, end_color_a = -1;

    int local_frame_number = 0;
    std::vector<int> free_list;
    size_t free_head = 0;
    size_t free_tail = 0;
    size_t free_count = 0;

    RandomEngine emit_angle_distribution;
    RandomEngine emit_radius_distribution;
    RandomEngine scale_distribution;
    RandomEngine rotation_distribution;
    RandomEngine speed_distribution;
    RandomEngine rotation_speed_distribution;

    bool is_playing = true;

    std::vector<uint8_t> is_active;
    std::vector<int> start_frame;
    std::vector<float> p_x, p_y; 
    std::vector<float> x_vel, y_vel;
    std::vector<float> p_rotation, angular_velocity;
    std::vector<float> initial_scale, current_scale;
    std::vector<int> initial_r, initial_g, initial_b, initial_a;
    std::vector<int> current_r, current_g, current_b, current_a;

    ParticleSystem();

    void OnStart();
    void OnUpdate();
    void OnDestroy();

    void Play();
    void Stop();
    void Burst();
};

#endif /* ParticleSystem_h */
