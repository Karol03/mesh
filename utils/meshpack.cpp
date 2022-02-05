/**
 * Created by Karol Dudzic @ 2022
 */
#include "meshpack.hpp"

#include <fstream>
#include <optional>
#include <sstream>


namespace mesh
{
namespace utils
{
namespace
{
void skipWhiteChars(mesh::utils::MeshPack::StringFile& str)
{
    if (str.cursor >= str.data.size())
        return;

    while (str.data[str.cursor] == '\n' ||
           str.data[str.cursor] == '\t' ||
           str.data[str.cursor] == ' '  ||
           str.data[str.cursor] == ';')
        ++str.cursor;
}

std::optional<std::size_t> get_size_t(mesh::utils::MeshPack::StringFile& str)
{
    skipWhiteChars(str);
    const auto bPos = str.cursor;
    if (bPos >= str.data.size())
    {
        return {};
    }

    while (isdigit(str.data[str.cursor]))
        ++str.cursor;

    const auto number = std::string(&str.data[bPos], str.cursor - bPos);
    return static_cast<std::size_t>(std::atoll(number.c_str()));
}

std::optional<std::size_t> get_size_t(std::ifstream& str)
{
    auto result = std::size_t{};
    if ((str >> result))
    {
        return result;
    }
    return {};
}

std::optional<std::string> get_string(mesh::utils::MeshPack::StringFile& str)
{
    skipWhiteChars(str);
    const auto bPos = str.cursor;
    if (bPos >= str.data.size())
    {
        return {};
    }
    else if (str.data[bPos] != '"')
    {
        return {};
    }

    while (str.data[str.cursor] != '\0')
    {
        if (str.data[str.cursor] == '"' && str.data[str.cursor + 1] == '\n')
        {
            ++str.cursor;
            break;
        }
        ++str.cursor;
    }

    return std::string(&str.data[bPos + 1], str.cursor - bPos - 2);
}

std::optional<std::string> get_string(std::ifstream& str)
{
    auto result = std::string{};
    if ((str >> result))
    {
        return result;
    }
    return {};
}
}  // namespace


MeshPack::MeshPack(Mesh& mesh)
    : m_mesh{mesh}
{}

std::string MeshPack::to_string() const
{
    auto result = std::stringstream{};

    result << m_mesh.m_nodes.size() << ';' << m_mesh.m_edges.size() << '\n';
    for (const auto& nodeItem : m_mesh.m_nodes)
    {
        result << nodeItem.first << ";\"" << nodeItem.second.value().to_string() << "\"\n";
    }
    for (const auto& edgeItem : m_mesh.m_edges)
    {
        result << edgeItem.first << ';'
               << edgeItem.second.nodes().first << ';'
               << edgeItem.second.nodes().second << ";\""
               << edgeItem.second.value().to_string() << "\"\n";
    }

    return result.str();
}


template <typename T>
void mesh_load(Mesh& m_mesh,
               T& str,
               std::function<uint32_t(Mesh&, objects::Description)> nodeInsertion,
               std::function<uint32_t(Mesh&, uint32_t, uint32_t, objects::Description)> edgeInsertion,
               std::function<void(Mesh&, uint32_t, uint32_t)> nodeEdgesInsertion)
{
    auto nodeIdsMapping = objects::types::U32U32Map{};
    Mesh mesh;

    auto nodesNumber = get_size_t(str);
    auto edgesNumber = get_size_t(str);

    for (auto i = 0u; i < *nodesNumber; ++i)
    {
        const auto nodeId = get_size_t(str);
        const auto nodeDesc = get_string(str);

        if (!nodeId || !nodeDesc)
        {
            auto result = std::stringstream{};
            result << "At node element " << i << '/' << *nodesNumber << ". ID = "
                   << (!nodeId ? std::string{"'missing node ID'"} : std::to_string(*nodeId))
                   << ", DESCRIPTION = "
                   << (!nodeDesc ? std::string{"'missing node description'"} : *nodeDesc);
            throw std::invalid_argument{result.str()};
        }
        else
        {
            auto newNodeId = nodeInsertion(mesh, objects::Description{*nodeDesc});
            nodeIdsMapping.insert({*nodeId, newNodeId});
        }
    }

    for (auto i = 0u; i < *edgesNumber; ++i)
    {
        const auto edgeId = get_size_t(str);
        const auto edgeFirstNodeId = get_size_t(str);
        const auto edgeSecondNodeId = get_size_t(str);
        const auto edgeDesc = get_string(str);

        if (!edgeId || !edgeFirstNodeId || !edgeSecondNodeId || !edgeDesc)
        {
            auto result = std::stringstream{};
            result << "At edge element " << i << '/' << *edgesNumber << ". ID = "
                   << (!edgeId ? std::string{"'missing edge ID'"} : std::to_string(*edgeId))
                   << ", FIRST ENDPOINT = "
                   << (!edgeFirstNodeId ? std::string{"'missing first endpoint id'"} : std::to_string(*edgeFirstNodeId))
                   << ", SECOND ENDPOINT = "
                   << (!edgeSecondNodeId ? std::string{"'missing second endpoint id'"} : std::to_string(*edgeSecondNodeId))
                   << ", DESCRIPTION = "
                   << (!edgeDesc ? std::string{"'missing edge description'"} : *edgeDesc);
            throw std::invalid_argument{result.str()};
        }
        else
        {
            auto firstNodeId = nodeIdsMapping[*edgeFirstNodeId];
            auto secondNodeId = nodeIdsMapping[*edgeSecondNodeId];

            if (firstNodeId == 0 || secondNodeId == 0)
            {
                auto result = std::stringstream{};
                result << "At edge element " << i << '/' << *edgesNumber << ". ID = "
                       << std::to_string(*edgeId) << ", FIRST ENDPOINT = "
                       << std::to_string(*edgeFirstNodeId) << ", SECOND ENDPOINT = "
                       << std::to_string(*edgeSecondNodeId) << ", DESCRIPTION = " << *edgeDesc
                       << ", ENDPOINT MAPPING {First endpoint == "
                       << (firstNodeId == 0 ? std::string{"doesn't map to anything"} : std::to_string(firstNodeId))
                       << ", second endpoint == "
                       << (secondNodeId == 0 ? std::string{"doesn't map to anything"} : std::to_string(secondNodeId))
                       << ")";
                throw std::invalid_argument{result.str()};
            }

            auto newEdgeId = edgeInsertion(mesh, firstNodeId, secondNodeId, objects::Description{*edgeDesc});
            nodeEdgesInsertion(mesh, firstNodeId, newEdgeId);
            nodeEdgesInsertion(mesh, secondNodeId, newEdgeId);
        }
    }

    std::swap(m_mesh, mesh);
}





bool MeshPack::from_string(std::string data)
{
    constexpr auto POS_ZERO = 0u;
    StringFile str{POS_ZERO, std::move(data)};
    mesh_load(m_mesh,
              str,
              [](auto& mesh, auto desc) { return mesh.insertNode(std::move(desc)); },
              [](auto& mesh, auto f, auto s, auto desc) { return mesh.insertEdge({f, s}, std::move(desc)); },
              [](auto& mesh, auto nId, auto eId) { return mesh.m_nodes[nId].edges().insert(eId); });
    return true;
}

bool MeshPack::to_file(std::filesystem::path filename) const
{
    auto output = std::ofstream{filename, std::ios::binary};
    output << m_mesh.m_nodes.size() << m_mesh.m_edges.size();

    for (const auto& nodeItem : m_mesh.m_nodes)
    {
        auto description = nodeItem.second.value().to_string();
        output << nodeItem.first << description.c_str();
    }

    for (const auto& edgeItem : m_mesh.m_edges)
    {
        auto description = edgeItem.second.value().to_string();
        output << edgeItem.first << edgeItem.second.nodes().first
               << edgeItem.second.nodes().second
               << description.c_str();
    }

    return true;
}

bool MeshPack::from_file(std::filesystem::path filename)
{
    auto input = std::ifstream{filename, std::ios::binary};
    mesh_load(m_mesh,
              input,
              [](auto& mesh, auto desc) { return mesh.insertNode(std::move(desc)); },
              [](auto& mesh, auto f, auto s, auto desc) { return mesh.insertEdge({f, s}, std::move(desc)); },
              [](auto& mesh, auto nId, auto eId) { return mesh.m_nodes[nId].edges().insert(eId); });
    return true;
}

}  // namespace utils
}  // namespace mesh
