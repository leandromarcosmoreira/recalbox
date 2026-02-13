#include <utils/locale/LocaleHelper.h>
#include <guis/GuiInputConfig.h>
#include <components/TextComponent.h>
#include <components/MenuComponent.h>
#include <components/ButtonComponent.h>
#include <hardware/Board.h>
#include <rendering/textures/TextureManager.h>

GuiInputConfig::~GuiInputConfig()
{
  mTargetDevice->SetConfiguringState(false);
  InputManager::Instance().RemoveNotificationInterface(this);
}

GuiInputConfig::GuiInputConfig(WindowManager&window, InputDevice* target, const std::function<void()>& doneCallback)
  : Gui(window)
  , mBackground(window, Path(":/frame.png"))
  , mGrid(window, Vector2i(1, 5))
  , mPreviousConfig(*target)
  , mDoneCallback(doneCallback)
  , mTargetDevice(target)
  , mActiveButtons(0)
  , mActiveAxis(0)
  , mActiveHats(0)
  , mButtonReferenceTime { 0 }
  , mIconWidth(0)
  , mNeutralPositionSet(false)
  , mCursorOnList(true)
  , mFirstEventReceived(true)
  , mCancelled(false)
{
  memset(&mButtonReferenceTime, 0, sizeof(mButtonReferenceTime));

  mTargetDevice->SetConfiguringState(true);
  { LOG(LogInfo) << "[GuiInput] Configuring device " << mTargetDevice->Index() << " (" << mTargetDevice->Name() << ")."; }

  mTargetDevice->ClearAll();

  const MenuThemeData& menuTheme = ThemeManager::Instance().Menu();

  mBackground.setImagePath(menuTheme.Background().path);
  mBackground.setCenterColor(menuTheme.Background().color);
  mBackground.setEdgeColor(menuTheme.Background().color);

  mMainColor = menuTheme.Text().color;

  addChild(&mBackground);
  addChild(&mGrid);

  mTitle = std::make_shared<TextComponent>(mWindow, _("CONFIGURING"), menuTheme.Title().font, menuTheme.Title().color, ::Alignment::Center);
  mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

  String deviceName = _("KEYBOARD");
  if (mTargetDevice->Identifier() != InputEvent::sKeyboardDevice)
    deviceName = _("GAMEPAD %i").Replace("%i", mTargetDevice->Name());

  mSubtitle1 = std::make_shared<TextScrollComponent>(mWindow, deviceName.UpperCaseUTF8(), menuTheme.Text().font, menuTheme.Footer().color, ::Alignment::Center);
  mGrid.setEntry(mSubtitle1, Vector2i(0, 1), false, true);

  mSubtitle2 = std::make_shared<TextComponent>(mWindow, "", menuTheme.SmallText().font, menuTheme.SmallText().color, ::Alignment::Center);
  mGrid.setEntry(mSubtitle2, Vector2i(0, 2), false, true);

  mCancel = std::make_shared<TextScrollComponent>(mWindow, _("Press any button for 5s to cancel !"), menuTheme.SmallText().font, menuTheme.SmallText().color, ::Alignment::Center);
  mGrid.setEntry(mCancel, Vector2i(0, 4), false, true);

  mList = std::make_shared<TextListComponent<FormInput*>>(mWindow);
  const MenuThemeData& menu = ThemeManager::Instance().Menu();
  mList->setFont(menu.Text().font);
  mList->setColorAt(0, menu.Text().color);
  mList->setColorAt(1, menu.Text().color);
  mList->setSelectorColor(menu.Text().selectorColor);
  mList->setSelectedColor(menu.Text().selectedColor);
  mList->SetDefaultSelectorHeight();
  mList->setHorizontalMargin(0.01f * Renderer::Instance().DisplayWidthAsFloat());
  mList->setAlignment(HorizontalAlignment::Left);
  mList->setAutoAlternate(true);
  mList->SetOverlayInterface(this);

  mGrid.setEntry(mList, Vector2i(0, 3), true, true);

  initFormInputs();
  for (auto& formInput: mFormInputs)
    mList->add(formInput.Label(), &formInput, 0, formInput.Entry() != InputDevice::Entry::None ? ::HorizontalAlignment::Left : ::HorizontalAlignment::Center);

  mList->setCursorChangedCallback([this](CursorState state)
  {
    (void)state;
    setHelpMessage();
  });

  setHelpMessage();
  setPress();

  float width = Renderer::Instance().Is480pOrLower() ? Renderer::Instance().DisplayWidthAsFloat() * 0.80f : Renderer::Instance().DisplayWidthAsFloat() * 0.60f;
  float height = Renderer::Instance().Is480pOrLower() ? Renderer::Instance().DisplayHeightAsFloat() * 0.70f : Renderer::Instance().DisplayHeightAsFloat() * 0.85f;
  setSize(width, height);
  setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

  InputManager::Instance().AddNotificationInterface(this);
}

