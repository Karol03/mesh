/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <functional>
#include <vector>

#include "objects/edge.hpp"
#include "objects/imobject.hpp"
#include "objects/node.hpp"
#include "objects/types.hpp"


namespace mesh
{
template <typename NodeDescription, typename EdgeDescription>
class MeshBuilder;

namespace utils
{
template <typename NodeDescription, typename EdgeDescription>
class MeshPack;
}  // namespace utils

template <typename NodeDescription, typename EdgeDescription = NodeDescription>
struct Mesh
{
    friend class MeshBuilder<NodeDescription, EdgeDescription>;
    friend class utils::MeshPack<NodeDescription, EdgeDescription>;

    using U32PairMap = objects::types::U32PairMap;
    using U32EdgeMap = objects::types::U32EdgeMap<EdgeDescription>;
    using U32NodeMap = objects::types::U32NodeMap<NodeDescription>;
    using U32Pair = objects::types::U32Pair;
    using U32PairPriorityQueue = objects::types::U32PairPriorityQueue;
    using U32Set = objects::types::U32Set;
    using NodePredicate = std::function<bool(const NodeDescription&)>;
    using NodeVisitFunction = std::function<void(const NodeDescription&)>;
    using EdgeVisitFunction = std::function<void(const EdgeDescription&)>;

public:
    explicit Mesh()
        : m_nodes{}
        , m_edges{}
        , m_current{}
    {}

    void attach(NodeDescription nodeDescription = NodeDescription{},
                EdgeDescription edgeDescription = EdgeDescription{})
    {
        if (m_current == 0)
        {
            auto node = objects::Node{std::move(nodeDescription)};
            auto nodeId = nodeIdGenerator();
            m_current = nodeId;
            m_nodes.insert({nodeId, std::move(node)});
        }
        else
        {
            auto node = objects::Node{std::move(nodeDescription)};
            auto edge = objects::Edge{std::move(edgeDescription)};
            auto nodeId = nodeIdGenerator();
            auto edgeId = edgeIdGenerator();

            m_nodes[m_current].edges().insert(edgeId);
            node.edges().insert(edgeId);
            edge.nodes().first = m_current;
            edge.nodes().second = nodeId;

            m_current = nodeId;

            m_nodes.insert({nodeId, std::move(node)});
            m_edges.insert({edgeId, std::move(edge)});
        }
    }

    void tie(uint32_t firstNodeId, uint32_t secondNodeId,
             EdgeDescription edgeDescription = EdgeDescription{})
    {
        if (!contains(m_nodes, firstNodeId) ||
            !contains(m_nodes, secondNodeId))
        {
            return;
        }

        auto& leftNodeEdges = m_nodes[firstNodeId].edges();
        auto& rightNodeEdges = m_nodes[secondNodeId].edges();
        if (isIntersection(leftNodeEdges, rightNodeEdges))
        {
            return;
        }

        auto edge = objects::Edge{std::move(edgeDescription)};
        auto edgeId = edgeIdGenerator();
        edge.nodes().first = firstNodeId;
        edge.nodes().second = secondNodeId;
        leftNodeEdges.insert(edgeId);
        rightNodeEdges.insert(edgeId);
        m_edges.insert({edgeId, std::move(edge)});
    }

    void tie(uint32_t firstNodeId,
             NodePredicate secondNodePredicate,
             EdgeDescription edgeDescription = EdgeDescription{})
    {
        if (!contains(m_nodes, firstNodeId))
        {
            return;
        }

        auto rightNodeId = 0u;
        for (const auto& item : m_nodes)
        {
            if (secondNodePredicate(item.second.value()))
            {
                rightNodeId = item.first;
                if (firstNodeId == rightNodeId)
                {
                    return;
                }
                tie(firstNodeId, rightNodeId, std::move(edgeDescription));
            }
        }
    }

