#pragma once

#include <vector>
#include <span>
#include <functional>
#include <algorithm>

#include <glm/vec2.hpp>

#include "OptionalRef.hpp"

template<typename T>
class Grid
{
public:
    Grid() noexcept = default;

    Grid(glm::ivec2 dimensions)
        : Grid(dimensions.x, dimensions.y)
    {}

    Grid(int width, int height)
    {
        assert(width >= 0);
        assert(height >= 0);

        m_Size = {width, height};
        m_Cells.resize(width * height);
    }

    [[nodiscard]]
    auto width() const noexcept -> int
    {
        return m_Size.x;
    }

    [[nodiscard]]
    auto height() const noexcept -> int
    {
        return m_Size.y;
    }

    [[nodiscard]]
    auto size() const noexcept -> glm::ivec2
    {
        return m_Size;
    }

    void resize(glm::ivec2 size)
    {
        clear();

        m_Size = size;
        m_Cells.resize(size.x * size.y);
    }

    [[nodiscard]]
    auto cells() const noexcept -> std::span<const T>
    {
        return m_Cells;
    }

    template<typename Callback>
    void each(Callback&& callback)
    {
        for(std::size_t y = 0; y < m_Size.y; y++)
        {
            for(std::size_t x = 0; x < m_Size.x; x++)
            {
                glm::ivec2 const coordinate(x, y);
                callback(m_Cells[toIndex(coordinate)], coordinate);
            }
        }
    }

    template<typename Callback>
    void each(Callback&& callback) const
    {
        for(std::size_t y = 0; y < m_Size.y; y++)
        {
            for(std::size_t x = 0; x < m_Size.x; x++)
            {
                glm::ivec2 const coordinate(x, y);
                callback(m_Cells[toIndex(coordinate)], coordinate);
            }
        }
    }

    [[nodiscard]]
    auto at(glm::ivec2 coordinate) -> asd::OptionalRef<T>
    {
        if(!contains(coordinate))
            return asd::NullOption;
        
        return m_Cells[toIndex(coordinate)];    
    }

    [[nodiscard]]
    auto at(glm::ivec2 coordinate) const -> asd::OptionalRef<const T>
    {
        if(!contains(coordinate))
            return asd::NullOption;
        
        return m_Cells[toIndex(coordinate)];
    }

    [[nodiscard]]
    auto empty() const noexcept -> bool 
    {
        return m_Cells.empty();
    }

    void clear() noexcept
    {
        m_Cells.clear();
    }

    void reset() noexcept
    {
        std::ranges::fill(m_Cells, T{});
    }

    [[nodiscard]]
    auto contains(glm::ivec2 coordinate) const noexcept -> bool
    {
        return coordinate.x >= 0 && coordinate.x < m_Size.x && coordinate.y >= 0 && coordinate.y < m_Size.y;
    }

    [[nodiscard]]
    auto operator[](glm::ivec2 coordinate) -> T&
    {
        return m_Cells[toIndex(coordinate)];
    }

    [[nodiscard]]
    auto operator[](glm::ivec2 coordinate) const -> T const&
    {
        return m_Cells[toIndex(coordinate)];
    }

private:

    [[nodiscard]]
    auto toIndex(glm::ivec2 coordinate) const noexcept -> std::size_t
    {
        return (coordinate.y * m_Size.x + coordinate.x);
    }

private:
    glm::ivec2 m_Size;
    std::vector<T> m_Cells;
};