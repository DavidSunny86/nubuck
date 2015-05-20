#include <Nubuck\polymesh.h>
#include <Nubuck\face_vertex_mesh.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\math_conv.h>

namespace leda {
namespace nb {

static void leda_side(GRAPH<d3_rat_point,int>& poly, node* L, float z)
{
  L[0]= poly.new_node(d3_rat_point(-70,-20,z,1));
  L[1]= poly.new_node(d3_rat_point(-40,-20,z,1));
  L[2]= poly.new_node(d3_rat_point(-40,-10,z,1));
  L[3]= poly.new_node(d3_rat_point(-60,-10,z,1));
  L[4]= poly.new_node(d3_rat_point(-60,+20,z,1));
  L[5]= poly.new_node(d3_rat_point(-70,+20,z,1));

  poly.new_edge(L[0],L[5],0);
  int i;
  for(i = 1; i<=5; i++) poly.new_edge(L[i], L[i-1], 0);

  L[6] = poly.new_node(d3_rat_point(-30,-20,z,1));
  L[7] = poly.new_node(d3_rat_point( -5,-20,z,1));
  L[8] = poly.new_node(d3_rat_point( -5,-10,z,1));
  L[9] = poly.new_node(d3_rat_point(-20,-10,z,1));
  L[10]= poly.new_node(d3_rat_point(-20, -5,z,1));
  L[11]= poly.new_node(d3_rat_point( -5, -5,z,1));
  L[12]= poly.new_node(d3_rat_point( -5, +5,z,1));
  L[13]= poly.new_node(d3_rat_point(-20, +5,z,1));
  L[14]= poly.new_node(d3_rat_point(-20,+10,z,1));
  L[15]= poly.new_node(d3_rat_point( -5,+10,z,1));
  L[16]= poly.new_node(d3_rat_point( -5,+20,z,1));
  L[17]= poly.new_node(d3_rat_point(-30,+20,z,1));

  poly.new_edge(L[6],L[17],0);
  for(i = 7; i<=17; i++) poly.new_edge(L[i], L[i-1], 0);

  L[18]= poly.new_node(d3_rat_point( 5,-20,z,1));
  L[19]= poly.new_node(d3_rat_point(20,-20,z,1));
  L[20]= poly.new_node(d3_rat_point(35,-10,z,1));
  L[21]= poly.new_node(d3_rat_point(35,+10,z,1));
  L[22]= poly.new_node(d3_rat_point(20,+20,z,1));
  L[23]= poly.new_node(d3_rat_point( 5,+20,z,1));
  L[24]= poly.new_node(d3_rat_point( 5,-20,z,1));

  L[25]= poly.new_node(d3_rat_point(15,-10,z,1));
  L[26]= poly.new_node(d3_rat_point(15,+10,z,1));
  L[27]= poly.new_node(d3_rat_point(20,+10,z,1));
  L[28]= poly.new_node(d3_rat_point(25, +5,z,1));
  L[29]= poly.new_node(d3_rat_point(25, -5,z,1));
  L[30]= poly.new_node(d3_rat_point(20,-10,z,1));
  L[31]= poly.new_node(d3_rat_point(15,-10,z,1));

  poly.new_edge(L[18],L[31],1);
  for(i = 19; i<=24; i++) poly.new_edge(L[i], L[i-1], 0);

  poly.new_edge(L[25],L[24],1);
  for(i = 26; i<=31; i++) poly.new_edge(L[i], L[i-1], 0);


  L[32]= poly.new_node(d3_rat_point(55, 20,z,1));
  L[33]= poly.new_node(d3_rat_point(40,-20,z,1));
  L[34]= poly.new_node(d3_rat_point(50,-20,z,1));
  L[35]= poly.new_node(d3_rat_point(55, -5,z,1));
  L[36]= poly.new_node(d3_rat_point(65, -5,z,1));
  L[37]= poly.new_node(d3_rat_point(70,-20,z,1));
  L[38]= poly.new_node(d3_rat_point(80,-20,z,1));
  L[39]= poly.new_node(d3_rat_point(65, 20,z,1));
  L[40]= poly.new_node(d3_rat_point(55, 20,z,1));

  L[41]= poly.new_node(d3_rat_point(60,15,z,1));
  L[42]= poly.new_node(d3_rat_point(65, 0,z,1));
  L[43]= poly.new_node(d3_rat_point(55, 0,z,1));
  L[44]= poly.new_node(d3_rat_point(60,15,z,1));

  poly.new_edge(L[32],L[44],1);
  for(i = 33; i<=40; i++) poly.new_edge(L[i], L[i-1], 0);

  poly.new_edge(L[41],L[40],1);
  for(i = 42; i<=44; i++) poly.new_edge(L[i], L[i-1], 0);
}


NUBUCK_API void make_leda(RatPolyMesh& poly)
{ 
  poly.clear();

  node L[45];
  node R[45];
  
  leda_side(poly,L,-5);
  leda_side(poly,R,+5);

  for(int i = 0; i<45; i++) poly.new_edge(L[i],R[i]);

  poly.make_bidirected();
  poly.make_map();

  node v;
  forall_nodes(v,poly) poly[v] = 2 * poly[v].to_vector();
}

template<typename VEC3>
struct FromFloat3 { };

template<> struct FromFloat3<leda::d3_rat_point> {
    static leda::d3_rat_point Conv(float x, float y, float z) {
        return leda::d3_rat_point(leda::rational(x), leda::rational(y), leda::rational(z));
    }
};

NUBUCK_API size_t make_from_obj(const char* filename, RatPolyMesh& mesh) {
    FILE* file = fopen(filename, "r");
    assert(file);

    list<leda::d3_rat_point>    positions;
    list<unsigned>              faces;

    unsigned numVertices = 0, numFaces = 0;
    while(!feof(file)) {
        char buffer[512];
        float f[3];
        unsigned d[4];

        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, 512, file);

        if(3 == sscanf(buffer, "v %f %f %f", &f[0], &f[1], &f[2])) {
            float scale = 1.0f;
            positions.push_back(FromFloat3<leda::d3_rat_point>::Conv(scale * f[0], scale * f[1], scale * f[2]));
            numVertices++;
        }
        if(
            4 == sscanf(buffer, "f %d %d %d %d", &d[0], &d[1], &d[2], &d[3]) ||
            4 == sscanf(buffer, "f %d/%*d %d/%*d %d/%*d %d/%*d", &d[0], &d[1], &d[2], &d[3]) ||
            4 == sscanf(buffer, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &d[0], &d[1], &d[2], &d[3]) ||
            4 == sscanf(buffer, "f %d//%*d %d//%*d %d//%*d %d//%*d", &d[0], &d[1], &d[2], &d[3]))
        {
            faces.push_back(d[0] - 1);
            faces.push_back(d[1] - 1);
            faces.push_back(d[2] - 1);
            faces.push_back(d[3] - 1);
            faces.push_back(d[0] - 1);
            numFaces++;
        }
        else if(
            3 == sscanf(buffer, "f %d %d %d", &d[0], &d[1], &d[2]) ||
            3 == sscanf(buffer, "f %d/%*d %d/%*d %d/%*d", &d[0], &d[1], &d[2]) ||
            3 == sscanf(buffer, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &d[0], &d[1], &d[2]) ||
            3 == sscanf(buffer, "f %d//%*d %d//%*d %d//%*d", &d[0], &d[1], &d[2]))
        {
            faces.push_back(d[0] - 1);
            faces.push_back(d[1] - 1);
            faces.push_back(d[2] - 1);
            faces.push_back(d[0] - 1);
            numFaces++;
        }
    }

