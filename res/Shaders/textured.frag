uniform sampler2D uniTexDiffuse;

in vec2 vTexCoord0;

out vec4 fragColor;
 
void main() {
	fragColor = texture2D(uniTexDiffuse, vTexCoord0);
}