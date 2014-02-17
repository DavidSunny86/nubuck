#include <Nubuck\math\box.h>

namespace M {

Vector3 CenterOf(const Box& b) {
    return 0.5f * (b.max - b.min) + b.min;
}

Vector3 SizeOf(const Box& b) {
    return b.max - b.min;
}

Box Scale(const Box& b, float s) {
    Vector3 sz = b.max - b.min;
    return Box::FromCenterSize(b.min + 0.5f * sz, s * sz);
}

} // namespace M