//
//  main.cpp
//  game_engine
//
//  Created by Gracie Hou on 1/20/26.
//
//please grade homework5 osx

#include <iostream>
#include <vector>

#include "gea/Engine.h"
#include "src/glm/glm.hpp"
//#include "rapidjson/document.h"
#include "src/rapidjson/document.h"

#include "src/SDL2/SDL.h"
#include "src/lua/lua.hpp"
#include "src/LuaBridge/LuaBridge.h"

using namespace std;

int main(int argc, char* argv[]) {    
    
    Engine game;
    game.GameLoop();
    return 0;
}
