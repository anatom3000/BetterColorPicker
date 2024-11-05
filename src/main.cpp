#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/ColorSelectPopup.hpp>
#include <Geode/modify/SetupPulsePopup.hpp>

#include "CCSpriteBatchNode.h"
#include "ShaderCache.h"

const double PI = 3.1415926535897932384626433;
const double TRIANGLE_SIZE = .85;

using ColorChangedCallback = std::function<void(ccColor3B)>;

class BetterColorPicker : public CCControl {
public:
    float m_radius;
    CCPoint m_center;
    bool m_touching;
    bool m_touchedHue;
    
    Ref<CCSprite> m_sprite;
    Ref<CCGLProgram> m_shader;
    GLuint m_location;
    Ref<CCSprite> m_hueNipple;
    Ref<CCSprite> m_svNipple;

    ColorChangedCallback m_colorChangedCallback;

    // between 0.0 and 1.0
    double m_hue;
    double m_saturation;
    double m_value;

    static CCPoint v1;
    static CCPoint v2;
    static CCPoint v3;

    static auto* create(ColorChangedCallback callback) {
        auto* node = new BetterColorPicker();
        if (node->init(callback)) {
            node->autorelease();
        } else {
            delete node;
            node = nullptr;
        }

        return node;
    }

    bool init(ColorChangedCallback callback) {
        if (!CCControl::init()) return false;
        this->setTouchEnabled(true);
        this->registerWithTouchDispatcher();

        m_colorChangedCallback = callback;

        m_sprite = CCSprite::create("frame.png"_spr);
        m_sprite->setCascadeOpacityEnabled(true);
        m_radius = 0.5 * m_sprite->getContentSize().width;

        // TODO: AA
        // The Alias/Antialias property belongs to CCSpriteBatchNode, so you can’t individually set the aliased property.
        m_shader = ShaderCache::get()->getProgram("colorPicker");
        m_shader->setUniformsForBuiltins();
        m_sprite->setShaderProgram(m_shader);
        m_shader->use();
        m_location = m_shader->getUniformLocationForName("currentHsv");

        m_sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        this->addChild(m_sprite);


        this->m_hueNipple = CCSprite::create("nipple.png"_spr);
        this->m_svNipple = CCSprite::create("nipple.png"_spr);
        m_hueNipple->setScale(0.8f);
        m_svNipple->setScale(0.8f);
        
        this->addChild(m_hueNipple);
        this->addChild(m_svNipple);

        this->setContentSize(m_sprite->getContentSize());

        return true;
    }

    void updateValues() {
        //m_shader->setUniformLocationWith3f(m_location, m_hue, m_saturation, m_value);
        m_shader->use();
        glUniform3f(m_location, m_hue, m_saturation, m_value);

        this->m_colorChangedCallback(this->getRgbValue());
    }

    ccColor3B getRgbValue() {
        auto rgb = CCControlUtils::RGBfromHSV({.h = m_hue*360.0, .s = m_saturation, .v = m_value});

        return ccc3(
            (GLubyte)255.0*rgb.r,
            (GLubyte)255.0*rgb.g,
            (GLubyte)255.0*rgb.b
        );
    }

    void setRgbValue(ccColor3B color) {
        auto hsv = CCControlUtils::HSVfromRGB({
            .r = color.r / 255.0f,
            .g = color.g / 255.0f,
            .b = color.b / 255.0f
        });

        m_hue = hsv.h / 360.0f;
        m_saturation = hsv.s;
        m_value = hsv.v;

        double radius = m_radius * (1.0+TRIANGLE_SIZE) / 2.0;
        double angle = (m_hue - 0.5) * 2 * PI;
        double y = v1.y + m_value * (v2.y - v1.y);
        double width = widthAt(y);
        double x = width == 0 ? 0.0 : (m_saturation - 0.5) * width ;

        m_hueNipple->setPosition(ccp(radius*cos(angle), radius*sin(angle)));
        m_svNipple->setPosition(ccp(m_radius * TRIANGLE_SIZE * x, m_radius * TRIANGLE_SIZE * y));
        this->updateValues();
    }

