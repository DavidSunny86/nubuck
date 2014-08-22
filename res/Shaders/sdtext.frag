uniform sampler2D font;

varying vec2 vTexCoords;

void main() {
    vec4 diffuse = texture2D(font, vTexCoords);
    if(0.5 > diffuse.a) discard;
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
