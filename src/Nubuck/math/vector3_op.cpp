#include <Nubuck\math\vector3.h>

namespace M {

	Vector3 Normalize(const Vector3& vector) {
		float length = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
		return vector / length;
	}

	float Length(const Vector3& vector) {
		return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	}

	float SquaredLength(const Vector3& vector) {
		return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
	}

	float Distance(const Vector3& lhp, const Vector3& rhp) {
		return M::Length(rhp - lhp);
	}

	float SquaredDistance(const Vector3& lhp, const Vector3& rhp) {
        return SquaredLength(rhp - lhp);
	}

	bool LinearlyDependent(const Vector3& u, const Vector3& v) {
		return AlmostEqual(u.x * v.y - u.y * v.x, 0.0f) &&
			   AlmostEqual(u.x * v.z - u.z * v.x, 0.0f);
	}

    void Orthogonalize(Vector3& v0, Vector3& v1, Vector3& v2) {
        v1 = v1 - v1.ProjectOn(v0);
        v2 = v2 - v2.ProjectOn(v0) - v2.ProjectOn(v0);
    }

} // namespace M
