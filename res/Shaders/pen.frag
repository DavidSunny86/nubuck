in AxisData {
    float   s;
    float   t;
    vec3    color;
} inData;

void main() {
    if(inData.s * inData.s + inData.t * inData.t > 1.0) discard;
    gl_FragColor = vec4(inData.color, 1.0);
}
