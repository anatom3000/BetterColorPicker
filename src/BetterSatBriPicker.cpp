class BetterSatBriPicker : public CCControl
{
public:
    /** Contains the receiver¡¯s current saturation value. */
    float m_saturation;
    /** Contains the receiver¡¯s current brightness value. */
    float m_brightness;

    //not sure if these need to be there actually. I suppose someone might want to access the sprite?
    CCSprite* m_background;
    CCSprite* m_overlay;
    CCSprite* m_shadow;
    CCSprite* m_slider;
    CCPoint m_startPos;

    int boxPos;
    int boxSize;
    
    ~BetterSatBriPicker() {
        removeAllChildrenWithCleanup(true);

        m_background = NULL;
        m_overlay = NULL;
        m_shadow = NULL;
        m_slider = NULL;
    }
    bool init(CCPoint pos) {
        if (!CCControl::init()) return false;
        setTouchEnabled(true);

        // Add background and slider sprites
        m_background = CCSprite::create("colourPickerBackground.png"_spr);
        m_background->setPosition(pos);
        m_background->setAnchorPoint(ccp(0.0f, 0.0f));
        this->addChild(m_background);

        m_overlay = CCSprite::create("colourPickerOverlay.png"_spr);
        m_overlay->setPosition(pos);
        m_overlay->setAnchorPoint(ccp(0.0f, 0.0f));
        this->addChild(m_overlay);

        m_shadow = CCSprite::create("colourPickerShadow.png"_spr);
        m_shadow->setPosition(pos);
        m_shadow->setAnchorPoint(ccp(0.0f, 0.0f));
        this->addChild(m_shadow);

        m_slider = CCSprite::create("colourPicker.png"_spr);
        m_slider->setPosition(pos);
        m_slider->setAnchorPoint(ccp(0.5f, 0.5f));
        this->addChild(m_shadow);

        m_startPos = pos; // starting position of the colour picker
        boxPos = 35; // starting position of the virtual box area for picking a colour
        boxSize = m_background->getContentSize().width / 2;
        // the size (width and height) of the virtual box for picking a colour
        // from
        return true;
    }

    static BetterSatBriPicker* create(CCPoint pos) {
        BetterSatBriPicker *pRet =
            new BetterSatBriPicker();
        pRet->init(pos);
        pRet->autorelease();
        return pRet;
    }

    void updateWithHSV(HSV hsv) {
        HSV hsvTemp;
        hsvTemp.s = 1;
        hsvTemp.h = hsv.h;
        hsvTemp.v = 1;

        RGBA rgb = CCControlUtils::RGBfromHSV(hsvTemp);
        m_background->setColor(ccc3((GLubyte)(rgb.r * 255.0f),
                                  (GLubyte)(rgb.g * 255.0f),
                                  (GLubyte)(rgb.b * 255.0f)));
    }

    void updateDraggerWithHSV(HSV hsv) {
        // Set the position of the slider to the correct saturation and brightness
        CCPoint pos = CCPointMake(m_startPos.x + boxPos + (boxSize * (1 - hsv.s)),
                                m_startPos.y + boxPos + (boxSize * hsv.v));

        // update
        updateSliderPosition(pos);
    }

    void updateSliderPosition(CCPoint sliderPosition) {
        // Clamp the position of the icon within the circle

        // Get the center point of the bkgd image
        float centerX = m_startPos.x + m_background->boundingBox().size.width * 0.5f;
        float centerY = m_startPos.y + m_background->boundingBox().size.height * 0.5f;

        // Work out the distance difference between the location and center
        float dx = sliderPosition.x - centerX;
        float dy = sliderPosition.y - centerY;
        float dist = sqrtf(dx * dx + dy * dy);

        // Update angle by using the direction of the location
        float angle = atan2f(dy, dx);

        // Set the limit to the slider movement within the colour picker
        float limit = m_background->boundingBox().size.width * 0.5f;

        // Check distance doesn't exceed the bounds of the circle
        if (dist > limit) {
        sliderPosition.x = centerX + limit * cosf(angle);
        sliderPosition.y = centerY + limit * sinf(angle);
        }

        // Set the position of the dragger
        m_slider->setPosition(sliderPosition);

        // Clamp the position within the virtual box for colour selection
        if (sliderPosition.x < m_startPos.x + boxPos)
        sliderPosition.x = m_startPos.x + boxPos;
        else if (sliderPosition.x > m_startPos.x + boxPos + boxSize - 1)
        sliderPosition.x = m_startPos.x + boxPos + boxSize - 1;
        if (sliderPosition.y < m_startPos.y + boxPos)
        sliderPosition.y = m_startPos.y + boxPos;
        else if (sliderPosition.y > m_startPos.y + boxPos + boxSize)
        sliderPosition.y = m_startPos.y + boxPos + boxSize;

        // Use the position / slider width to determin the percentage the dragger is
        // at
        m_saturation = 1.0f - fabs((m_startPos.x + (float)boxPos - sliderPosition.x) /
                                 (float)boxSize);
        m_brightness =
          fabs((m_startPos.y + (float)boxPos - sliderPosition.y) / (float)boxSize);
    }

    bool checkSliderPosition(CCPoint location) {
        // Clamp the position of the icon within the circle

        // get the center point of the bkgd image
        float centerX = m_startPos.x + m_background->boundingBox().size.width * 0.5f;
        float centerY = m_startPos.y + m_background->boundingBox().size.height * 0.5f;

        // work out the distance difference between the location and center
        float dx = location.x - centerX;
        float dy = location.y - centerY;
        float dist = sqrtf(dx * dx + dy * dy);

        // check that the touch location is within the bounding rectangle before
        // sending updates
        if (dist <= m_background->boundingBox().size.width * 0.5f) {
            updateSliderPosition(location);
            sendActionsForControlEvents(CCControlEventValueChanged);
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
