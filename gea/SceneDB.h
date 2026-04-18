//
//  SceneDB.hpp
//  game_engine
//
//  Created by Gracie Hou on 1/29/26.
//

#ifndef SceneDB_h
#define SceneDB_h

#include <stdio.h>
#include <string>
#include <vector>
#include "Actor.h"
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"

namespace SceneDB {
    void LoadScene(const std::string& sceneName, std::vector<Actor*>& actors, std::vector<Actor*>& actors_to_add, lua_State* state);
};

#endif /* SceneDB_h */
