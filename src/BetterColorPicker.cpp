#include "BetterSatBriPicker.cpp"
#include "BetterHuePicker.cpp"

class BetterColorPicker : public CCControl {
public:
    HSV m_hsv;
    BetterSatBriPicker* m_colourPicker;
    BetterHuePicker* m_huePicker;
    CCSprite* m_background;

    ~BetterColorPicker() {
        if (m_background) {
            m_background->removeFromParentAndCleanup(true);
        }

        if (m_huePicker) {
            m_huePicker->removeFromParentAndCleanup(true);
        }

        if (m_colourPicker) {
            m_colourPicker->removeFromParentAndCleanup(true);
        }

        m_background = NULL;
        m_huePicker = NULL;
        m_colourPicker = NULL;
    }

    bool init() {
        if (!CCControl::init()) return false;

        setTouchEnabled(true);

        // Init default color
        m_hsv.h = 0;
        m_hsv.s = 0;
        m_hsv.v = 0;

        m_background = CCSprite::create("huePickerBackground.png"_spr);
        m_background->setPosition(ccp(0, 0));
        m_background->setAnchorPoint(ccp(.5f, .5f));
        this->addChild(m_background);
        CC_SAFE_RETAIN(m_background);

        CCPoint backgroundPointZero =
            m_background->getPosition() - ccp(
                m_background->getContentSize().width / 2,
                m_background->getContentSize().height / 2
            );

        // Setup panels
        float hueShift = 8;
        float colourShift = 28;

        m_huePicker = new BetterHuePicker();
        m_huePicker->init(ccp(backgroundPointZero.x + hueShift, backgroundPointZero.y + hueShift));
        m_colourPicker = new BetterSatBriPicker();
        m_colourPicker->init(ccp(backgroundPointZero.x + colourShift, backgroundPointZero.y + colourShift));

        // Setup events
        m_huePicker   ->addTargetWithActionForControlEvents(this, cccontrol_selector(CCControlColourPicker::hueSliderValueChanged),    CCControlEventValueChanged);
        m_colourPicker->addTargetWithActionForControlEvents(this, cccontrol_selector(CCControlColourPicker::colourSliderValueChanged), CCControlEventValueChanged);

        this->addChild(m_huePicker);
        this->addChild(m_colourPicker);

        // Set defaults
        this->updateHueAndControlPicker();

        // Set content size
        this->setContentSize(m_background->getContentSize());

        return true;
    }

    static BetterColorPicker* create() {
        BetterColorPicker *pRet = new BetterColorPicker();
        pRet->init();
        pRet->autorelease();
        return pRet;
    }

    void setColor(const ccColor3B &color) {
        // XXX fixed me if not correct
        CCControl::setColor(color);

        RGBA rgba;
        rgba.r = color.r / 255.0f;
        rgba.g = color.g / 255.0f;
        rgba.b = color.b / 255.0f;
        rgba.a = 1.0f;

        m_hsv = CCControlUtils::HSVfromRGB(rgba);
        updateHueAndControlPicker();
    }

    void setEnabled(bool enabled) {
        CCControl::setEnabled(enabled);
        if (m_huePicker != NULL) {
            m_huePicker->setEnabled(enabled);
        }
        if (m_colourPicker) {
            m_colourPicker->setEnabled(enabled);
        }
    }

    // need two events to prevent an infinite loop! (can't update huePicker when the
    // huePicker triggers the callback due to CCControlEventValueChanged)
    void updateControlPicker() {
        m_huePicker->setHue(m_hsv.h);
        m_colourPicker->updateWithHSV(m_hsv);
    }

    void updateHueAndControlPicker() {
        m_huePicker->setHue(m_hsv.h);
        m_colourPicker->updateWithHSV(m_hsv);
        m_colourPicker->updateDraggerWithHSV(m_hsv);
    }

    void hueSliderValueChanged(CCObject *sender, CCControlEvent controlEvent) {

        m_hsv.h = ((BetterHuePicker*)sender)->m_hue;

        // Update the value
        RGBA rgb = CCControlUtils::RGBfromHSV(m_hsv);
        CCControl::setColor(ccc3(
            (GLubyte)(rgb.r * 255.0f), 
            (GLubyte)(rgb.g * 255.0f),
            (GLubyte)(rgb.b * 255.0f)
        ));

        // Send CCControl callback
        sendActionsForControlEvents(CCControlEventValueChanged);
        updateControlPicker();
    }

    void colourSliderValueChanged(CCObject *sender, CCControlEvent controlEvent) {
        m_hsv.s = ((BetterSatBriPicker*)sender)->m_saturation;
        m_hsv.v = ((BetterSatBriPicker*)sender)->m_brightness;

        // Update the value
        RGBA rgb = CCControlUtils::RGBfromHSV(m_hsv);
        // XXX fixed me if not correct
        CCControl::setColor(ccc3(
            (GLubyte)(rgb.r * 255.0f),
            (GLubyte)(rgb.g * 255.0f),
            (GLubyte)(rgb.b * 255.0f)
        ));

        // Send CCControl callback
        sendActionsForControlEvents(CCControlEventValueChanged);
    }

    bool ccTouchBegan(CCTouch *touch, CCEvent *pEvent) {
        return false;
    }
};

