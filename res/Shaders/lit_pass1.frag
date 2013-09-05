uniform vec3    uLightPosition;
uniform vec4    uLightDiffuseColor;
uniform float   uLightConstantAttenuation;
uniform float   uLightLinearAttenuation;
uniform float   uLightQuadricAttenuation;

uniform vec4    uMatDiffuseColor;

in vec3     vNormal;
in vec4     vColor;
in float    vDist;

out vec4 fragColor;
 
void main() {	
    vec3 lightDir = normalize(uLightPosition);

    float att = 1.0 / (uLightConstantAttenuation + uLightLinearAttenuation * vDist + uLightQuadricAttenuation * vDist * vDist);
    fragColor = uLightDiffuseColor * uMatDiffuseColor * vColor;
	fragColor.xyz = fragColor.xyz * att * clamp(dot(lightDir, normalize(vNormal)), 0.0, 1.0);
}
