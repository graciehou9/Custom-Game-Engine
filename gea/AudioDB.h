//
//  AudioDB.hpp
//  game_engine
//
//  Created by Gracie Hou on 2/6/26.
//

#ifndef AudioDB_h
#define AudioDB_h

#include <stdio.h>
#include <string>
#include <unordered_map>
#include "../AudioHelper.h"

class AudioDB {
public:
    static void Init();
    static Mix_Chunk* LoadSound(const std::string& soundName);
    static void PlaySound(const std::string& soundName);
    static void PlaySoundChannel(const std::string& soundName, int channel, int loops);
private:
    static std::unordered_map<std::string, Mix_Chunk*> sounds;
};

#endif /* AudioDB_h */
