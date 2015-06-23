#pragma once

#include <LEDA\numbers\rational.h>

// a pointer to this type is passed to op_set_transform
// in the args[] buffer of SetOperatorEvent
struct EntityVector {
    enum VectorType {
        VectorType_Position,
        VectorType_Scale,
    };

    enum { MAGIC_PATTERN = 0xCAFED00D };

    int             m_magic;
    int             m_entityID;
    VectorType      m_type;
    leda::rational  m_vector[4];
};