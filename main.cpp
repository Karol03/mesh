/**
 * Created by Karol Dudzic @ 2022
 */
#include <iostream>

#include "mesh.hpp"
#include "meshbuilder.hpp"
#include "utils/meshpack.hpp"


using namespace std;

template <typename Description>
void printMesh(mesh::Mesh<Description>& mesh, std::vector<uint32_t> ids)
{
    using namespace mesh;
    using namespace mesh::objects;

    auto mb = MeshBuilder{mesh};
    auto value = mb.hopToUniquePathEnd(ids).currentValue();
    if (value)
    {
        auto result = std::stringstream{};
        result << "Path {";
        if (!ids.empty())
        {
            result << ids[0];
        }

        for (auto i = 1u; i < ids.size(); ++i)
        {
            result << " -> " << ids[i];
        }
        result << "}  FOUND UNIQUE and result is : \"" << *value << "\"\n";

        std::cout << result.str();
    }
    else
    {
        auto result = std::stringstream{};
        result << "Path {";
        if (!ids.empty())
        {
            result << ids[0];
        }

        for (auto i = 1u; i < ids.size(); ++i)
        {
            result << " -> " << ids[i];
        }
        result << "}  NOT FOUND UNIQUE\n";

        std::cout << result.str();
    }
}

template <typename Description>
void printMeshSentence(mesh::Mesh<Description>& mesh, std::vector<std::string> sentence)
{
    using namespace mesh;
    using namespace mesh::objects;

    auto wordMatch = [](const auto& word) {
        return [word](const auto& node)
        {
            return node.value() == word;
        };
    };

    auto predicates = std::vector<std::function<bool(const Node<Description>&)>>{};
    for (const auto& word : sentence)
    {
        predicates.push_back(wordMatch(word));
    }

    auto mb = MeshBuilder{mesh};
    auto value = mb.hopToPathEnd(predicates).currentValue();
    std::cout << "Computation finished\n";
    if (value)
    {
        auto result = std::stringstream{};
        result << "Path {";
        if (!sentence.empty())
        {
            result << sentence[0];
        }

        for (auto i = 1u; i < sentence.size(); ++i)
        {
            result << " " << sentence[i];
        }
        result << "}  FOUND and result is : \"" << *value << "\"\n";

        std::cout << result.str();
    }
    else
    {
        auto result = std::stringstream{};
        result << "Path {";
        if (!sentence.empty())
        {
            result << sentence[0];
        }

        for (auto i = 1u; i < sentence.size(); ++i)
        {
            result << " " << sentence[i];
        }
        result << "}  NOT FOUND\n";

        std::cout << result.str();
    }
}


int main()
{
    using namespace mesh;
    using namespace mesh::objects;

    Mesh<std::string> mesh;

    const auto print = [](const auto& elem) { std::cout << elem.to_string() << '\n'; };
    MeshBuilder{mesh}.create(std::string("my"))
                     .create(std::string("name"))
                     .create(std::string("is"))
                     .create(std::string("jeff"))
                     .hopTo(3)
                     .create(std::string("what"))
                     .hopTo(3)
                     .create(std::string("going"))
                     .create(std::string("on"))
                     .hopTo(3)
                     .create(std::string("there"))
                     .create(std::string("any"))
                     .create(std::string("open"))
                     .create(std::string("beer"))
                     .hopTo(1)
                     .create(std::string("own"))
                     .connectTo(11);

    auto mp = utils::MeshPack{mesh};
    std::cout << mp.to_string() << '\n';

    printMesh(mesh, {1, 2, 3, 4});
    printMesh(mesh, {1, 12, 11});
    printMesh(mesh, {1, 12, 11, 1});
    printMesh(mesh, {1, 2, 3, 9, 10, 11, 12});
    printMesh(mesh, {1, 2, 3, 8});
    printMesh(mesh, {1, 2, 3, 8, 9, 10, 11, 12, 1});

    printMeshSentence(mesh, {"my", "name", "is", "jeff"});
    printMeshSentence(mesh, {"my", "own", "beer"});
    printMeshSentence(mesh, {"my", "own", "beer", "my"});
    printMeshSentence(mesh, {"my", "name", "is", "any", "open", "beer", "own"});
    printMeshSentence(mesh, {"my", "name", "is", "there"});
    printMeshSentence(mesh, {"my", "name", "is", "there", "any", "open", "beer", "own", "my"});
}
