#include <string>
#include <vector>
#include <common\types.h>
#include "ent_polyhedron.h"

namespace W {

struct SizeNode {
    std::string             name;
    std::vector<SizeNode*>  children;
    unsigned                size;
};

static std::vector<SizeNode*>   nodes;

static SizeNode* AllocNode(const std::string& name, unsigned size) {
    SizeNode* node = new SizeNode;
    nodes.push_back(node);
    node->name = name;
    node->size = size;
    return node;
}

static void FreeNodes() {
    for(unsigned i = 0; i < nodes.size(); ++i)
        delete nodes[i];
    nodes.clear();
}

static SizeNode* BuildTree(ENT_Polyhedron& ph) {
    SizeNode* nodes = AllocNode("nodes", sizeof(PolyhedronNodes));
    nodes->children.push_back(AllocNode("nodes", sizeof(PolyhedronNode) * ph.nodes.nodes.size()));
    nodes->children.push_back(AllocNode("positions", sizeof(M::Vector3) * ph.nodes.positions.size()));
    nodes->children.push_back(AllocNode("oldPositions", sizeof(M::Vector3) * ph.nodes.oldPositions.size()));
    nodes->children.push_back(AllocNode("colors", sizeof(R::Color) * ph.nodes.colors.size()));

    SizeNode* edges = AllocNode("edges", sizeof(PolyhedronEdges));
    edges->children.push_back(AllocNode("edges", sizeof(PolyhedronEdge) * ph.edges.edges.size()));
    edges->children.push_back(AllocNode("colors", sizeof(PolyhedronEdgeColor) * ph.edges.colors.size()));

    SizeNode* faces = AllocNode("faces", sizeof(PolyhedronFaces));
    faces->children.push_back(AllocNode("faces", sizeof(PolyhedronFace) * ph.faces.faces.size()));
    faces->children.push_back(AllocNode("lists", sizeof(PolyhedronFaceList) * ph.faces.lists.size()));
    faces->children.push_back(AllocNode("colors", sizeof(PolyhedronFaceColor) * ph.faces.colors.size()));
    faces->children.push_back(AllocNode("trans", sizeof(PolyhedronFaceTrans) * ph.faces.trans.size()));
    faces->children.push_back(AllocNode("freeIndices", sizeof(unsigned) * ph.faces.freeIndices.size()));

    SizeNode* mesh = AllocNode("mesh", sizeof(PolyhedronMesh));
    mesh->children.push_back(AllocNode("vnmap", sizeof(unsigned) * ph.mesh.vnmap.size()));
    mesh->children.push_back(AllocNode("vertices", sizeof(R::Mesh::Vertex) * ph.mesh.vertices.size()));
    mesh->children.push_back(AllocNode("indices", sizeof(R::Mesh::Index) * ph.mesh.indices.size()));

    SizeNode* selection = AllocNode("selection", sizeof(PolyhedronSelection)); 
    selection->children.push_back(AllocNode("nodes", sizeof(bool) * ph.selection.nodes.size()));

    SizeNode* root = AllocNode("ph", sizeof(ENT_Polyhedron));
    root->children.push_back(nodes);
    root->children.push_back(edges);
    root->children.push_back(faces);
    root->children.push_back(mesh);
    root->children.push_back(selection);
    return root;
}

static unsigned ComputeSizes(SizeNode* node) {
    for(unsigned i = 0; i < node->children.size(); ++i)
        node->size += ComputeSizes(node->children[i]);
    return node->size;
}

static void FormatBytes(unsigned size) {
    enum Multiples {
        BYTES       = 0,
        KILO_BYTES,
        MEGA_BYTES,
        GIGA_BYTES,

        NUM_MULTIPLES
    };
    const char* units[] = {
        "b", "kb", "mb", "gb"
    };

    int mult = 0;
    while(1000 <= size && mult < NUM_MULTIPLES) {
        size /= 1000;
        mult++;
    }

    printf("%d%s", size, units[mult]);
}

static void PrintTree(unsigned depth, SizeNode* node) {
    if(0 < depth) {
        for(unsigned i = 0; i < depth; ++i) printf("--");
        printf(" ");
    }
    printf("%s = ", node->name.c_str());
    FormatBytes(node->size);
    printf("\n");
    for(unsigned i = 0; i < node->children.size(); ++i)
        PrintTree(depth + 1, node->children[i]);
}

void Polyhedron_PrintSize(ENT_Polyhedron& ph) {
    SizeNode* root = BuildTree(ph);
    ComputeSizes(root);
    PrintTree(0, root);
    FreeNodes();
}

} // namespace W