#include <utils/String.h>
#include <guis/GuiDetectDevice.h>
#include <WindowManager.h>
#include <Renderer.h>
#include <guis/GuiInputConfig.h>
#include <components/TextComponent.h>
#include <utils/locale/LocaleHelper.h>
#include <themes/MenuThemeData.h>
#include <themes/ThemeManager.h>

#define HOLD_TIME 1000

GuiDetectDevice::GuiDetectDevice(WindowManager& window, bool firstRun, const std::function<void()>& doneCallback)
  : Gui(window),
    mFirstRun(firstRun),
    mHoldingConfig(nullptr),
    mHoldTime(0),
    mBackground(window, Path(":/frame.png")),
    mGrid(window, Vector2i(1, 5)),
    mDoneCallback(doneCallback)
{

 	const MenuThemeData& menuTheme = ThemeManager::Instance().Menu();
	mColor = menuTheme.Text().color & 0xFFFFFF00;
	
	mBackground.setImagePath(menuTheme.Background().path);
	mBackground.setCenterColor(menuTheme.Background().color);
	mBackground.setEdgeColor(menuTheme.Background().color);
	
	addChild(&mBackground);
	addChild(&mGrid);
	
	// title
	mTitle = std::make_shared<TextComponent>(mWindow, firstRun ? _("WELCOME") : _("CONFIGURE INPUT"), menuTheme.Title().font, menuTheme.Title().color, ::Alignment::Center);
	//mGrid.setEntry(mTitle, Vector2i(0, 0), false, true, Vector2i(1, 1), GridFlags::BORDER_BOTTOM);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);
	
	// device info
	String deviceInfo;
	int numDevices = InputManager::Instance().ConfigurableDeviceCount();
	
	if(numDevices > 0) deviceInfo = _N("%i GAMEPAD DETECTED", "%i GAMEPADS DETECTED", numDevices).Replace("%i", String(numDevices));
	else               deviceInfo = _("NO GAMEPADS DETECTED");
	mDeviceInfo = std::make_shared<TextComponent>(mWindow, deviceInfo, menuTheme.SmallText().font, menuTheme.SmallText().color, ::Alignment::Center);
	mGrid.setEntry(mDeviceInfo, Vector2i(0, 1), false, true);

	// message
	mMsg1 = std::make_shared<TextComponent>(mWindow, _("HOLD A BUTTON ON YOUR DEVICE TO CONFIGURE IT."), menuTheme.SmallText().font, menuTheme.SmallText().color, ::Alignment::Center);
	mGrid.setEntry(mMsg1, Vector2i(0, 2), false, true);

  mMsg2 = std::make_shared<TextComponent>(mWindow, _("PRESS ESC OR THE HOTKEY TO CANCEL."), menuTheme.SmallText().font, menuTheme.SmallText().color, ::Alignment::Center);
	mGrid.setEntry(mMsg2, Vector2i(0, 3), false, true);

	// currently held device
	mDeviceHeld = std::make_shared<TextComponent>(mWindow, "", menuTheme.Text().font, mColor, ::Alignment::Center);
	mGrid.setEntry(mDeviceHeld, Vector2i(0, 4), false, true);

	float y= ((mMsg1->getFont()->Height() * 4.0f) + mTitle->getFont()->Height() ) / Renderer::Instance().DisplayHeightAsFloat() + 0.08f;

	setSize(Renderer::Instance().Is480pOrLower() ? Renderer::Instance().DisplayWidthAsFloat() * 0.9f : Renderer::Instance().DisplayWidthAsFloat() * 0.6f, Renderer::Instance().DisplayHeightAsFloat() * y);
	setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);
}

void GuiDetectDevice::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	// grid
	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, mTitle->getFont()->Height() / mSize.y());
	mGrid.setRowHeightPerc(1, mDeviceInfo->getFont()->Height() / mSize.y());
	mGrid.setRowHeightPerc(2, mMsg1->getFont()->Height() / mSize.y());
	mGrid.setRowHeightPerc(3, mMsg2->getFont()->Height() / mSize.y());
	mGrid.setRowHeightPerc(4, mDeviceHeld->getFont()->Height() *1.5f / mSize.y());
}


bool GuiDetectDevice::ProcessInput(const InputCompactEvent& event)
{
  if (!mFirstRun && (event.KeyCode() == SDLK_ESCAPE || event.HotkeyPressed()))
  {
    // cancel configuring
    Close();
    return true;
  }

  if (event.Device().IsConfigurable())
  {
    if (event.RawEvent().AnyButtonPressed() && mHoldingConfig == nullptr)
    {
      // started holding
      mHoldingConfig = &event.Device();
      mHoldTime = HOLD_TIME;
      mDeviceHeld->setText(event.Device().Name().ToUpperCase());
    }
    else if (event.RawEvent().AnyButtonReleased() && mHoldingConfig == &event.Device())
    {
      // cancel
      mHoldingConfig = nullptr;
      mDeviceHeld->setText("");
      mAlpha = 0;
    }
  }

	return true;
}

void GuiDetectDevice::Update(int deltaTime)
{
	if(mHoldingConfig != nullptr)
	{
		mHoldTime -= deltaTime;
		int t = HOLD_TIME / deltaTime;
		mAlpha += 255 / t;
		mDeviceHeld->setColor(mColor | mAlpha);
		if(mHoldTime <= 0)
		{
			// picked one!
			mWindow.pushGui(new GuiInputConfig(mWindow, mHoldingConfig, mDoneCallback));
			Close();
		}
	}
}
