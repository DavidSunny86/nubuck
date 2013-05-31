#include "line3.h"

namespace M {

	float Distance(const Line3& l, const Vector3& p) {
		return Cross(l.p1 - l.p0, l.p0 - p).Length() / (l.p1 - l.p0).Length();
	}

} // namespace M