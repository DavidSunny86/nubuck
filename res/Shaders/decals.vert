layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat3 uNormalMat;
};

attribute(0) vec4    aPosition;
attribute(1) vec3    aNormal;
attribute(2) vec4    aColor;
attribute(3) vec2    aTexCoords;
attribute(4) vec3    aA0;
attribute(5) vec3    aA1;
attribute(6) vec3    aA2;
attribute(7) vec3    aA3;

varying vec2 vTexCoords;
varying vec3 vColor;

void main() {
    // decal-space coordinates are encoded in texture coordinates and normal
    float decalSize = aNormal.x;
    vec3 pos = decalSize * vec3(aTexCoords, 0.0);

    // decal-space to mesh-space
    mat3 decalToMesh = mat3(aA0, aA1, aA2);
    pos = decalToMesh * pos + aPosition.xyz;

    // [-1, 1] to [0, 1]
    vTexCoords = 0.5 * (aTexCoords + vec2(1.0, 1.0));

    vColor = aColor.xyz;

    // mesh-space to clip-space
    gl_Position = uProjection * (uTransform * vec4(pos, 1.0));
}
