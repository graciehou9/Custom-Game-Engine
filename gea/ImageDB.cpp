//
//  ImageDB.cpp
//  game_engine
//
//  Created by Gracie Hou on 2/5/26.
//

#include <filesystem>
#include <iostream>
#include <algorithm>
#include "ImageDB.h"
#include "Helper.h"
#include "../src/SDL2_image/SDL_image.h"

using namespace std;

unordered_map<string, SDL_Texture*> ImageDB::images;
vector<ImageDrawRequest> ImageDB::image_draw_queue;

SDL_Texture* ImageDB::LoadImage(const string& imageName, SDL_Renderer* renderer){
    if (images.find(imageName) != images.end()) {
        return images[imageName];
    }

    string path = "resources/images/" + imageName + ".png";

    if (!filesystem::exists(path)) {
        cout << "error: missing image " << imageName;
        exit(0);
    }

    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr) {
        cout << "error: missing image " << imageName;
        exit(0);
    }

    images[imageName] = texture;

    return texture;
}

void ImageDB::RenderAndClearAllImages(SDL_Renderer* renderer, glm::vec2 camera_pos, float zoom_factor, int window_width, int window_height){
    float cam_fixed_x = (window_width*0.5f)*(1.0f - 1.0f/zoom_factor);
    float cam_fixed_y = (window_height*0.5f)*(1.0f - 1.0f/zoom_factor);

    auto partition_point = stable_partition(image_draw_queue.begin(), image_draw_queue.end(), [](const ImageDrawRequest& req) {
        return !req.is_ui;
    });
    auto sort_images = [](const ImageDrawRequest& a, const ImageDrawRequest& b) {
        return a.sorting_order < b.sorting_order;
    };
    stable_sort(image_draw_queue.begin(), partition_point, sort_images);
    stable_sort(partition_point, image_draw_queue.end(), sort_images);

    SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);
    
    SDL_Texture* last_texture = nullptr;
    string last_image_name = "";
    float last_w = 0, last_h = 0;
    int last_r = -1, last_g = -1, last_b = -1, last_a = -1;

    for (auto it = image_draw_queue.begin(); it != partition_point; ++it) {
        const auto& req = *it;
        if (req.custom_draw != nullptr) {
            if (last_texture != nullptr) {
                SDL_SetTextureColorMod(last_texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(last_texture, 255);
                last_texture = nullptr;
                last_image_name = "";
                last_r = -1; last_g = -1; last_b = -1; last_a = -1;
            }
            req.custom_draw(req.custom_data, renderer, camera_pos, zoom_factor, window_width, window_height);
            continue;
        }
        if (req.image_name != last_image_name || last_texture == nullptr) {
            if (last_texture != nullptr) {
                SDL_SetTextureColorMod(last_texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(last_texture, 255);
            }
            last_texture = LoadImage(req.image_name, renderer);
            Helper::SDL_QueryTexture(last_texture, &last_w, &last_h);
            last_image_name = req.image_name;
            last_r = -1; last_g = -1; last_b = -1; last_a = -1; 
        }
        float w = last_w;
        float h = last_h;

        float scale_x = glm::abs(req.scale_x);
        float scale_y = glm::abs(req.scale_y);

        float scaled_pivot_x = req.pivot_x * (w * scale_x);
        float scaled_pivot_y = req.pivot_y * (h * scale_y);

        float dst_x = (req.x * 100.0f) - scaled_pivot_x;
        float dst_y = (req.y * 100.0f) - scaled_pivot_y;

        dst_x -= camera_pos.x * 100.0f;
        dst_y -= camera_pos.y * 100.0f;
        dst_x += (window_width * 0.5f) - cam_fixed_x;
        dst_y += (window_height * 0.5f) - cam_fixed_y;

        float dst_w = w * scale_x;
        float dst_h = h * scale_y;

        SDL_FRect dst = {dst_x, dst_y, dst_w, dst_h};
        SDL_FPoint center = {scaled_pivot_x, scaled_pivot_y};

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (req.scale_x < 0){
            flip = SDL_RendererFlip(flip | SDL_FLIP_HORIZONTAL);
        }
        if (req.scale_y < 0){
            flip = SDL_RendererFlip(flip | SDL_FLIP_VERTICAL);
        }

        if (req.r != last_r || req.g != last_g || req.b != last_b) {
            SDL_SetTextureColorMod(last_texture, req.r, req.g, req.b);
            last_r = req.r; last_g = req.g; last_b = req.b;
        }
        if (req.a != last_a) {
            SDL_SetTextureAlphaMod(last_texture, req.a);
            last_a = req.a;
        }

        Helper::SDL_RenderCopyEx(-1, "", renderer, last_texture, nullptr, &dst, req.rotation_degrees, &center, flip);
    }
    if (last_texture != nullptr) {
        SDL_SetTextureColorMod(last_texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(last_texture, 255);
    }

    SDL_RenderSetScale(renderer, 1.0f, 1.0f);
    last_texture = nullptr;
    last_image_name = "";
    last_r = -1; last_g = -1; last_b = -1; last_a = -1;
    for (auto it = partition_point; it != image_draw_queue.end(); ++it) {
        const auto& req = *it;
        if (req.custom_draw != nullptr) {
            if (last_texture != nullptr) {
                SDL_SetTextureColorMod(last_texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(last_texture, 255);
                last_texture = nullptr; last_image_name = ""; last_r = -1; last_g = -1; last_b = -1; last_a = -1;
            }
            req.custom_draw(req.custom_data, renderer, camera_pos, zoom_factor, window_width, window_height);
            continue;
        }
        if (req.image_name != last_image_name || last_texture == nullptr) {
            if (last_texture != nullptr) {
                SDL_SetTextureColorMod(last_texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(last_texture, 255);
            }
            last_texture = LoadImage(req.image_name, renderer);
            Helper::SDL_QueryTexture(last_texture, &last_w, &last_h);
            last_image_name = req.image_name;
            last_r = -1; last_g = -1; last_b = -1; last_a = -1; 
        }
        SDL_FRect dst = {req.x, req.y, last_w, last_h};

        if (req.r != last_r || req.g != last_g || req.b != last_b) {
            SDL_SetTextureColorMod(last_texture, req.r, req.g, req.b);
            last_r = req.r; last_g = req.g; last_b = req.b;
        }
        if (req.a != last_a) {
            SDL_SetTextureAlphaMod(last_texture, req.a);
            last_a = req.a;
        }

        Helper::SDL_RenderCopyEx(-1, "", renderer, last_texture, nullptr, &dst, 0, nullptr, SDL_FLIP_NONE);

    }
    if (last_texture != nullptr) {
        SDL_SetTextureColorMod(last_texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(last_texture, 255);
    }
    image_draw_queue.clear();
}

void ImageDB::CreateDefaultParticleTextureWithName(const std::string & name, SDL_Renderer* renderer) {
    if (images.find(name) != images.end()) return;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);

    Uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
    SDL_FillRect(surface, NULL, white_color);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    images[name] = texture;
}