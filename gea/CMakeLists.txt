file(GLOB_RECURSE ENGINE_SOURCES 
    "${CMAKE_SOURCE_DIR}/gea/*.cpp"
    "${CMAKE_SOURCE_DIR}/gea/*.c"
)

add_library(EngineCore STATIC ${ENGINE_SOURCES})

target_include_directories(EngineCore PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/gea
)

target_link_libraries(EngineCore PUBLIC ExternalLibs)