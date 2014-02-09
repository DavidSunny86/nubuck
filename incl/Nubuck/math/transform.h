#pragma once

#include <Nubuck\math\vector3.h>
#include <Nubuck\math\quaternion.h>

namespace M {

	struct TransformTRS {
		M::Vector3		trans;
		M::Quaternion	rot;
		float			scale;
	};

} // namespace M