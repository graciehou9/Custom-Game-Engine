cmake_minimum_required(VERSION 3.10)
project(GameEngine C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(src)
add_subdirectory(gea)

add_executable(GameEngine main.cpp)

target_link_libraries(GameEngine PRIVATE EngineCore ExternalLibs)

if(EMSCRIPTEN)
    set_target_properties(GameEngine PROPERTIES SUFFIX ".html")

    target_link_options(GameEngine PRIVATE
        "--preload-file" "${CMAKE_SOURCE_DIR}/resources@/resources"
        "-sALLOW_MEMORY_GROWTH=1" 
        "-sINITIAL_MEMORY=67108864"
        "-sASYNCIFY=1"
    )
endif()

set_target_properties(GameEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)









# #source files
# file(GLOB ROOT_SOURCES "*.cpp" "*.c")
# file(GLOB_RECURSE ENGINE_SOURCES "gea/*.cpp" "gea/*.c")
# file(GLOB_RECURSE LIB_SOURCES "src/*.cpp" "src/*.c")

# #create executable
# add_executable(GameEngine ${ENGINE_SOURCES} ${LIB_SOURCES} ${ROOT_SOURCES})

# #include headers
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR})
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/gea)
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/src/lua)
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/src/LuaBridge)
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/src/box2d/include)
# target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/src/box2d/src)

# #requires SDL2
# if (NOT WIN32)
#     find_package(SDL2 REQUIRED)
#     if(NOT SDL2_FOUND) #error message
#         message(FATAL_ERROR "SDL2 not found")
#     endif()
#     target_include_directories(GameEngine PRIVATE ${SDL2_INCLUDE_DIRS})
#     target_link_libraries(GameEngine PRIVATE ${SDL2_LIBRARIES} SDL2_image SDL2_mixer SDL2_ttf)
# endif()
# target_include_directories(GameEngine PRIVATE ${SDL2_INCLUDE_DIRS})
# target_link_libraries(GameEngine PRIVATE ${SDL2_LIBRARIES} SDL2_image SDL2_mixer SDL2_ttf)

# if (APPLE)
#     target_link_directories(GameEngine PRIVATE "/opt/homebrew/lib")
#     target_include_directories(GameEngine PRIVATE "/opt/homebrew/opt/lua@5.4/include")
#     target_link_libraries(GameEngine PRIVATE "/opt/homebrew/opt/lua@5.4/lib/liblua.a")
# elseif (UNIX)
#     find_package(Lua 5.4 REQUIRED)
#     target_include_directories(GameEngine PRIVATE ${LUA_INCLUDE_DIR})
#     target_link_libraries(GameEngine PRIVATE ${LUA_LIBRARIES} dl m)
# elseif (WIN32)
#     target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2)
#     target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_mixer)
#     target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_image)
#     target_include_directories(GameEngine PRIVATE ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_ttf)
#     target_link_directories(GameEngine PRIVATE 
#         ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2/lib
#         ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_mixer/lib
#         ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_image/lib
#         ${CMAKE_SOURCE_DIR}/VS_SDL/src/SDL2_ttf/lib
#     )
#     target_link_libraries(GameEngine PRIVATE SDL2main SDL2 SDL2_image SDL2_mixer SDL2_ttf)
#     target_compile_definitions(GameEngine PRIVATE 
#         RAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN
#         _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#     )
#     file(GLOB_RECURSE LUA_C_SOURCES "src/lua/*.c")
#     set_source_files_properties(${LUA_C_SOURCES} PROPERTIES LANGUAGE C)
   
# endif()
