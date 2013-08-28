uniform vec4 uMatDiffuseColor;

in vec3 vPosition;
in vec2 vTexCoord0;

out vec4 fragColor;
 
void main() {
	vec3 lightPos = vec3(20.0, 20.0, 50.0);
	float clip = dot(vTexCoord0, vTexCoord0);
	if(clip <= 1.0) {
		vec3 normal = normalize(vec3(vTexCoord0, sqrt(1.0 - clip)));
		vec3 lightDir = normalize(lightPos - vPosition);
		float d = clamp(dot(lightDir, normal), 0.0, 1.0);
		fragColor = d * uMatDiffuseColor;
	}
	else discard;
}