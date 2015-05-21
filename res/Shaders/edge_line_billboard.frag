material vec4 uDiffuseColor;

varying vec3 vColor;

void main() {
    gl_FragColor = vec4(uDiffuseColor.rgb * vColor, 1.0);
}
