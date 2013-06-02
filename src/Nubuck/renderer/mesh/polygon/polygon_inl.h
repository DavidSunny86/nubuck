#pragma once

namespace R {

    template<typename TYPE>
    void Subdiv(leda::list<TYPE>& polygon) {
        leda::list_item next, it = polygon.first();
        while(NULL != it) {
            next = polygon.succ(it);
            const TYPE& v0 = polygon[polygon.cyclic_succ(it)];
            const TYPE& v1 = polygon[it];
            polygon.insert(0.5f * (v0 + v1), it, leda::behind);
            it = next;
        }
    }

    template<typename TYPE>
    leda::list<TYPE> ChaikinSubdiv(const leda::list<TYPE>& polygon) {
        leda::list<TYPE> refined;
        leda::list_item it = polygon.first();
        while(NULL != it) {
            const TYPE& v0 = polygon[polygon.cyclic_pred(it)];
            const TYPE& v1 = polygon[it];

            refined.push_back(0.75 * v0 + 0.25 * v1);
            refined.push_back(0.25 * v0 + 0.75 * v1);

            it = polygon.succ(it);
        }

        return refined;
    }

} // namespace R