uniform mat4 uProjection;
uniform mat4 uTransform;
uniform mat3 uNormalMat;

uniform vec3 uLightPosition;

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

out vec3    vNormal;
out vec4    vColor;
out float   vDist; // distance between light source and vertex

void main() {
    vNormal = uNormalMat * aNormal;
    vColor = aColor;

    vec3 worldPos = (uTransform * aPosition).xyz;
    vDist = length(uLightPosition - worldPos);

    gl_Position = uProjection * vec4(worldPos, 1.0);
}
