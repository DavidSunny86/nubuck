layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

attribute(0) vec4 aPosition;
attribute(1) vec3 aNormal;
attribute(2) vec4 aColor;
attribute(4) vec3 aA0;
attribute(5) vec3 aA1;

varying vec4 vPosition;
varying vec3 vNormal;
varying vec4 vColor;
varying vec4 vPatternColor;

/*
NOTE: strangely this does't work on nv gtx285
mat3 NormalMat(mat4 m) {
    return transpose(inverse(mat3(m)));
}
*/

void main() {
    vPosition = uTransform * aPosition;
    vNormal = mat3(uNormalMat) * aNormal;
    vColor = aColor;
    vPatternColor = vec4(aA0, aA1.x);
    gl_Position = uProjection * vPosition;
}
