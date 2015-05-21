/*
OPTIONS:
LIGHTING_ENABLED
LIGHTING_TWOSIDED_ENABLED
PREMULT_ALPHA
PERFORM_DEPTH_PEEL
PERFORM_DEPTH_TEST
*/

layout(std140) uniform UniformsLights {
    vec3 uLightVec[3];
    vec4 uLightDiffuseColor[3];
    float uLambertFactor;
    float uShininess;
    float uRoughness;
    float uFresnel;
    float uDiffuseReflectance;
    int   uLightingModel;
};

layout(std140) uniform UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

uniform sampler2D depthTex;
uniform sampler2D solidDepth;

material vec4 uDiffuseColor;
// pattern is enabled if patternColor.a > 0.0
material vec4        patternColor;
material sampler2D   patternTex;

#include <Shaders\lighting.glsl>

varying vec4 vPosition;
varying vec3 vNormal;
varying vec4 vColor;
varying vec4 vPatternColor;

void main() {
    vec4 color = uDiffuseColor * vColor;

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

        color.rgb = lighting(normal, view, color.rgb);
    }

    if(0.0 < patternColor.a) {
        // the pattern texture occupies 8x8 pixels, which looks pretty good for small textures.
        vec2 texCoords = vec2(gl_FragCoord.x, gl_FragCoord.y) / 8.0;
        vec4 pattern = texture2D(patternTex, texCoords);
        float alpha = pattern.a * patternColor.a * vPatternColor.a;
        color.rgb = (1.0 - alpha) * color.rgb + alpha * patternColor.rgb * vPatternColor.rgb * pattern.rgb;
    }

    if(PREMULT_ALPHA) color.rgb *= color.a;

    gl_FragColor = color;
}
