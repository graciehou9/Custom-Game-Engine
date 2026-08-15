# A custom game engine

Supports CMake!

A C++ game engine with equal functionality to Unity. Integrates Lua-driven components, Box2D physics, and SDL2 rendering/audio. This repository contains the engine core, resource loaders, and example game data (scenes, actor templates, images, audio).

 **Features**
 - **Component model:** C++ `Actor` objects composed with Lua components for flexible behavior.
 - **Scene & template system:** JSON-based scenes and cached actor templates (`resources/scenes`, `resources/actor_templates`).
 - **Rendering pipeline:** batched image draw queue and custom draw callbacks via `ImageDB`.
 - **Audio manager:** cached audio loading and playback helpers via `AudioDB`.
 - **Physics:** Box2D-backed `Rigidbody` wrapper with raycasts and contact handling.
 - **Input & events:** frame-accurate `Input` helpers and an `EventBus` for pub/sub between components.

 **Requirements**
 - C++17 toolchain and `cmake`.
 - SDL2 (and SDL2_image, SDL2_mixer, SDL2_ttf), Box2D, Lua, LuaBridge, RapidJSON, glm.
 - For web build: Emscripten SDK (optional).

 **Build (native)**
 Run from the repository root:

 ```bash
 mkdir -p build
 cd build
 cmake ..
 make -j$(sysctl -n hw.ncpu)
 ```

 The executable will be in `build/` (e.g. `./build/GameEngine`).

 **Build (web / Emscripten)**
 If you have Emscripten installed:

 ```bash
 emcmake cmake -S . -B build_web
 cmake --build build_web --config Release
 # then open the generated `index.html` in `build_web/` or serve it
 ```

 **Run**
 - Native: `./build/GameEngine` (ensure `resources/` is next to the executable).
 - Web: open `build_web/index.html` in a browser served by a local HTTP server.

 **Project layout (key files)**
 - [gea/Engine.h](gea/Engine.h#L1) / [gea/Engine.cpp](gea/Engine.cpp#L1): engine core, game loop, scene switching, draw queues.
 - [gea/Actor.h](gea/Actor.h#L1) / [gea/Actor.cpp](gea/Actor.cpp#L1): actor struct and Lua component integration.
 - [gea/SceneDB.h](gea/SceneDB.h#L1) / [gea/SceneDB.cpp](gea/SceneDB.cpp#L1): scene loader (resources/scenes).
 - [gea/ActorTemplateDB.h](gea/ActorTemplateDB.h#L1): template caching and application (resources/actor_templates).
 - [gea/ImageDB.h](gea/ImageDB.h#L1): image cache and draw queue.
 - [gea/AudioDB.h](gea/AudioDB.h#L1): audio cache and playback helpers.
 - [gea/Input.h](gea/Input.h#L1): input state helpers (`GetKeyDown`, etc.).
 - [gea/Rigidbody.h](gea/Rigidbody.h#L1): Box2D integration and body lifecycle.
 - [gea/EventBus.h](gea/EventBus.h#L1): lightweight pub/sub for components.
 - `resources/`: scenes, actor templates, images, audio, fonts, and scripts.

 **How the pieces fit (brief)**
 - `Engine` owns the main loop, exposes C++ helper functions to Lua, and manages actor lifecycle.
 - `SceneDB`/`ActorTemplateDB` populate `Actor` instances (C++), which then hold Lua components (tables/functions).
 - Rendering and audio are performed by batching requests (`ImageDB`, `AudioDB`) produced by actors/components.
 - Physics lives in Box2D; `Rigidbody` binds bodies/fixtures to `Actor` pointers for collisions and raycasts.

 **Adding content**
 - Create actor templates in `resources/actor_templates/*.template` and scenes in `resources/scenes/*.scene`.
 - Implement component logic in `resources/component_types/*.lua` and attach via templates or scene entries.

 **Development notes**
 - Use `actors_to_add` / `actors_to_remove` mutation patterns — avoid modifying actor lists mid-update.
 - Lua components are registered through `addLuaComponent` and may require `InjectReferences` after creation.

 **Contributing & License**
 - Feel free to open issues or submit PRs. Add a `LICENSE` file to set terms; none included by default.

 ---
