#pragma once

#include <optional>
#include <vector>

#include "Grid.hpp"

enum class CellType
{
    Empty,
    Wall,
    Start,
    Goal
};

class GridMap
{
public:
    GridMap() = default;
    
    GridMap(glm::ivec2 gridSize)
        : m_Grid(gridSize)
    {}

    void resize(glm::ivec2 size)
    {
        m_Grid.resize(size);
    }

    auto setStart(glm::ivec2 coordinate) -> bool
    {
        return set(coordinate, CellType::Start);
    }

    auto setGoal(glm::ivec2 coordinate) -> bool
    {
        return set(coordinate, CellType::Goal);
    }

    auto setWall(glm::ivec2 coordinate) -> bool
    {
        return set(coordinate, CellType::Wall);
    }

    auto setEmpty(glm::ivec2 coordinate) -> bool
    {
        return set(coordinate, CellType::Empty);
    }

    [[nodiscard]]
    auto getGrid() const -> Grid<CellType> const&
    {
        return m_Grid;
    }

    [[nodiscard]]
    auto getStart() const -> std::optional<glm::ivec2> 
    {
        return m_Start;
    }

    [[nodiscard]]
    auto getGoal() const -> std::optional<glm::ivec2>
    {
        return m_Goal;
    }

    [[nodiscard]]
    auto isWalkable(glm::ivec2 coordinate) const noexcept -> bool
    {
        return m_Grid.contains(coordinate) && m_Grid[coordinate] != CellType::Wall;
    }

    [[nodiscard]]
    auto size() const noexcept -> glm::ivec2
    {
        return m_Grid.size();
    }

private:
    [[nodiscard]]
    auto set(glm::ivec2 coordinate, CellType type) -> bool
    {
        if(!m_Grid.contains(coordinate))
            return false;
        
        switch(type)
        {
        case CellType::Wall:
        case CellType::Empty:
        {
            if(m_Grid[coordinate] == CellType::Start || m_Grid[coordinate] == CellType::Goal)
            {
                return false;
            }
            
            m_Grid[coordinate] = type;
        } break;
        case CellType::Start:
        {
            if(m_Start.has_value() && coordinate == m_Goal)
            {
                std::swap(m_Grid[*m_Start], m_Grid[*m_Goal]);
                std::swap(m_Start, m_Goal);
            }
            else
            {
                if(m_Start.has_value())
                    m_Grid[*m_Start] = CellType::Empty;

                m_Start = coordinate;
                m_Grid[coordinate] = type;
            }
        } break;
        case CellType::Goal:
        {
            if(m_Goal.has_value() && coordinate == m_Start)
            {
                std::swap(m_Grid[*m_Goal], m_Grid[*m_Start]);
                std::swap(m_Goal, m_Start);
            }
            else
            {   
                if(m_Goal.has_value())
                    m_Grid[*m_Goal] = CellType::Empty;

                m_Goal = coordinate;
                m_Grid[coordinate] = type;
            }
        } break;
        }

        return true;
    }

private:
    Grid<CellType> m_Grid{};
    std::optional<glm::ivec2> m_Start{};
    std::optional<glm::ivec2> m_Goal{};
};