    printf("NUM VERTICES: %d, NUM FACES %d\n", numVertices, numFaces);

    make_from_indices(positions, faces, mesh);

    fclose(file);
    return 0;
}

static M::Vector3 ProjectOnParaboloid_F(const M::Vector3& v) {
    return M::Vector3(v.x, v.y, v.x * v.x + v.y * v.y);
}

// somehow this doesn't work when using exact arithmetic. oh well...
NUBUCK_API void make_grid(RatPolyMesh& mesh, const int subdiv, const float size) {
    const leda::rational zero(0, 1);

    mesh.clear();

    const int N = (1 << subdiv) + 1;

    leda::node* nodes = new leda::node[N * N];

    const float         hsize = 0.5f * size;
    const M::Vector3    off = M::Vector3(hsize, hsize, 0.0f);
    const float         ds = size / (N - 1);

    float x = 0.0f, y = 0.0f;

    for(unsigned i = 0; i < N; ++i) {
        y = 0.0f;
        for(unsigned j = 0; j < N; ++j) {
            const leda::node v = mesh.new_node();
            M::Vector3 pos = M::Vector3(x, y, 0.0f) - off;
            mesh.set_position(v, ToRatPoint(ProjectOnParaboloid_F(pos)));
            nodes[N * i + j] = v;

            y += ds;
        }

        x += ds;
    }

    for(unsigned i = 0; i < N; ++i) {
        for(unsigned j = 0; j < N; ++j) {
            if(i + 1 < N)   mesh.new_edge(nodes[N * i + j], nodes[N * (i + 1) + j]);
            if(j + 1 < N)   mesh.new_edge(nodes[N * i + j], nodes[N * i + (j + 1)]);
            if(i > 0)       mesh.new_edge(nodes[N * i + j], nodes[N * (i - 1) + j]);
            if(j > 0)       mesh.new_edge(nodes[N * i + j], nodes[N * i + (j - 1)]);
        }
    }

    delete[] nodes;

    bool isBidirected = mesh.make_map();
    assert(isBidirected);
    mesh.compute_faces();

    mesh.set_visible(mesh.face_of(mesh.reversal(mesh.first_edge())), false);
}

} // namespace nb
} // namespace leda