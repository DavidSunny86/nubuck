#include <fstream>
#include <Nubuck\polymesh.h>
#include <world\entities\ent_geometry\ent_geometry.h>

// i know this has nothing to do with proper serialization
// and is highly platform dependent. yet it gets the job done...

namespace {

void WriteString(std::ofstream& file, const std::string& str) {
    unsigned length = str.length();
    file << length;
    file.write(str.c_str(), length);
}

std::string ReadString(std::ifstream& file) {
    unsigned length;
    file >> length;
    unsigned size = length + 1;
    char* buffer = new char[size];
    memset(buffer, 0, size);
    file.read(buffer, length);
    std::string str(buffer);
    delete[] buffer;
    return str;
}

} // unnamed namespace

namespace W {

void SaveGeometryToFile(const std::string& filename, IGeometry* geom)
{
    std::ofstream file(filename.c_str());
    WriteString(file, geom->GetName());
    const leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
    mesh.write(file);
    file.close();
}

void LoadGeometryFromFile(const std::string& filename, IGeometry* geom)
{
    std::ifstream file(filename.c_str());
    geom->SetName(ReadString(file));
    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
    mesh.clear();
    mesh.read(file);
    file.close();
}

} // namespace W