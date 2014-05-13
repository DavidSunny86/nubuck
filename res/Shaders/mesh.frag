/*
OPTIONS:
LIGHTING_ENABLED
LIGHTING_TWOSIDED_ENABLED
PREMULT_ALPHA
PERFORM_DEPTH_PEEL
PERFORM_DEPTH_TEST
*/

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
uniform sampler2D solidDepth;

varying vec4 vPosition;
varying vec3 vNormal;
varying vec4 vColor;

void main() {
    vec4 color = vColor;

    if(PERFORM_DEPTH_TEST || PERFORM_DEPTH_PEEL) {
        vec2 texCoords;
        texCoords.x = gl_FragCoord.x / uWidth;
        texCoords.y = gl_FragCoord.y / uHeight;
        if(PERFORM_DEPTH_TEST) {
            float sdepth = texture2D(solidDepth, texCoords).a;
            if(gl_FragCoord.z > sdepth) discard;
        }
        if(PERFORM_DEPTH_PEEL) {
            float depth = texture2D(depthTex, texCoords).a;
            if(gl_FragCoord.z <= depth) discard;
        }
    }

    if(LIGHTING_ENABLED) {
        vec3 view = normalize(-vPosition.xyz);
        vec3 normal = normalize(vNormal);

        if(LIGHTING_TWOSIDED_ENABLED) {
            if(dot(normal, view) < 0.0) {
                normal *= -1.0;
                color.rgb *= 0.9f;
            }
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

        color.rgb *= (diff + spec).rgb;
    }

    if(PREMULT_ALPHA) color.rgb *= color.a;

    gl_FragColor = color;
}
