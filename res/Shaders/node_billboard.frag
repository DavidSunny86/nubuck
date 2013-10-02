layout(std140) uniform UniformsHot {
    mat4 uProjection;
    mat4 uTransform;
    mat4 uInvTransform;
};

layout(std140) uniform UniformsLights {
  vec3 uLightVec0;
  vec3 uLightVec1;
  vec3 uLightVec2;
  vec4 uLightDiffuseColor0;
  vec4 uLightDiffuseColor1;
  vec4 uLightDiffuseColor2;
};

in vec3 vPosition;
in vec2 vTexCoord0;

out vec4 fragColor;
 
void main() {
	float clip = dot(vTexCoord0, vTexCoord0);
	if(clip <= 1.0) {
		float n = sqrt(1.0 - clip);
		vec3 position = vPosition;
		position.z += 2.0 * n;
		vec3 normal = normalize(vec3(vTexCoord0, n));
		float d0 = clamp(dot(normalize(uLightVec0), normal), 0.0, 1.0);
		float d1 = clamp(dot(normalize(uLightVec1), normal), 0.0, 1.0);
		float d2 = clamp(dot(normalize(uLightVec2), normal), 0.0, 1.0);
		fragColor = d0 * uLightDiffuseColor0 + d1 * uLightDiffuseColor1 + d2 * uLightDiffuseColor2;
		
		vec4 proj = uProjection * vec4(position, 1.0);
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
	}
	else discard;
}
