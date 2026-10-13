#pragma once

#include <queue>
#include <array>

#include <glm/glm.hpp>

#include "GridMap.hpp"
#include "OptionalRef.hpp"

template<typename H>
concept Heuristic = requires(H h, glm::ivec2 point1, glm::ivec2 point2)
{
    { std::invoke(h, point1, point2) } -> std::convertible_to<float>;
};

namespace heuristics
{

[[nodiscard]]
constexpr auto manhattan(glm::ivec2 point1, glm::ivec2 point2) noexcept -> float
{
    auto const diff = glm::abs(point1 - point2);
    return static_cast<float>(diff.x + diff.y);
}

[[nodiscard]]
auto euclidean(glm::ivec2 point1, glm::ivec2 point2) noexcept -> float
{
    return glm::distance(glm::vec2(point1), glm::vec2(point2));
}

} /* -Heuristics */

constexpr std::array<glm::ivec2, 8> directionsArray = 
{{
    {  0,  1 },
    {  1,  0 },
    {  0, -1 },
    { -1,  0 },
    {  1,  1 }, // Use for full 8 directions
    {  1, -1 },
    { -1, -1 },
    { -1,  1 } 
}};

constexpr std::span<const glm::ivec2> directions4 {directionsArray.data(), 4};
constexpr std::span<const glm::ivec2> directions8 {directionsArray.data(), 8};

enum class SearchState
{
    Ready,
    Running,
    FoundPath,
    NoPath
};

enum class NodeState
{
    Unvisited,
    Open,
    Closed
};

class SearchNode
{
public:
    constexpr SearchNode() noexcept = default;
    
    constexpr SearchNode(float g, float h, std::optional<glm::ivec2> previous = std::nullopt) noexcept
        : gCost(g), hCost(h), previous(previous) {}

    [[nodiscard]]
    constexpr auto fCost() const noexcept -> float
    {
        return gCost + hCost;
    }

public:
    float gCost{std::numeric_limits<float>::max()};
    float hCost{0.0f};

    NodeState state{NodeState::Unvisited};

    std::optional<glm::ivec2> previous;
};

struct QueueEntry
{
    glm::ivec2 coordinate{};
    float fCost{};
    float gCost{};
};

[[nodiscard]]
constexpr auto operator>(QueueEntry const& lhs, QueueEntry const& rhs) noexcept -> bool
{
    return lhs.fCost > rhs.fCost;
}
    

class AStar
{
public:
    using HeuristicFn = std::function<float(glm::ivec2 p1, glm::ivec2 p2)>;

    AStar() noexcept = default;

    AStar(GridMap const& map)
        : m_Map(map)
    {}

    void setNodeMap(GridMap const& map) noexcept { m_Map = map;}

    auto start() -> bool
    {
        if(!m_Map)
            return false;
        
        if(!m_Map->getStart())
            return false;
        
        if(!m_Map->getGoal())
            return false;

        m_Nodes.resize(m_Map->size());
        auto const start = m_Map->getStart().value();
        auto const goal = m_Map->getGoal().value();

        auto& startNode = m_Nodes[start];
        startNode.gCost = 0.0f;
        startNode.hCost = m_Heuristic(start, goal);
        startNode.state = NodeState::Open;

        // Push start into open set
        m_OpenSet.push({ start, startNode.fCost(), startNode.gCost });

        m_State = SearchState::Running;
        return true;
    }
    
    void step()
    {
        if(m_State != SearchState::Running)
            return;
        
        if(m_OpenSet.empty())
        {
            m_State = SearchState::NoPath;
            return;
        }

        auto currentEntry = m_OpenSet.top();
        m_OpenSet.pop();

        auto& currentNode = m_Nodes[currentEntry.coordinate];

        if(currentEntry.gCost != currentNode.gCost)
            return;

        if(currentNode.state == NodeState::Closed)
            return;

        currentNode.state = NodeState::Closed;

        if(currentEntry.coordinate == m_Map->getGoal())
        {
            m_State = SearchState::FoundPath;
            constructPath(currentEntry.coordinate);
            return;
        }
        
        for(auto const& direction : directions8)
        {
            auto const neighborCoordinate = currentEntry.coordinate + direction;

            if(!m_Map->isWalkable(neighborCoordinate))
                continue;

            auto& neighborNode = m_Nodes[neighborCoordinate];

            // +1 should be replaced with the distance from current to neighbor
            auto gCost = currentNode.gCost + 1;
            if(gCost < neighborNode.gCost)
            {
                neighborNode.previous = currentEntry.coordinate;
                neighborNode.gCost = gCost;
                neighborNode.hCost = m_Heuristic(neighborCoordinate, m_Map->getGoal().value());
                neighborNode.state = NodeState::Open;

                m_OpenSet.push({ neighborCoordinate, neighborNode.fCost(), neighborNode.gCost });
            }
        }
    }

    void reset()
    {
        m_State = SearchState::Ready;
        m_Nodes.reset();
        m_OpenSet = {};
        m_Path.clear();
    }

    [[nodiscard]]
    auto state() const -> SearchState
    {
        return m_State;
    }

    [[nodiscard]]
    auto getPath() const noexcept -> asd::OptionalRef<const std::vector<glm::ivec2>>
    {
        if(m_State != SearchState::FoundPath)
            return asd::NullOption;
        
        if(m_Path.empty())
            return asd::NullOption;
        
        return m_Path;
    }

private:
    void constructPath(glm::ivec2 fromCoordinate)
    {
        std::optional<glm::ivec2> from = fromCoordinate;
        while(from.has_value())
        {
            m_Path.push_back(*from);
            from = m_Nodes[*from].previous;
        }

        std::reverse(m_Path.begin(), m_Path.end());
    }

private:
    SearchState m_State{SearchState::Ready};
    asd::OptionalRef<const GridMap> m_Map;

    Grid<SearchNode> m_Nodes;
    using PriorityQueue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>>;
    PriorityQueue m_OpenSet;

    HeuristicFn m_Heuristic {heuristics::manhattan};
    
    std::vector<glm::ivec2> m_Path;
};