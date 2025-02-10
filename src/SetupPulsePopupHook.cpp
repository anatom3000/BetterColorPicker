#include "BetterColorPicker.h"
#include <Geode/modify/SetupPulsePopup.hpp>

using namespace geode::prelude;

class $modify(MySetupPulsePopup, SetupPulsePopup) {
    struct Fields {
        BetterColorPicker* picker;

        CCMenuItemToggler* pickerToggle;
        CCLabelBMFont* pickerToggleLabel;

        bool hsvEnabled;
    };

	bool init(EffectGameObject* effect, cocos2d::CCArray* objects) {
        if (!SetupPulsePopup::init(effect, objects)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto center = winSize / 2.0f;

        m_fields->picker = BetterColorPicker::create([this](ccColor3B color) {
            m_colorPicker->setColorValue(color);
        });

        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);
        m_fields->picker->setScale(0.8f);

        this->addChild(m_fields->picker);
        this->updatePickerPositions();

        m_fields->pickerToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MySetupPulsePopup::onPickerToggle), .7);
        m_fields->pickerToggle->setPosition(95, 171);

        m_fields->pickerToggleLabel = CCLabelBMFont::create("Better Picker", "bigFont.fnt");
        m_fields->pickerToggleLabel->setScale(0.35);
        m_fields->pickerToggleLabel->setAnchorPoint(ccp(0.0, 0.5));
        m_fields->pickerToggleLabel->setPosition(115.25, 171);

        bool on = m_colorPicker->isVisible();

        m_buttonMenu->addChild(m_fields->pickerToggle);
        m_buttonMenu->addChild(m_fields->pickerToggleLabel);

        m_fields->pickerToggle->toggle(Mod::get()->getSettingValue<bool>("enable-picker"));

        m_fields->picker->setVisible(on);
        m_fields->pickerToggle->setVisible(on);
        m_fields->pickerToggle->setEnabled(on);
        m_fields->pickerToggleLabel->setVisible(on);

        auto pasteButton = static_cast<CCMenuItemSpriteExtra*>(
            static_cast<CCNode*>(
            static_cast<CCNode*>(
                this->getChildren()->objectAtIndex(0)
            )->getChildren()->objectAtIndex(1)
            )->getChildren()->objectAtIndex(16)
        );
        
        pasteButton->setTarget(this, menu_selector(MySetupPulsePopup::onBetterPaste));

        return true;
    }

    void onPickerToggle(CCObject* target) {
        auto toggler = static_cast<CCMenuItemToggler*>(target);
        Mod::get()->setSettingValue<bool>("enable-picker", !toggler->isToggled());
        this->updatePickerPositions();
    }

	void onSelectPulseMode(cocos2d::CCObject* sender) {
        SetupPulsePopup::onSelectPulseMode(sender);

        if (!sender || !m_fields->picker) return;

        m_fields->hsvEnabled = sender->getTag();
        m_fields->picker->setVisible(!m_fields->hsvEnabled);
        m_fields->pickerToggle->setVisible(!m_fields->hsvEnabled);
        m_fields->pickerToggle->setEnabled(!m_fields->hsvEnabled);
        m_fields->pickerToggleLabel->setVisible(!m_fields->hsvEnabled);
    }

    void updatePickerPositions() {
        bool enable = Mod::get()->getSettingValue<bool>("enable-picker");

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto center = winSize / 2.0f;
        
        if (enable) {
            // i could not find a way to prevent the vanilla picker to pick up touch events
            m_colorPicker->setPosition(ccp(100000, 0));
            m_fields->picker->setPosition(center + ccp(-8, 52));
        } else {
            m_fields->picker->setPosition(ccp(100000, 0));
            m_colorPicker->setPosition(center + ccp(-25, 35));
        }
    }
    
    void onBetterPaste(CCObject* sender) {
        SetupPulsePopup::onPaste(sender);

        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);
    }

    void textChanged(CCTextInputNode* input) {
        SetupPulsePopup::textChanged(input);

        if (!m_fields->picker || m_fields->picker->m_touching) return;
        if (input->getTag() != 14) return;

        m_fields->picker->setRgbValue(m_colorPicker->m_rgb, false);
    }
};
