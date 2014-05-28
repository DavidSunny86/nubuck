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

    float sdepth = texture2D(depthTex, texCoords).a;
    if(spinePos_ss.z > sdepth) discard;

    vec3 v0_ss = WorldToScreenSpace(vec4(inData.v0, 1.0));

    float d_ss = length(spinePos_ss.xy - v0_ss.xy);

    float alpha = 1.0;

    // stippling:
    // float s = sin(0.25 * d_ss);
    // if(0.0 > s) alpha = 0.0;

    gl_FragColor = vec4(inData.color, alpha);
}
