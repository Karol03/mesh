/**
 * Created by Karol Dudzic @ 2022
 */
#include "meshbuilder.hpp"


namespace mesh
{

MeshBuilder& MeshBuilder::hopVia(uint32_t id)
{
    if (m_mesh.m_current == 0)
    {
        return *this;
    }

    const auto& edges = m_mesh.m_nodes[m_mesh.m_current].edges();
    const auto edgeIdIt = edges.find(id);
    if (edgeIdIt == edges.end())
    {
        return *this;
    }

    const auto& matchEdge = m_mesh.m_edges[*edgeIdIt];
    if (matchEdge.nodes().first == id)
    {
        m_mesh.m_current = matchEdge.nodes().second;
    }
    else
    {
        m_mesh.m_current = matchEdge.nodes().first;
    }
    return *this;
}

MeshBuilder& MeshBuilder::hopTo(uint32_t id)
{
    if (m_mesh.m_nodes.find(id) != m_mesh.m_nodes.end())
    {
        m_mesh.m_current = id;
    }
    return *this;
}

MeshBuilder& MeshBuilder::hopTo(std::function<bool(const objects::Node&)> predicate)
{
    const auto predicateWrapper  = [&predicate](const auto& item) { return predicate(item.second); };
    const auto it = std::find_if(m_mesh.m_nodes.cbegin(), m_mesh.m_nodes.cend(), predicateWrapper);
    if (it != m_mesh.m_nodes.end())
    {
        m_mesh.m_current = it->first;
    }
    return *this;
}

}  // namespace mesh
