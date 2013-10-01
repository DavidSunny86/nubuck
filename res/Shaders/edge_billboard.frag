uniform mat4 uTransform;
out vec4 fragColor;

in mat4 vWorldToObject;
in vec4 vPosition;

void main() {
    float r = 1.0;
    mat4 eyeToObject = vWorldToObject * inverse(uTransform);
    vec4 s = eyeToObject * vec4(0.0, 0.0, 0.0, 1.0);
    vec4 v = normalize(vPosition - s);
    float f = (s.x * v.x + s.y * v.y) / (v.x * v.x + v.y * v.y);
    float d = sqrt((r * r - (s.x * s.x + s.y * s.y)) / (v.x * v.x + v.y * v.y) + f * f);
    if(0 <= d) {
        vec4 p = s + (-d - f) * v;
        if(p.z * p.z > 25) p = s + ( d - f) * v;
        if(p.z * p.z > 25) discard;
        
        vec3 normal = vec3(p.x, p.y, 0.0);
        vec3 lightDir = (eyeToObject * vec4(0.0, 0.0, 1.0, 0.0)).xyz;
        float d = clamp(dot(normal, normalize(lightDir)), 0.0, 1.0);
        fragColor = d * vec4(1.0, 1.0, 1.0, 1.0);
    } else discard;
}
