uniform mat4 uProjection;
uniform mat4 uTransform;
uniform mat3 uNormalMat;

uniform vec3 uLightPosition;

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

layout(location = 4) in vec3 aInsTransformA0;
layout(location = 5) in vec3 aInsTransformA1;
layout(location = 6) in vec3 aInsTransformA2;
layout(location = 7) in vec3 aInsTransformA3;

out vec3    vNormal;
out vec4    vColor;
out float   vDist; // distance between light source and vertex

void main() {
    vNormal = uNormalMat * aNormal;
    vColor = aColor;
	
	mat4 transform;
	transform[0] = vec4(aInsTransformA0, 0.0);
	transform[1] = vec4(aInsTransformA1, 0.0);
	transform[2] = vec4(aInsTransformA2, 0.0);
	transform[3] = vec4(aInsTransformA3, 1.0);

    vec3 worldPos = (transform * aPosition).xyz;
    vDist = length(uLightPosition - worldPos);

    gl_Position = uProjection * vec4(worldPos, 1.0);
}
