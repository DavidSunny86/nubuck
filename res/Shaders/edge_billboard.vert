uniform mat4 uProjection;
uniform mat4 uTransform;

layout(location = 0) in vec4 aPosition;
layout(location = 4) in vec3 aA0;
layout(location = 5) in vec3 aA1;
layout(location = 6) in vec3 aA2;
layout(location = 7) in vec3 aA3;

out mat4 vWorldToObject;
out vec4 vPosition;

void main() {
    vWorldToObject = mat4(
        vec4(aA0, 0.0),
        vec4(aA1, 0.0),
        vec4(aA2, 0.0),
        vec4(aA3, 1.0)
    );
    vPosition = vWorldToObject * aPosition;
	gl_Position = uProjection * (uTransform * aPosition);
}
