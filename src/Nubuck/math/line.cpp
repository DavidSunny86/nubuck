#include "line.h"

namespace M {

	float Distance(const Line& l, const Vector3& p) {
		return Cross(l.p1 - l.p0, l.p0 - p).Length() / (l.p1 - l.p0).Length();
	}

} // namespace M