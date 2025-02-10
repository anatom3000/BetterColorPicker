using ColorChangedCallback = std::function<void(cocos2d::ccColor3B)>;

class BetterColorPicker : public cocos2d::extension::CCControl {
public:
    float m_radius;
    bool m_touching;
    bool m_touchedHue;
    
    geode::Ref<cocos2d::CCSprite> m_sprite;
    geode::Ref<cocos2d::CCGLProgram> m_shader;
    GLuint m_location;
    geode::Ref<cocos2d::CCSprite> m_hueNipple;
    geode::Ref<cocos2d::CCSprite> m_svNipple;

    ColorChangedCallback m_colorChangedCallback;

    // between 0.0 and 1.0
    double m_hue;
    double m_saturation;
    double m_value;

    inline static cocos2d::CCPoint v1 = ccp(0.0, 1.0);;
    inline static cocos2d::CCPoint v2 = ccp(-sqrt(3.0)/2.0, -0.5);
    inline static cocos2d::CCPoint v3 = ccp(+sqrt(3.0)/2.0, -0.5);

    static BetterColorPicker* create(ColorChangedCallback callback);

    cocos2d::ccColor3B getRgbValue();
    void setRgbValue(cocos2d::ccColor3B color, bool call);

    bool init(ColorChangedCallback callback);
    void updateValues(bool call);
    double widthAt(double y);
    bool touchesHueWheel(cocos2d::CCPoint position);
    std::tuple<double, double, double> barycentricCoords(cocos2d::CCPoint pos);
    double dot(cocos2d::CCPoint u, cocos2d::CCPoint v);
    std::tuple<double, double, double> closestPointInTriangle(double x, double y, double z);
    cocos2d::CCPoint cartesianCoords(double x, double y, double z);
    bool touchesTriangle(cocos2d::CCPoint position);
    void updateHue(cocos2d::CCPoint position);
    void updateSV(cocos2d::CCPoint position);
    bool ccTouchBegan(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void ccTouchMoved(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void ccTouchEnded(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void registerWithTouchDispatcher() override;
};
