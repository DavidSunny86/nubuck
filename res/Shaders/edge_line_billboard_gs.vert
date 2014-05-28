attribute(0) vec4 aPosition;
attribute(2) vec4 aColor;

out VertexData {
    vec4 color;
} outData;

void main() {
    outData.color = aColor;

    gl_Position = aPosition;
}
