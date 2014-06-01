#pragma once

#include <Nubuck\nubuck.h>

// encapsulates data shared among all phases
struct Globals {
    Nubuck      nb;
    IGeometry*  geom; 
};
