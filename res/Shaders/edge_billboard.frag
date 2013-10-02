layout(std140) uniform UniformsLights {
  vec3 uLightVec0;
  vec3 uLightVec1;
  vec3 uLightVec2;
  vec4 uLightDiffuseColor0;
  vec4 uLightDiffuseColor1;
  vec4 uLightDiffuseColor2;
};

uniform float uEdgeRadiusSq;

out vec4 fragColor;

// in edge's local space
in mat4     vObjectToEye;
in mat4     vObjectToWindow; 
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
		fragColor = d0 * uLightDiffuseColor0 + d1 * uLightDiffuseColor1 + d2 * uLightDiffuseColor2;

		vec4 proj = vObjectToWindow * p;
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
    } else discard;
}
