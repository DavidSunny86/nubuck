layout(std140) uniform UniformsLights {
    vec3 uLightVec0;
    vec3 uLightVec1;
    vec3 uLightVec2;
    vec4 uLightDiffuseColor0;
    vec4 uLightDiffuseColor1;
    vec4 uLightDiffuseColor2;
    float uShininess;
};

layout(std140) uniform UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

uniform sampler2D depthTex;

varying vec4 vPosition;
varying vec3 vNormal;
varying vec4 vColor;

/*
float linDepth(float z_b) {
    float zNear = 0.1;
    float zFar = 100.0;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
    return z_e;
}
*/

float linDepth(float z_b) {
    float n = 0.1;
    float f = 100.0;
    float z_n = 2.0 * z_b - 1.0;
    float z = z_b;
    return (z * n) / (f - z * (f - n));
}

void main() {
    vec2 texCoords;
    texCoords.x = gl_FragCoord.x / uWidth;
    texCoords.y = gl_FragCoord.y / uHeight;
    float depth = texture2D(depthTex, texCoords).a;
    if(gl_FragCoord.z <= depth) discard;

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

    // gl_FragColor = linDepth(depth) * vec4(1.0, 1.0, 1.0, 1.0);
}
