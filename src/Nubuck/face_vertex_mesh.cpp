#include <iostream>
#include <algorithm> // max_element
#include <Nubuck\common\common.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\face_vertex_mesh.h>

typedef unsigned vertIdx_t;
typedef unsigned faceIdx_t;

struct Face {
    const vertIdx_t* const  vertices;
    const int               numVertices;

    Face(const vertIdx_t* vertices, int numVertices)
        : vertices(vertices), numVertices(numVertices)
    { }
};

static void BuildFaceList(const std::vector<vertIdx_t>& indices, std::vector<Face>& faces) {
    COM_assert(faces.empty());

    const unsigned numIndices = indices.size();
    unsigned i = 0;
    while(i < numIndices) {
        const vertIdx_t* fverts = &indices[i];
        i++;
        while(i < numIndices && indices[i] != fverts[0]) i++;
        COM_assert(i < numIndices); // valid faces must repeat first vertex
        int numVertices = static_cast<int>(&indices[i] - fverts);
        Face face(fverts, numVertices);
        faces.push_back(face);
        i++;
    }
}

static void PrintFaceList(const std::vector<Face>& faces) {
    const unsigned numFaces = faces.size();
    for(unsigned i = 0; i < numFaces; ++i) {
        std::cout << i << ":";
        const Face& face = faces[i];
        for(unsigned j = 0; j < face.numVertices; ++j) {
            std::cout << " " << face.vertices[j];
        }
        std::cout << std::endl;
    }
}

// returns face f in faces that's incident on vertex v,
// and vertex order ord_fv of v in f i.e., f.vertices[ord_fv] = v
// precond: there is such a face
static bool FindFace(const std::vector<Face>& faces, vertIdx_t v, faceIdx_t& f, int& ord_fv) {
    const unsigned numFaces = faces.size();
    for(unsigned i = 0; i < numFaces; ++i) {
        const vertIdx_t* fverts = faces[i].vertices;
        f = i;
        ord_fv = -1;
        for(unsigned j = 0; j < faces[i].numVertices; ++j) {
            if(v == fverts[++ord_fv]) return true;
        }
    }
    return false;
}

// ord_f0v0 is the vertex order of v0 in f0 i.e., f0.vertices[ord_f0v0] = v0
// returns face f1 = (v0, vk, ...) in faces that's incident on vertex v0 
// and adjacient to face f0 = (v0, ..., vk).
// => faces f0, f1 share edge (v0, vk) and f0, f1 are in ccw order around v0.
inline bool FindNextFace(const std::vector<Face>& faces, faceIdx_t f0, int ord_f0v0, faceIdx_t& f1, int& ord_f1v0) {
    const unsigned f0_sz = faces[f0].numVertices;
    const unsigned v0 = faces[f0].vertices[ord_f0v0];
    // vertex vk is cyclic predecessor of v0
    const unsigned vk = faces[f0].vertices[(ord_f0v0 + f0_sz - 1) % f0_sz];
    for(unsigned i = 0; i < faces.size(); ++i) {
        const vertIdx_t* fverts = faces[i].vertices;
        for(unsigned j = 0; j < faces[i].numVertices; ++j) {
            if(v0 == fverts[j] && vk == fverts[(j + 1) % faces[i].numVertices]) { // excludes f0
                f1 = i;
                ord_f1v0 = j;
                return true;
            }
        }
    }
    return false;
}

static void FromFaceVertexMesh(
    const std::vector<leda::d3_rat_point>& positions,
    const std::vector<vertIdx_t>& indices,
    leda::nb::RatPolyMesh& polyMesh)
{
    polyMesh.clear();

    std::vector<Face> faces;
    BuildFaceList(indices, faces);

    const unsigned numVertices = positions.size();
    COM_assert(numVertices > *std::max_element(indices.begin(), indices.end()));

    std::vector<leda::node> vmap; // maps indices to polymesh vertices
    vmap.resize(numVertices);

    // create polymesh vertices
    for(vertIdx_t v = 0; v < numVertices; ++v) {
        leda::node pv = polyMesh.new_node();
        polyMesh.set_position(pv, positions[v]);
        vmap[v] = pv;
    }

    for(vertIdx_t v = 0; v < numVertices; ++v) {
        faceIdx_t f0;
        int ord_v;

        if(!FindFace(faces, v, f0, ord_v)) {
            // allow disconnected vertices
            continue;
        }

        faceIdx_t f = f0;

        bool n = false;

        do {
            vertIdx_t w = faces[f].vertices[(ord_v + 1) % faces[f].numVertices];
            polyMesh.new_edge(vmap[v], vmap[w]);

            faceIdx_t next_f;
            int next_ord_v;

            n = FindNextFace(faces, f, ord_v, next_f, next_ord_v);

            f = next_f;
            ord_v = next_ord_v;
        } while(n && f0 != f);
    }

    leda::list<leda::edge> E;

    polyMesh.make_map(E);
    polyMesh.compute_faces();

    leda::edge e;
    // forall(e, E) polyMesh.set_masked(e);

    printf("size(E) = %d\n", E.size());
}

void add_triangle(leda::list<unsigned>& indices,
    unsigned i0,
    unsigned i1,
    unsigned i2)
{
    indices.push_back(i0);
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i0);
}

void add_quad(leda::list<unsigned>& indices,
    unsigned i0,
    unsigned i1,
    unsigned i2,
    unsigned i3)
{
    indices.push_back(i0);
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
    indices.push_back(i0);
}

template<typename T>
static void CopyContainer(const leda::list<T>& l, std::vector<T>& v) {
    v.reserve(l.size());
    leda::list_item it;
    forall_items(it, l) {
        v.push_back(l[it]);
    }
}

void make_from_indices(
    const leda::list<leda::d3_rat_point>& positions,
    const leda::list<unsigned>& indices,
    leda::nb::RatPolyMesh& polyMesh)
{
    std::vector<leda::d3_rat_point> v_positions;
    std::vector<unsigned> v_indices;

    CopyContainer(positions, v_positions);
    CopyContainer(indices, v_indices);

    FromFaceVertexMesh(v_positions, v_indices, polyMesh);
}