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

attribute(0) vec4 aPosition;
attribute(2) vec4 aColor;
attribute(3) vec2 aTexCoords;
attribute(4) vec3 aA0; // origin
attribute(5) vec3 aA1; // bbox center (in string space)
attribute(6) vec3 aA2; // depth proxy

varying vec4 vColor;
varying vec2 vTexCoords;
varying vec3 vDepthProxy_ss;

vec3 EyeToScreenSpace(vec4 v) {
    vec4 v_cs = uProjection * v;
    vec4 v_ns = v_cs / v_cs.w;
    vec3 v_ss = 0.5 * (vec3(1.0, 1.0, 1.0) + v_ns.xyz);
    v_ss.x *= uWidth;
    v_ss.y *= uHeight;
    return v_ss;
}

void main() {
    vColor = aColor;
    vTexCoords = aTexCoords;
    vDepthProxy_ss = EyeToScreenSpace(uTransform * vec4(aA2, 1.0));

    vec3 origin = aA0;
    vec3 center_s = aA1; // bbox center in string space
    vec3 center_o = origin + aA1; // bbox center in object space
    vec4 center_e = uTransform * vec4(center_o, 1.0); // bbox center in eye space

    vec3 relPos = aPosition.xyz - center_s;

    gl_Position = uProjection * (center_e + vec4(relPos, 0.0));
}
