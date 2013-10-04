layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

out vec4 vPosition;
out vec3 vNormal;
out vec4 vColor;

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
    gl_Position = uProjection * vPosition;
}
