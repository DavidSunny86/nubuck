#pragma once

#include <LEDA/graph/graph.h>
#include "vector_traits.h"

namespace LM {

    template<typename VECTOR> 
    void Translate(leda::GRAPH<VECTOR, int>& G, const VECTOR& v);

    template<typename VECTOR> 
    void RotateX(leda::GRAPH<VECTOR, int>& G, float angle);

    template<typename VECTOR> 
    void RotateY(leda::GRAPH<VECTOR, int>& G, float angle);

    template<typename VECTOR> 
    void RotateZ(leda::GRAPH<VECTOR, int>& G, float angle);

    template<typename VECTOR> 
    void RotateZ(leda::list<VECTOR>& L, float angle);

    template<typename VECTOR> 
    void Scale(leda::GRAPH<VECTOR, int>& G, typename VectorTraits<VECTOR>::scalar_t f);

} // namespace LM

#include "d3_graph_inl.h"