    double widthAt(double y) {
        double a = (v2.x - v3.x) / (v2.y - v1.y);
        double b = -a*v1.y;

        return (a*y + b);
    }

    bool touchesHueWheel(CCPoint position) {
        double radius = sqrt(position.x*position.x + position.y*position.y) / m_radius;
        
        return (TRIANGLE_SIZE <= radius && radius <= 1.0);
    }

    std::tuple<double, double, double> barycentricCoords(CCPoint pos) {
        pos /= m_radius;
        pos /= TRIANGLE_SIZE;
        
        double l1 = ((v2.y - v3.y)*(pos.x - v3.x) + (v3.x - v2.x)*(pos.y - v3.y))
            / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

        double l2 = ((v3.y - v1.y)*(pos.x - v3.x) + (v1.x - v3.x) * (pos.y - v3.y))
            / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

        double l3 = 1.0 - l1 - l2;
        
        return {l1, l2, l3};
    }

    double dot(CCPoint u, CCPoint v) {
        return u.x*v.x + u.y*v.y;
    }

    // https://math.stackexchange.com/questions/1092912/find-closest-point-in-triangle-given-barycentric-coordinates-outside
    std::tuple<double, double, double> closestPointInTriangle(double x, double y, double z) {
        CCPoint p = cartesianCoords(x, y, z);

        if (x >= 0 && y < 0) {
            if (z < 0 && dot(p-v1, v2-v1) > 0) {
                y = fmin(1.0, dot(p-v1, v2-v1) / dot(v2-v1, v2-v1));
                z = 0.0;
            } else {
                y = 0.0;
                z = clamp(dot(p-v1, v3-v1) / dot(v3-v1, v3-v1), 0.0, 1.0);
            }

            x = 1.0 - y - z;
        } else if (y >= 0 && z < 0) {
            if (x < 0 && dot(p-v2, v3-v2) > 0) {
                z = fmin(1.0, dot(p-v2, v3-v2) / dot(v3-v2, v3-v2));
                x = 0.0;
            } else {
                z = 0.0;
                x = clamp(dot(p-v2, v1-v2) / dot(v1-v2, v1-v2), 0.0, 1.0);
            }

            y = 1.0 - z - x;
        } else if (z >= 0 && x < 0) {
            if (y < 0 && dot(p-v3, v1-v3) > 0) {
                x = fmin(1.0, dot(p-v3, v1-v3) / dot(v1-v3, v1-v3));
                y = 0.0;
            } else {
                x = 0.0;
                y = clamp(dot(p-v3, v2-v3) / dot(v2-v3, v2-v3), 0.0, 1.0);
            }

            z = 1.0 - x - y;
        }

        return {x, y, z};
    }

    CCPoint cartesianCoords(double x, double y, double z) {
        return ccp(
            x*v1.x + y*v2.x + z*v3.x,
            x*v1.y + y*v2.y + z*v3.y
        );
    }

    bool touchesTriangle(CCPoint position) {
        auto [x, y, z] = barycentricCoords(position);

        return 0.0 <= x && x <= 1.0
            && 0.0 <= y && y <= 1.0
            && 0.0 <= z && z <= 1.0;
    }

    void updateHue(CCPoint position) {
        double angle = atan2(position.y, position.x);

        m_hue = fmod(0.5 + angle / (2.0 * PI), 1.0);

        double radius = m_radius * (1.0+TRIANGLE_SIZE) / 2.0;

        m_hueNipple->setPosition(ccp(radius*cos(angle), radius*sin(angle)));
        this->updateValues();
    }

    void updateSV(CCPoint position) {
        auto [bx, by, bz] = barycentricCoords(position);
        auto [cx, cy, cz] = closestPointInTriangle(bx, by, bz);
        auto pos = cartesianCoords(cx, cy, cz);

        double width = widthAt(pos.y);
        m_saturation = width == 0.0 ? 0.0 : 0.5 + pos.x/width;
        m_value = (pos.y - v1.y) / (v2.y - v1.y);
        
        m_svNipple->setPosition(pos * ccp(TRIANGLE_SIZE * m_radius, TRIANGLE_SIZE * m_radius));
        this->updateValues();
    }

