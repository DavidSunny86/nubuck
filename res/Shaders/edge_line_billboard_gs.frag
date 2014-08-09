/*
OPTIONS:
PERFORM_DEPTH_PEEL
STIPPLE
*/

layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

layout(std140) uniform UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

uniform sampler2D depthTex;
uniform sampler2D solidDepth;
uniform sampler2D peelDepth;

in AxisData {
    vec3    v0;
    vec3    v1;
    float   t;
    vec3    color;
} inData;

vec3 WorldToScreenSpace(vec4 v) {
    vec4 v_cs = uProjection * v;
    vec4 v_ns = v_cs / v_cs.w;
    vec3 v_ss = 0.5 * (vec3(1.0, 1.0, 1.0) + v_ns.xyz);
    v_ss.x *= uWidth;
    v_ss.y *= uHeight;
    return v_ss;
}

void main() {
    vec3 spinePos_ws = inData.v0 + inData.t * (inData.v1 - inData.v0);
    vec3 spinePos_ss = WorldToScreenSpace(vec4(spinePos_ws, 1.0));

    vec2 texCoords;
    texCoords.x = spinePos_ss.x / uWidth;
    texCoords.y = spinePos_ss.y / uHeight;

    float sdepth = texture2D(solidDepth, texCoords).a;
    if(spinePos_ss.z > sdepth) discard;

    if(PERFORM_DEPTH_PEEL) {
        float pdepth = texture2D(peelDepth, texCoords).a;
        if(spinePos_ss.z > pdepth) discard;

        float depth = texture2D(depthTex, texCoords).a;
        if(spinePos_ss.z <= depth) discard;
    }

    float alpha = 1.0;

    if(STIPPLE) {
        vec3 v0_ss = WorldToScreenSpace(vec4(inData.v0, 1.0));
        float d_ss = length(spinePos_ss.xy - v0_ss.xy);

        float s = sin(0.25 * d_ss);
        if(0.0 > s) alpha = 0.0;
    }

    gl_FragColor = vec4(inData.color, alpha);
}
