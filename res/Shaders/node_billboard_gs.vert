layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

attribute(0) vec4 aPosition;
attribute(1) vec3 aNormal;
attribute(2) vec4 aColor;

out VertexData {
    vec3    color;
    float   size;
} outData;

void main() {
    outData.color = aColor.rgb;
    outData.size = aNormal.z;
    gl_Position = uTransform * aPosition;
}
