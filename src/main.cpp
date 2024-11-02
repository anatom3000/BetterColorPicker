#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/ColorSelectPopup.hpp>

#include "BetterColorPicker.cpp"

class $modify(MyColorSelectPopup, ColorSelectPopup) {
    struct Fields {
        BetterColorPicker* picker;
    };

	bool init(EffectGameObject* effect, cocos2d::CCArray* array, ColorAction* action) {
        if (!ColorSelectPopup::init(effect, array, action)) return false;

        // if you see this, please find a better way to do this
        m_colorPicker->setPosition(ccp(0, 1000000000000000000));

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Click me!"),
            this,
            nullptr
        );
        btn->setPosition(200.f, 200.f);
        this->addChild(btn);

        m_fields->picker = BetterColorPicker::create();
        m_fields->picker->setPosition(ccp(329, 180));
        //m_fields->picker->setPosition(ccp(200, 200));
        m_fields->picker->setAnchorPoint(ccp(.5f, .5f));
        this->addChild(m_fields->picker);

        return true;
	}
};
