//
//  ImageDB.hpp
//  game_engine
//
//  Created by Gracie Hou on 2/5/26.
//

#ifndef ImageDB_h
#define ImageDB_h

#include <stdio.h>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include "../src/SDL2/SDL.h"
#include "../src/glm/glm.hpp"

typedef void (*DrawCallback)(void* data, SDL_Renderer* renderer, glm::vec2 camera_pos, float zoom_factor, int window_width, int window_height);

struct ImageDrawRequest {
    std::string image_name;
    float x, y;
    int rotation_degrees;
    float scale_x, scale_y, pivot_x, pivot_y;
    int r, g, b, a;
    int sorting_order;
    bool is_ui;

    DrawCallback custom_draw = nullptr;
    void* custom_data = nullptr;
};

class ImageDB {
public:
    static SDL_Texture* LoadImage(const std::string& imageName, SDL_Renderer* renderer);
    static void RenderAndClearAllImages(SDL_Renderer* renderer, glm::vec2 camera_pos, float zoom_factor, int window_width, int window_height);
    static void CreateDefaultParticleTextureWithName(const std::string& name, SDL_Renderer* renderer);

    static std::vector<ImageDrawRequest> image_draw_queue;
private:
    static std::unordered_map<std::string, SDL_Texture*> images;
};

#endif /* ImageDB_h */
