/****************************************************************************
 Copyright (c) 2014 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/


#include "ShaderTest2.h"
#include "ShaderTest.h"
#include "../testResource.h"
#include "cocos2d.h"
#include <tuple>

namespace ShaderTest2
{
    static std::function<Layer*()> createFunctions[] =
    {
        CL(EffectSpriteTest),
    };
    
    static unsigned int TEST_CASE_COUNT = sizeof(ShaderTest2::createFunctions) / sizeof(ShaderTest2::createFunctions[0]);
    
    static int sceneIdx=-1;
    Layer* createTest(int index)
    {
        auto layer = (createFunctions[index])();;
        return layer;
    }
    
    Layer* nextAction();
    Layer* backAction();
    Layer* restartAction();
    
    Layer* nextAction()
    {
        sceneIdx++;
        sceneIdx = sceneIdx % TEST_CASE_COUNT;
        
        return createTest(sceneIdx);
    }
    
    Layer* backAction()
    {
        sceneIdx--;
        if( sceneIdx < 0 )
            sceneIdx = TEST_CASE_COUNT -1;
        
        return createTest(sceneIdx);
    }
    
    Layer* restartAction()
    {
        return createTest(sceneIdx);
    }
    
}

ShaderTestDemo2::ShaderTestDemo2()
{
    
}

void ShaderTestDemo2::backCallback(Ref* sender)
{
    auto s = ShaderTestScene2::create();
    s->addChild( ShaderTest2::backAction() );
    Director::getInstance()->replaceScene(s);
}

void ShaderTestDemo2::nextCallback(Ref* sender)
{
    auto s = ShaderTestScene2::create();
    s->addChild( ShaderTest2::nextAction() );
    Director::getInstance()->replaceScene(s);
}

void ShaderTestDemo2::restartCallback(Ref* sender)
{
    auto s = ShaderTestScene2::create();
    s->addChild(ShaderTest2::restartAction());    
    Director::getInstance()->replaceScene(s);
}

void ShaderTestScene2::runThisTest()
{
    auto layer = ShaderTest2::nextAction();
    addChild(layer);
    Director::getInstance()->replaceScene(this);
}

//
// EffectSprite
//
static int tuple_sort( const std::tuple<ssize_t,Effect*,QuadCommand> &tuple1, const std::tuple<ssize_t,Effect*,QuadCommand> &tuple2 )
{
    return std::get<0>(tuple1) < std::get<0>(tuple2);
}

class Effect;
class EffectSprite : public Sprite
{
public:
    static EffectSprite *create(const std::string& filename) {
        auto ret = new (std::nothrow) EffectSprite;
        if(ret && ret->initWithFile(filename)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_RELEASE(ret);
        return nullptr;
    }

    void setEffect(Effect* effect) {
        if(_defaultEffect != effect) {
            effect->setTarget(this);

            CC_SAFE_RELEASE(_defaultEffect);
            _defaultEffect = effect;
            CC_SAFE_RETAIN(_defaultEffect);

            setGLProgramState(_defaultEffect->getGLProgramState());
        }
    }
    void addEffect(Effect *effect, ssize_t order) {
        effect->retain();
        effect->setTarget(this);

        _effects.push_back(std::make_tuple(order,effect,QuadCommand()));

        std::sort(std::begin(_effects), std::end(_effects), tuple_sort);
    }

    void draw(Renderer *renderer, const Mat4 &transform, bool transformUpdated) override
    {
        // Don't do calculate the culling if the transform was not updated
        _insideBounds = transformUpdated ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;

        if(_insideBounds)
        {
            // negative effects: order < 0
            int idx=0;
            for(auto &effect : _effects) {

                if(std::get<0>(effect) >=0)
                    break;
                QuadCommand &q = std::get<2>(effect);
                q.init(_globalZOrder, _texture->getName(), std::get<1>(effect)->getGLProgramState(), _blendFunc, &_quad, 1, transform);
                renderer->addCommand(&q);
                idx++;

            }

            // normal effect: order == 0
            _quadCommand.init(_globalZOrder, _texture->getName(), getGLProgramState(), _blendFunc, &_quad, 1, transform);
            renderer->addCommand(&_quadCommand);

            // postive effects: oder >= 0
            for(auto it = std::begin(_effects)+idx; it != std::end(_effects); ++it) {
                QuadCommand &q = std::get<2>(*it);
                q.init(_globalZOrder, _texture->getName(), std::get<1>(*it)->getGLProgramState(), _blendFunc, &_quad, 1, transform);
                renderer->addCommand(&q);
                idx++;
            }
        }
    }
protected:
    EffectSprite() : _defaultEffect(nullptr)
    {
        _effects.reserve(2);
    }
    ~EffectSprite() {
        for(auto &tuple : _effects) {
            std::get<1>(tuple)->release();
        }
        CC_SAFE_RELEASE(_defaultEffect);
    }

    std::vector<std::tuple<ssize_t,Effect*,QuadCommand>> _effects;
    Effect* _defaultEffect;
};

//
// Effect
//

bool Effect::initGLProgramState(const std::string &fragmentFilename)
{
    auto fileUtiles = FileUtils::getInstance();
    auto fragmentFullPath = fileUtiles->fullPathForFilename(fragmentFilename);
    auto fragSource = fileUtiles->getStringFromFile(fragmentFullPath);
    auto glprogram = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, fragSource.c_str());
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    _fragSource = fragSource;
#endif
    
    _glprogramstate = GLProgramState::getOrCreateWithGLProgram(glprogram);
    _glprogramstate->retain();

    return _glprogramstate != nullptr;
}

Effect::Effect()
: _glprogramstate(nullptr)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    _backgroundListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND,
                                                      [this](EventCustom*)
                                                      {
                                                          auto glProgram = _glprogramstate->getGLProgram();
                                                          glProgram->reset();
                                                          glProgram->initWithByteArrays(ccPositionTextureColor_noMVP_vert, _fragSource.c_str());
                                                          glProgram->link();
                                                          glProgram->updateUniforms();
                                                      }
                                                      );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_backgroundListener, -1);
#endif
}

Effect::~Effect()
{
    CC_SAFE_RELEASE_NULL(_glprogramstate);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    Director::getInstance()->getEventDispatcher()->removeEventListener(_backgroundListener);
#endif
}

// Blur
class EffectBlur : public Effect
{
public:
    CREATE_FUNC(EffectBlur);

    virtual void setTarget(EffectSprite *sprite) override;

    void setGaussian(float value);
    void setCustomUniforms();
    void setBlurSize(float f);

protected:
    bool init(float blurSize=3.0);

    int       _blurRadius;
    Vec2   _pixelSize;

    int       _samplingRadius;
    float     _scale;
    float     _cons;
    float     _weightSum;
};

void EffectBlur::setTarget(EffectSprite *sprite)
{
    Size s = sprite->getTexture()->getContentSizeInPixels();
    _pixelSize = Vec2(1/s.width, 1/s.height);
    _glprogramstate->setUniformVec2("onePixelSize", _pixelSize);
}

bool EffectBlur::init(float blurSize)
{
    initGLProgramState("Shaders/example_Blur.fsh");
    auto s = Size(100,100);

    _blurRadius = 0;
    _pixelSize = Vec2(1/s.width, 1/s.height);
    _samplingRadius = 0;

    setBlurSize(blurSize);

    _glprogramstate->setUniformVec2("onePixelSize", _pixelSize);
    _glprogramstate->setUniformVec4("gaussianCoefficient", Vec4(_samplingRadius, _scale, _cons, _weightSum));
    return true;
}

void EffectBlur::setBlurSize(float f)
{
    if(_blurRadius == (int)f)
        return;
    _blurRadius = (int)f;

    _samplingRadius = _blurRadius;
    if (_samplingRadius > 10)
    {
        _samplingRadius = 10;
    }
    if (_blurRadius > 0)
    {
        float sigma = _blurRadius / 2.0f;
        _scale = -0.5f / (sigma * sigma);
        _cons = -1.0f * _scale / 3.141592f;
        _weightSum = -_cons;

        float weight;
        int squareX;
        for(int dx = 0; dx <= _samplingRadius; ++dx)
        {
            squareX = dx * dx;
            weight = _cons * exp(squareX * _scale);
            _weightSum += 2.0 * weight;
            for (int dy = 1; dy <= _samplingRadius; ++dy)
            {
                weight = _cons * exp((squareX + dy * dy) * _scale);
                _weightSum += 4.0 * weight;
            }
        }
    }
}

// Outline
class EffectOutline : public Effect
{
public:
    CREATE_FUNC(EffectOutline);

    bool init()
    {
        initGLProgramState("Shaders/example_outline.fsh");

        Vec3 color(1.0, 0.2, 0.3);
        GLfloat radius = 0.01;
        GLfloat threshold = 1.75;

        _glprogramstate->setUniformVec3("u_outlineColor", color);
        _glprogramstate->setUniformFloat("u_radius", radius);
        _glprogramstate->setUniformFloat("u_threshold", threshold);
        return true;
    }
};

// Noise
class EffectNoise : public Effect
{
public:
    CREATE_FUNC(EffectNoise);

protected:
    bool init() {
        initGLProgramState("Shaders/example_Noisy.fsh");
        return true;
    }

    virtual void setTarget(EffectSprite* sprite) override
    {
        auto s = sprite->getTexture()->getContentSizeInPixels();
        getGLProgramState()->setUniformVec2("resolution", Vec2(s.width, s.height));
    }
};

// Edge Detect
class EffectEdgeDetect : public Effect
{
public:
    CREATE_FUNC(EffectEdgeDetect);

protected:
    bool init() {
        initGLProgramState("Shaders/example_edgeDetection.fsh");
        return true;
    }

    virtual void setTarget(EffectSprite* sprite) override
    {
        auto s = sprite->getTexture()->getContentSizeInPixels();
        getGLProgramState()->setUniformVec2("resolution", Vec2(s.width, s.height));
    }
};

// Grey
class EffectGreyScale : public Effect
{
public:
    CREATE_FUNC(EffectGreyScale);

protected:
    bool init() {
        initGLProgramState("Shaders/example_greyScale.fsh");
        return true;
    }
};

// Sepia
class EffectSepia : public Effect
{
public:
    CREATE_FUNC(EffectSepia);

protected:
    bool init() {
        initGLProgramState("Shaders/example_sepia.fsh");
        return true;
    }
};

// bloom
class EffectBloom : public Effect
{
public:
    CREATE_FUNC(EffectBloom);

protected:
    bool init() {
        initGLProgramState("Shaders/example_bloom.fsh");
        return true;
    }

    virtual void setTarget(EffectSprite* sprite) override
    {
        auto s = sprite->getTexture()->getContentSizeInPixels();
        getGLProgramState()->setUniformVec2("resolution", Vec2(s.width, s.height));
    }
};

// cel shading
class EffectCelShading : public Effect
{
public:
    CREATE_FUNC(EffectCelShading);

protected:
    bool init() {
        initGLProgramState("Shaders/example_celShading.fsh");
        return true;
    }

    virtual void setTarget(EffectSprite* sprite) override
    {
        auto s = sprite->getTexture()->getContentSizeInPixels();
        getGLProgramState()->setUniformVec2("resolution", Vec2(s.width, s.height));
    }
};

// Lens Flare
class EffectLensFlare : public Effect
{
public:
    CREATE_FUNC(EffectLensFlare);

protected:
    bool init() {
        initGLProgramState("Shaders/example_lensFlare.fsh");
        return true;
    }

    virtual void setTarget(EffectSprite* sprite) override
    {
        auto s = sprite->getTexture()->getContentSizeInPixels();
        getGLProgramState()->setUniformVec2("textureResolution", Vec2(s.width, s.height));

        s = Director::getInstance()->getWinSize();
        getGLProgramState()->setUniformVec2("resolution", Vec2(s.width, s.height));

    }
};


EffectSpriteTest::EffectSpriteTest()
{
    if (ShaderTestDemo2::init()) {

        auto s = Director::getInstance()->getWinSize();

        auto itemPrev = MenuItemImage::create("Images/b1.png", "Images/b2.png",
                                          [&](Ref *sender) {
                                              _vectorIndex--;
                                              if(_vectorIndex<0)
                                                  _vectorIndex = _effects.size()-1;
                                              _sprite->setEffect(_effects.at(_vectorIndex));
                                          });

        auto itemNext = MenuItemImage::create("Images/f1.png", "Images/f2.png",
                                          [&](Ref *sender) {
                                              _vectorIndex++;
                                              if(_vectorIndex>=_effects.size())
                                                  _vectorIndex = 0;
                                              _sprite->setEffect(_effects.at(_vectorIndex));
                                          });

        auto menu = Menu::create(itemPrev, itemNext, NULL);
        menu->alignItemsHorizontally();
        menu->setScale(0.5);
        menu->setAnchorPoint(Vec2(0,0));
        menu->setPosition(Vec2(s.width/2,70));
        addChild(menu);

        _sprite = EffectSprite::create("Images/grossini.png");
        _sprite->setPosition(Vec2(0, s.height/2));
        addChild(_sprite);

        auto jump = JumpBy::create(4, Vec2(s.width,0), 100, 4);
        auto rot = RotateBy::create(4, 720);
        auto spawn = Spawn::create(jump, rot, NULL);
        auto rev = spawn->reverse();
        auto seq = Sequence::create(spawn, rev, NULL);
        auto repeat = RepeatForever::create(seq);
        _sprite->runAction(repeat);

        // set the Effects
        _effects.pushBack(EffectBlur::create());
        _effects.pushBack(EffectOutline::create());
        _effects.pushBack(EffectNoise::create());
        _effects.pushBack(EffectEdgeDetect::create());
        _effects.pushBack(EffectGreyScale::create());
        _effects.pushBack(EffectSepia::create());
        _effects.pushBack(EffectBloom::create());
        _effects.pushBack(EffectCelShading::create());
        _effects.pushBack(EffectLensFlare::create());

        _vectorIndex = 0;
        _sprite->setEffect( _effects.at(_vectorIndex) );

//        _sprite->addEffect( _effects.at(8), -10 );
//        _sprite->addEffect( _effects.at(1), 1 );

    }
}

