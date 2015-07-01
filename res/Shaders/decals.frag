material sampler2D decalTex;

varying vec2 vTexCoords;
varying vec3 vColor;

/*
see http://chimera.labs.oreilly.com/books/1234000001814/ch07.html#SmoothingAndDerivatives
*/

void main() {
    float sd = texture2D(decalTex, vTexCoords).a; // signed-distance
    float smoothWidth = fwidth(sd);

    const float centerAlpha = 0.5;

    float alpha = smoothstep(
            centerAlpha - smoothWidth,
            centerAlpha + smoothWidth,
            sd);

    gl_FragColor = vec4(vColor, alpha);
}
