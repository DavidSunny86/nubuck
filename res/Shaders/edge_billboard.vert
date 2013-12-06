layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat3 uNormalMat;
};

attribute(0) vec4    aPosition;
attribute(2) vec4    aColor;
attribute(4) vec3    aA0;
attribute(5) vec3    aA1;
attribute(6) vec3    aA2;
attribute(7) vec3    aA3;
attribute(8) float   aHalfHeightSq;
attribute(9) float   aRadiusSq;

// in edge's local space
varying vec4    vColor;
varying mat4    vObjectToEye;
varying mat4    vObjectToClip;
varying vec4    vPosition;
varying float   vHalfHeightSq;
varying float   vRadiusSq;
varying vec4    vEyePos;
varying vec3    vLightDir;

mat4 InvertTR(mat4 m) {
    mat3 R = transpose(mat3(m));
    vec3 dt = R * -vec3(m[3]);
    return mat4(
        vec4(R[0], 0.0), 
        vec4(R[1], 0.0), 
        vec4(R[2], 0.0),
        vec4(dt, 1.0)
    );
}

void main() {
    vColor = aColor;
    mat4 objectToWorld = mat4(
        vec4(aA0, 0.0),
        vec4(aA1, 0.0),
        vec4(aA2, 0.0),
        vec4(aA3, 1.0)
    );
    mat4 worldToObject = InvertTR(objectToWorld);
    vObjectToEye = uTransform * objectToWorld;
    vObjectToClip = uProjection * vObjectToEye;
    mat4 eyeToObject = worldToObject * uInvTransform;
    vPosition = worldToObject * aPosition;
    vHalfHeightSq = aHalfHeightSq;
    vRadiusSq = aRadiusSq;
    vEyePos = eyeToObject * vec4(0.0, 0.0, 0.0, 1.0);
    vLightDir = (eyeToObject * vec4(0.0, 0.0, 1.0, 0.0)).xyz;
	gl_Position = uProjection * (uTransform * aPosition);
}
