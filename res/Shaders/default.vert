uniform mat4 uProjection;
uniform mat4 uTransform;

uniform vec3 uInsPosition;

layout(location = 0) in vec4 aPosition;

void main() {
	gl_Position = uProjection * uTransform * (aPosition + vec4(uInsPosition, 1.0));
}
