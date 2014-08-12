layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

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

in VertexData {
    vec3    color;
    float   size;
} inData[];

out BillboardData {
    vec3 spinePos_ss;
    vec3 color;
    vec2 texCoords; // in [-1, 1]
} outData;

vec3 EyeToScreenSpace(vec4 v) {
    vec4 v_cs = uProjection * v;
    vec4 v_ns = v_cs / v_cs.w;
    vec3 v_ss = 0.5 * (vec3(1.0, 1.0, 1.0) + v_ns.xyz);
    v_ss.x *= uWidth;
    v_ss.y *= uHeight;
    return v_ss;
}

void main() {
    vec3 center = gl_in[0].gl_Position.xyz;

    float size = inData[0].size;

    vec3 spinePos_ss = EyeToScreenSpace(vec4(center, 1.0));

    gl_Position = uProjection * vec4(center + vec3(-size, -size, 0.0), 1.0);
    outData.spinePos_ss = spinePos_ss;
    outData.color = inData[0].color;
    outData.texCoords = vec2(-1.0, -1.0);
    EmitVertex();

    gl_Position = uProjection * vec4(center + vec3( size, -size, 0.0), 1.0);
    outData.spinePos_ss = spinePos_ss;
    outData.color = inData[0].color;
    outData.texCoords = vec2( 1.0, -1.0);
    EmitVertex();

    gl_Position = uProjection * vec4(center + vec3(-size,  size, 0.0), 1.0);
    outData.spinePos_ss = spinePos_ss;
    outData.color = inData[0].color;
    outData.texCoords = vec2(-1.0,  1.0);
    EmitVertex();

    gl_Position = uProjection * vec4(center + vec3( size,  size, 0.0), 1.0);
    outData.spinePos_ss = spinePos_ss;
    outData.color = inData[0].color;
    outData.texCoords = vec2( 1.0,  1.0);
    EmitVertex();
}
