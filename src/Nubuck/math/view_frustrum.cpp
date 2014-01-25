#include <math.h>
#include "math.h"
#include "view_frustrum.h"

namespace M {

// cnf. http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/
ViewFrustrum ComputeViewFrustrum(float fovy, float aspect, float zNear, float zFar) {
    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    float fH = tan( fovy / 360 * M::PI ) * zNear;
    float fW = fH * aspect;

    ViewFrustrum f;
    f.left = -fW;
    f.right = fW;
    f.bottom = -fH;
    f.top = fH;
    f.nearVal = zNear;
    f.farVal = zFar;

    return f;
}

} // namespace M