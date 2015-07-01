material sampler2D decalTex;

varying vec2 vTexCoords;
varying vec3 vColor;

void main() {
    vec4 color = texture2D(decalTex, vTexCoords);
    if(0.0 < color.a) {
        gl_FragColor = vec4(vColor, 1.0) * color;
    } else discard; // cannot use blending, so we do alpha-test
}
