class BetterHuePicker : public CCControl {
public:
    float m_hue;
    float m_huePercentage;

    CCSprite* m_background;
    CCSprite* m_slider;
    CCPoint m_startPos;

    ~BetterHuePicker() {
        removeAllChildrenWithCleanup(true);
        CC_SAFE_RELEASE(m_background);
        CC_SAFE_RELEASE(m_slider);
    }

    static BetterHuePicker* create(CCPoint pos) {
        BetterHuePicker *pRet = new BetterHuePicker();
        pRet->init(pos);
        pRet->autorelease();
        return pRet;
    }
    bool init(CCPoint pos) {
        if (!CCControl::init()) return false;

        setTouchEnabled(true);
        // Add background and slider sprites
        
        this->m_background = CCSprite::create("huePickerBackground.png"_spr);
        this->m_background->setPosition(pos);
        this->m_background->setAnchorPoint(ccp(.5f, .5f));
        this->addChild(this->m_background);

        this->m_slider = CCSprite::create("colourPicker.png"_spr);
        this->m_slider->setPosition(pos);
        this->m_slider->setAnchorPoint(ccp(.5f, .5f));
        this->addChild(this->m_slider);

        m_slider->setPosition(ccp(pos.x, pos.y + m_background->boundingBox().size.height * 0.5f));
        m_startPos = pos;

        // Sets the default value
        m_hue = 0.0f;
        m_huePercentage = 0.0f;

        return true;
    }

    void setHue(float hueValue) {
        m_hue = hueValue;
        float huePercentage = hueValue / 360.0f;
        setHuePercentage(huePercentage);
    }

    void setHuePercentage(float hueValueInPercent) {
        m_huePercentage = hueValueInPercent;
        m_hue = m_huePercentage * 360.0f;

        // Clamp the position of the icon within the circle
        CCRect backgroundBox = this->m_background->boundingBox();

        // Get the center point of the background image
        float centerX = m_startPos.x + backgroundBox.size.width * 0.5f;
        float centerY = m_startPos.y + backgroundBox.size.height * 0.5f;

        // Work out the limit to the distance of the picker when moving around the hue
        // bar
        float limit = backgroundBox.size.width * 0.5f - 15.0f;

        // Update angle
        float angleDeg = m_huePercentage * 360.0f - 180.0f;
        float angle = CC_DEGREES_TO_RADIANS(angleDeg);

        // Set new position of the slider
        float x = centerX + limit * cosf(angle);
        float y = centerY + limit * sinf(angle);
        m_slider->setPosition(ccp(x, y));
    }

    void setEnabled(bool enabled) {
        CCControl::setEnabled(enabled);
        if (m_slider != NULL) {
            m_slider->setOpacity(enabled ? 255 : 128);
        }
    }

    void updateSliderPosition(CCPoint location) {
        // Clamp the position of the icon within the circle
        CCRect backgroundBox = m_background->boundingBox();

        // Get the center point of the background image
        float centerX = m_startPos.x + backgroundBox.size.width * 0.5f;
        float centerY = m_startPos.y + backgroundBox.size.height * 0.5f;

        // Work out the distance difference between the location and center
        float dx = location.x - centerX;
        float dy = location.y - centerY;

        // Update angle by using the direction of the location
        float angle = atan2f(dy, dx);
        float angleDeg = CC_RADIANS_TO_DEGREES(angle) + 180.0f;

        // use the position / slider width to determin the percentage the dragger is
        // at
        setHue(angleDeg);

        // send CCControl callback
        sendActionsForControlEvents(CCControlEventValueChanged);
    }

    bool checkSliderPosition(CCPoint location) {
        // compute the distance between the current location and the center
        double distance = sqrt(pow(location.x + 10, 2) + pow(location.y, 2));

        // check that the touch location is within the circle
        if (80 > distance && distance > 59) {
            updateSliderPosition(location);
            return true;
        } else {
            return false;
        }
    }

    bool ccTouchBegan(CCTouch *touch, CCEvent *event) {
        if (!isEnabled() || !isVisible()) {
            return false;
        }

        // Get the touch location
        CCPoint touchLocation = getTouchLocation(touch);

        // Check the touch position on the slider
        return checkSliderPosition(touchLocation);
    }

    void ccTouchMoved(CCTouch *touch, CCEvent *event) {
        // Get the touch location
        CCPoint touchLocation = getTouchLocation(touch);

        // small modification: this allows changing of the colour, even if the touch
        // leaves the bounding area
        //     updateSliderPosition(touchLocation);
        //     sendActionsForControlEvents(CCControlEventValueChanged);
        // Check the touch position on the slider
        checkSliderPosition(touchLocation);
    }
};
