#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;
uniform vec3 currentHsv;

#define tint 0.95
#define outlineColor vec4(tint, tint, tint, 1.0)

// https://github.com/hughsk/glsl-hsv2rgb
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 barycentricCoords(vec2 pos, vec2 v1, vec2 v2, vec2 v3) {
    float l1 = ((v2.y - v3.y)*(pos.x - v3.x) + (v3.x - v2.x)*(pos.y - v3.y))
        / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

    float l2 = ((v3.y - v1.y)*(pos.x - v3.x) + (v1.x - v3.x) * (pos.y - v3.y))
        / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

    float l3 = 1.0 - l1 - l2;
    
    return vec3(l1, l2, l3);
}

float widthAt(vec2 pos, vec2 v1, vec2 v2, vec2 v3) {
    float a = (v2.x - v3.x) / (v2.y - v1.y);
    float b = -a*v1.y;

    return (a*pos.y + b);
}

void main() {
    vec2 uv = 2.0*v_texCoord - vec2(1.0, 1.0);
    float r = sqrt(uv.x*uv.x + uv.y*uv.y);
    float r3over2 = sqrt(3.0) * .5;
    float triangleSize = 0.80;
    float outlineWidth = 0.04;

    if (r < triangleSize) {
        vec2 v1 = triangleSize * vec2(0.0, -1.0);
        vec2 v2 = triangleSize * vec2(-r3over2, 0.5);
        vec2 v3 = triangleSize * vec2(r3over2, 0.5);

        vec3 bary = barycentricCoords(uv, v1, v2, v3);

        if (0.0 < bary.x && bary.x <= 1.0 
         && 0.0 < bary.y && bary.y <= 1.0 
         && 0.0 < bary.z && bary.z <= 1.0) {
            float h = currentHsv.x;
            float width = widthAt(uv, v1, v2, v3);
            float s = width == 0.0 ? 0.0 : 0.5 + uv.x/width;
            float v = (uv.y - v1.y) / (v2.y - v1.y);

            vec3 col = hsv2rgb(vec3(h, s, v));
            gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
        } else {
            float outerTriangleSize = triangleSize + 2.0 * outlineWidth;
            vec2 v1 = outerTriangleSize * vec2(0.0, -1.0);
            vec2 v2 = outerTriangleSize * vec2(-r3over2, 0.5);
            vec2 v3 = outerTriangleSize * vec2(r3over2, 0.5);

            vec3 bary = barycentricCoords(uv, v1, v2, v3);

            if (0.0 < bary.x && bary.x <= 1.0 
             && 0.0 < bary.y && bary.y <= 1.0 
             && 0.0 < bary.z && bary.z <= 1.0) {
                gl_FragColor = outlineColor;
            } else if (r > triangleSize - outlineWidth) {
                gl_FragColor = outlineColor;
            } else {
                gl_FragColor = vec4(0.0);
            }
        }
    } else if (r < 1.0 - outlineWidth) {
        float angle = -0.5 - atan(uv.y, uv.x) / 2.0 / 3.141592;
        vec3 col = hsv2rgb(vec3(angle, 1.0, 1.0));
        gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
    } else if (r < 1.0) {
        gl_FragColor = outlineColor;
    } else {
        gl_FragColor = vec4(0.0);
    }

}
