uniform mat4 uProjection;
uniform mat4 uTransform;

layout(location = 0) in vec4 aPosition;

void main() {
	gl_Position = uProjection * (uTransform * aPosition);
}
