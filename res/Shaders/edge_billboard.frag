uniform mat4 uTransform;
uniform mat4 uWorldToObject;
out vec4 fragColor;

in vec4 vPosition;
in mat4 vWorldToObject;
in mat4 vObjectToWorld;

void main() {
    float r = 0.5;
    vec4 s = uWorldToObject * inverse(uTransform) * vec4(0.0, 0.0, 0.0, 1.0);
    vec4 v = normalize(vPosition - s);
    float f = (s.x * v.x + s.y * v.y) / (v.x * v.x + v.y * v.y);
    float d = sqrt((r * r - (s.x * s.x + s.y * s.y)) / (v.x * v.x + v.y * v.y) + f * f);
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    if(0 <= d) {
        vec4 p = s + (d - f) * v;
        if(p.z * p.z <= 25) fragColor = vec4(0.0, 0.0, 1.0, 1.0);
        p = s + (-d - f) * v;
        if(p.z * p.z <= 25) fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
}
