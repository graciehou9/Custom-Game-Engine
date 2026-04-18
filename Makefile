CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -I. -Iinclude -I./src -I./src/SDL2 -I./src/SDL2_image -I./src/SDL2_mixer -I./src/SDL2_ttf -I./src/box2d/include -I./src/box2d/src
LDFLAGS  = -L./src
LDLIBS   = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -llua5.4
TARGET = game_engine_linux
BOX2D_SRCS = $(wildcard src/box2d/src/collision/*.cpp) \
             $(wildcard src/box2d/src/common/*.cpp) \
             $(wildcard src/box2d/src/dynamics/*.cpp) \
             $(wildcard src/box2d/src/rope/*.cpp)
SRCS = main.cpp gea/Engine.cpp gea/SceneDB.cpp gea/Actor.cpp gea/ActorTemplateDB.cpp gea/AudioDB.cpp gea/ImageDB.cpp gea/TextDB.cpp gea/Input.cpp $(BOX2D_SRCS) gea/ParticleSystem.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(SRCS) $(LDLIBS)

# game_engine_linux: main.cpp gea/Engine.cpp
# 	clang++ -std=c++17 -Wall -Wextra -O2 -I. -Iinclude -o game_engine_linux main.cpp gea/Engine.cpp