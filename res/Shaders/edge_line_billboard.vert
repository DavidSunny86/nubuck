layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

attribute(0) vec4 aPosition;
attribute(1) vec3 aNormal;
attribute(2) vec4 aColor;
attribute(3) vec2 aTexCoord0;

varying vec3 vColor;

void main() {
    vec3 axis = (uTransform * vec4(aNormal, 0.0)).xyz; // edge axis in eye space

    vec3 p0 = (uTransform * aPosition).xyz;

    vec3 axisY = normalize(axis);
    vec3 axisX = normalize(cross(axisY, -p0));
    vec3 axisZ = normalize(cross(axisX, axisY));

    mat3 rotate = mat3(axisX, axisY, axisZ);

    float height = length(axis);
    float radius = aColor.a;

    mat3 scale = mat3(
        radius, 0.0, 0.0,
        0.0, height, 0.0,
        0.0, 0.0, 1.0);
    mat3 M = rotate * scale;

    vec3 pos = p0 + M * vec3(aTexCoord0, 0.0);

    vColor = aColor.xyz;

    mat4 projection = uProjection;
    if(PROJECTION_OFFSET_ENABLED) projection[2][2] *= (1.0 - PROJECTION_OFFSET);

    gl_Position = projection * vec4(pos, 1.0);
}
