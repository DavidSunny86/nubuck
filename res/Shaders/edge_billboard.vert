uniform mat4 uProjection;
uniform mat4 uTransform;
uniform mat4 uInvTransform;

layout(location = 0) in vec4    aPosition;
layout(location = 4) in vec3    aA0;
layout(location = 5) in vec3    aA1;
layout(location = 6) in vec3    aA2;
layout(location = 7) in vec3    aA3;
layout(location = 8) in float   aHalfHeightSq;

// in edge's local space
out mat4    vObjectToWindow;
out vec4    vPosition;
out float   vHalfHeightSq;
out vec4    vEyePos;
out vec3    vLightDir;

void main() {
    mat4 worldToObject = mat4(
        vec4(aA0, 0.0),
        vec4(aA1, 0.0),
        vec4(aA2, 0.0),
        vec4(aA3, 1.0)
    );
    vObjectToWindow = uProjection * uTransform * inverse(worldToObject);
    mat4 eyeToObject = worldToObject * uInvTransform;
    vPosition = worldToObject * aPosition;
    vHalfHeightSq = aHalfHeightSq;
    vEyePos = eyeToObject * vec4(0.0, 0.0, 0.0, 1.0);
    vLightDir = (eyeToObject * vec4(0.0, 0.0, 1.0, 0.0)).xyz;
	gl_Position = uProjection * (uTransform * aPosition);
}
