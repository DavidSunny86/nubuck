uniform sampler2D texture;

varying vec2 vTexCoords;

void main() {
    vec4 col = texture2D(texture, vTexCoords);
    gl_FragColor.rgb = col.a * vec3(1.0, 1.0, 1.0);
    gl_FragColor.a = 1.0;
}
