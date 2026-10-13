#include <print>
#include <optional>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <glm/vec2.hpp>
#include <glm/common.hpp>

#include "GridMap.hpp"
#include "AStar.hpp"

auto mouseToGrid(glm::vec2 mouse, glm::vec2 gridOrigin, float cellSize) -> std::optional<glm::uvec2>
{
    auto const local = mouse - gridOrigin;
    
    if(local.x < 0 || local.y < 0)
        return std::nullopt;

    auto const grid = glm::trunc(local / cellSize);
    auto const result = glm::uvec2((local + grid) / cellSize); // In the rendered grid each cell is offset by one pixel to have 1px borders rather than 2px

    return result;
}

enum class EditMode
{
    AddWall,
    RemoveWall,
    MoveStart,
    MoveGoal
};

void editMap(GridMap& map, glm::uvec2 coordinate, EditMode mode)
{
    switch(mode)
    {
    case EditMode::AddWall:
        map.setWall(coordinate);
    break;
    case EditMode::RemoveWall:
        map.setEmpty(coordinate);
    break;
    case EditMode::MoveStart:
        map.setStart(coordinate);
    break;
    case EditMode::MoveGoal:
        map.setGoal(coordinate);
    break;
    }
}

void updateModifierStates(bool& shiftDown, bool& ctrlDown, SDL_KeyboardEvent& event)
{
    shiftDown = (event.mod & SDL_KMOD_SHIFT) != 0;    
    ctrlDown = (event.mod & SDL_KMOD_CTRL) != 0;
}

int main(int argc, char* argv[])
{
    if(!SDL_Init(SDL_INIT_VIDEO))
        return EXIT_FAILURE;
        
    constexpr glm::ivec2 resolution(1280, 720);

    auto window = SDL_CreateWindow("Pathfinding", resolution.x, resolution.y, 0);
    auto renderer = SDL_CreateRenderer(window, nullptr);

    ImGui::CreateContext();
    ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    glm::vec2 gridOrigin{100, 100};
    float cellSize = 32.0f;

    GridMap map;
    map.resize({5, 5});
    map.setStart({0, 0});
    map.setGoal({4, 4});
    
    AStar pathfinder(map);
    pathfinder.setNodeMap(map);

    EditMode editMode{};

    bool mouseDown = false;
    bool shiftDown = false;
    bool ctrlDown = false;

    bool runPathfinding = false;

    bool open = true;
    while(open)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            
            switch(event.type)
            {
            case SDL_EVENT_QUIT:
                open = false;
            break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    if(shiftDown)
                    {
                        auto coordinate = mouseToGrid({event.button.x, event.button.y}, gridOrigin, cellSize);

                        if(coordinate)
                            editMap(map, *coordinate, EditMode::MoveStart);
                    }
                    else if(ctrlDown)
                    {
                        auto coordinate = mouseToGrid({event.button.x, event.button.y}, gridOrigin, cellSize);

                        if(coordinate)
                            editMap(map, *coordinate, EditMode::MoveGoal);
                    }
                    else
                    {
                        editMode = EditMode::AddWall;
                        mouseDown = true;
                    }
                }
                else if(event.button.button == SDL_BUTTON_RIGHT)
                {
                    editMode = EditMode::RemoveWall;
                    mouseDown = true;
                }
            }
            break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                mouseDown = false;
            break;
            case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_RETURN && !event.key.repeat)
                {
                    if(pathfinder.state() == SearchState::Ready)
                    {
                        pathfinder.start();
                    }
                    else
                    {
                        pathfinder.reset();
                        pathfinder.start();
                    }
                }

                updateModifierStates(shiftDown, ctrlDown, event.key);
            } break;
            case SDL_EVENT_KEY_UP:
            {
                updateModifierStates(shiftDown, ctrlDown, event.key);
            } break;
            }
        }

        if(mouseDown)
        {
            glm::vec2 mouse{};
            SDL_GetMouseState(&mouse.x, &mouse.y);

            auto coordinate = mouseToGrid(mouse, gridOrigin, cellSize);

            if(coordinate)
                editMap(map, *coordinate, editMode);
        }

        if(pathfinder.state() == SearchState::Running)
        {
            pathfinder.step();
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Draw
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderClear(renderer);

        map.getGrid().each([renderer, gridOrigin](CellType const& type, glm::uvec2 position)
        {
            SDL_FColor color{};
            switch(type)
            {
            case CellType::Empty:
                color = {180, 180, 180, 255};
            break;
            case CellType::Wall:
                color = {0, 0, 0, 255};
            break;
            case CellType::Start:
                color = {255, 0, 0, 255};
            break;
            case CellType::Goal:
                color = {0, 255, 0, 255};
            break;
            }
            
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_FRect rect{gridOrigin.x + position.x * 32.0f, gridOrigin.y + position.y * 32.0f, 32.0f, 32.0f};
            
            if(position.x > 0)
                rect.x -= position.x;
            
            if(position.y > 0)
                rect.y -= position.y;

            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderRect(renderer, &rect);
        });

        if(auto path = pathfinder.getPath(); path.has_value())
        {
            for(auto coordinate : *path)
            {
                SDL_SetRenderDrawColor(renderer, 128, 218, 0, 255);
                SDL_FRect rect{gridOrigin.x + 1 + coordinate.x * 32.0f, gridOrigin.y + 1 + coordinate.y * 32.0f, 30.0f, 30.0f};

                if(coordinate.x > 0)
                    rect.x -= coordinate.x;
                
                if(coordinate.y > 0)
                    rect.y -= coordinate.y;
                
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}