    bool ccTouchBegan(CCTouch *touch, CCEvent *event) override {
        auto position = this->getTouchLocation(touch);
        
        if (this->touchesTriangle(position)) {
            this->updateSV(position);
            this->m_touching = true;
            this->m_touchedHue = false;
            return true;
        } else if (this->touchesHueWheel(position)) {
            this->updateHue(position);
            this->m_touching = true;
            this->m_touchedHue = true;
            return true;
        }

        return false;
    }

    void ccTouchMoved(CCTouch *touch, CCEvent *event) override {
        CCPoint position = this->getTouchLocation(touch);
        
        if (this->m_touchedHue) {
            this->updateHue(position);
        } else {
            this->updateSV(position);
        }
    }

    void ccTouchEnded(CCTouch *touch, CCEvent *event) override {
        this->m_touching = false;
    }

    void registerWithTouchDispatcher() override {
        cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(this, -510, true);
    }
};

CCPoint BetterColorPicker::v1 = ccp(0.0, 1.0);
CCPoint BetterColorPicker::v2 = ccp(-sqrt(3.0)/2.0, -0.5);
CCPoint BetterColorPicker::v3 = ccp(+sqrt(3.0)/2.0, -0.5);

class $modify(MyColorSelectPopup, ColorSelectPopup) {
    struct Fields {
        BetterColorPicker* picker;
    };

	bool init(EffectGameObject* effect, cocos2d::CCArray* array, ColorAction* action) {
        if (!ColorSelectPopup::init(effect, array, action)) return false;

        m_colorPicker->setPosition(ccp(0, 10000));

        m_fields->picker = BetterColorPicker::create([this](ccColor3B color) {
            m_colorPicker->setColorValue(color);
        });
        m_fields->picker->setPosition(ccp(284.f, 196.f) - m_buttonMenu->getPosition());
        m_fields->picker->setRgbValue(m_colorPicker->getColorValue());

        m_buttonMenu->addChild(m_fields->picker);

        return true;
	}

    // ColorSelectPopup::getColorValue and ColorSelectPopup::selectColor are inlined on Windows :/

    void onDefault(CCObject* sender) {
        ColorSelectPopup::onDefault(sender);
        m_fields->picker->setRgbValue(m_colorPicker->getColorValue());
    }

    void onPaste(CCObject* sender) {
        ColorSelectPopup::onPaste(sender);
        m_fields->picker->setRgbValue(m_colorPicker->getColorValue());
    }

    void textChanged(CCTextInputNode* input) {
        ColorSelectPopup::textChanged(input);
        return;
        
        if (!m_fields->picker || m_fields->picker->m_touching) return;

        std::string hex = input->getString();
        
        if (hex.size() == 6) {
            m_fields->picker->setRgbValue(ColorSelectPopup::hexToColor(hex));
        }
    }

    void onToggleHSVMode(CCObject* sender) {
        ColorSelectPopup::onToggleHSVMode(sender);

        if (!static_cast<CCMenuItemToggler*>(sender)->isOn()) {
            m_fields->picker->setPosition(ccp(284.f, 10196.f) - m_buttonMenu->getPosition());
        } else {
            m_fields->picker->setPosition(ccp(284.f, 196.f) - m_buttonMenu->getPosition());
        }
    }
};

class $modify(MySetupPulsePopup, SetupPulsePopup) {
    struct Fields {
        CCControlColourPicker* vanillaPicker;
        BetterColorPicker* picker;
    };

    //TODO
};

$on_mod(Loaded) {
	std::string frag = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;
uniform vec3 currentHsv;

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
    float triangleSize = .85;

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
            gl_FragColor = vec4(0.0);
        }

    } else if (r < 1.0) {
        float angle = -0.5 - atan(uv.y, uv.x) / 2.0 / 3.141592;
        vec3 col = hsv2rgb(vec3(angle, 1.0, 1.0));
        gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
    } else {
        gl_FragColor = vec4(0.0);
    }

}
	)";

	ShaderCache::get()->createShader("colorPicker", frag);
};
