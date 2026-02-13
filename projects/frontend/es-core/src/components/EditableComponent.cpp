//
// Created by bkg2k on 21/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <components/EditableComponent.h>
#include <guis/GuiArcadeVirtualKeyboard.h>
#include <themes/ThemeData.h>
#include <WindowManager.h>
#include <rendering/fonts/FontManager.h>

EditableComponent::EditableComponent(WindowManager& window)
  : Component(window)
  , mBackground(window, Path(":/textinput_ninepatch.png"))
  , mFont(&FontManager::Instance().FromDefault(FONT_SIZE_MEDIUM, false))
  , mInterface(nullptr)
  , mTextWidth(0)
  , mIndentifier(0)
  , mColor(0x000000FF)
  , mOriginColor(0x000000FF)
  , mLineSpacing(1.5f)
  , mAlignment(::Alignment::CenterLeft)
  , mColorOpacity(0xFF)
  , mUppercase(false)
  , mAutoCalcExtentX(true)
  , mAutoCalcExtentY(true)
  , mMasked(false)
{
  mPosition = Vector3f::Zero();
  mSize = Vector2f::Zero();
  mBackground.setCenterColor(0xFFFFFF80/*0x00000020*/);
  mBackground.setEdgeColor(0xFFFFFF80/*0x00000020*/);
}

EditableComponent::EditableComponent(WindowManager&window, const String& editTitle, const String& text, NewFont* font, unsigned int color, int id, IEditableComponent* interface, bool masked)
  : EditableComponent(window)
{
  mMasked = masked;
  mIndentifier = id;
  mInterface = interface;
  mFont = font;
  mText = text;
  mEditTitle = editTitle;
  mColor = color;
  mColorOpacity = (unsigned char)(color & 0xFF);
  mOriginColor = color;
}

EditableComponent::EditableComponent(WindowManager&window, const String& editTitle, const String& text, NewFont* font, unsigned int color, const std::function<void(const String&)>& callback)
  : EditableComponent(window)
{
  mTextChanged = callback;
  mFont = font;
  mText = text;
  mEditTitle = editTitle;
  mColor = color;
  mColorOpacity = (unsigned char)(color & 0xFF);
  mOriginColor = color;
}

EditableComponent::EditableComponent(WindowManager&window, const String& editTitle, const String& text, NewFont* font, unsigned int color, ::Alignment align, const std::function<void(const String&)>& callback)
  : EditableComponent(window)
{
  mTextChanged = callback;
  mFont = font;
  mText = text;
  mEditTitle = editTitle;
  mColor = color;
  mColorOpacity = (unsigned char)(color & 0xFF);
  mOriginColor = color;
  mAlignment = align;
}

void EditableComponent::onSizeChanged()
{
  mAutoCalcExtentX = (getSize().x() == 0);
  mAutoCalcExtentY = (getSize().y() == 0);
}

void EditableComponent::setFont(NewFont* font)
{
  mFont = font;
}

void EditableComponent::setColor(unsigned int color)
{
  mColor = color;
  unsigned char opacity = mColor & 0x000000FF;
  Component::setOpacity(opacity);

  mColorOpacity = mColor & 0x000000FF;
}

//  Scale the opacity
void EditableComponent::setOpacity(unsigned char opacity)
{
  // This method is mostly called to do fading in-out of the Text component element.
  // Therefore, we assume here that opacity is a fractional value (expressed as an int 0-255),
  // of the opacity originally set with setColor() or setBackgroundColor().

  unsigned char o = (unsigned char)((float)opacity / 255.f * (float) mColorOpacity);
  mColor = (mColor & 0xFFFFFF00) | (unsigned char) o;

  Component::setOpacity(opacity);
}

void EditableComponent::setText(const String& text)
{
  mText = text;
}

void EditableComponent::setUppercase(bool uppercase)
{
  mUppercase = uppercase;
}

void EditableComponent::Render(const Transform4x4f& parentTrans)
{
  if(mThemeDisabled) return;
  Transform4x4f trans = parentTrans * getTransform();

  Renderer::SetMatrix(trans);
  if (mBackground.getSize().x() != mSize.x() &&
      mBackground.getSize().y() != mSize.y())
    mBackground.setSize(mSize);
  mBackground.Render(trans);

  float yOff = 0;
  switch(AlignmentExtractVertical(mAlignment))
  {
    case VerticalAlignment::Center: yOff = (getSize().y() - mTextWidth) / 2.0f; break;
    case VerticalAlignment::Bottom: yOff = (getSize().y() - mTextWidth); break;
    case VerticalAlignment::Top:
    default: break;
  }
  Vector3f off(mBackground.MargingX(), yOff, 0);

  trans.translate(off);
  trans.round();
  Renderer::SetMatrix(trans);

  mFont->DrawText(mText, 0, 0, (mColor & 0xFFFFFF00) | mOpacity, mAlignment);
}

void EditableComponent::calculateExtent()
{
  if(mAutoCalcExtentX)
  {
    mSize = mFont->TextSize(mText) + mBackground.MargingX() * 2;
  }else{
    if(mAutoCalcExtentY)
    {
      mSize.y() = mFont->SplitText(mUppercase ? mText.ToUpperCaseUTF8() : mText, mSize.x()).Chunks.Count() * mLineSpacing;
    }
  }
}

void EditableComponent::setAlignment(::Alignment align)
{
  mAlignment = align;
}

void EditableComponent::setLineSpacing(float spacing)
{
  mLineSpacing = spacing;
}

/*void EditableComponent::applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties)
{
  Component::applyTheme(theme, view, element, properties);

  const ThemeElement* elem = theme.getElement(view, element, "text");
  if(elem == nullptr) return;

  if (hasFlag(properties, ThemeProperties::Color) && elem->HasProperty("color"))
    setColor((unsigned int)elem->AsInt("color"));

  if(hasFlag(properties, ThemeProperties::Alignment) && elem->HasProperty("alignment"))
  {
    String str = elem->AsString("alignment");
    if(str == "left")
      setHorizontalAlignment(::Alignment::CenterLeft);
    else if(str == "center")
      setHorizontalAlignment(::Alignment::Center);
    else if(str == "right")
      setHorizontalAlignment(::Alignment::CenterRight);
    else
    { LOG(LogError) << "[EditableComponent] Unknown text alignment string: " << str; }
  }

  if (hasFlag(properties, ThemeProperties::Text) && elem->HasProperty("text"))
    setText(elem->AsString("text"));

  if (hasFlag(properties, ThemeProperties::ForceUppercase) && elem->HasProperty("forceUppercase"))
    setUppercase(elem->AsBool("forceUppercase"));

  if (hasFlag(properties, ThemeProperties::LineSpacing) && elem->HasProperty("lineSpacing"))
    setLineSpacing(elem->AsFloat("lineSpacing"));

  setFont(Font::getFromTheme(elem, properties, mFont));
}*/

void EditableComponent::StartEditing()
{
  mTextBackup = mText;
  mWindow.pushGui(new GuiArcadeVirtualKeyboard(mWindow, mEditTitle, mTextBackup, this));
}

void EditableComponent::VirtualKeyboardTextChange(IVirtualKeyboardBase& vk, const String& text)
{
  (void)vk;
  setText(text);
}

void EditableComponent::VirtualKeyboardValidated(IVirtualKeyboardBase& vk, const String& text)
{
  (void)vk;
  (void)text;
  if (mTextChanged)
    mTextChanged(text);
  if (mInterface != nullptr)
  {
    mInterface->EditableComponentTextChanged(mIndentifier, text);
    mParent->onSizeChanged();
  }
}
