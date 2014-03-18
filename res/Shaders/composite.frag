uniform sampler2D texture;

varying vec2 vTexCoords;

void main() {
    vec4 col = texture2D(texture, vTexCoords);
    col.rgb = col.rgb * col.a;
    gl_FragColor = col;
}
