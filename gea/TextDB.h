//
//  TextDB.hpp
//  game_engine
//
//  Created by Gracie Hou on 2/6/26.
//

#ifndef TextDB_h
#define TextDB_h

#include <stdio.h>
#include <string>
#include <unordered_map>
#include "../src/SDL2/SDL.h"
#include "../src/SDL2_ttf/SDL_ttf.h"

class TextDB {
public:
    static void Init();
    static void setRenderer(SDL_Renderer* renderer);
    static TTF_Font* LoadText(const std::string& textName, int fontSize);
    static void DrawText(const std::string& textContent, const std::string& fontName, int fontSize, SDL_Color fontColor, int x, int y);
private:
    static std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> texts;
    static SDL_Renderer* renderer;
};

#endif /* TextDB_h */
