//
// Created by bkg2k on 12/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/base/Menu.h>
#include <guis/GuiWaitLongExecution.h>
#include <input/InputMapper.h>
#include <btautopair/BTAutopairManager.h>

// Forward declaration
class SystemManager;
class SystemData;

class MenuPads : public Menu
                  , ILongExecution<bool, String::List>
                  , IInputChange
                  , ISingleSelectorChanged<int>
                  , ISingleSelectorChanged<String>
                  , ISingleSelectorChanged<RecalboxConf::PadOSDType>
                  , ISwitchChanged
                  , ISubMenuSelected
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit MenuPads(WindowManager& window);

    //! Build menu items
    void BuildMenuItems() final;

    // Destructor
    ~MenuPads();

  private:
    enum class Components
    {
      Pads,
      Configure = (int)Pads + Input::sMaxInputDevices,
      Pair,
      Unpair,
      Driver,
      PadOSD,
      PadOSDType,
      AutoPairOnBoot,
      Rumble,
    };

    //! Pad lists
    ItemSelector<int>* mDevices[Input::sMaxInputDevices];

    //! Pad Mapper
    InputMapper& mMapper;

    //! Refreshing component, ignore change event
    volatile bool mRefreshing;

    //! MQTT Topic
    static constexpr const char* sEventTopic = "bluetooth/operation";

    //! Called when the user start configuring a pad
    void StartConfiguring();

    //! Detect the device to configure
    static void RunDeviceDetection(MenuPads* thiz);

    //! Refresh the device list
    void RefreshDevices();

    //! Get device list
    SelectorEntry<int>::List GetDeviceList();

    //! Called when the user starts bluetooth discovery
    void StartScanningDevices();

    //! Unpair all device
    void UnpairAll();

    //! Get modes
    static SelectorEntry<RecalboxConf::PadOSDType>::List GetPadOSDType();

    /*
     * ILongExecution
     */

    /*!
     * @brief Scan bluetooth devices
     * @param from Source interface
     * @param parameter Useless "in" parameter
     * @return List of available devices
     */
    String::List Execute(GuiWaitLongExecution<bool, String::List>& from, const bool& parameter) override;

    /*!
     * @brief Called when the device scan is complete
     * @param parameter Useless "in" parameter
     * @param result List of available devices
     */
    void Completed(const bool& parameter, const String::List& result) override;

    /*
     * IInputChange implementation
     */

    //! Refresh pad list
    void PadsAddedOrRemoved(bool removed) override;

    /*
     * ISubMenuSelected implementation
     */

    void SubMenuSelected(const ItemSubMenu& item, int id) override;

    /*
     * IOptionListComponent<int> implementation
     */

    void MenuSingleChanged(ItemSelectorBase<int>& item, int id, int index, const int& value) override;

    /*
     * IOptionListComponent<String> implementation
     */

    void MenuSingleChanged(ItemSelectorBase<String>& item, int id, int index, const String& value) override;

    /*
     * IOptionListComponent<String> implementation
     */

    void MenuSingleChanged(ItemSelectorBase<RecalboxConf::PadOSDType>& item, int id, int index, const RecalboxConf::PadOSDType& value) override;

    /*
     * ISwitchChanged implementation
     */
    void MenuSwitchChanged(const ItemSwitch& item, bool& status, int id) override;
};



