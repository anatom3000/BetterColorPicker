#include "BetterColorPicker.h"
#include <Geode/modify/ColorSelectPopup.hpp>

using namespace geode::prelude;

class $modify(MyColorSelectPopup, ColorSelectPopup) {
    struct Fields {
        BetterColorPicker* picker;

        CCMenuItemToggler* pickerToggle;
        CCLabelBMFont* pickerToggleLabel;
    };

	bool init(EffectGameObject* effect, cocos2d::CCArray* array, ColorAction* action) {
        if (!ColorSelectPopup::init(effect, array, action)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto center = winSize / 2.0f;

        m_fields->picker = BetterColorPicker::create([this](ccColor3B color) {
            m_colorPicker->setColorValue(color);
        });
        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);

        this->addChild(m_fields->picker);
        this->updatePickerPositions();

        m_fields->pickerToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MyColorSelectPopup::onPickerToggle), .7);
        m_fields->pickerToggle->setPosition(110, 144);

        m_fields->pickerToggleLabel = CCLabelBMFont::create("Better Picker", "bigFont.fnt");
        m_fields->pickerToggleLabel->setScale(0.35);
        m_fields->pickerToggleLabel->setAnchorPoint(ccp(0.0, 0.5));
        m_fields->pickerToggleLabel->setPosition(130.25, 144);

        //auto palette = CCSprite::createWithSpriteFrameName("GJ_paintBtn_001.png");
        //palette->setPosition(ccp(145.0, 144.0));
        //palette->setScale(0.6);

        m_buttonMenu->addChild(m_fields->pickerToggle);
        m_buttonMenu->addChild(m_fields->pickerToggleLabel);
        //m_buttonMenu->addChild(palette);

        m_fields->pickerToggle->toggle(Mod::get()->getSettingValue<bool>("enable-picker"));

        bool on = m_colorPicker->isVisible();
        m_fields->picker->setVisible(on);
        m_fields->pickerToggle->setVisible(on);
        m_fields->pickerToggle->setEnabled(on);
        m_fields->pickerToggleLabel->setVisible(on);

        auto pasteButton = static_cast<CCMenuItemSpriteExtra*>(
            static_cast<CCNode*>(
            static_cast<CCNode*>(
                this->getChildren()->objectAtIndex(0)
            )->getChildren()->objectAtIndex(1) // 20-21
            )->getChildren()->objectAtIndex(2)
        );

        pasteButton->setTarget(this, menu_selector(MyColorSelectPopup::onBetterPaste));

        auto defaultButton = static_cast<CCMenuItemSpriteExtra*>(
            static_cast<CCNode*>(
            static_cast<CCNode*>(
                this->getChildren()->objectAtIndex(0)
            )->getChildren()->objectAtIndex(1)
            )->getChildren()->objectAtIndex(3)
        );

        defaultButton->setTarget(this, menu_selector(MyColorSelectPopup::onBetterDefault));

        return true;
	}

    void onPickerToggle(CCObject* target) {
        auto toggler = static_cast<CCMenuItemToggler*>(target);
        Mod::get()->setSettingValue<bool>("enable-picker", !toggler->isToggled());
        this->updatePickerPositions();
    }

    void updatePickerPositions() {
        bool enable = Mod::get()->getSettingValue<bool>("enable-picker");

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto center = winSize / 2.0f;
        
        if (enable) {
            // i could not find a way to prevent the vanilla picker to pick up touch events
            m_colorPicker->setPosition(ccp(100000, 0));
            m_fields->picker->setPosition(center + ccp(0, 36));
        } else {
            m_fields->picker->setPosition(ccp(100000, 0));
            m_colorPicker->setPosition(center + ccp(0, 36));
        }
    }

    void onBetterDefault(CCObject* sender) {
        ColorSelectPopup::onDefault(sender);
        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);
    }

    void onBetterPaste(CCObject* sender) {
        ColorSelectPopup::onPaste(sender);
        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);
    }

    void textChanged(CCTextInputNode* input) {
        ColorSelectPopup::textChanged(input);

        if (!m_fields->picker || m_fields->picker->m_touching) return;
        if (input->getTag() != 11) return;

        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);
    }

    void onToggleHSVMode(CCObject* sender) {
        ColorSelectPopup::onToggleHSVMode(sender);

        bool on = static_cast<CCMenuItemToggler*>(sender)->isOn();

        m_fields->picker->setVisible(on);
        m_fields->pickerToggle->setVisible(on);
        m_fields->pickerToggle->setEnabled(on);
        m_fields->pickerToggleLabel->setVisible(on);

    }
};