void GuiInputConfig::initFormInputs()
{
  addFormInput(InputDevice::Entry::Up   , _("UP"), false, InputEvent::EventType::Hat);
  addFormInput(InputDevice::Entry::Down , _("DOWN"), false, InputEvent::EventType::Hat);
  addFormInput(InputDevice::Entry::Left , _("LEFT"), false, InputEvent::EventType::Hat);
  addFormInput(InputDevice::Entry::Right, _("RIGHT"), false, InputEvent::EventType::Hat);

  addFormInput(InputDevice::Entry::Joy1AxisV, _("JOYSTICK 1 UP"), true, InputEvent::EventType::Axis);
  addFormInput(InputDevice::Entry::Joy1AxisH, _("JOYSTICK 1 LEFT"), true, InputEvent::EventType::Axis);
  addFormInput(InputDevice::Entry::Joy2AxisV, _("JOYSTICK 2 UP"), true, InputEvent::EventType::Axis);
  addFormInput(InputDevice::Entry::Joy2AxisH, _("JOYSTICK 2 LEFT"), true, InputEvent::EventType::Axis);

  addFormInput(InputDevice::Entry::A, "A", false, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::B, "B", false, InputEvent::EventType::Button);

  addFormInput(InputDevice::Entry::X, "X", true, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::Y, "Y", true, InputEvent::EventType::Button);

  addFormInput(InputDevice::Entry::Start, "START", false, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::Select, "SELECT", false, InputEvent::EventType::Button);

  addFormInput(InputDevice::Entry::L1, "L1", true, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::R1, "R1", true, InputEvent::EventType::Button);

  addFormInput(InputDevice::Entry::L2, "L2", true, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::R2, "R2", true, InputEvent::EventType::Button);

  addFormInput(InputDevice::Entry::L3, "L3", true, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::R3, "R3", true, InputEvent::EventType::Button);

  if (Board::Instance().HasPhysicalVolumeButtons())
  {
    addFormInput(InputDevice::Entry::VolumeDown, _("VOLUME -"), true, InputEvent::EventType::Button);
    addFormInput(InputDevice::Entry::VolumeUp, _("VOLUME +"), true, InputEvent::EventType::Button);
  }

  if (Board::Instance().HasPhysicalBrightnessButtons())
  {
    addFormInput(InputDevice::Entry::BrightnessDown, _("BRIGHTNESS -"), true, InputEvent::EventType::Button);
    addFormInput(InputDevice::Entry::BrightnessUp, _("BRIGHTNESS +"), true, InputEvent::EventType::Button);
  }

  addFormInput(InputDevice::Entry::Hotkey, _("HOTKEY"), false, InputEvent::EventType::Button);
  addFormInput(InputDevice::Entry::None, String("\uf1c0 ").Append(_("VALIDATE")), false, InputEvent::EventType::Button);

  // Load icons
  int height  = mList->getFont()->Height();
  mIconWidth = 0;
  for(FormInput& data : mFormInputs)
  {
    int hk = (int) data.Entry();
    if (!mIcons[hk].Valid())
    {
      // Load Hotkey
      Path path = IconPathMap().get_or_return_default(data.Entry());
      mIcons[hk] = TextureManager::Instance().Create(path, 0, height, TextureHolder::Properties::ImmediateLoad);
      TextureManager::Instance().RequestTextureLoading(mIcons[hk], true);
      if (int w = mIcons[hk].Width(); w > mIconWidth) mIconWidth = w;
    }
  }
}

