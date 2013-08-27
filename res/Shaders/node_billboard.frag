uniform vec4 uMatDiffuseColor;

in vec2 vTexCoord0;

out vec4 fragColor;
 
void main() {
	if(dot(vTexCoord0, vTexCoord0) <= 1.0)
		fragColor = uMatDiffuseColor;
	else discard;
}