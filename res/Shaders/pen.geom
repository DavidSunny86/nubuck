layout(lines) in;
layout(triangle_strip) out;
layout(max_vertices = 8) out;

layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

in VertexData {
    vec4 color;
} inData[];

out AxisData {
    float   s;
    float   t;
    vec3    color;
} outData;

vec2 cross2(vec2 v) {
    return vec2(-v.y, v.x);
}

void main() {
    vec2 v0 = (uTransform * gl_in[0].gl_Position).xy;
    vec2 v1 = (uTransform * gl_in[1].gl_Position).xy;

    vec2 axis = v1 - v0;

    vec2 normal = normalize(cross2(axis));

    float height = length(axis);
    float radius = inData[0].color.a;

    vec2 ext = 0.5 * radius * normal;

    gl_Position = uProjection * vec4(v0 - ext, 0.0, 1.0);
    outData.s = 0.0;
    outData.t = 0.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + ext, 0.0, 1.0);
    outData.s = 0.0;
    outData.t = 0.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v1 - ext, 0.0, 1.0);
    outData.s = 0.0;
    outData.t = 0.0;
    outData.color = inData[1].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v1 + ext, 0.0, 1.0);
    outData.s = 0.0;
    outData.t = 0.0;
    outData.color = inData[1].color.rgb;
    EmitVertex();

    EndPrimitive();

    float halfRad = 0.5 * radius;

    gl_Position = uProjection * vec4(v0 + halfRad * vec2( 1.0, -1.0), 0.0, 1.0);
    outData.s =  1.0;
    outData.t = -1.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + halfRad * vec2( 1.0,  1.0), 0.0, 1.0);
    outData.s =  1.0;
    outData.t =  1.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + halfRad * vec2(-1.0, -1.0), 0.0, 1.0);
    outData.s = -1.0;
    outData.t = -1.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + halfRad * vec2(-1.0,  1.0), 0.0, 1.0);
    outData.s = -1.0;
    outData.t =  1.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();
}
