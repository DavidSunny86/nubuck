uniform sampler2D font;

varying vec4 vColor;
varying vec2 vTexCoords;

void main() {
    vec4 diffuse = texture2D(font, vTexCoords);
    vec3 color = vec3(0.0);
    // color alpha encodes outline threshold
    // a = 0 is no outline
    if(vColor.a < diffuse.a) color = vColor.rgb;
    gl_FragColor = vec4(color, diffuse.a);
}