void GuiInputConfig::addFormInput(InputDevice::Entry entry, const String& label, bool skippable, InputEvent::EventType preferredType)
{
  FormInput formInput(entry, label, skippable, preferredType);
  mFormInputs.push_back(formInput);
}

void GuiInputConfig::onSizeChanged()
{
  mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

  // update grid
  mGrid.setSize(mSize);

  mGrid.setRowHeightPerc(0, (float)mTitle->getFont()->Height() / mSize.y());
  mGrid.setRowHeightPerc(1, (float)mSubtitle1->getFont()->Height() / mSize.y());
  mGrid.setRowHeightPerc(2, (float)mSubtitle2->getFont()->Height() / mSize.y());
  mGrid.setRowHeightPerc(4, (float)mCancel->getFont()->Height() * 1.5f / mSize.y());
}

// move cursor to the next thing if we're configuring all, 
// or come out of "configure mode" if we were only configuring one row
void GuiInputConfig::rowDone()
{
  if (mList->IsAtEnd())
  {
    // try to move to the next one
    setNotDefined();
    mCursorOnList = false;
    mGrid.moveCursor(Vector2i(0, 1));
  }
  else
  {
    mList->setCursorIndex(mList->getCursorIndex() + 1);
    setPress(); // on another one
  }
}

void GuiInputConfig::setText(const String& msg)
{
  mList->getSelected()->SetAction(msg);
}

void GuiInputConfig::setHelpMessage()
{
  String msg;
  InputEvent input;
  FormInput& formInput = *mList->getSelected();
  bool assigned = mTargetDevice->GetEntryConfiguration(formInput.Entry(), input);
  //std::shared_ptr<TextComponent>& text = mMappings[inputId];
  const MenuThemeData& menuTheme = ThemeManager::Instance().Menu();

  if (formInput.Entry() == InputDevice::Entry::None) msg = _("VALIDATE");
  else if (assigned)
  {
    if (RecalboxConf::Instance().GetSwapValidateAndCancel())
    {
      if (mTargetDevice->IsSet(InputDevice::Entry::B)) msg = msg + _("B TO UNSET");
    }
    else
    {
      if (mTargetDevice->IsSet(InputDevice::Entry::A)) msg = msg + _("A TO UNSET");
    }
    if (mTargetDevice->IsSet(InputDevice::Entry::Down))
    {
      msg = (!msg.empty() ? msg + " - " : "") + _("UP/DOWN TO SKIP");
    }
  }
  else if (formInput.Skippable())	msg = _("UP/DOWN TO SKIP");
  else                            msg = _("INPUT REQUIRED");

  mSubtitle2->setText(msg);
  mSubtitle2->setColor(formInput.Skippable() || assigned ? menuTheme.SmallText().color : 0xff4141ff);
}

void GuiInputConfig::setPress()
{
  InputEvent input;
  FormInput& formInput = *mList->getSelected();
  if (mTargetDevice->GetEntryConfiguration(formInput.Entry(), input)) {
    setAssignedTo(input);
  } else {
    setText(_("PRESS ANYTHING"));
  }
}

