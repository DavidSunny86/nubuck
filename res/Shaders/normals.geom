layout(triangles) in;
layout(line_strip) out;
layout(max_vertices = 4) out;

layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat4 uNormalMat;
};

void main() {
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec3 center_os = vec3(0.0);
    center_os += p0;
    center_os += p1;
    center_os += p2;
    center_os /= 3;

    vec3 normal = normalize(cross(p1 - p0, p2 - p0));

    mat4 mvp = uProjection * uTransform;

    float lineLength = 0.5;

    gl_Position = mvp * vec4(center_os, 1.0);
    EmitVertex();

    gl_Position = mvp * vec4(center_os + lineLength * normal, 1.0);
    EmitVertex();

    EndPrimitive();
}
