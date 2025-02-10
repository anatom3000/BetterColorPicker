#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;
uniform vec3 currentHsv;

#define tint 0.95
#define outlineColor vec3(tint, tint, tint)
#define outlineColorA vec4(outlineColor, 1.0)
#define bgColor vec4(0.0, 0.0, 0.0, 0.0)

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
//void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    //vec2 v_texCoord = fragCoord/iResolution.yy * 1.1 - 0.05;
    //vec3 currentHsv = vec3(0.2, 0.7, 0.8);

    vec2 uv = 2.0*v_texCoord - vec2(1.0, 1.0);
    float r = sqrt(uv.x*uv.x + uv.y*uv.y);

    float delta = fwidth(r);

    float r3over2 = sqrt(3.0) * .5;
    float triangleSize = 0.80;
    float outlineWidth = 0.04;
    
    vec2 innerV1 = triangleSize * vec2(0.0, -1.0);
    vec2 innerV2 = triangleSize * vec2(-r3over2, 0.5);
    vec2 innerV3 = triangleSize * vec2(r3over2, 0.5);
    vec3 innerBary = barycentricCoords(uv, innerV1, innerV2, innerV3);
    vec3 innerDelta = vec3(fwidth(innerBary.x), fwidth(innerBary.y), fwidth(innerBary.z));

    float outerTriangleSize = triangleSize + 2.0 * outlineWidth;
    vec2 outerV1 = outerTriangleSize * vec2(0.0, -1.0);
    vec2 outerV2 = outerTriangleSize * vec2(-r3over2, 0.5);
    vec2 outerV3 = outerTriangleSize * vec2(r3over2, 0.5);
    vec3 outerBary = barycentricCoords(uv, outerV1, outerV2, outerV3);
    vec3 outerDelta = vec3(fwidth(outerBary.x), fwidth(outerBary.y), fwidth(outerBary.z)); 

    if (r < triangleSize) {
        if (0.0 < innerBary.x && innerBary.x <= 1.0 
         && 0.0 < innerBary.y && innerBary.y <= 1.0 
         && 0.0 < innerBary.z && innerBary.z <= 1.0) {
            float h = currentHsv.x;
            float width = widthAt(uv, innerV1, innerV2, innerV3);
            float s = width == 0.0 ? 0.0 : 0.5 + uv.x/width;
            float v = (uv.y - innerV1.y) / (innerV2.y - innerV1.y);
            
            float alphaX = smoothstep(innerDelta.x, 0.0, innerBary.x);
            float alphaY = smoothstep(innerDelta.y, 0.0, innerBary.y);
            float alphaZ = smoothstep(innerDelta.z, 0.0, innerBary.z);

            vec4 col = vec4(hsv2rgb(vec3(h, s, v)), 1.0);
            gl_FragColor = mix(col, outlineColorA, max(alphaX, max(alphaY, alphaZ)));
        } else {

            if (0.0 < outerBary.x && outerBary.x <= 1.0 
             && 0.0 < outerBary.y && outerBary.y <= 1.0 
             && 0.0 < outerBary.z && outerBary.z <= 1.0) {
                float alphaX = (r > triangleSize - outlineWidth) ? 0.0 : smoothstep(outerDelta.x, 0.0, outerBary.x);
                float alphaY = (r > triangleSize - outlineWidth) ? 0.0 : smoothstep(outerDelta.y, 0.0, outerBary.y);
                float alphaZ = (r > triangleSize - outlineWidth) ? 0.0 : smoothstep(outerDelta.z, 0.0, outerBary.z);

                gl_FragColor = vec4(outlineColor, 1.0-max(alphaX, max(alphaY, alphaZ)));
            } else if (r > triangleSize - outlineWidth) {
                float alpha = smoothstep(triangleSize-outlineWidth+delta, triangleSize-outlineWidth, r);
                gl_FragColor = vec4(outlineColor, 1.0-alpha);
            } else {
                gl_FragColor = bgColor;
            }
        }
    } else if (r < 1.0 - outlineWidth) {
        float angle = -0.5 - atan(uv.y, uv.x) / 2.0 / 3.141592;
        vec4 col = vec4(hsv2rgb(vec3(angle, 1.0, 1.0)), 1.0);
        float outerAlpha = smoothstep(1.0-outlineWidth-delta, 1.0-outlineWidth, r);
        float innerAlpha = smoothstep(triangleSize+delta, triangleSize, r);
        
        gl_FragColor = mix(col, outlineColorA, max(innerAlpha, outerAlpha));
    } else if (r < 1.0) {
        float alpha = smoothstep(1.0-delta, 1.0, r);
        
        gl_FragColor = vec4(outlineColor, 1.0-alpha);
    } else {
        gl_FragColor = bgColor;
    }

}