bool GuiInputConfig::assign(InputEvent input)
{
  FormInput& formInput = *mList->getSelected();

  // input is from InputConfig* mTargetDevice
  // if this input is mapped to something other than "nothing" or the current row, error
  // (if it's the same as what it was before, allow it)
  if (formInput.Entry() != InputDevice::Entry::Hotkey &&
      mTargetDevice->GetMatchedEntry(input) != InputDevice::Entry::None &&
      !mTargetDevice->IsMatching(formInput.Entry(), input))
  {
    setMapped();
    return false;
  }

  // Trigger axis are required to be positive
  if ((formInput.Entry() == InputDevice::Entry::L2) || (formInput.Entry() == InputDevice::Entry::R2))
    if ((input.Type() == InputEvent::EventType::Axis) && (input.Value() < 0))
      input = InputEvent(input.Device(), input.Type(), input.Id(), -input.Value(), input.Code());

  setAssignedTo(input);

  // set a code while the device is still connected
  input.StoreSDLCode(mTargetDevice->Index());
  mTargetDevice->Set(formInput.Entry(), input);

  { LOG(LogInfo) << "[GuiInput]   Mapping [" << input.ToString() << "] -> " << formInput.Label(); }

  return true;
}

void GuiInputConfig::unAssign()
{
  FormInput& formInput = *mList->getSelected();

  InputEvent input;
  mTargetDevice->GetEntryConfiguration(formInput.Entry(), input);

  setNotDefined();

  mTargetDevice->Unset(formInput.Entry());

  { LOG(LogInfo) << "[GuiInput]   Unmapping [" << input.ToString() << "] -> " << formInput.Label(); }
}

void GuiInputConfig::restaurePreviousAssignment()
{
  InputEvent input;
  FormInput formInput = *mList->getSelected();
  if(mTargetDevice->GetEntryConfiguration(formInput.Entry(), input))
    setAssignedTo(input);
  else
    setNotDefined();
}

bool GuiInputConfig::isAssigned()
{
  FormInput& formInput = *mList->getSelected();
  return mTargetDevice->IsSet(formInput.Entry());
}

bool GuiInputConfig::ProcessInput(const InputCompactEvent& event)
{
  // ignore input not from our target device
  if(&event.Device() != mTargetDevice)
    return false;

  if (mCancelled)
  {
    if (mTargetDevice->CheckNeutralPosition()) Close();
    return true;
  }

  // 8bitdo DInput bug
  if (mFirstEventReceived)
  {
    event.Device().RecordAxisNeutralPosition();
    mFirstEventReceived = false;
  }

  // Neutral position has been reached?
  if (!mNeutralPositionSet)
  {
    if (mTargetDevice->CheckNeutralPosition(event.Device().Name() == "Namco GunCon 2")) mNeutralPositionSet = true;
    return true;
  }

  // Record
  RecordRawInput(event.RawEvent());
  if (!mEventList.empty() && NeutralPosition())
    ProcessEvents();

  return true;
}

bool GuiInputConfig::CollectHelpItems(Help &help)
{
    help.Set(Help::Cancel(), _("CANCEL"));
    return true;
}

void GuiInputConfig::ProcessEvents()
{
  const FormInput& formInput = *mList->getSelected();

  std::vector<InputEvent> events = std::move(mEventList);
  mEventList.clear();

  for (auto input : events)
  {
    // Key Up
    if (mTargetDevice->IsMatching(InputDevice::Entry::Up, input))
    {
      if (!mList->IsAtStart() && mTargetDevice->IsSet(InputDevice::Entry::Down))
      {
        restaurePreviousAssignment();
        if (!isAssigned() && formInput.Skippable()) setSkipped();
        if (!mList->IsAtStart())
          mList->setCursorIndex(mList->getCursorIndex() - 1);
        setPress();
      }
      return;
    }

    // Key Down
    if (mTargetDevice->IsMatching(InputDevice::Entry::Down, input))
    {
      bool assigned = isAssigned();
      bool skippable = formInput.Skippable();
      if (!assigned && !skippable) return;
      restaurePreviousAssignment();
      if (!isAssigned()) setSkipped();
      rowDone();
      return;
    }

    if (isAssigned() && mTargetDevice->IsMatching((RecalboxConf::Instance().GetSwapValidateAndCancel() ? InputDevice::Entry::B : InputDevice::Entry::A), input))
    {
      unAssign();
      setHelpMessage();
      setPress();
      return;
    }

    if (mList->getSelected()->Entry() == InputDevice::Entry::None && mTargetDevice->IsMatching((RecalboxConf::Instance().GetSwapValidateAndCancel() ? InputDevice::Entry::A : InputDevice::Entry::B), input))
    {
      InputManager::Instance().WriteDeviceXmlConfiguration(*mTargetDevice); // save
      if (mDoneCallback)
        mDoneCallback();
      Close();
      return;
    }
  }

  if (isAssigned())
  {
    setHelpMessage();
    setPress();
    return;
  }

  // At first, try to find the preferred type, on the second pass, we ignore the preferred type
  for (auto input: events)
    if ((input.Type() == formInput.PreferedType()) && assign(input)) { rowDone(); return; }
  for (auto input: events)
    if (assign(input)) { rowDone(); break; }
}

