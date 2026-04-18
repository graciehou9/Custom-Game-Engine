//
//  ParticleSystem.cpp
//  game_engine
//
//  Created by Gracie Hou on 3/27/26.
//

#include "ParticleSystem.h"
#include "ImageDB.h"


ParticleSystem::ParticleSystem() {}

void ParticleSystem::OnStart() {
    emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
    emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);
    rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
    scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);
    speed_distribution = RandomEngine(start_speed_min, start_speed_max, 498);
    rotation_speed_distribution = RandomEngine(rotation_speed_min, rotation_speed_max, 305);
}

void ParticleSystem::Play() {
    is_playing = true;
}

void ParticleSystem::Stop() {
    is_playing = false;
}

void ParticleSystem::Burst() {
    if (burst_quantity < 1) burst_quantity = 1;

    for (int i = 0; i < burst_quantity; ++i) {
        float angle_radians = glm::radians(emit_angle_distribution.Sample());
        float radius = emit_radius_distribution.Sample();
        float cos_angle = glm::cos(angle_radians);
        float sin_angle = glm::sin(angle_radians);
        float speed = speed_distribution.Sample();
        
        int idx = -1;
         if (free_count > 0) {
            idx = free_list[free_head];
            free_head = (free_head + 1) % free_list.size();
            free_count--;

            is_active[idx] = 1;
            start_frame[idx] = local_frame_number;
            p_x[idx] = this->x + (cos_angle * radius);
            p_y[idx] = this->y + (sin_angle * radius);
            x_vel[idx] = cos_angle * speed;
            y_vel[idx] = sin_angle * speed;
            float sc = scale_distribution.Sample();
            initial_scale[idx] = sc;
            current_scale[idx] = sc;
            p_rotation[idx] = rotation_distribution.Sample();
            angular_velocity[idx] = rotation_speed_distribution.Sample();
            initial_r[idx] = current_r[idx] = start_color_r;
            initial_g[idx] = current_g[idx] = start_color_g;
            initial_b[idx] = current_b[idx] = start_color_b;
            initial_a[idx] = current_a[idx] = start_color_a;
        } else {
            is_active.push_back(1);
            free_list.push_back(0);
            start_frame.push_back(local_frame_number);
            p_x.push_back(this->x + (cos_angle * radius));
            p_y.push_back(this->y + (sin_angle * radius));
            x_vel.push_back(cos_angle * speed);
            y_vel.push_back(sin_angle * speed);
            float sc = scale_distribution.Sample();
            initial_scale.push_back(sc);
            current_scale.push_back(sc);
            p_rotation.push_back(rotation_distribution.Sample());
            angular_velocity.push_back(rotation_speed_distribution.Sample());
            initial_r.push_back(start_color_r); current_r.push_back(start_color_r);
            initial_g.push_back(start_color_g); current_g.push_back(start_color_g);
            initial_b.push_back(start_color_b); current_b.push_back(start_color_b);
            initial_a.push_back(start_color_a); current_a.push_back(start_color_a);
        }
    }
}

static void ParticleSystemDrawCallback(void* data, SDL_Renderer* renderer, glm::vec2 camera_pos, float zoom_factor, int window_width, int window_height) {
    ParticleSystem* ps = static_cast<ParticleSystem*>(data);
    
    float cam_fixed_x = (window_width*0.5f)*(1.0f - 1.0f/zoom_factor);
    float cam_fixed_y = (window_height*0.5f)*(1.0f - 1.0f/zoom_factor);

    std::string render_image = ps->image == "" ? "default_p" : ps->image;
    SDL_Texture* tex = ImageDB::LoadImage(render_image, renderer);
    float w, h;
    Helper::SDL_QueryTexture(tex, &w, &h);

    float global_offset_x = (window_width * 0.5f) - cam_fixed_x - (camera_pos.x * 100.0f);
    float global_offset_y = (window_height * 0.5f) - cam_fixed_y - (camera_pos.y * 100.0f);
    float half_w = w * 0.5f;
    float half_h = h * 0.5f;

    float f_window_w = static_cast<float>(window_width);
    float f_window_h = static_cast<float>(window_height);

    int last_r = -1, last_g = -1, last_b = -1, last_a = -1;
    size_t count = ps->is_active.size();

    for (size_t i = 0; i < count; i++) {
        if (!ps->is_active[i]) continue;
        
        float s = ps->current_scale[i];
        float abs_s = glm::abs(s);
        int a = ps->current_a[i];

        if (a <= 0 || abs_s <= 0.001f) continue;

        float scaled_pivot_x = half_w * abs_s;
        float scaled_pivot_y = half_h * abs_s;

        float dst_x = (ps->p_x[i] * 100.0f) - scaled_pivot_x + global_offset_x;
        float dst_y = (ps->p_y[i] * 100.0f) - scaled_pivot_y + global_offset_y;

        float dst_w = w * abs_s;
        float dst_h = h * abs_s;

        float pad = glm::max(dst_w, dst_h);
        if (dst_x + dst_w < -pad || dst_x > f_window_w + pad || dst_y + dst_h < -pad || dst_y > f_window_h + pad) {
            continue;
        }

        SDL_FRect dst = {dst_x, dst_y, dst_w, dst_h};
        SDL_FPoint center = {scaled_pivot_x, scaled_pivot_y};

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (s < 0) flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
        int r = ps->current_r[i]; 
        int g = ps->current_g[i]; 
        int b = ps->current_b[i];

        if (r != last_r || g != last_g || b != last_b) {
            SDL_SetTextureColorMod(tex, r, g, b);
            last_r = r; last_g = g; last_b = b;
        }
        if (a != last_a) {
            SDL_SetTextureAlphaMod(tex, a);
            last_a = a;
        }

        Helper::SDL_RenderCopyEx(-1, "", renderer, tex, nullptr, &dst, ps->p_rotation[i], &center, flip);
    }
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex, 255);
}

