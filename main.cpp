/**
 * Created by Karol Dudzic @ 2022
 */
#include <iostream>

#include "mesh.hpp"
#include "meshbuilder.hpp"
#include "utils/meshpack.hpp"


using namespace std;


int main()
{
    using namespace mesh;
    using namespace mesh::objects;

    Mesh mesh;

    std::string data;

    {
        const auto print = [](const auto& elem) { std::cout << elem.to_string() << '\n'; };
        MeshBuilder{mesh}.create(Description("Node 1"))
                        .create(Description("Node 2"))
                        .hopTo(1)
                        .create(Description("Node 3"))
                        .hopTo(1)
                        .create(Description("Node 4"))
                        .hopTo(1)
                        .create(Description("Node 5"))
                        .hopTo([](const auto& node) { return node.value().to_string() == "Node 3"; })
                        .create(Description("Node 6"))
                        .create(Description("Node 7"))
                        .hopTo(6)
                        .create(Description("Node 8"))
                        .hopTo(3)
                        .create(Description("Node 9"))
                        .create(Description("Node 10"))
                        .connect(8, 10, Description("Relation strength: 10"))
                        .connect(9, 10)
                        .connect(2, 9);
         MeshBuilder{mesh}.remove(3)
             .remove(6)
             .remove(8)
             .remove(9);

        utils::MeshPack pack{mesh};
        data = pack.to_string();
        std::cout << data << '\n';

        pack.to_file("save_mesh");
    }

//    {
//        utils::MeshPack pack(mesh);
//        if (pack.from_string(data))
//        {
//            std::cout << "Parsed correctly!\n";
//        }

//        std::cout << pack.to_string() << '\n';
//    }
}