void GuiInputConfig::RecordRawInput(const InputEvent& raw)
{
  switch(raw.Type())
  {
    case InputEvent::EventType::Axis:
    {
      // Calculate
      int value = 0;
      if (int neutral = mTargetDevice->NeutralAxisValue(raw.Id()); neutral != 0)
        value = raw.Value() < 0 && neutral > 0 ? 1 : (raw.Value() > 0 && neutral < 0 ? 3 : 0);
      else
        value = raw.Value() < 0 ? 1 : (raw.Value() > 0 ? 3 : 0); // 1 = -1 & 3 = 1 => value - 2 = +/-1

      // Affect?
      int mask = ~(0x3 << (2 * raw.Id()));
      if ((mActiveAxis & ~mask) != 0) // This axis has been activated?
        if (value == 0)
          mEventList.push_back(InputEvent(raw.Device(), raw.Type(), raw.Id(), ((mActiveAxis >> (2 * raw.Id())) & 0x03) - 2));

      // Assign value
      mActiveAxis &= mask;
      mActiveAxis |= value << (2 * raw.Id());
      break;
    }
    case InputEvent::EventType::Button:
    {
      int id = raw.Id() & 0x1F;

      // Affect?
      if (raw.Value() == 0) mEventList.push_back(InputEvent(raw.Device(), raw.Type(), raw.Id(), 1));

      // Assign
      mActiveButtons &= ~(1 << id);
      mActiveButtons |= (raw.Value() << id);

      if (raw.Value() != 0) mButtonReferenceTime[id] = 5000;
      else mButtonReferenceTime[id] = 0;

      break;
    }
    case InputEvent::EventType::Hat:
    {
      int mask = ~(0xF << (4 * raw.Id()));
      int value = raw.Value() << (4 * raw.Id());

      // Affect?
      if ((mActiveHats & ~mask) != 0) // This axis has been activated?
        if (raw.Value() == 0)
          mEventList.push_back(InputEvent(raw.Device(), raw.Type(), raw.Id(), (mActiveHats >> (4 * raw.Id())) & 0x0F));

      // Assign value
      mActiveHats &= mask;
      mActiveHats |= value;
      break;
    }
    case InputEvent::EventType::Unknown:
    case InputEvent::EventType::Key:
    case InputEvent::EventType::MouseButton:
    case InputEvent::EventType::MouseWheel:
    default: break;
  }
}

