layout(lines) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

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
    vec3    v0;
    vec3    v1;
    float   s;
    float   t;
    vec3    color;
} outData;

void main() {
    vec3 v0 = (uTransform * gl_in[0].gl_Position).xyz;
    vec3 v1 = (uTransform * gl_in[1].gl_Position).xyz;

    vec3 axis = v1 - v0;

    vec3 axisY = normalize(axis);
    vec3 axisX = normalize(cross(axisY, -v0));
    vec3 axisZ = normalize(cross(axisX, axisY));

    mat3 rotate = mat3(axisX, axisY, axisZ);

    float height = length(axis);
    float radius = inData[0].color.a;

    mat3 scale = mat3(
        radius, 0.0, 0.0,
        0.0, height, 0.0,
        0.0, 0.0, 1.0);
    mat3 M = rotate * scale;

    gl_Position = uProjection * vec4(v0 + M * vec3(-0.5, 0.0, 0.0), 1.0);
    outData.v0 = v0;
    outData.v1 = v1;
    outData.s = 0.0;
    outData.t = 0.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + M * vec3( 0.5, 0.0, 0.0), 1.0);
    outData.v0 = v0;
    outData.v1 = v1;
    outData.s = 1.0;
    outData.t = 0.0;
    outData.color = inData[0].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + M * vec3(-0.5, 1.0, 0.0), 1.0);
    outData.v0 = v0;
    outData.v1 = v1;
    outData.s = 0.0;
    outData.t = 1.0;
    outData.color = inData[1].color.rgb;
    EmitVertex();

    gl_Position = uProjection * vec4(v0 + M * vec3( 0.5, 1.0, 0.0), 1.0);
    outData.v0 = v0;
    outData.v1 = v1;
    outData.s = 1.0;
    outData.t = 1.0;
    outData.color = inData[1].color.rgb;
    EmitVertex();
}