    void tie(NodePredicate firstNodePredicate,
             NodePredicate secondNodePredicate,
             EdgeDescription edgeDescription = EdgeDescription{})
    {
        auto firstNodeId = 0u;
        auto rightNodeId = 0u;

        for (const auto& item : m_nodes)
        {
            if (firstNodePredicate(item.second.value()))
            {
                firstNodeId = item.first;
                if (firstNodeId != 0 && rightNodeId != 0)
                {
                    if (firstNodeId == rightNodeId)
                    {
                        return;
                    }
                    tie(firstNodeId, rightNodeId, std::move(edgeDescription));
                }
            }
            else if (secondNodePredicate(item.second.value()))
            {
                rightNodeId = item.first;
                if (firstNodeId != 0 && rightNodeId != 0)
                {
                    if (firstNodeId == rightNodeId)
                    {
                        return;
                    }
                    tie(firstNodeId, rightNodeId, std::move(edgeDescription));
                }
            }
        }
    }

    void detach()
    {
        if (m_current != 0)
        {
            detach(m_current);
        }
    }

    void detach(uint32_t id)
    {
        const auto nodeItemIt = m_nodes.find(id);
        if (nodeItemIt == m_nodes.end())
        {
            return;
        }

        const auto itemEdgeIds = nodeItemIt->second.edges();
        if (itemEdgeIds.empty())
        {
            m_nodes.erase(id);
        }
        else if (itemEdgeIds.size() == 1)
        {
            const auto edgeId = *itemEdgeIds.begin();
            auto nodeFirst = m_edges[edgeId].nodes().first;
            auto nodeSecond = m_edges[edgeId].nodes().second;

            m_nodes[nodeFirst].edges().erase(edgeId);
            m_nodes[nodeSecond].edges().erase(edgeId);
            m_edges.erase(edgeId);
            m_nodes.erase(id);
        }
        else
        {
            auto relatedNodes = U32Set{};
            for (const auto edgeId : itemEdgeIds)
            {
                auto nodeFirst = m_edges[edgeId].nodes().first;
                auto nodeSecond = m_edges[edgeId].nodes().second;

                m_nodes[nodeFirst].edges().erase(edgeId);
                m_nodes[nodeSecond].edges().erase(edgeId);
                m_edges.erase(edgeId);
                m_nodes.erase(id);

                if (nodeFirst == id)
                {
                    relatedNodes.insert(nodeSecond);
                }
                else
                {
                    relatedNodes.insert(nodeFirst);
                }
            }
            rebranch(std::move(relatedNodes));
        }

        if (m_current == id)
        {
            m_current = 0;
        }
        else if (m_nodes.find(m_current) == m_nodes.end())
        {
            m_current = 0;
        }
    }

    void detach(std::vector<uint32_t> range)
    {
        for (const auto& id : range)
        {
            detach(id);
        }
    }

    void detach(NodePredicate predicate)
    {
        auto detachRange = std::vector<uint32_t>{};
        for (const auto& item : m_nodes)
        {
            if (predicate(item.second.value()))
            {
                detachRange.push_back(item.first);
            }
        }
        detach(std::move(detachRange));
    }

    void visit(NodeVisitFunction nodeVisit, EdgeVisitFunction edgeVisit) const
    {
        for (const auto& node : m_nodes)
        {
            nodeVisit(node.second.value());
        }

        for (const auto& edge : m_edges)
        {
            edgeVisit(edge.second.value());
        }
    }

    void clear()
    {
        m_current = 0;
        m_nodes.clear();
        m_edges.clear();
    }

private:
    template <typename Container, typename T>
    bool contains(const Container& container, const T& element)
    {
        return container.find(element) != container.end();
    }

    static uint32_t nodeIdGenerator()
    {
        static uint32_t id = 0;
        return ++id;
    }

    static uint32_t edgeIdGenerator()
    {
        static uint32_t id = 0;
        return ++id;
    }

    bool isIntersection(const U32Set& lhs, const U32Set& rhs)
    {
        return intersectionPoint(lhs, rhs) != 0;
    }

