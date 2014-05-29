layout(std140) uniform UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

uniform sampler2D depthTex;

in BillboardData {
    vec3 spinePos_ss;
    vec3 color;
} inData;

void main() {
    vec2 texCoords;
    texCoords.x = inData.spinePos_ss.x / uWidth;
    texCoords.y = inData.spinePos_ss.y / uHeight;

    float sdepth = texture2D(depthTex, texCoords).a;
    if(inData.spinePos_ss.z > sdepth) discard;

    gl_FragColor.rgb = inData.color;
}
