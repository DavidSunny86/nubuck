#pragma once

#include <Nubuck\nubuck.h>

struct Color {
    enum Enum {
        BLACK = 0,  // inner edges
        BLUE        // hull edges
    };
};

struct Globals {
    Nubuck nb;

    IGeometry* inputGeom;
    IGeometry* circle;
};

Color::Enum GetColor(leda::nb::RatPolyMesh& mesh, leda::edge e);
void        SetColorU(leda::nb::RatPolyMesh& mesh, leda::edge e, Color::Enum color);

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh);