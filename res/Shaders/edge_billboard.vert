uniform mat4 uProjection;
uniform mat4 uTransform;
uniform mat4 uWorldToObject;

layout(location = 0) in vec4 aPosition;
layout(location = 4) in vec3 aA0;
layout(location = 5) in vec3 aA1;
layout(location = 6) in vec3 aA2;
layout(location = 7) in vec3 aA3;

out vec4 vPosition;
out mat4 vWorldToObject;
out mat4 vObjectToWorld;

void main() {
    vPosition = uWorldToObject * aPosition;
    vWorldToObject = mat4(
        vec4(aA0, 0.0),
        vec4(aA1, 0.0),
        vec4(aA2, 0.0),
        vec4(aA3, 1.0)
    );
    vObjectToWorld = inverse(vWorldToObject);
	gl_Position = uProjection * (uTransform * aPosition);
}