const HashMap<InputDevice::Entry, Path>& GuiInputConfig::IconPathMap()
{
  static const HashMap<InputDevice::Entry, Path> sIconPathMap =
    {
      { InputDevice::Entry::Up,             Path(":/help/dpad_up.svg") },
      { InputDevice::Entry::Down,           Path(":/help/dpad_down.svg") },
      { InputDevice::Entry::Left,           Path(":/help/dpad_left.svg") },
      { InputDevice::Entry::Right,          Path(":/help/dpad_right.svg") },
      { InputDevice::Entry::Start,          Path(":/help/button_start.svg") },
      { InputDevice::Entry::Select,         Path(":/help/button_select.svg") },
      { InputDevice::Entry::A,              Path(":/help/button_a.svg") },
      { InputDevice::Entry::B,              Path(":/help/button_b.svg") },
      { InputDevice::Entry::X,              Path(":/help/button_x.svg") },
      { InputDevice::Entry::Y,              Path(":/help/button_y.svg") },
      { InputDevice::Entry::L1,             Path(":/help/button_l.svg") },
      { InputDevice::Entry::R1,             Path(":/help/button_r.svg") },
      { InputDevice::Entry::L2,             Path(":/help/button_l2.svg") },
      { InputDevice::Entry::R2,             Path(":/help/button_r2.svg") },
      { InputDevice::Entry::L3,             Path(":/help/button_l3.svg") },
      { InputDevice::Entry::R3,             Path(":/help/button_r3.svg") },
      { InputDevice::Entry::Joy1AxisV,      Path(":/help/joystick_up.svg") },
      { InputDevice::Entry::Joy1AxisH,      Path(":/help/joystick_left.svg") },
      { InputDevice::Entry::Joy2AxisV,      Path(":/help/joystick_up.svg") },
      { InputDevice::Entry::Joy2AxisH,      Path(":/help/joystick_left.svg") },
      { InputDevice::Entry::Hotkey,         Path(":/help/button_hotkey.svg") },
      { InputDevice::Entry::VolumeDown,     Path(":/help/volume_down.svg") },
      { InputDevice::Entry::VolumeUp,       Path(":/help/volume_up.svg") },
      { InputDevice::Entry::BrightnessDown, Path(":/help/brightness_down.svg") },
      { InputDevice::Entry::BrightnessUp,   Path(":/help/brightness_up.svg") },
    };
  return sIconPathMap;
}

void GuiInputConfig::OverlayApply(const Transform4x4f& parentTrans, int leftWidth, int rightWidth, int labelWidth,
                                  const Rectangle& area, FormInput* const& data, int index, unsigned int& color)
{
  (void)parentTrans;
  (void)labelWidth;
  (void)leftWidth;
  (void)rightWidth;
  (void)index;
  if (data->Entry() != InputDevice::Entry::None)
  {
    // Icon
    Texture texture = mIcons[(int) data->Entry()];
    int height = mList->getFont()->Height();
    texture.Render((mIconWidth - texture.Width()) / 2, (Math::roundi(area.Height()) - height) / 2, texture.Width(), height, true, false, color);
    // Text
    if (!data->Action().empty())
      mList->getFont()->DrawText(data->Action(), area, color, ::Alignment::CenterRight);
  }
}

float GuiInputConfig::OverlayGetLeftOffset(FormInput* const& data, int labelWidth)
{
  (void)labelWidth;

  if (data->Entry() == InputDevice::Entry::None) return 0.f;
  return (float)mIconWidth * 1.5f;
}

float GuiInputConfig::OverlayGetRightOffset(FormInput* const& data, int labelWidth)
{
  (void)labelWidth;

  if (data->Entry() == InputDevice::Entry::None) return 0.f;

  return (float)mList->getFont()->TextWidth(data->Action());
}

void GuiInputConfig::Update(int deltaTime)
{
  String msg = _("Press any button for 5s to cancel !");
  for(int& v : mButtonReferenceTime)
    if (v > 0)
      if (v -= deltaTime; v < 4000)
      {
        msg = _("Cancelling...");
        if (v <= 0)
        {
          v = 0;
          mTargetDevice->LoadFrom(mPreviousConfig);
          mCancelled = true;
        }
      }
  if (mCancelled) msg = _("CANCELLED! Please release all buttons.");
  mCancel->setText(msg);
}

void GuiInputConfig::Render(const Transform4x4f& parentTrans)
{
  Component::Render(parentTrans);

  Transform4x4f trans = (parentTrans * getTransform()).round();
  Renderer::SetMatrix(trans);

  int bar = 0;
  for(int& v : mButtonReferenceTime)
    if (v != 0 && v < 4000)
      bar = 5000 - v;

  if (bar != 0 || mCancelled)
  {
    Rectangle area(0.f, mGrid.getRowHeight(0, 3), (mGrid.getColWidth(0) * (mCancelled ? 5000 :  (float)bar)) / 5000.f, mGrid.getRowHeight(4));
    Renderer::DrawRectangle(area, 0xFF000080);
  }
}
