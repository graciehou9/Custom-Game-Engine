//
//  TextDB.cpp
//  game_engine
//
//  Created by Gracie Hou on 2/6/26.
//

#include <iostream>
#include <filesystem>
#include "../Helper.h"
#include "TextDB.h"

using namespace std;

std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> TextDB::texts;
SDL_Renderer* TextDB::renderer = nullptr;

void TextDB::Init() {
    if (TTF_Init() == -1) {
        exit(0);
    }
}

void TextDB::setRenderer(SDL_Renderer* renderer){
    TextDB::renderer = renderer;
}

TTF_Font* TextDB::LoadText(const std::string& textName, int fontSize) {
    if (texts[textName].find(fontSize) != texts[textName].end()) {
        return texts[textName][fontSize];
    }

    string path = "resources/fonts/" + textName + ".ttf";
    if (!filesystem::exists(path)) {
        std::cout << "error: font " << textName << " missing";
        exit(0);
    }

    TTF_Font* font = TTF_OpenFont(path.c_str(), fontSize);
    if (font == nullptr) {
        std::cout << "error: font " << textName << " missing";
        exit(0);
    }

    texts[textName][fontSize] = font;
    return font;

}
void TextDB::DrawText(const std::string& textContent, const std::string& fontName, int fontSize, SDL_Color fontColor, int x, int y){
    if (renderer==nullptr) return;
    if (textContent.empty()) return;

    TTF_Font* font = LoadText(fontName, fontSize);
    if (font == nullptr) return;

    SDL_Surface* surface = TTF_RenderText_Solid(font, textContent.c_str(), fontColor); // TTF_RenderText_Blended(font, textContent.c_str(), fontColor);
    if (surface == nullptr) {
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        SDL_FreeSurface(surface);
        return;
    }   

    SDL_FRect destRect = {static_cast<float>(x),static_cast<float>(y),static_cast<float>(surface->w),static_cast<float>(surface->h)};
    Helper::SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}