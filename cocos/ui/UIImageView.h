/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#ifndef __UIIMAGEVIEW_H__
#define __UIIMAGEVIEW_H__

#include "ui/UIWidget.h"
#include "ui/GUIExport.h"

/**
 * @addtogroup ui
 * @{
 */
NS_CC_BEGIN

struct CC_DLL ResourceData;

namespace ui {
    class Scale9Sprite;
/**
 * @brief A widget to display images.
 */
class CC_GUI_DLL ImageView : public Widget , public cocos2d::BlendProtocol
{
    
    DECLARE_CLASS_GUI_INFO
    
public:
    /**
     * Default constructor
     * @js ctor
     * @lua new
     */
    ImageView();

    /**
     * Default destructor
     * @js NA
     * @lua NA
     */
    virtual ~ImageView();

    /**
     * Create a empty ImageView.
     */
    static ImageView* create();
    
    /**
     * Create a  imageview  with a image name.
     *
     * @param imageFileName   file name of texture.
     * @param texType    @see `Widget::TextureResType`
     * @return A ImageView instance.
     */
    static ImageView* create(const std::string& imageFileName, TextureResType texType = TextureResType::LOCAL);
    

    /**
     * Load texture for imageview.
     *
     * @param fileName   file name of texture.
     * @param texType    @see `Widget::TextureResType`
     */
    void loadTexture(const std::string& fileName,TextureResType texType = TextureResType::LOCAL);

    /**
     * Updates the texture rect of the ImageView in points.
     * It will call setTextureRect:rotated:untrimmedSize with rotated = NO, and utrimmedSize = rect.size.
     */
    void setTextureRect(const Rect& rect);

    /**
     * Enable scale9 renderer.
     *
     * @param enabled Set to true will use scale9 renderer, false otherwise.
     */
    void setScale9Enabled(bool enabled);

    /**
     * Query whether button is using scale9 renderer or not.
     *@return whether button use scale9 renderer or not.
     */
    bool isScale9Enabled()const;
    
    /**
     * Enable tile renderer.
     *
     * @param enabled Set to true will use tile renderer, false otherwise.
     */
    void setTileEnabled(bool enabled);
    
    /**
     * Query whether button is using tile renderer or not.
     *@return whether button use tile renderer or not.
     */
    bool isTileEnabled()const;
    

    /**
     * Sets capInsets for imageview.
     * The capInsets affects the ImageView's renderer only if `setScale9Enabled(true)` is called.
     *
     * @param capInsets    capinsets for imageview
     */
    void setCapInsets(const Rect &capInsets);

    /**
     * Get ImageView's capInsets size.
     * @return Query capInsets size in Rect
     * @see `setCapInsets(const Rect&)`
     */
    const Rect& getCapInsets()const;
    
    /**
     * Sets image internal scale.
     * @return internal image scale.
     */
    void setImageScale(float scale);
    
    /**
     * Get image internal scale.
     * @param scale image internal scale
     */
    float getImageScale() const;
    
    /**
     * Sets the source blending function.
     *
     * @param blendFunc A structure with source and destination factor to specify pixel arithmetic. e.g. {GL_ONE, GL_ONE}, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}.
     * @js NA
     * @lua NA
     */
    virtual void setBlendFunc(const BlendFunc &blendFunc) override;
    
    /**
     * Returns the blending function that is currently being used.
     *
     * @return A BlendFunc structure with source and destination factor which specified pixel arithmetic.
     * @js NA
     * @lua NA
     */
    virtual const BlendFunc &getBlendFunc() const override;

    //override methods.
    virtual void ignoreContentAdaptWithSize(bool ignore) override;
    virtual std::string getDescription() const override;
    virtual Size getVirtualRendererSize() const override;
    virtual Node* getVirtualRenderer() override;
    
    /**
     * Sets whether the widget should be flipped horizontally or not.
     *
     * @param flippedX true if the widget should be flipped horizontally, false otherwise.
     */
    virtual void setFlippedX(bool flippedX) override;
    
    /**
     * Returns the flag which indicates whether the widget is flipped horizontally or not.
     *
     * It only flips the texture of the widget, and not the texture of the widget's children.
     * Also, flipping the texture doesn't alter the anchorPoint.
     * If you want to flip the anchorPoint too, and/or to flip the children too use:
     * widget->setScaleX(sprite->getScaleX() * -1);
     *
     * @return true if the widget is flipped horizontally, false otherwise.
     */
    virtual bool isFlippedX() const  override;
    
    /**
     * Sets whether the widget should be flipped vertically or not.
     *
     * @param flippedY true if the widget should be flipped vertically, false otherwise.
     */
    virtual void setFlippedY(bool flippedY) override;
    
    /**
     * Return the flag which indicates whether the widget is flipped vertically or not.
     *
     * It only flips the texture of the widget, and not the texture of the widget's children.
     * Also, flipping the texture doesn't alter the anchorPoint.
     * If you want to flip the anchorPoint too, and/or to flip the children too use:
     * widget->setScaleY(widget->getScaleY() * -1);
     *
     * @return true if the widget is flipped vertically, false otherwise.
     */
    virtual bool isFlippedY() const  override;

    ResourceData getRenderFile();

    virtual void setGLProgram(GLProgram* glProgram) override;
    virtual void setGLProgramState(cocos2d::GLProgramState* glProgramState) override;
CC_CONSTRUCTOR_ACCESS:
    //initializes state of widget.
    virtual bool init() override;
    virtual bool init(const std::string& imageFileName, TextureResType texType = TextureResType::LOCAL);

protected:
    virtual void initRenderer() override;
    virtual void onSizeChanged() override;
    
    virtual void adaptRenderers() override;
    void loadTexture(SpriteFrame* spriteframe);
    void setupTexture();
    
    void imageTextureScaleChangedWithSize();
    virtual Widget* createCloneInstance() override;
    virtual void copySpecialProperties(Widget* model) override;
    
    void updateRenderType();
    
protected:
    bool _scale9Enabled;
    bool _tileEnabled;
    bool _prevIgnoreSize;
    Rect _capInsets;
    Scale9Sprite* _imageRenderer;
    TextureResType _imageTexType;
    Size _imageTextureSize;
    bool _imageRendererAdaptDirty;
    std::string _textureFile;
    float _imageScale;
};

}

NS_CC_END
// end of ui group
/// @}

#endif /* defined(__CocoGUI__ImageView__) */
