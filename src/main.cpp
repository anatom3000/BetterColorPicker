#include "BetterColorPicker.h"

using namespace geode::prelude;

#include <Geode/modify/GameManager.hpp>

#include "ShaderCache.h"

const double PI = 3.1415926535897932384626433;
const double TRIANGLE_SIZE = .80;
const double OUTLINE_WIDTH = .04;

BetterColorPicker* BetterColorPicker::create(ColorChangedCallback callback) {
    auto* node = new BetterColorPicker();
    if (node->init(callback)) {
        node->autorelease();
    } else {
        delete node;
        node = nullptr;
    }

    return node;
}

bool BetterColorPicker::init(ColorChangedCallback callback) {
    if (!CCControl::init()) return false;
    this->setTouchEnabled(true);
    this->registerWithTouchDispatcher();

    m_colorChangedCallback = callback;

    m_sprite = CCSprite::create("frame.png"_spr);

    m_sprite->setCascadeOpacityEnabled(true);
    m_radius = 0.5 * m_sprite->getContentSize().width;

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
    this->setAnchorPoint({ 0.f, 0.f });

    return true;
}

void BetterColorPicker::updateValues(bool call) {
    m_shader->use();
    glUniform3f(m_location, m_hue, m_saturation, m_value);

    if (call) this->m_colorChangedCallback(this->getRgbValue());
}

ccColor3B BetterColorPicker::getRgbValue() {
    auto rgb = CCControlUtils::RGBfromHSV({.h = m_hue*360.0, .s = m_saturation, .v = m_value});

    return ccc3(
        (GLubyte)255.0*rgb.r,
        (GLubyte)255.0*rgb.g,
        (GLubyte)255.0*rgb.b
    );
}

void BetterColorPicker::setRgbValue(ccColor3B color, bool call) {
    if (color.r == 0 && color.g == 0 && color.b == 0) {
        // hsvFromRgb does weird stuff when converting pure black
        m_hue = 0.0;
        m_saturation = 0.0;
        m_value = 0.0;
    } else {
        auto hsv = CCControlUtils::HSVfromRGB({
            .r = color.r / 255.0f,
            .g = color.g / 255.0f,
            .b = color.b / 255.0f
        });

        m_hue = hsv.h / 360.0f;
        m_saturation = hsv.s;
        m_value = hsv.v;
    }


    double radius = m_radius * (1.0-OUTLINE_WIDTH+TRIANGLE_SIZE) / 2.0;
    double angle = (m_hue - 0.5) * 2 * PI;
    double y = v1.y + m_value * (v2.y - v1.y);
    double width = widthAt(y);
    double x = width == 0 ? 0.0 : (m_saturation - 0.5) * width ;

    m_hueNipple->setPosition(ccp(radius*cos(angle), radius*sin(angle)));
    m_svNipple->setPosition(ccp(m_radius * TRIANGLE_SIZE * x, m_radius * TRIANGLE_SIZE * y));
    this->updateValues(call);
}

double BetterColorPicker::widthAt(double y) {
    double a = (v2.x - v3.x) / (v2.y - v1.y);
    double b = -a*v1.y;

    return (a*y + b);
}

bool BetterColorPicker::touchesHueWheel(CCPoint position) {
    double radius = sqrt(position.x*position.x + position.y*position.y) / m_radius;
    
    return (TRIANGLE_SIZE <= radius && radius <= 1.0);
}

std::tuple<double, double, double> BetterColorPicker::barycentricCoords(CCPoint pos) {
    pos /= m_radius;
    pos /= TRIANGLE_SIZE;
    
    double l1 = ((v2.y - v3.y)*(pos.x - v3.x) + (v3.x - v2.x)*(pos.y - v3.y))
        / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

    double l2 = ((v3.y - v1.y)*(pos.x - v3.x) + (v1.x - v3.x) * (pos.y - v3.y))
        / ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));

    double l3 = 1.0 - l1 - l2;
    
    return {l1, l2, l3};
}

double BetterColorPicker::dot(CCPoint u, CCPoint v) {
    return u.x*v.x + u.y*v.y;
}

