//
//  AudioDB.cpp
//  game_engine
//
//  Created by Gracie Hou on 2/6/26.
//

#include "AudioDB.h"

std::unordered_map<std::string, Mix_Chunk*> AudioDB::sounds;

void AudioDB::Init() {
    AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    AudioHelper::Mix_AllocateChannels(50);
}

Mix_Chunk* AudioDB::LoadSound(const std::string& soundName) {
    if (sounds.find(soundName) != sounds.end()) {
        return sounds[soundName];
    }

    std::string path_wav = "resources/audio/" + soundName + ".wav";
    std::string path_ogg = "resources/audio/" + soundName + ".ogg";
    std::string soundPath = "";

    if (std::filesystem::exists(path_wav)) {
        soundPath = path_wav;
    } else if (std::filesystem::exists(path_ogg)) {
        soundPath = path_ogg;
    } else {
        std::cout << "error: failed to play audio clip " << soundName << std::endl;
        exit(0);
    }

    Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV(soundPath.c_str());
    
    sounds[soundName] = chunk;
    return chunk;
}

void AudioDB::PlaySound(const std::string& soundName) {
    Mix_Chunk* chunk = LoadSound(soundName);
    AudioHelper::Mix_PlayChannel(0, chunk, -1);
}

void AudioDB::PlaySoundChannel(const std::string& soundName, int channel, int loops){
    if (soundName.empty()) return;
    Mix_Chunk* chunk = LoadSound(soundName);
    AudioHelper::Mix_PlayChannel(channel, chunk, loops);
}