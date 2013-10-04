layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
    mat3 uNormalMat;
};

layout(std140) uniform UniformsLights {
    vec3    uLightVec0;
    vec3    uLightVec1;
    vec3    uLightVec2;
    vec4    uLightDiffuseColor0;
    vec4    uLightDiffuseColor1;
    vec4    uLightDiffuseColor2;
    float   uShininess;
};

layout(std140) uniform UniformsSkeleton {
    vec4 uColor;
};

uniform float uEdgeRadiusSq;

out vec4 fragColor;

// in edge's local space
in mat4     vObjectToEye;
in mat4     vObjectToClip; 
in vec4     vPosition;
in float    vHalfHeightSq;
in vec4     vEyePos;
in vec3     vLightDir;

void main() {
    vec4 s = vEyePos;
    vec4 v = normalize(vPosition - s);
    float f = (s.x * v.x + s.y * v.y) / (v.x * v.x + v.y * v.y);
    float d = sqrt((uEdgeRadiusSq - (s.x * s.x + s.y * s.y)) / (v.x * v.x + v.y * v.y) + f * f);
    if(0 <= d) {
        vec4 p = s + (-d - f) * v;
        if(p.z * p.z > vHalfHeightSq) p = s + ( d - f) * v;
        if(p.z * p.z > vHalfHeightSq) discard;
        
        vec3 normal = normalize(vObjectToEye * vec4(p.x, p.y, 0.0, 0.0)).xyz;
        float d0 = clamp(dot(normal, normalize(uLightVec0)), 0.0, 1.0);
        float d1 = clamp(dot(normal, normalize(uLightVec1)), 0.0, 1.0);
        float d2 = clamp(dot(normal, normalize(uLightVec2)), 0.0, 1.0);
		vec4 diff = d0 * uLightDiffuseColor0 + d1 * uLightDiffuseColor1 + d2 * uLightDiffuseColor2;

        vec4 tp = vObjectToEye * p;
        vec3 view = -normalize(tp.xyz);

        float h0 = pow(clamp(dot(normal, normalize(view + uLightVec0)), 0.0, 1.0), uShininess);
        float h1 = pow(clamp(dot(normal, normalize(view + uLightVec1)), 0.0, 1.0), uShininess);
        float h2 = pow(clamp(dot(normal, normalize(view + uLightVec2)), 0.0, 1.0), uShininess);
		vec4 spec = h0 * uLightDiffuseColor0 + h1 * uLightDiffuseColor1 + h2 * uLightDiffuseColor2;

        fragColor = uColor * (diff + spec);

		vec4 proj = uProjection * tp;
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
    } else discard;
}