// https://math.stackexchange.com/questions/1092912/find-closest-point-in-triangle-given-barycentric-coordinates-outside
std::tuple<double, double, double> BetterColorPicker::closestPointInTriangle(double x, double y, double z) {
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

CCPoint BetterColorPicker::cartesianCoords(double x, double y, double z) {
    return ccp(
        x*v1.x + y*v2.x + z*v3.x,
        x*v1.y + y*v2.y + z*v3.y
    );
}

bool BetterColorPicker::touchesTriangle(CCPoint position) {
    auto [x, y, z] = barycentricCoords(position);

    return 0.0 <= x && x <= 1.0
        && 0.0 <= y && y <= 1.0
        && 0.0 <= z && z <= 1.0;
}

void BetterColorPicker::updateHue(CCPoint position) {
    double angle = atan2(position.y, position.x);

    m_hue = fmod(0.5 + angle / (2.0 * PI), 1.0);

    double radius = m_radius * (1.0-OUTLINE_WIDTH+TRIANGLE_SIZE) / 2.0;

    m_hueNipple->setPosition(ccp(radius*cos(angle), radius*sin(angle)));
    this->updateValues(true);
}

void BetterColorPicker::updateSV(CCPoint position) {
    auto [bx, by, bz] = barycentricCoords(position);
    auto [cx, cy, cz] = closestPointInTriangle(bx, by, bz);
    auto pos = cartesianCoords(cx, cy, cz);

    double width = widthAt(pos.y);
    m_saturation = width == 0.0 ? 0.0 : 0.5 + pos.x/width;
    m_value = (pos.y - v1.y) / (v2.y - v1.y);
    
    m_svNipple->setPosition(pos * ccp(TRIANGLE_SIZE * m_radius, TRIANGLE_SIZE * m_radius));
    this->updateValues(true);
}

bool BetterColorPicker::ccTouchBegan(CCTouch *touch, CCEvent *event) {
    if (!this->isVisible()) return false;
    
    auto position = this->getTouchLocation(touch);
    
    if (this->touchesTriangle(position)) {
        this->m_touching = true;
        this->m_touchedHue = false;
        this->updateSV(position);
        return true;
    } else if (this->touchesHueWheel(position)) {
        this->m_touching = true;
        this->m_touchedHue = true;
        this->updateHue(position);
        return true;
    }

    return false;
}

void BetterColorPicker::ccTouchMoved(CCTouch *touch, CCEvent *event) {
    CCPoint position = this->getTouchLocation(touch);
    
    if (this->m_touchedHue) {
        this->updateHue(position);
    } else {
        this->updateSV(position);
    }
}

void BetterColorPicker::ccTouchEnded(CCTouch *touch, CCEvent *event) {
    this->m_touching = false;
}

void BetterColorPicker::registerWithTouchDispatcher() {
    cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(this, -510, true);
}

// unused, left here to document what didn't work
void disableTouch(CCControlColourPicker* target) {
    return;

    target->setTouchPriority(0);
    target->setTouchEnabled(false);
    cocos2d::CCTouchDispatcher::get()->removeDelegate(target);
    cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(target, 0, false);
    cocos2d::CCTouchDispatcher::get()->unregisterForcePrio(target);
}

void loadPickerShader() {
    log::info("loading picker shader");
#ifdef GEODE_IS_ANDROID
    auto shaderPath = Mod::get()->getResourcesDir() / "picker_android.fsh";
#else
    std::filesystem::path shaderPath = Mod::get()->getResourcesDir() / "picker_main.fsh";
#endif
    log::info("{}", shaderPath);
    // why is c++ so ass
    // i want my std::fs::read_to_string
    std::ifstream file(shaderPath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    auto shaderCode = buffer.str();

    log::info("{}", shaderCode);

	ShaderCache::get()->createShader("colorPicker", shaderCode);
}


class $modify(MyGameManager, GameManager) {
	void reloadAllStep5() {
		GameManager::reloadAllStep5();
		ShaderCache::get()->clearShaders();
		loadPickerShader();
	}
};

$on_mod(Loaded) { loadPickerShader(); };
