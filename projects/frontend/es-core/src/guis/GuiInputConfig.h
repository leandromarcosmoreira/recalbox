#pragma once

#include <guis/Gui.h>
#include <components/NinePatchComponent.h>
#include <components/ComponentGrid.h>
#include <components/ComponentList.h>
#include <input/InputStack.h>
#include <utils/locale/LocaleHelper.h>
#include <input/IInputChange.h>
#include "components/TextScrollComponent.h"
#include "components/ProgressBarComponent.h"
#include <components/TextListComponent.h>

class TextComponent;

class FormInput
{
  public:
    FormInput(InputDevice::Entry entry, const String& inLabel, bool inSkippable, InputEvent::EventType inPreferredType)
      : mEntry(entry),
        mLabel(inLabel),
        mSkippable(inSkippable),
        mPreferredType(inPreferredType)
    {
    };

    /*
     * Accessors
     */
    [[nodiscard]] InputDevice::Entry Entry() const { return mEntry; }
    [[nodiscard]] String Label() const { return mLabel; }
    [[nodiscard]] String Action() const { return mAction; }
    [[nodiscard]] bool Skippable() const { return mSkippable; }
    [[nodiscard]] InputEvent::EventType PreferedType() const { return mPreferredType; }

    /*
     * Setters
     */

    void SetAction(const String& action) { mAction = action; }

  private:
    const InputDevice::Entry mEntry;
    const String mLabel;
    String mAction;
    const bool mSkippable;
    const InputEvent::EventType mPreferredType;
};

class GuiInputConfig : public Gui
                     , private IInputChange
//                     , private ITextListComponentInterface<FormInput*>
                     , private ITextListComponentOverlay<FormInput*>
{
  public:
    GuiInputConfig(WindowManager& window, InputDevice* target, const std::function<void()>& doneCallback);
    ~GuiInputConfig() override;
    void onSizeChanged() override;
      bool CollectHelpItems(Help& help) override;

  private:
    //! Max axis
    static constexpr int sMaxAxis = 16;
    static constexpr int sMaxButtons = 32;

    void setMapped() { setText(_("ALREADY TAKEN")); }
    void setSkipped() { setText(_("(skipped)")); }
    void setNotDefined() { setText(""); }
    void setAssignedTo(InputEvent input) { setText(input.ToString().UpperCase()); }

    void initFormInputs();
    void addFormInput(InputDevice::Entry entry, const String& label, bool skippable, InputEvent::EventType preferredType);

    void setHelpMessage();
    void setPress();
    void setText(const String& msg);

    bool assign(InputEvent input);
    void unAssign();
    void restaurePreviousAssignment();
    bool isAssigned();

    void rowDone();

    void Render(const Transform4x4f& parentTrans) override;
    bool ProcessInput(const InputCompactEvent& event) override;
    void Update(int deltaTime) override;

    NinePatchComponent mBackground;
    ComponentGrid mGrid;

    InputDevice mPreviousConfig;

    std::function<void()> mDoneCallback;

    std::vector< FormInput > mFormInputs;

    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextScrollComponent> mSubtitle1;
    std::shared_ptr<TextComponent> mSubtitle2;
    std::shared_ptr<TextListComponent<FormInput*>> mList;
    std::vector< std::shared_ptr<TextComponent> > mMappings;
    std::shared_ptr<TextScrollComponent> mCancel;

    InputDevice* mTargetDevice;

    std::vector<InputEvent> mEventList; //!< Recorded events
    int mActiveButtons;                 //!< Active (pressed) buttons
    int mActiveAxis;                    //!< Active (far enough from their original position) axis
    int mActiveHats;                    //!< Active (non zero) hats
    int mButtonReferenceTime[sMaxButtons]; // Time record
    int mIconWidth;
    bool mNeutralPositionSet;           //!< Neutral position has been reached, we can start recording
    bool mCursorOnList;
    bool mFirstEventReceived;           //!< First event received flag
    unsigned int mMainColor;
    bool mCancelled;

    //! Images for every single help type
    Texture mIcons[(int)HelpType::__Count + 1];

    /*!
     * @brief Analyse and record raw event
     * @param raw raw event
     */
    void RecordRawInput(const InputEvent& raw);

    //! Check if the current pad is in neutral position
    [[nodiscard]] bool NeutralPosition() const { return (mActiveHats | mActiveButtons | mActiveAxis) == 0; }

    /*!
     * @brief Process all received events
     */
    void ProcessEvents();

    //! Internal image map
    static const HashMap<InputDevice::Entry, Path>& IconPathMap();

    /*
     * IInputChange implementation
     */

    //! Refresh pad list
    void PadsAddedOrRemoved(bool removed) override { if (removed) Close(); };

    /*!
     * @brief Apply (draw) an overlay in the given item rectangle and adjust rectangle position/size
     * so that the text won't draw over the overlay if required
     * @param parentTrans Parent transform matrice
     * @param leftWidth Left overlay width
     * @param rightWidth Right overlay width
     * @param labelWidth Cell's label width in pixel
     * @param area item rectangle
     * @param data Linked data
     * @param color draw color
     */
    void OverlayApply(const Transform4x4f& parentTrans, int leftWidth, int rightWidth, int labelWidth, const Rectangle& area, FormInput* const& data, int index, unsigned int& color) final;

    /*!
     * @brief Get the left offset (margin to the text) if any
     * @param data Linked data
     * @param labelWidth Cell's label width in pixel
     * @return left offset
     */
     float OverlayGetLeftOffset(FormInput* const& data, int labelWidth) final;

    /*!
     * @brief Get the right offset (margin from text to right limit) if any
     * @param data Linked data
     * @param labelWidth Cell's label width in pixel
     * @return right offset
     */
    float OverlayGetRightOffset(FormInput* const& data, int labelWidth) final;
};
