layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat3 uNormalMat;
};

layout(std140) uniform UniformsLights {
    vec3    uLightVec[3];
    vec4    uLightDiffuseColor[3];
    float   uLambertFactor;
    float   uShininess;
    float   uRoughness;
    float   uFresnel;
    float   uDiffuseReflectance;
    int     uLightingModel;
};

layout(std140) uniform UniformsSkeleton {
    vec4    uColor;
    float   uNodeSize;
};

#include <Shaders\lighting.glsl>

material vec4 uDiffuseColor;

// in edge's local space
varying vec4     vColor;
varying mat4     vObjectToEye;
varying mat4     vObjectToClip; 
varying vec4     vPosition;
varying float    vHalfHeightSq;
varying float    vRadiusSq;
varying vec4     vEyePos;
varying vec3     vLightDir;

void main() {
    vec4 s = vEyePos;
    vec4 v = normalize(vPosition - s);
    float f = (s.x * v.x + s.y * v.y) / (v.x * v.x + v.y * v.y);
    float d = sqrt((vRadiusSq - (s.x * s.x + s.y * s.y)) / (v.x * v.x + v.y * v.y) + f * f);
    if(0 <= d) {
        vec4 p = s + (-d - f) * v;
        if(p.z * p.z > vHalfHeightSq) p = s + ( d - f) * v;
        if(p.z * p.z > vHalfHeightSq) discard;
        
        vec4 tp = vObjectToEye * p;
        vec3 view = -normalize(tp.xyz);
        vec3 normal = normalize(vObjectToEye * vec4(p.x, p.y, 0.0, 0.0)).xyz;

        gl_FragColor = vec4(lighting(normal, view, uDiffuseColor.rgb * vColor.rgb), 1.0);

		vec4 proj = uProjection * tp;
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
    } else discard;
}
