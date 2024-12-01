#include <Geode/modify/ColorSelectPopup.hpp>

class $modify(MyColorSelectPopup, ColorSelectPopup) {
    struct Fields {
        BetterColorPicker* picker;

        CCMenuItemToggler* pickerToggle;
        CCLabelBMFont* pickerToggleLabel;

        bool isColorSelectPopup;
    };

	bool init(EffectGameObject* effect, cocos2d::CCArray* array, ColorAction* action) {
        if (!ColorSelectPopup::init(effect, array, action)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto center = winSize / 2.0f;

        m_fields->picker = BetterColorPicker::create([this](ccColor3B color) {
            m_colorPicker->setColorValue(color);
        });
        m_fields->picker->setRgbValue(static_cast<WhyTheFuckIsGetColorValueInlinedOnAndroid*>(m_colorPicker)->getTheFuckingColor(), false);

        m_fields->isColorSelectPopup = true;

        this->addChild(m_fields->picker);
        this->updatePickerPositions();

        m_fields->pickerToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MyColorSelectPopup::onPickerToggle), .7);
        m_fields->pickerToggle->setPosition(110, 144);

        m_fields->pickerToggleLabel = CCLabelBMFont::create("Better Picker", "bigFont.fnt");
        m_fields->pickerToggleLabel->setScale(0.35);
        m_fields->pickerToggleLabel->setAnchorPoint(ccp(0.0, 0.5));
        m_fields->pickerToggleLabel->setPosition(130.25, 144);

        bool on = m_colorPicker->isVisible();

        m_buttonMenu->addChild(m_fields->pickerToggle);
        m_buttonMenu->addChild(m_fields->pickerToggleLabel);

        m_fields->pickerToggle->toggle(Mod::get()->getSettingValue<bool>("enable-picker"));

        m_fields->picker->setVisible(on);
        m_fields->pickerToggle->setVisible(on);
        m_fields->pickerToggle->setEnabled(on);
        m_fields->pickerToggleLabel->setVisible(on);

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

    void onDefault(CCObject* sender) {
        ColorSelectPopup::onDefault(sender);
        m_fields->picker->setRgbValue(static_cast<WhyTheFuckIsGetColorValueInlinedOnAndroid*>(m_colorPicker)->getTheFuckingColor(), false);
    }

    void onPaste(CCObject* sender) {
        ColorSelectPopup::onPaste(sender);
        if (!m_fields->isColorSelectPopup) return; // may be called from MySetupPulsePopup::onPaste

        m_fields->picker->setRgbValue(static_cast<WhyTheFuckIsGetColorValueInlinedOnAndroid*>(m_colorPicker)->getTheFuckingColor(), false);
    }

    void textChanged(CCTextInputNode* input) {
        ColorSelectPopup::textChanged(input);

        if (!m_fields->picker || m_fields->picker->m_touching) return;
        if (input->getTag() != 11) return;

        m_fields->picker->setRgbValue(static_cast<WhyTheFuckIsGetColorValueInlinedOnAndroid*>(m_colorPicker)->getTheFuckingColor(), false);
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
