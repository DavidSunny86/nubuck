layout(std140) uniform UniformsLights {
    vec3 uLightVec0;
    vec3 uLightVec1;
    vec3 uLightVec2;
    vec4 uLightDiffuseColor0;
    vec4 uLightDiffuseColor1;
    vec4 uLightDiffuseColor2;
    float uShininess;
};

varying vec4 vPosition;
varying vec3 vNormal;
varying vec4 vColor;

void main() {
    vec3 view = normalize(-vPosition.xyz);
    vec3 normal = normalize(vNormal);
    vec3 color = vColor.xyz;
    if(dot(normal, view) < 0.0) {
        normal *= -1.0;
        color *= 0.9f;
    }
    float d0 = clamp(dot(normal, normalize(uLightVec0)), 0.0, 1.0);
    float d1 = clamp(dot(normal, normalize(uLightVec1)), 0.0, 1.0);
    float d2 = clamp(dot(normal, normalize(uLightVec2)), 0.0, 1.0);
    float h0 = pow(clamp(dot(normal, normalize(view + uLightVec0)), 0.0, 1.0), uShininess);
    float h1 = pow(clamp(dot(normal, normalize(view + uLightVec1)), 0.0, 1.0), uShininess);
    float h2 = pow(clamp(dot(normal, normalize(view + uLightVec2)), 0.0, 1.0), uShininess);
    vec4 diff = d0 * uLightDiffuseColor0 + d1 * uLightDiffuseColor1 + d2 * uLightDiffuseColor2;
    // vec4 spec = h0 * uLightDiffuseColor0 + h1 * uLightDiffuseColor1 + h2 * uLightDiffuseColor2;
    vec4 spec = vec4(0.0, 0.0, 0.0, 0.0);
    gl_FragColor = vec4(color, 1.0) * (diff + spec);
    gl_FragColor.a = vColor.a;
}
