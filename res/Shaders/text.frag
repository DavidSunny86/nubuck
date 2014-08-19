uniform sampler2D font;

varying vec2 vTexCoords;

void main() {
    vec4 diffuse = texture2D(font, vTexCoords);
    gl_FragColor = diffuse;
}