void ParticleSystem::OnUpdate() {
    if (frames_between_bursts < 1) frames_between_bursts = 1;
    if (duration_frames < 1) duration_frames = 1;

    if (local_frame_number % frames_between_bursts == 0 && is_playing) {
        //generate new particles
        Burst();
    }
    size_t count = is_active.size();
    const bool do_scale = end_scale >= 0.0f;
    const bool do_r = end_color_r >= 0;
    const bool do_g = end_color_g >= 0;
    const bool do_b = end_color_b >= 0;
    const bool do_a = end_color_a >= 0;

    const float f_r = static_cast<float>(end_color_r);
    const float f_g = static_cast<float>(end_color_g);
    const float f_b = static_cast<float>(end_color_b);
    const float f_a = static_cast<float>(end_color_a);
    const float f_dur = static_cast<float>(duration_frames);

    const float gx = gravity_scale_x;
    const float gy = gravity_scale_y;
    const float df = drag_factor;
    const float adf = angular_drag_factor;

    //physics
    for (size_t i = 0; i < count; i++) {
        if (!is_active[i]) continue;

        const int frames_alive = local_frame_number - start_frame[i];
        if (frames_alive >= duration_frames) {
            is_active[i] = 0;
            free_list[free_tail] = static_cast<int>(i);
            free_tail = (free_tail + 1) % free_list.size();
            free_count++;
            continue;
        }

        x_vel[i] = (x_vel[i] + gx) * df;
        y_vel[i] = (y_vel[i] + gy) * df;
        angular_velocity[i] *= adf;

        p_x[i] += x_vel[i];
        p_y[i] += y_vel[i];

        p_rotation[i] += angular_velocity[i];
        if (do_scale || do_r || do_g || do_b || do_a) {
            float progress = static_cast<float>(frames_alive) / f_dur;
            if (do_scale) current_scale[i] = glm::mix(initial_scale[i], end_scale, progress);
            if (do_r) current_r[i] = static_cast<int>(glm::mix(static_cast<float>(initial_r[i]), f_r, progress));
            if (do_g) current_g[i] = static_cast<int>(glm::mix(static_cast<float>(initial_g[i]), f_g, progress));
            if (do_b) current_b[i] = static_cast<int>(glm::mix(static_cast<float>(initial_b[i]), f_b, progress));
            if (do_a) current_a[i] = static_cast<int>(glm::mix(static_cast<float>(initial_a[i]), f_a, progress));
        }
    }

    //rendering
    ImageDrawRequest req;
    req.sorting_order = this->sorting_order;
    req.is_ui = false;
    req.custom_draw = ParticleSystemDrawCallback;
    req.custom_data = this;
    ImageDB::image_draw_queue.push_back(req);

    local_frame_number++;
}

void ParticleSystem::OnDestroy() {
    is_active.clear();
    start_frame.clear();
    p_x.clear();
    p_y.clear();
    x_vel.clear();
    y_vel.clear();
    p_rotation.clear();
    angular_velocity.clear();
    initial_scale.clear();
    current_scale.clear();
    initial_r.clear();
    initial_g.clear();
    initial_b.clear();
    initial_a.clear();
    current_r.clear();
    current_g.clear();
    current_b.clear();
    current_a.clear();
    free_list.clear();
    free_head = 0;
    free_tail = 0;
    free_count = 0;
}