    uint32_t intersectionPoint(const U32Set& lhs, const U32Set& rhs)
    {
        if (lhs.size() < rhs.size())
        {
            for (const auto& x : lhs)
            {
                if (rhs.find(x) != rhs.end())
                {
                    return x;
                }
            }
        }
        else
        {
            for (const auto& x : rhs)
            {
                if (lhs.find(x) != lhs.end())
                {
                    return x;
                }
            }
        }
        return 0;
    }

    std::pair<U32Set, U32Set> dfs(uint32_t nodeId)
    {
        auto visitedNodes = U32Set{};
        auto visitedEdges = U32Set{};

        auto toVisit = std::vector<uint32_t>{nodeId};
        while (!toVisit.empty())
        {
            const auto nodeId = toVisit.back();
            toVisit.pop_back();
            visitedNodes.insert(nodeId);

            for (const auto edgeId : m_nodes[nodeId].edges())
            {
                if (contains(visitedEdges, edgeId))
                {
                    continue;
                }
                visitedEdges.insert(edgeId);

                auto nextNode = m_edges[edgeId].nodes().first == nodeId ?
                                m_edges[edgeId].nodes().second :
                                m_edges[edgeId].nodes().first;

                if (!contains(visitedNodes, nextNode))
                {
                    toVisit.push_back(nextNode);
                }
            }
        }

        return {visitedNodes, visitedEdges};
    }

    std::pair<bool, int> bidirectionalAStart(const uint32_t leftBranchRoot,
                                             const uint32_t rightBranchRoot,
                                             U32Set& leaves)
    {
        constexpr auto pathExist = true;
        constexpr auto pathNotExist = false;
        constexpr auto BOTH_BRANCHES_EQUAL = 0;
        constexpr auto LEFT_BRANCH_IS_BIGGER = -1;
        constexpr auto RIGHT_BRANCH_IS_BIGGER = 1;

        auto visitedBegin = U32Set{};
        auto visitedEnd = U32Set{};

        auto pQueueLeftBranch = U32PairPriorityQueue{};
        auto pQueueRightBranch = U32PairPriorityQueue{};

        pQueueLeftBranch.push({0, leftBranchRoot});
        pQueueRightBranch.push({0, rightBranchRoot});

        while (!pQueueLeftBranch.empty() && !pQueueRightBranch.empty())
        {
            auto [bPrio, begin] = pQueueLeftBranch.top();
            pQueueLeftBranch.pop();
            auto [ePrio, end] = pQueueRightBranch.top();
            pQueueRightBranch.pop();

            visitedBegin.insert(begin);
            visitedEnd.insert(end);

            if (contains(leaves, begin))
            {
                leaves.erase(begin);
            }

            if (contains(leaves, end))
            {
                leaves.erase(end);
            }

            if (isIntersection(visitedBegin, visitedEnd))
            {
                return {pathExist, BOTH_BRANCHES_EQUAL};
            }

            for (const auto bConnect : getConnectedNodes(begin))
            {
                if (!contains(visitedBegin, bConnect))
                {
                    pQueueLeftBranch.push({bPrio + 1, bConnect});
                }
            }

            for (const auto eConnect : getConnectedNodes(end))
            {
                if (!contains(visitedEnd, eConnect))
                {
                    pQueueRightBranch.push({ePrio + 1, eConnect});
                }
            }
        }

        if (pQueueLeftBranch.empty())
        {
            return {pathNotExist, RIGHT_BRANCH_IS_BIGGER};
        }
        else
        {
            return {pathNotExist, LEFT_BRANCH_IS_BIGGER};
        }
    }

