uniform sampler2D font;

varying vec4 vColor;
varying vec2 vTexCoords;

void main() {
    vec4 diffuse = texture2D(font, vTexCoords);
    gl_FragColor = vec4(vColor.rgb, diffuse.a);
}
