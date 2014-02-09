#pragma once

namespace M {

struct ViewFrustrum { float left, right, bottom, top, nearVal, farVal; };

ViewFrustrum ComputeViewFrustrum(float fovy, float aspect, float zNear, float zFar);

} // namespace M