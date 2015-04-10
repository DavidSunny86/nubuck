// vertex already transformed and projected,
// just pass through attribs

layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

attribute(0) vec4 aPosition;
attribute(1) vec3 aNormal;
attribute(2) vec4 aColor;

varying vec4 vPosition;
varying vec3 vNormal;
varying vec4 vColor;

/*
NOTE: strangely this does't work on nv gtx285
mat3 NormalMat(mat4 m) {
    return transpose(inverse(mat3(m)));
}
*/

void main() {
    vPosition = aPosition;
    vNormal = aNormal;
    vColor = aColor;

    /*
    set z = 1 - eps, w = 1, so that z/w \approx 1 and vertex maps
    to far clipping plane in NDC for proper depth tests.
    note that z = 1 - eps \neq 1 to account for floating point errors.
    */
    gl_Position = vec4(vPosition.xy, 0.99999, 1.0);
}
