uniform sampler2D uniTexDiffuse;
uniform vec4 uMatDiffuseColor;

in vec2 vTexCoord0;

out vec4 fragColor;
 
void main() {
	fragColor = uMatDiffuseColor * texture2D(uniTexDiffuse, vTexCoord0);
}