#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/ColorSelectPopup.hpp>

#include "CCSpriteBatchNode.h"
#include "ShaderCache.h"

const double PI = 3.1415926535897932384626433;
const double TRIANGLE_SIZE = .85;

class BetterColorPicker : public CCControl {
public:
    float m_radius;
    CCPoint m_center;
    bool m_touchedHue;

    Ref<CCSprite> m_hueNipple;
    Ref<CCSprite> m_svNipple;

    // between 0.0 and 1.0
    double m_hue;
    double m_saturation;
    double m_value;

    static CCPoint v1;
    static CCPoint v2;
    static CCPoint v3;

    static auto* create() {
        auto* node = new BetterColorPicker();
        if (node->init()) {
            node->autorelease();
        } else {
            delete node;
            node = nullptr;
        }

        return node;
    }

    bool init() override {
        if (!CCControl::init()) return false;
        this->setTouchEnabled(true);
        this->registerWithTouchDispatcher();
        cocos2d::CCTouchDispatcher::get()->registerForcePrio(this, 2);

        auto sprite = CCSprite::create("picker.png"_spr);
        sprite->setCascadeOpacityEnabled(true);
        m_radius = 0.5 * sprite->getContentSize().width;

        // TODO: AA
        // The Alias/Antialias property belongs to CCSpriteBatchNode, so you can’t individually set the aliased property.
        CCGLProgram* shader = ShaderCache::get()->getProgram("colorPicker");
        shader->setUniformsForBuiltins();
        sprite->setShaderProgram(shader);
        shader->use();
        sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        this->addChild(sprite);

        this->m_hueNipple = CCSprite::create("colourPicker.png"_spr);
        this->m_svNipple = CCSprite::create("colourPicker.png"_spr);
        
        this->addChild(m_hueNipple);
        this->addChild(m_svNipple);

        this->setContentSize(sprite->getContentSize());

        this->updateHue(ccp(-1, 0));
        this->updateSV(ccp(m_radius*v3.x, m_radius*v3.y));

        return true;
    }

    bool touchesHueWheel(CCPoint position) {
        double radius = sqrt(position.x*position.x + position.y*position.y) / m_radius;
        
        return (TRIANGLE_SIZE <= radius && radius <= 1.0);
    }

    std::tuple<double, double, double> barycentricCoords(CCPoint pos) {
        pos /= m_radius;
        
        double l1 = ((v2.y - v3.y)*(pos.x - v3.x) + (v3.x - v2.x)*(pos.y - v3.y))
            / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

        double l2 = ((v3.y - v1.y)*(pos.x - v3.x) + (v1.x - v3.x) * (pos.y - v3.y))
            / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

        double l3 = 1.0 - l1 - l2;
        
        return {l1, l2, l3};
    }

    CCPoint cartesianCoords(double x, double y, double z) {
        return ccp(
            m_radius * (x*v1.x + y*v2.x + z*v3.x),
            m_radius * (x*v1.y + y*v2.y + z*v3.y)
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

        m_hue = angle / (2.0 * PI);

        double radius = m_radius * (1.0+TRIANGLE_SIZE) / 2.0;

        m_hueNipple->setPosition(ccp(radius*cos(angle), radius*sin(angle)));
    }

    void updateSV(CCPoint position) {
        auto [x, y, z] = barycentricCoords(position);
        x = clamp(x, 0.0, 1.0);
        y = clamp(y, 0.0, 1.0);
        z = clamp(z, 0.0, 1.0);
        
        m_saturation = y;
        m_value = 1.0-x;

        m_svNipple->setPosition(cartesianCoords(x, y, z));
    }

    bool ccTouchBegan(CCTouch *touch, CCEvent *event) override {
        auto position = this->getTouchLocation(touch);
        
        log::debug("ccTouchBegan: {}, {}", position.x, position.y);
        m_hueNipple->setPosition(position);

        if (this->touchesHueWheel(position)) {
            this->updateHue(position);
            this->m_touchedHue = true;
            return true;
        } else if (this->touchesTriangle(position)) {
            this->updateSV(position);
            this->m_touchedHue = false;
            return true;
        } else {
            return false;
        }
    }

    void ccTouchMoved(CCTouch *touch, CCEvent *event) override {
        CCPoint position = this->getTouchLocation(touch);
        log::debug("ccTouchMoved: {}, {}", position.x, position.y);
        
        if (this->m_touchedHue) {
            this->updateHue(position);
        } else {
            this->updateSV(position);
        }
    }

    void registerWithTouchDispatcher() override {
        cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
    }
};

CCPoint BetterColorPicker::v1 = ccp(0.0, 1.0);
CCPoint BetterColorPicker::v2 = ccp(-sqrt(3.0)/2.0, -0.5);
CCPoint BetterColorPicker::v3 = ccp(-sqrt(3.0)/2.0, +0.5);

class $modify(MyColorSelectPopup, ColorSelectPopup) {
    struct Fields {
        BetterColorPicker* picker;
    };

	bool init(EffectGameObject* effect, cocos2d::CCArray* array, ColorAction* action) {
        if (!ColorSelectPopup::init(effect, array, action)) return false;

        // TODO: add node IDs for ColorSelectPopup
        m_colorPicker->setPosition(ccp(0, 1000));

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Click me!"),
            this,
            nullptr
        );
        btn->setPosition(200.f - m_buttonMenu->getPosition().x, 200.f - m_buttonMenu->getPosition().y);
        m_buttonMenu->addChild(btn);

        m_fields->picker = BetterColorPicker::create();
        m_fields->picker->setPosition(ccp(284.f, 180.f) - m_buttonMenu->getPosition());

        //menu->setPosition(ccp(284.f, 180.f));
        m_buttonMenu->addChild(m_fields->picker);
        //

        auto p = CCControlColourPicker::colourPicker();
        p->setPosition(ccp(284.f, 180.f) - m_buttonMenu->getPosition());
        //m_buttonMenu->addChild(p);

        return true;
	}
};

$on_mod(Loaded) {
	std::string frag = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;

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

void main() {
    vec2 uv = 2.0*v_texCoord - vec2(1.0, 1.0);
    float r = sqrt(uv.x*uv.x + uv.y*uv.y);
    float someAngle = 0.7;
    float r3over2 = sqrt(3.0) * .5;
    float triangleSize = .85;

    if (r < triangleSize) {
        vec3 bary = barycentricCoords(
            uv,
            triangleSize * vec2(0.0, -1.0),
            triangleSize * vec2(-r3over2, 0.5),
            triangleSize * vec2(r3over2, 0.5)
        );
        if (0.0 < bary.x && bary.x <= 1.0 
         && 0.0 < bary.y && bary.y <= 1.0 
         && 0.0 < bary.z && bary.z <= 1.0) {
            vec3 col = hsv2rgb(vec3(someAngle, bary.y, 1.0-bary.x));
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
