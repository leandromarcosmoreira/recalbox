//
// Created by bkg2k on 21/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/IVirtualKeyboardInterface.h>
#include "components/base/Component.h"
#include "NinePatchComponent.h"
#include "IEditableComponent.h"
#include <rendering/fonts/Font.h>

class ThemeData;

class EditableComponent : public Component, private IVirtualKeyboardInterface
{
  public:
    explicit EditableComponent(WindowManager&window);
    EditableComponent(WindowManager&window, const String& editTitle, const String& text, NewFont* font, unsigned int color, int id, IEditableComponent* interface, bool masked);
    EditableComponent(WindowManager&window, const String& editTitle, const String& text, NewFont* font, unsigned int color, const std::function<void(const String&)>& callback);
    EditableComponent(WindowManager&window, const String& editTitle, const String& text, NewFont* font, unsigned int color, ::Alignment align, const std::function<void(const String&)>& callback);

    void setFont(NewFont*font);
    void setUppercase(bool uppercase);
    void onSizeChanged() override;
    void setText(const String& text);
    void setColor(unsigned int color) override;
    void setOriginColor(unsigned int color) { mOriginColor = color; }
    unsigned int getOriginColor() override { return mOriginColor; }

    void setAlignment(::Alignment align);

    void setLineSpacing(float spacing);

    void Render(const Transform4x4f& parentTrans) override;

    void setValue(const String& value) override { setText(value); }

    unsigned char getOpacity() const override {	return (unsigned char)(mColor & 0xFF); }

    void setOpacity(unsigned char opacity) override;

    NewFont* getFont() const { return mFont; }

    //void applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties) override;

    //! Start the Virtual Keyboard to edit this component text
    void StartEditing();

  private:
    void calculateExtent();

    NinePatchComponent mBackground;
    NewFont* mFont;
    String mText;
    String mTextBackup;
    String mEditTitle;
    std::function<void(const String&)> mTextChanged;
    IEditableComponent* mInterface;
    int mTextWidth;
    int mIndentifier;
    unsigned int mColor;
    unsigned int mOriginColor;
    float mMargin;
    float mLineSpacing;
    ::Alignment mAlignment;
    unsigned char mColorOpacity;
    bool mUppercase;
    bool mAutoCalcExtentX;
    bool mAutoCalcExtentY;
    bool mMasked;

    /*
     * IGuiArcadeVirtualKeyboardInterface implementation
     */

    /*!
     * @brief Called when the edited text change.
     * Current text is available from the Text() method.
     */
    void VirtualKeyboardTextChange(IVirtualKeyboardBase& vk, const String& text) final;

    /*!
     * @brief Called when the edited text is validated (Enter or Start)
     * Current text is available from the Text() method.
     */
    void VirtualKeyboardValidated(IVirtualKeyboardBase& vk, const String& text) final;
};



