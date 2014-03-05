#pragma once

#include "matrix3.h"

namespace LM {

    template<typename VECTOR>
    void Translate(leda::GRAPH<VECTOR, int>& G, const VECTOR& v) {
        leda::node n;
        forall_nodes(n, G) { 
            G[n] = G[n].translate(v.xcoord(), v.ycoord(), v.zcoord());
        }
    }

    template<typename VECTOR>
    void RotateX(leda::GRAPH<VECTOR, int>& G, float angle) {
        typedef typename VectorTraits<VECTOR>::scalar_t scalar_t;

        Matrix3<scalar_t> rot(Mat3::RotateX<scalar_t>(angle));
        
        leda::node n;
        forall_nodes(n, G) {
            Transform(rot, G[n]);
        }
    }

    template<typename VECTOR>
    void RotateY(leda::GRAPH<VECTOR, int>& G, float angle) {
        typedef typename VectorTraits<VECTOR>::scalar_t scalar_t;

        Matrix3<scalar_t> rot(Mat3::RotateY<scalar_t>(angle));
        
        leda::node n;
        forall_nodes(n, G) {
            Transform(rot, G[n]);
        }
    }

    template<typename VECTOR>
    void RotateZ(leda::GRAPH<VECTOR, int>& G, float angle) {
        typedef typename VectorTraits<VECTOR>::scalar_t scalar_t;

        Matrix3<scalar_t> rot(Mat3::RotateZ<scalar_t>(angle));
        
        leda::node n;
        forall_nodes(n, G) {
            Transform(rot, G[n]);
        }
    }

    template<typename VECTOR>
    void RotateZ(leda::list<VECTOR>& L, float angle) {
        typedef typename VectorTraits<VECTOR>::scalar_t scalar_t;

        Matrix3<scalar_t> rot(Mat3::RotateZ<scalar_t>(angle));
        
        leda::list_item it;
        forall_items(it, L) {
            Transform(rot, L[it]);
        }
    }

    template<typename VECTOR>
    void Scale(leda::GRAPH<VECTOR, int>& G, typename VectorTraits<VECTOR>::scalar_t f) {
        typedef typename VectorTraits<VECTOR>::scalar_t scalar_t;

        Matrix3<scalar_t> scale(Mat3::Scale(f));

        leda::node n;
        forall_nodes(n, G) {
            Transform(scale, G[n]);
        }
    }

} // namespace LM
