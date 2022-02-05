/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <filesystem>

#include "mesh.hpp"
#include "objects/types.hpp"


namespace mesh
{
namespace utils
{

class MeshPack
{
public:
    struct StringFile
    {
        std::size_t cursor = {};
        std::string data = {};
    };

public:
    explicit MeshPack(Mesh& mesh);

    std::string to_string() const;
    bool from_string(std::string data);

    bool to_file(std::filesystem::path filename) const;
    bool from_file(std::filesystem::path filename);

private:
    Mesh& m_mesh;
};

}  // namespace utils
}  // namespace mesh

