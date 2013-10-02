layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

layout(location = 0) in vec4 aPosition;
layout(location = 3) in vec2 aTexCoord0;

out vec3 vPosition;
out vec2 vTexCoord0;

void main() {
	vPosition = aPosition.xyz;
	vTexCoord0 = aTexCoord0;
	gl_Position = uProjection * (mat4(1.0) * aPosition);
}
