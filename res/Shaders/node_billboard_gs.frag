/*
OPTIONS:
PERFORM_DEPTH_PEEL
*/

layout(std140) uniform UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

uniform sampler2D depthTex;
uniform sampler2D solidDepth;
uniform sampler2D peelDepth;

in BillboardData {
    vec3 spinePos_ss;
    vec3 color;
} inData;

void main() {
    vec2 texCoords;
    texCoords.x = inData.spinePos_ss.x / uWidth;
    texCoords.y = inData.spinePos_ss.y / uHeight;

    float sdepth = texture2D(solidDepth, texCoords).a;
    if(inData.spinePos_ss.z > sdepth) discard;

    if(PERFORM_DEPTH_PEEL) {
        float pdepth = texture2D(peelDepth, texCoords).a;
        if(inData.spinePos_ss.z > pdepth) discard;

        float depth = texture2D(depthTex, texCoords).a;
        if(inData.spinePos_ss.z <= depth) discard;
    }

    gl_FragColor = vec4(inData.color, 1.0);
}
