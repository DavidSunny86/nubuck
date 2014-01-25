layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

attribute(0) vec4 aPosition;
attribute(2) vec4 aColor;

varying vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProjection * (uTransform * aPosition);
}