    std::vector<uint32_t> bidirectionalAStart(const uint32_t leftBranchRoot,
                                              const uint32_t rightBranchRoot)
    {
        auto visitedBegin = U32Set{};
        auto visitedEnd = U32Set{};
        auto nodeToParentMapBegin = U32PairMap{};
        auto nodeToParentMapEnd = U32PairMap{};

        auto pQueueLeftBranch = U32PairPriorityQueue{};
        auto pQueueRightBranch = U32PairPriorityQueue{};

        pQueueLeftBranch.push({0, leftBranchRoot});
        pQueueRightBranch.push({0, rightBranchRoot});

        while (!pQueueLeftBranch.empty() && !pQueueRightBranch.empty())
        {
            auto [bPrio, begin] = pQueueLeftBranch.top();
            pQueueLeftBranch.pop();
            auto [ePrio, end] = pQueueRightBranch.top();
            pQueueRightBranch.pop();

            visitedBegin.insert(begin);
            visitedEnd.insert(end);

            auto commonNode = intersectionPoint(visitedBegin, visitedEnd);
            if (commonNode != 0)
            {
                auto result = std::vector<uint32_t>{};

                while (nodeToParentMapBegin[commonNode] != 0)
                {
                    result.push_back(nodeToParentMapBegin[begin]);
                }

                result = std::vector(result.rbegin(), result.rend());

                while (nodeToParentMapBegin[commonNode] != 0)
                {
                    result.push_back(nodeToParentMapBegin[begin]);
                }

                return result;
            }

            for (const auto bConnect : getConnectedNodes(begin))
            {
                if (!contains(visitedBegin, bConnect))
                {
                    pQueueLeftBranch.push({bPrio + 1, bConnect});
                    nodeToParentMapBegin[bConnect] = begin;
                }
            }

            for (const auto eConnect : getConnectedNodes(end))
            {
                if (!contains(visitedEnd, eConnect))
                {
                    pQueueRightBranch.push({ePrio + 1, eConnect});
                    nodeToParentMapEnd[eConnect] = begin;
                }
            }
        }

        return {};
    }

    std::vector<uint32_t> getConnectedNodes(uint32_t nodeId)
    {
        auto result = std::vector<uint32_t>{};
        result.reserve(m_nodes[nodeId].edges().size());

        for (const auto edgeId : m_nodes[nodeId].edges())
        {
            if (m_edges[edgeId].nodes().first == nodeId)
            {
                result.push_back(m_edges[edgeId].nodes().second);
            }
            else
            {
                result.push_back(m_edges[edgeId].nodes().first);
            }
        }

        return result;
    }

    void deleteBranch(uint32_t nodeId, U32Set& leaves)
    {
        auto [visitedNodes, visitedEdges] = dfs(nodeId);

        for (const auto edgeId : visitedEdges)
        {
            m_edges.erase(edgeId);
        }

        for (const auto nodeId : visitedNodes)
        {
            m_nodes.erase(nodeId);
            leaves.erase(nodeId);
        }
    }

    void rebranch(U32Set leaves)
    {
        auto leftBranch = *leaves.begin();
        leaves.erase(leaves.begin());

        while (!leaves.empty())
        {
            auto rightBranch = *leaves.begin();
            leaves.erase(leaves.begin());

            const auto [isPathExist, biggerBranch] = bidirectionalAStart(leftBranch, rightBranch, leaves);
            if (!isPathExist)
            {
                constexpr auto LEFT_BRANCH_NO = -1;
                if (biggerBranch != LEFT_BRANCH_NO)
                {
                    std::swap(leftBranch, rightBranch);
                }
                deleteBranch(rightBranch, leaves);
            }
        }
    }

    uint32_t insertNode(NodeDescription description)
    {
        auto node = objects::Node{std::move(description)};
        auto nodeId = nodeIdGenerator();
        m_nodes.insert({nodeId, std::move(node)});
        return nodeId;
    }

    uint32_t insertEdge(U32Pair endpointNodes, EdgeDescription description)
    {
        auto edge = objects::Edge{std::move(description)};
        auto edgeId = edgeIdGenerator();
        edge.nodes() = endpointNodes;
        m_edges.insert({edgeId, std::move(edge)});
        return edgeId;
    }

private:
    U32NodeMap m_nodes;
    U32EdgeMap m_edges;
    uint32_t m_current;
};

}  // namespace mesh
