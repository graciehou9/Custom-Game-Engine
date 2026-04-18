file(GLOB_RECURSE BOX2D_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/box2d/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/box2d/src/*.c"
)

file(GLOB_RECURSE LUA_SOURCES "${CMAKE_SOURCE_DIR}/src/lua/*.c")

file(GLOB_RECURSE OTHER_SOURCES
    "${CMAKE_SOURCE_DIR}/src/glm/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/rapidjson/*.cpp"
)

add_library(ExternalLibs STATIC 
    ${BOX2D_SOURCES} 
    ${LUA_SOURCES}
    ${OTHER_SOURCES}
)

target_include_directories(ExternalLibs PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lua
    ${CMAKE_SOURCE_DIR}/src/LuaBridge
    ${CMAKE_SOURCE_DIR}/src/box2d/include
    ${CMAKE_SOURCE_DIR}/src/box2d/src
)

if(EMSCRIPTEN)
    set(EMSCRIPTEN_SDL_FLAGS
        "-sUSE_SDL=2"
        "-sUSE_SDL_IMAGE=2"
        "-sUSE_SDL_MIXER=2"
        "-sUSE_SDL_TTF=2"
        "-sSDL2_IMAGE_FORMATS=[\"png\"]" 
    )

    target_compile_options(ExternalLibs PUBLIC ${EMSCRIPTEN_SDL_FLAGS})
    target_link_options(ExternalLibs PUBLIC ${EMSCRIPTEN_SDL_FLAGS})
elseif(APPLE)
    target_link_directories(ExternalLibs PUBLIC "/opt/homebrew/lib")
    target_include_directories(ExternalLibs PUBLIC "/opt/homebrew/opt/lua@5.4/include")
    target_link_libraries(ExternalLibs PUBLIC "/opt/homebrew/opt/lua@5.4/lib/liblua.a")

    find_package(SDL2 REQUIRED)
    target_include_directories(ExternalLibs PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(ExternalLibs PUBLIC ${SDL2_LIBRARIES} SDL2_image SDL2_mixer SDL2_ttf)

elseif(UNIX AND NOT APPLE)
    find_package(SDL2 REQUIRED)
    find_package(Lua 5.4 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

    target_include_directories(ExternalLibs PUBLIC 
        ${SDL2_INCLUDE_DIRS} 
        ${LUA_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
    )
    target_link_libraries(ExternalLibs PUBLIC 
        -Wl,--start-group
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${LUA_LIBRARIES}
        -Wl,--end-group
        dl m
    )
    # target_include_directories(ExternalLibs PUBLIC ${SDL2_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})
    # target_link_libraries(ExternalLibs PUBLIC ${SDL2_LIBRARIES} SDL2_image SDL2_mixer SDL2_ttf ${LUA_LIBRARIES} dl m)

elseif(WIN32)
    target_compile_definitions(ExternalLibs PUBLIC
        RAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
    )
    target_include_directories(ExternalLibs PUBLIC
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_image
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_mixer
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_ttf
    )
    target_link_directories(ExternalLibs PUBLIC
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2/lib
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_image/lib
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_mixer/lib
        ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_ttf/lib
    )
    target_link_libraries(ExternalLibs PUBLIC SDL2main SDL2 SDL2_image SDL2_mixer SDL2_ttf)
endif()