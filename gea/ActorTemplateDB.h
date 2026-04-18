//
//  ActorTemplateDB.hpp
//  game_engine
//
//  Created by Gracie Hou on 1/30/26.
//

#ifndef ActorTemplateDB_h
#define ActorTemplateDB_h

#include <stdio.h>
#include <string>
#include "Actor.h"
#include "../src/lua/lua.hpp"
#include "../src/LuaBridge/LuaBridge.h"

namespace ActorTemplateDB {
    void LoadTemplate(const std::string& name, Actor& actor, lua_State* state);
};

#endif /* ActorTemplateDB_h */
