/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include "mesh.hpp"


namespace mesh
{

class MeshBuilder
{
public:
    explicit MeshBuilder(Mesh& mesh)
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
        m_mesh.attach(std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    MeshBuilder& remove(Args&&... args)
    {
        m_mesh.detach(std::forward<Args>(args)...);
        return *this;
    }

    MeshBuilder& hopVia(uint32_t id);
    MeshBuilder& hopTo(uint32_t id);
    MeshBuilder& hopTo(std::function<bool(const objects::Node&)> predicate);

private:
    Mesh& m_mesh;
};

}  // namespace mesh
