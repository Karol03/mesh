/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <optional>
#include <set>

#include "mesh.hpp"


namespace mesh
{

template <typename NodeDescription, typename EdgeDescription>
class MeshBuilder
{
    using NodePredicate = std::function<bool(const objects::Node<NodeDescription>&)>;
    using NodePredicateVec = std::vector<std::function<bool(const objects::Node<NodeDescription>&)>>;

public:
    explicit MeshBuilder(Mesh<NodeDescription, EdgeDescription>& mesh)
        : m_mesh{mesh}
    {}

    template <typename... Args>
    MeshBuilder& connect(Args&&... args)
    {
        m_mesh.tie(std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    MeshBuilder& connectTo(Args&&... args)
    {
        if (m_mesh.m_current != 0)
        {
            m_mesh.tie(m_mesh.m_current, std::forward<Args>(args)...);
        }
        return *this;
    }

    template <typename... Args>
    MeshBuilder& create(Args&&... args)
    {
        if (m_mesh.m_nodes.empty() || (m_mesh.m_current != 0))
        {
            m_mesh.attach(std::forward<Args>(args)...);
        }
        return *this;
    }

    template <typename... Args>
    MeshBuilder& remove(Args&&... args)
    {
        m_mesh.detach(std::forward<Args>(args)...);
        return *this;
    }

    MeshBuilder& hopVia(uint32_t edgeId)
    {
        if (m_mesh.m_current == 0)
        {
            return *this;
        }

        const auto& edges = m_mesh.m_nodes[m_mesh.m_current].edges();
        const auto edgeIdIt = edges.find(edgeId);
        if (edgeIdIt == edges.end())
        {
            m_mesh.m_current = 0;
            return *this;
        }

        const auto& matchEdge = m_mesh.m_edges[*edgeIdIt];
        if (matchEdge.nodes().first == edgeId)
        {
            m_mesh.m_current = matchEdge.nodes().second;
        }
        else
        {
            m_mesh.m_current = matchEdge.nodes().first;
        }
        return *this;
    }

    MeshBuilder& hopTo(uint32_t nodeId)
    {
        if (m_mesh.m_nodes.find(nodeId) != m_mesh.m_nodes.end())
        {
            m_mesh.m_current = nodeId;
        }
        else
        {
            m_mesh.m_current = 0;
        }
        return *this;
    }

    MeshBuilder& hopTo(const NodePredicate& nodePredicate)
    {
        const auto predicateWrapper  = [&nodePredicate](const auto& item) { return nodePredicate(item.second); };
        const auto it = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), predicateWrapper);
        if (it != m_mesh.m_nodes.end())
        {
            m_mesh.m_current = it->first;
        }
        else
        {
            m_mesh.m_current = 0;
        }
        return *this;
    }

    MeshBuilder& hopToPathEnd(const std::vector<uint32_t>& pathIds)
    {
        if (pathIds.empty())
        {
            m_mesh.m_current = 0;
            return *this;
        }

        for (auto i = 1u; i < pathIds.size(); ++i)
        {
            const auto& firstNodeId = pathIds[i - 1];
            const auto& secondNodeId = pathIds[i];
            if (!isConnected(firstNodeId, secondNodeId))
            {
                m_mesh.m_current = 0;
                return *this;
            }
        }

        m_mesh.m_current = pathIds.back();
        return *this;
    }

    MeshBuilder& hopToPathEnd(const NodePredicateVec& predicates)
    {
        if (predicates.empty())
        {
            m_mesh.m_current = 0;
            return *this;
        }

        auto nodeIds = std::vector<uint32_t>{};
        const auto firstPredicate = predicates[0];
        const auto firstPredicateWrapper = [&firstPredicate](const auto& item) { return firstPredicate(item.second); };

        auto it = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), firstPredicateWrapper);
        while (it != m_mesh.m_nodes.cend())
        {
            const auto currentNodeId = it->first;
            const auto lastNodeId = pathLastNodeId(predicates, currentNodeId);
            if (lastNodeId != 0)
            {
                m_mesh.m_current = lastNodeId;
                return *this;
            }

            ++it;
            it = std::find_if(it, m_mesh.m_nodes.cend(), firstPredicateWrapper);
        }

