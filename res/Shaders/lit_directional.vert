layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat3 uNormalMat;
};

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;

out vec4 vPosition;
out vec3 vNormal;

mat3 NormalMat(mat4 m) {
    return transpose(inverse(mat3(m)));
}

void main() {
    vPosition = uTransform * aPosition;
    vNormal = NormalMat(uTransform) * aNormal;
    gl_Position = uProjection * vPosition;
}
