#pragma once

#include <math\vector3.h>
#include <math\quaternion.h>

namespace M {

	struct TransformTRS {
		M::Vector3		trans;
		M::Quaternion	rot;
		float			scale;
	};

} // namespace M