        m_mesh.m_current = 0;
        return *this;
    }

    MeshBuilder& hopToUniquePathEnd(const std::vector<uint32_t>& pathIds)
    {
        if (pathIds.size() > m_mesh.m_nodes.size())
        {
            m_mesh.m_current = 0;
            return *this;
        }

        const auto uniquePathIdsNumber = std::set(pathIds.begin(), pathIds.end()).size();
        if (uniquePathIdsNumber != pathIds.size())
        {
            m_mesh.m_current = 0;
            return *this;
        }

        return hopToPathEnd(pathIds);
    }

    MeshBuilder& hopToUniquePathEnd(const NodePredicateVec& predicates)
    {
        if (predicates.empty() || (predicates.size() > m_mesh.m_nodes.size()))
        {
            m_mesh.m_current = 0;
            return *this;
        }

        auto visitedNodeIds = std::set<uint32_t>{};
        const auto& firstPredicate = predicates[0];
        const auto firstPredicateWrapper = [&firstPredicate](const auto& item) { return firstPredicate(item.second); };

        auto it = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), firstPredicateWrapper);
        while (it == m_mesh.m_nodes.cend())
        {
            const auto& currentNodeId = it->first;
            const auto lastNodeId = uniquePathLastNodeId(predicates,
                                                         currentNodeId,
                                                         visitedNodeIds);
            if (lastNodeId != 0)
            {
                m_mesh.m_current = lastNodeId;
                return *this;
            }

            ++it;
            it = std::find_if(it, m_mesh.m_nodes.cend(), firstPredicateWrapper);
        }
        m_mesh.m_current = 0;
        return *this;
    }


    inline uint32_t currentId()
    {
        return m_mesh.m_current;
    }

    std::optional<NodeDescription> currentValue()
    {
        auto it = m_mesh.m_nodes.find(m_mesh.m_current);
        if (it != m_mesh.m_nodes.end())
        {
            return it->second.value();
        }
        return {};
    }

    std::vector<uint32_t> pathBetween(uint32_t begin, uint32_t end)
    {
        auto& nodes = m_mesh.m_nodes;
        if (begin == end)
        {
            return {begin};
        }
        else if (!m_mesh.contains(nodes, begin))
        {
            return {};
        }
        else if (!m_mesh.contains(nodes, end))
        {
            return {};
        }

        return m_mesh.bidirectionalAStart(begin, end);
    }

    std::vector<uint32_t> pathBetween(uint32_t begin, const NodePredicate& end)
    {
        const auto endPredicateWrapper = [&begin, &end](const auto& item) { return (begin != item.first) && end(item.second); };
        auto endIt = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), endPredicateWrapper);
        if (endIt == m_mesh.m_nodes.cend())
        {
            const auto& beginItem = m_mesh.m_nodes[begin];
            if (end(beginItem))
            {
                return {begin};
            }
            else
            {
                return {};
            }
        }
        return path(begin, endIt->first);
    }

    std::vector<uint32_t> pathBetween(const NodePredicate& begin, const NodePredicate& end)
    {
        const auto beginPredicateWrapper = [&begin](const auto& item) { return begin(item.second); };
        const auto beginIt = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), beginPredicateWrapper);
        if (beginIt == m_mesh.m_nodes.cend())
        {
            return {};
        }

        const auto beginNodeId = beginIt->first;
        const auto endPredicateWrapper = [&beginNodeId, &end](const auto& item) { return (beginNodeId != item.first) && end(item.second); };
        const auto endIt = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), endPredicateWrapper);
        if (endIt == m_mesh.m_nodes.cend())
        {
            const auto& beginItem = m_mesh.m_nodes[beginNodeId];
            if (end(beginItem))
            {
                return {begin};
            }
            else
            {
                return {};
            }
        }

        return path(beginIt->first, endIt->first);
    }

private:
    bool isConnected(uint32_t firstNodeId, uint32_t secondNodeId)
    {
        if (!m_mesh.contains(m_mesh.m_nodes, firstNodeId))
        {
            return false;
        }

        for (const auto& edgeId : m_mesh.m_nodes[firstNodeId].edges())
        {
            const auto& edge = m_mesh.m_edges[edgeId];
            const auto nodeId = (edge.nodes().first == firstNodeId ?
                                     edge.nodes().second :
                                     edge.nodes().first);
            if (nodeId == secondNodeId)
            {
                return true;
            }
        }
        return false;
    }

    uint32_t pathLastNodeId(const NodePredicateVec& predicates,
                            uint32_t fromNodeId,
                            uint32_t depth = 1)
    {
        if (depth == predicates.size())
        {
            return fromNodeId;
        }

        const auto& currentPredicate = predicates[depth];
        const auto& outcomingEdgeIds = m_mesh.m_nodes[fromNodeId].edges();
        for (const auto& edgeId : outcomingEdgeIds)
        {
            const auto& edge = m_mesh.m_edges[edgeId];
            const auto nodeId = (edge.nodes().first == fromNodeId ?
                                     edge.nodes().second :
                                     edge.nodes().first);
            const auto& node = m_mesh.m_nodes[nodeId];

            if (currentPredicate(node))
            {
                const auto lastNodeId = pathLastNodeId(predicates, nodeId, depth + 1);
                if (lastNodeId != 0)
                {
                    return lastNodeId;
                }
            }
        }
        return 0;
    }

    uint32_t uniquePathLastNodeId(const NodePredicate& predicates,
                                  uint32_t fromNodeId,
                                  std::set<uint32_t>& visitedNodeIds,
                                  uint32_t depth = 1)
    {
        if (depth == predicates.size())
        {
            return fromNodeId;
        }

        const auto& currentPredicate = predicates[depth];
        const auto& outcomingEdgeIds = m_mesh.m_nodes[fromNodeId].edges();
        for (const auto& edgeId : outcomingEdgeIds)
        {
            const auto& edge = m_mesh.m_edges[edgeId];
            const auto nodeId = (edge.nodes().first == fromNodeId ?
                                     edge.nodes().second :
                                     edge.nodes().first);
            const auto& node = m_mesh.m_nodes[nodeId];

            if (currentPredicate(node) && !visitedNodeIds.insert(nodeId).second)
            {
                const auto lastNodeId = uniquePathLastNodeId(predicates,
                                                             nodeId,
                                                             visitedNodeIds,
                                                             depth + 1);
                if (lastNodeId != 0)
                {
                    return lastNodeId;
                }
                visitedNodeIds.erase(nodeId);
            }
        }
        return false;
    }

private:
    Mesh<NodeDescription, EdgeDescription>& m_mesh;
};

}  // namespace mesh
