cmake_minimum_required(VERSION 3.30)

project(Pathfinding)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Libraries
include(FetchContent)

# Include SDL3
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(SDL3)

# Include ImGui
FetchContent_Declare(imgui_fetch
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
)
FetchContent_MakeAvailable(imgui_fetch)

add_library(imgui STATIC 
    ${imgui_fetch_SOURCE_DIR}/imgui.cpp
    ${imgui_fetch_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_fetch_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_fetch_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_fetch_SOURCE_DIR}/imgui_widgets.cpp

    ${imgui_fetch_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_fetch_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui PUBLIC ${imgui_fetch_SOURCE_DIR})
target_link_libraries(imgui PRIVATE SDL3::SDL3)

# Include glm
FetchContent_Declare(glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

#Project files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

add_executable(Application ${SourceFiles})
target_link_libraries(Application PRIVATE SDL3::SDL3 imgui glm::glm)
target_include_directories(Application PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Linker options
if(WIN32)
    if(MSVC)
        target_link_options(Application PRIVATE
            "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
            "$<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>"    
        )
    elseif(MINGW)
        target_link_options(Application PRIVATE
            "$<$<CONFIG:Debug>:-mconsole>"
            "$<$<NOT:$<CONFIG:Debug>>:-mwindows>"    
        )
    endif()
endif()

# Post build
if(WIN32)
    add_custom_command(TARGET Application POST_BUILD
        # Copy runtime deps
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Application>
            $<TARGET_FILE_DIR:Application>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime dependencies to output folder "
    )
endif()