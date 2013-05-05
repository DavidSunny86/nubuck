uniform mat4 uProjection;
uniform mat4 uTransform;
uniform mat3 uNormalMat;

uniform vec3 uInsPosition;

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;

out vec3 vNormal;

void main() {
	vNormal = normalize(aNormal);
	gl_Position = uProjection * uTransform * (aPosition + vec4(uInsPosition, 1.0));
}
