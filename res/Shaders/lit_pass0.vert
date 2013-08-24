uniform mat4 uProjection;

layout(location = 0) in vec4 aPosition;

layout(location = 4) in vec3 aInsTransformA0;
layout(location = 5) in vec3 aInsTransformA1;
layout(location = 6) in vec3 aInsTransformA2;
layout(location = 7) in vec3 aInsTransformA3;

void main() {
	mat4 transform;
	transform[0] = vec4(aInsTransformA0, 0.0);
	transform[1] = vec4(aInsTransformA1, 0.0);
	transform[2] = vec4(aInsTransformA2, 0.0);
	transform[3] = vec4(aInsTransformA3, 1.0);
	gl_Position = uProjection * (transform * aPosition);
}
