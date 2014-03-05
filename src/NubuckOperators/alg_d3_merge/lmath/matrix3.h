#pragma once

namespace LM {

    // SCALAR must be contructable from float
    template<typename SCALAR>
    struct Matrix3 {
        SCALAR m00, m10, m20;
        SCALAR m01, m11, m21;
        SCALAR m02, m12, m22;

        Matrix3(void) { }
        Matrix3(SCALAR m00, SCALAR m01, SCALAR m02,
                SCALAR m10, SCALAR m11, SCALAR m12,
                SCALAR m20, SCALAR m21, SCALAR m22);
    };

    /* IMPL */

    namespace {

        const float PI = 3.14159265f;

        inline float Deg2Rad(float deg) {
            const float factor = PI / 180.0f;
            return deg * factor;
        }

    } // unnamed namespace

#if defined(__linux__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreorder"
#endif

    template<typename SCALAR>
    Matrix3<SCALAR>::Matrix3(SCALAR m00, SCALAR m01, SCALAR m02, 
            SCALAR m10, SCALAR m11, SCALAR m12,
            SCALAR m20, SCALAR m21, SCALAR m22)
            :   m00(m00), m01(m01), m02(m02),
                m10(m10), m11(m11), m12(m12),
                m20(m20), m21(m21), m22(m22) { }

#if defined(__linux__)
    #pragma GCC diagnostic pop
#endif

    template<typename SCALAR, typename VECTOR>
    void Transform(const Matrix3<SCALAR>& mat, VECTOR& vec) {
        vec = VECTOR(mat.m00 * vec.xcoord() + mat.m01 * vec.ycoord() + mat.m02 * vec.zcoord(),
                mat.m10 * vec.xcoord() + mat.m11 * vec.ycoord() + mat.m12 * vec.zcoord(),
                mat.m20 * vec.xcoord() + mat.m21 * vec.ycoord() + mat.m22 * vec.zcoord());
    }

    namespace Mat3 {

        template<typename SCALAR>
        Matrix3<SCALAR> RotateX(float angle) {
            const float rad = Deg2Rad(angle);
            const float sina = sinf(rad);
            const float cosa = cosf(rad);

            return Matrix3<SCALAR>(1.0f, 0.0f, 0.0f,
                0.0f, cosa, sina,
                0.0f, -sina, cosa);
        }

        template<typename SCALAR>
        Matrix3<SCALAR> RotateY(float angle) {
            const float rad = Deg2Rad(angle);
            const float sina = sinf(rad);
            const float cosa = cosf(rad);

            return Matrix3<SCALAR>(cosa, 0.0f, -sina,
                0.0f, 1.0f, 0.0f,
                sina, 0.0f, cosa);
        }

        template<typename SCALAR>
        Matrix3<SCALAR> RotateZ(float angle) {
            const float rad = Deg2Rad(angle);
            const float sina = sinf(rad);
            const float cosa = cosf(rad);

            return Matrix3<SCALAR>(cosa, sina, 0.0f,
                    -sina, cosa, 0.0f,
                    0.0f, 0.0f, 1.0f);
        }

        template<typename SCALAR>
        Matrix3<SCALAR> Scale(SCALAR f) {
            return Matrix3<SCALAR>(f, 0, 0,
                    0, f, 0,
                    0, 0, f);
        }

    } // namespace Mat3

} // namespace LM
