layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

attribute(0) vec4 aPosition;
attribute(2) vec4 aColor;
attribute(3) vec2 aTexCoords;

varying vec4 vColor;
varying vec2 vTexCoords;

void main() {
    vColor = aColor;
    vTexCoords = aTexCoords;
    gl_Position = uProjection * uTransform * aPosition;
}
