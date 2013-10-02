layout(std140) uniform UniformsLights {
  vec3 uLightVec0;
  vec3 uLightVec1;
  vec3 uLightVec2;
  vec4 uLightDiffuseColor0;
  vec4 uLightDiffuseColor1;
  vec4 uLightDiffuseColor2;
};

in vec3 vNormal;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(vNormal);
    float d0 = clamp(dot(normal, normalize(uLightVec0)), 0.0, 1.0);
    float d1 = clamp(dot(normal, normalize(uLightVec1)), 0.0, 1.0);
    float d2 = clamp(dot(normal, normalize(uLightVec2)), 0.0, 1.0);
    fragColor = d0 * uLightDiffuseColor0 + d1 * uLightDiffuseColor1 + d2 * uLightDiffuseColor2;
}
