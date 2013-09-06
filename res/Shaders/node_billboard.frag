uniform vec4 uMatDiffuseColor0;
uniform mat4 uProjection;

uniform vec4 uMatDiffuseColor1;
uniform vec4 uMatDiffuseColor2;
uniform vec3 uLightPos0;
uniform vec3 uLightPos1;
uniform vec3 uLightPos2;
uniform float   uLightConstantAttenuation;
uniform float   uLightLinearAttenuation;
uniform float	uLightQuadricAttenuation;

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
		vec3 toLight0 = uLightPos0 - position;
		vec3 toLight1 = uLightPos1 - position;
		vec3 toLight2 = uLightPos2 - position;
		float dist0 = length(toLight0);
		float dist1 = length(toLight1);
		float dist2 = length(toLight2);
		float d0 = clamp(dot(normalize(toLight0), normal), 0.0, 1.0);
		float d1 = clamp(dot(normalize(toLight1), normal), 0.0, 1.0);
		float d2 = clamp(dot(normalize(toLight2), normal), 0.0, 1.0);
		float att0 = 1.0 / (uLightConstantAttenuation + uLightLinearAttenuation * dist0 + uLightQuadricAttenuation * dist0 * dist0);
		float att1 = 1.0 / (uLightConstantAttenuation + uLightLinearAttenuation * dist1 + uLightQuadricAttenuation * dist1 * dist1);
		float att2 = 1.0 / (uLightConstantAttenuation + uLightLinearAttenuation * dist2 + uLightQuadricAttenuation * dist2 * dist2);
		fragColor = att0 * d0 * uMatDiffuseColor0 + att1 * d1 * uMatDiffuseColor1 + att2 * d2 * uMatDiffuseColor2;
		
		vec4 proj = uProjection * vec4(position, 1.0);
		gl_FragDepth = 0.5 * (1.0 + proj.z / proj.w);
	}
	else discard;
}
