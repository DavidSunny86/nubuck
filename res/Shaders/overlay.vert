layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

layout(location = 0) in vec4 aPosition;

void main() {
    gl_Position = uProjection * (uTransform * aPosition);
}
