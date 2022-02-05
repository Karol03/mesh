<h1>Mesh</h1>

<h2>Purpose</h2>
<p>Application creates and maintenance mesh structure. By 'mesh' it is understand undirected cyclic tree with max one connection between distinguish nodes.
<p>Program allows to insert, remove, edit separate nodes and connection between them. Each node, as well as edge connected two nodes, contains own data storage to keep information.

<h2>Usage</h2>
<h3>Mesh building</h3>
<p>The main structure is 'Mesh' which is responsible for tree management. However, to simplify mesh creation, using of 'MeshBuilder' is the best approach.

Example program for create empty mesh:
```c++
#include "mesh.hpp"

int main()
{
    auto mesh = mesh::Mesh{};
}
```

<h3>Inserting nodes</h3>
<p>To manipulate mesh very easy we will use MeshBuilder class instead. Then, the program inserting two nodes into mesh will look like:

```c++
#include "mesh.hpp"
#include "meshbuilder.hpp"

int main()
{
    auto mesh = mesh::Mesh{};
    auto nodeDescription1 = mesh::objects::Description{"Node 1"};
    auto nodeDescription2 = mesh::objects::Description{"Node 2"};
    auto edgeDescription = mesh::objects::Description{"Edge between node 1 and node 2"};
    mesh::MeshBuilder{mesh}.create(std::move(nodeDescription1))
                           .create(std::move(nodeDescription2), std::move(edgeDescription));
}
```
![alt text](https://github.com/Karol03/mesh/blob/master/images/two_nodes_connection.jpg?raw=true)

The program contains two nodes into mesh and one edge between them.
<p>Note that 'create(...)' method takes up to two arguments, the first argument is descriptor of node, the second one is descriptor of edge.
Both arguments are optional so the node and edge not need to keep any value.
However, if there is no node in mesh yet, the first node is insert without any edge, so the edge descriptor is ignored (that's why we didn't pass any argument into first 'create(...)' method call).


<h3>Inserting multiply nodes</h3>
<p>When insert new node it is always connect to last choosen node. The choosen node is always the last inserted unless user set it manually. To do this we could use 'hop' methods.
The one of them is hopTo(...) method

```c++
#include "mesh.hpp"
#include "meshbuilder.hpp"

int main()
{
    using namespace mesh;
    using namespace mesh::objects;

    auto mesh = Mesh{};
    auto nodeDescription1 = Description{"Star topology center node"};
    auto nodeDescription2 = Description{"node 2"};
    auto nodeDescription3 = Description{"node 3"};
    auto nodeDescription4 = Description{"node 4"};
    mesh::MeshBuilder{mesh}.create(std::move(nodeDescription1))
                           .create(std::move(nodeDescription2))
                           .hopTo(1)
                           .create(std::move(nodeDescription3))
                           .hopTo([](const auto& node) { return node.value().to_string() == "Star topology center node"; } )
                           .create(std::move(nodeDescription4));
}
```
![alt text](https://github.com/Karol03/mesh/blob/master/images/star_nodes_connection.jpg?raw=true)

In the sample above we create center node 'Star topology center node', and then connects to it three nodes.
As by default new nodes are always connected to last created one we could create 'Node 2' without any hop.
Then the last inserted node is 'Node 2' so we need to back cursor to 'Star topology center node'. For this purpose we use 'hopTo(1)' method which hops
to node 1, where 1 means node internal ID.
To add last node we have to back to 'Star topology center node' again, this time we use lambda to find proper node. and the proper is this one with value equal to 'Star topology center node'.

<h3>Inserting additional edge</h3>
<p>To insert additional connection between not connected nodes use 'connect(...)' method
As for hop method you could point two node IDs explicitly, connect current node to choosen one, or choose both nodes using predicate.
<b>Be aware</b> node cannot be connected to itself.

```c++
#include "mesh.hpp"
#include "meshbuilder.hpp"

int main()
{
    using namespace mesh;
    using namespace mesh::objects;

    auto mesh = Mesh{};
    auto nodeDescription1 = Description{"Node 1"};
    auto nodeDescription2 = Description{"node 2"};
    auto nodeDescription3 = Description{"node 3"};
    mesh::MeshBuilder{mesh}.create(std::move(nodeDescription1))
                           .create(std::move(nodeDescription2))
                           .hopTo(1)
                           .create(std::move(nodeDescription3))
                           .connect(2, 3);
}
```
![alt text](https://github.com/Karol03/mesh/blob/master/images/add_nodes_connection.jpg?raw=true)


<h3>Remove nodes</h3>
<p>Removing node is as simple as adding it. To remove given node just hop to it and call 'remove()' on MeshBuilder.
You could also use one of other erase method like:
- 'remove(<uint32_t>)' - to remove node with given ID
- 'remove(<vector<uint32_t>>)' - to remove all nodes with ID in given range
- 'remove(<predicate bool<const Node&>>)' - to remove all node which meet the predicate

<b>Important!</b> The mesh must be consistent, so when you remove any node which is not a leaf you remove all branches. Only the biggest branch is kept.

```c++
#include "mesh.hpp"
#include "meshbuilder.hpp"

int main()
{
    using namespace mesh;
    using namespace mesh::objects;

    auto mesh = Mesh{};
    auto nodeDescription1 = Description{"Node 1"};
    auto nodeDescription2 = Description{"Node 2"};
    auto nodeDescription3 = Description{"Node 3"};
    auto nodeDescription4 = Description{"Node 4"};
    auto nodeDescription5 = Description{"Node 5"};
    auto nodeDescription6 = Description{"Node 6"};
    auto nodeDescription7 = Description{"Node 7"};
    mesh::MeshBuilder{mesh}.create(std::move(nodeDescription1))
                           .create(std::move(nodeDescription2))
                           .hopTo(1)
                           .create(std::move(nodeDescription3))
                           .hopTo(1)
                           .create(std::move(nodeDescription4))
                           .hopTo(1)
                           .create(std::move(nodeDescription5))
                           .create(std::move(nodeDescription6))
                           .hopTo(5)
                           .create(std::move(nodeDescription7))
                           .connect(3, 6);

    mesh::MeshBuilder{mesh}.remove(1);
}
```
Before remove:

![alt text](https://github.com/Karol03/mesh/blob/master/images/before_remove_nodes_connection.jpg?raw=true)

After remove:

![alt text](https://github.com/Karol03/mesh/blob/master/images/after_remove_nodes_connection.jpg?raw=true)


<h3>Export mesh to/from string</h3>
<p>For mesh string/binary manipulation use MeshPack class from utils. It allows you to export/import mesh to/from string.
In example we use simple two nodes mesh from 'Inserting nodes' example.

```c++
#include "mesh.hpp"
#include "meshbuilder.hpp"
#include "utils/meshpack.hpp"

int main()
{
    using namespace mesh;
    using namespace mesh::objects;
    using namespace mesh::utils;

    auto meshAsString = std::string{};
    
    {
        auto mesh = mesh::Mesh{};
        auto nodeDescription1 = mesh::objects::Description{"Node 1"};
        auto nodeDescription2 = mesh::objects::Description{"Node 2"};
        auto edgeDescription = mesh::objects::Description{"Edge between node 1 and node 2"};
        mesh::MeshBuilder{mesh}.create(std::move(nodeDescription1))
                               .create(std::move(nodeDescription2), std::move(edgeDescription));
        meshAsString = MeshPack{mesh}.to_string();  // here convert mesh to string
    }   // at the end of scope mesh was destroyed
    
    std::cout << "Our mesh:\n" << meshAsString << '\n';   // here print our mesh
    
    {
        auto mesh = mesh::Mesh{};
        
        try {
            auto meshPack = MeshPack{mesh};
            meshPack.from_string(meshAsString);   // here convert mesh from string 
            std::cout << "Our mesh again:\n" << meshPack.to_string() << '\n';     // here print it again
        } catch (std::invalid_argument& e) {
            std::cerr << "Failed: " << e.what();   // means loading mesh from file failed
        };
    }
}
```


<h3>Export mesh to/from binary</h3>
<p>MeshPack allows to save/load mesh to/from binary file as well. 
In example we use simple two nodes mesh from 'Inserting nodes' example.

```c++
#include "mesh.hpp"
#include "meshbuilder.hpp"
#include "utils/meshpack.hpp"

int main()
{
    using namespace mesh;
    using namespace mesh::objects;
    using namespace mesh::utils;

    {
        auto mesh = mesh::Mesh{};
        auto nodeDescription1 = mesh::objects::Description{"Node 1"};
        auto nodeDescription2 = mesh::objects::Description{"Node 2"};
        auto edgeDescription = mesh::objects::Description{"Edge between node 1 and node 2"};
        mesh::MeshBuilder{mesh}.create(std::move(nodeDescription1))
                               .create(std::move(nodeDescription2), std::move(edgeDescription));
        MeshPack{mesh}.to_file("meshfile.msh");  // here save mesh to binary
    }

    {
        auto mesh = mesh::Mesh{};
        try {
            auto meshPack = MeshPack{mesh};
            meshPack.from_file("meshfile.msh");   // here load mesh from binary file
            std::cout << "Our mesh:\n" << meshPack.to_string() << '\n';     // here print the result
        } catch (std::invalid_argument& e) {
            std::cerr << "Failed: " << e.what();   // means loading mesh from file failed
        };
    }
}
```

<h2>Requirements</h2>
C++17
