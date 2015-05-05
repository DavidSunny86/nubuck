/*
OPTIONS:
XRAY
*/

layout(std140) uniform UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

uniform sampler2D depthTex;
uniform sampler2D solidDepth;
uniform sampler2D peelDepth;

material sampler2D font;

varying vec4 vColor;
varying vec2 vTexCoords;
varying vec3 vDepthProxy_ss;

void main() {
    vec2 texCoords;
    texCoords.x = vDepthProxy_ss.x / uWidth;
    texCoords.y = vDepthProxy_ss.y / uHeight;

    vec4 diffuse = texture2D(font, vTexCoords);

    vec4 strokeColor = vec4(0.0, 0.0, 0.0, diffuse.a);
    vec4 brushColor = vec4(vColor.rgb, diffuse.a);

    float sdepth = texture2D(solidDepth, texCoords).a;
    if(vDepthProxy_ss.z > sdepth) {
        if(XRAY) {
            brushColor.rgb *= 0.5;
        }else {
            discard;
        }
    }

    vec4 color = strokeColor;
    // color alpha encodes outline threshold
    // a = 0 is no outline
    if(vColor.a < diffuse.a) color = brushColor;
    gl_FragColor = color;
}
