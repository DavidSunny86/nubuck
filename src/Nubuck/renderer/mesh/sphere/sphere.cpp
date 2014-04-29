#include "sphere.h"

// cnf. the red book, p.96

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace {

    const float X = 0.525731112119133606;
	const float Z = 0.850650808352039932;

	R::Mesh::Vertex g_vertices[] = {
        { M::Vector3(-X,  0,  Z), M::Vector3::Zero, R::Color::White, M::Vector2::Zero }, 
		{ M::Vector3( X,  0,  Z), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3(-X,  0, -Z), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( X,  0, -Z), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( 0,  Z,  X), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( 0,  Z, -X), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( 0, -Z,  X), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( 0, -Z, -X), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( Z,  X,  0), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3(-Z,  X,  0), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3( Z, -X,  0), M::Vector3::Zero, R::Color::White, M::Vector2::Zero },
		{ M::Vector3(-Z, -X,  0), M::Vector3::Zero, R::Color::White, M::Vector2::Zero }
	};

    R::Mesh::Index g_indices[][3] = {
		{ 1, 4, 0 }, { 4, 9, 0 }, { 4, 5, 9 }, { 8, 5, 4}, { 1, 8, 4 },
		{ 1, 10, 8 }, { 10, 3, 8 }, { 8, 3, 5 }, { 3, 2, 5 }, { 3, 7, 2},
		{ 3, 10, 7 }, { 10, 6, 7 }, { 6, 11, 7 }, { 6, 0, 11 }, { 6, 1, 0 },
		{ 10, 1, 6 }, { 11, 0, 9 }, { 2, 11, 9 }, { 5, 2, 9 }, { 11, 2, 7 }
	};

    inline unsigned NumTriangles(unsigned subdiv) {
		unsigned i = 1;
		while(0 < subdiv--) i *= 4;
		return i * ARRAY_SIZE(g_indices);
	}

	inline unsigned NumVertices(unsigned subdiv) { // num vertices = num indices
		return 3 * NumTriangles(subdiv);
	}

    void Subdivide(R::Mesh::Vertex** pvert, int depth, bool smooth,
		M::Vector3 v0,
		M::Vector3 v1,
		M::Vector3 v2) 
	{
		if(0 >= depth) {
			M::Vector3 normal = 
				M::Normalize(M::Cross(v1 - v0, v2 - v0));

			(*pvert)->position = v0;
			if(smooth) (*pvert)->normal = M::Normalize(v0); 
			else (*pvert)->normal = normal;
			(*pvert)->color = R::Color::White;
			(*pvert)++;

			(*pvert)->position = v1;
			if(smooth) (*pvert)->normal = M::Normalize(v1); 
			else (*pvert)->normal = normal;
			(*pvert)->color = R::Color::White;
			(*pvert)++;

			(*pvert)->position = v2;
			if(smooth) (*pvert)->normal = M::Normalize(v2); 
			else (*pvert)->normal = normal;
			(*pvert)->color = R::Color::White;
			(*pvert)++;

			return;
		}

		M::Vector3 v01 = M::Normalize(0.5f * (v0 + v1)); 
		M::Vector3 v12 = M::Normalize(0.5f * (v1 + v2));
		M::Vector3 v20 = M::Normalize(0.5f * (v2 + v0));


		Subdivide(pvert, depth - 1, smooth, v0, v01, v20);
		Subdivide(pvert, depth - 1, smooth, v1, v12, v01);
		Subdivide(pvert, depth - 1, smooth, v2, v20, v12);
		Subdivide(pvert, depth - 1, smooth, v01, v12, v20);
	}

} // unnamed namespace

namespace R {

    Sphere::Sphere(int numSubdiv, bool smooth) : _vertices(NULL), _indices(NULL), _numVerts(0) { // not exception-safe
        _numVerts = NumVertices(numSubdiv);

        _vertices = new Mesh::Vertex[_numVerts];
        _indices = new Mesh::Index[_numVerts];

        Mesh::Vertex* vert = _vertices;
        unsigned numTris = NumTriangles(numSubdiv);
        for(Mesh::Index i = 0; i < 20; ++i) {
			Subdivide(&vert, numSubdiv, smooth,
				g_vertices[g_indices[i][0]].position, 
				g_vertices[g_indices[i][1]].position, 
				g_vertices[g_indices[i][2]].position);
		}

        for(Mesh::Index i = 0; i < _numVerts; i++) {
			_indices[i] = i;
		}
    }

    Sphere::~Sphere(void) {
        if(_vertices) delete[] _vertices;
        if(_indices) delete[] _indices;
    }

    void Sphere::SetColor(const Color& color) {
        for(int i = 0; i < _numVerts; ++i) {
            _vertices[i].color = color;
        }
    }

    void Sphere::Scale(float scale) {
        for(int i = 0; i < _numVerts; ++i) {
			_vertices[i].position *= scale;
		}
    }

    Mesh::Desc Sphere::GetDesc(void) {
        Mesh::Desc desc;
        desc.vertices = _vertices;
        desc.numVertices = _numVerts;
        desc.indices = _indices;
        desc.numIndices = _numVerts;
        desc.primType = GL_TRIANGLES;
        return desc;
    }

} // namespace R