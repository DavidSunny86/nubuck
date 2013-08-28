uniform mat4 uProjection;
uniform mat4 uTransform;

layout(location = 0) in vec4 aPosition;
layout(location = 3) in vec2 aTexCoord0;

out vec3 vPosition;
out vec2 vTexCoord0;

void main() {
	vPosition = aPosition.xyz;
	vTexCoord0 = aTexCoord0;
	gl_Position = uProjection * (uTransform * aPosition);
}
