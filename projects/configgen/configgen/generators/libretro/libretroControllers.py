#!/usr/bin/env python
from typing import Dict, List

from configgen import recalboxFiles
from configgen.Emulator import Emulator
from configgen.controllers.JammaLayout import JammaLayout
from configgen.controllers.controller import Controller, InputItem, ControllerPerPlayer
from configgen.crt.CRTTypes import CRTAdapter
from configgen.settings.keyValueSettings import keyValueSettings


class LibretroControllers:

    # Map an emulationstation direction to the corresponding retroarch
    retroarchdirs: Dict[int, str] = \
    {
        InputItem.ItemUp   : 'up',
        InputItem.ItemDown : 'down',
        InputItem.ItemLeft : 'left',
        InputItem.ItemRight: 'right'
    }

    # Map an emulationstation joystick to the corresponding retroarch
    retroarchjoysticks: Dict[int, str] = \
    {
        InputItem.ItemJoy1Up  : 'l_y',
        InputItem.ItemJoy1Left: 'l_x',
        InputItem.ItemJoy2Up  : 'r_y',
        InputItem.ItemJoy2Left: 'r_x'
    }

# The tate mode on a handheld makes joy 2 the first joystick
    retroarchjoysticksTate: Dict[int, str] = \
    {
        InputItem.ItemJoy2Left  : 'l_y',
        InputItem.ItemJoy2Up: 'l_x',
        InputItem.ItemJoy1Left  : 'r_x',
        InputItem.ItemJoy1Up: 'r_y',
    }

    # Map an emulationstation input type to the corresponding retroarch type
    # 6.1: add _ in front of suffixes to allow empty ones
    typetoname: Dict[int, str] = \
    {
        InputItem.TypeButton: '_btn',
        InputItem.TypeHat   : '_btn',
        InputItem.TypeAxis  : '_axis',
        InputItem.TypeKey   : ''  # 6.1: no prefix for keys
    }

    # Map an emulationstation input hat to the corresponding retroarch hat value
    hatstoname: Dict[int, str] = \
    {
        1: 'up',
        2: 'right',
        4: 'down',
        8: 'left'
    }

    # List of controllers that works only with sdl2 driver
    sdl2driverControllers: List[str] = \
    [
        'Bluetooth Wireless Controller',
        'szmy-power',
        'XiaoMi Bluetooth Wireless GameController',
        'ipega Bluetooth Gamepad',
        'ipega Bluetooth Gamepad   ',
        'Moga Pro 2 HID'
    ]

    # Map buttons to the corresponding retroarch specials keys
    retroarchspecialsnomenu: Dict[int, str] = \
    {
        InputItem.ItemX        : 'load_state',
        InputItem.ItemY        : 'save_state',
        InputItem.ItemL1       : 'screenshot',
        InputItem.ItemStart    : 'exit_emulator',
        InputItem.ItemUp       : 'state_slot_increase',
        InputItem.ItemDown     : 'state_slot_decrease',
        InputItem.ItemLeft     : 'rewind',
        InputItem.ItemRight    : 'hold_fast_forward',
        InputItem.ItemL2       : 'shader_prev',
        InputItem.ItemR2       : 'shader_next',
        InputItem.ItemA        : 'reset',
        # Added in 6.1
        InputItem.ItemR3       : 'recording_toggle',
        InputItem.ItemJoy1Left : 'disk_prev',
        InputItem.ItemJoy1Right: 'disk_next',
        InputItem.ItemJoy1Up   : 'disk_eject_toggle',
        InputItem.ItemJoy1Down : 'ai_service',
        InputItem.ItemJoy2Left : 'cheat_index_minus',
        InputItem.ItemJoy2Right: 'cheat_index_plus',
        InputItem.ItemJoy2Up   : 'cheat_toggle',
        InputItem.ItemJoy2Down : 'fps_toggle',
    }

    # Map an emulationstation button name to the corresponding retroarch name
    retroarchbtns: Dict[int, str] = \
    {
        InputItem.ItemA     : 'a',
        InputItem.ItemB     : 'b',
        InputItem.ItemX     : 'x',
        InputItem.ItemY     : 'y',
        InputItem.ItemL1    : 'l',
        InputItem.ItemR1    : 'r',
        InputItem.ItemL2    : 'l2',
        InputItem.ItemR2    : 'r2',
        InputItem.ItemL3    : 'l3',
        InputItem.ItemR3    : 'r3',
        InputItem.ItemStart : 'start',
        InputItem.ItemSelect: 'select'
    }

    # Tate buttons
    retroarchbtnsTate: Dict[int, str] = \
    {
        InputItem.ItemA     : 'b',
        InputItem.ItemB     : 'y',
        InputItem.ItemX     : 'a',
        InputItem.ItemY     : 'x',
        InputItem.ItemL1    : 'l',
        InputItem.ItemR1    : 'r',
        InputItem.ItemL2    : 'l2',
        InputItem.ItemR2    : 'r2',
        InputItem.ItemL3    : 'l3',
        InputItem.ItemR3    : 'r3',
        InputItem.ItemStart : 'start',
        InputItem.ItemSelect: 'select'
    }

    # Retroarch buttons for megadrive
    retroarchmegadrivebtns: Dict[int, str] = \
    {
        InputItem.ItemA     : 'b',
        InputItem.ItemB     : 'y',
        InputItem.ItemX     : 'x',
        InputItem.ItemY     : 'a',
        InputItem.ItemL1    : 'l',
        InputItem.ItemR1    : 'r',
        InputItem.ItemL2    : 'l2',
        InputItem.ItemR2    : 'r2',
        InputItem.ItemL3    : 'l3',
        InputItem.ItemR3    : 'r3',
        InputItem.ItemStart : 'start',
        InputItem.ItemSelect: 'select'
    }
    # Retroarch buttons for megadrive 6 btn
    retroarchmegadrive6btns: Dict[int, str] = \
    {
        InputItem.ItemA     : 'b',
        InputItem.ItemB     : 'y',
        InputItem.ItemX     : 'x',
        InputItem.ItemY     : 'l',
        InputItem.ItemL1    : 'r',
        InputItem.ItemR1    : 'a',
        InputItem.ItemL2    : 'l2',
        InputItem.ItemR2    : 'r2',
        InputItem.ItemL3    : 'l3',
        InputItem.ItemR3    : 'r3',
        InputItem.ItemStart : 'start',
        InputItem.ItemSelect: 'select'
    }
    # Retroarch buttons for Dreamcast without L2/R2
    retroarchdreamcastbtns: Dict[int, str] = \
    {
        InputItem.ItemA     : 'a',
        InputItem.ItemB     : 'b',
        InputItem.ItemX     : 'x',
        InputItem.ItemY     : 'y',
        InputItem.ItemL1    : 'l2',
        InputItem.ItemR1    : 'r2',
        InputItem.ItemL2    : 'l',
        InputItem.ItemR2    : 'r',
        InputItem.ItemL3    : 'l3',
        InputItem.ItemR3    : 'r3',
        InputItem.ItemStart : 'start',
        InputItem.ItemSelect: 'select'
    }

    def __init__(self, system: Emulator, recalboxOptions: keyValueSettings, retroarchConfig: keyValueSettings, coreSettings: keyValueSettings, remap: keyValueSettings, controllers: ControllerPerPlayer, nodefaultkeymap: bool, rom: str):
        self.system: Emulator = system
        self.recalboxOptions: keyValueSettings = recalboxOptions
        self.settings: keyValueSettings = retroarchConfig
        self.coreSettings: keyValueSettings = coreSettings
        self.controllers: ControllerPerPlayer = controllers
        self.remap: keyValueSettings = remap
        self.nodefaultkeymap: bool = nodefaultkeymap
        self.rom: str = rom
        self.retroarchspecials: Dict[int, str] = dict(LibretroControllers.retroarchspecialsnomenu)
        self.retroarchspecials[InputItem.ItemB] = 'menu_toggle'
        self.jammaspecials: Dict[int, str] = {}
        if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
            jammaSettings = keyValueSettings("/boot/crt/recalbox-crt-options.cfg", extraSpaces=True).loadFile()
            if jammaSettings.getString("options.jamma.controls.hk_on_start", "1") == "0":
                # Need to remove the hotkeys shortcuts from configgen, as the driver always have START + LEFT RIGHT triggering the hotkey (because of clones folded)
                self.jammaspecials: Dict[int, str] = {InputItem.ItemStart: 'exit_emulator'}
            else:
                self.jammaspecials: Dict[int, str] = dict(LibretroControllers.retroarchspecialsnomenu)
                self.jammaspecials[InputItem.ItemA] = 'menu_toggle'

        self.button_mapping = {}
        inputDriver: str = self.recalboxOptions.getString("global.inputdriver", self.recalboxOptions.getString(self.system.Name + ".inputdriver", 'auto'))
        for player in controllers:
            if inputDriver == "sdl2":
                self.button_mapping[controllers[player].SdlIndex] = self._MapSdl2ControllerButtons(controllers[player])
            else:
                self.button_mapping[controllers[player].UdevIndex] = self._MapUdevControllerButtons(controllers[player])

    # Define the libretro device type corresponding to the libretro cores, when needed.
    # Not a list but a dict to start from 1 as controllers
    CORE_TO_PLAYER_DEVICE: Dict[int, Dict[str, int]] = {
        1: {
            'cap32': 513,
            '81': 257,
            'fuse': 513,
            'atari800': 513
        },
        2: {
            'fuse': 513,
            'atari800': 513,
            '81': 259,
        },
        3: {
            'fuse': 259,
        }
    }

    SPINNER_WITH_FIVE_BTN = [
        "TAITO_USB_Paddle___Trackball_Controller",
        "Arduino_Leonardo"
    ]

    SPINNER = [
        "BL_USB_spinner_#1"
    ]

    # Fill controllers configuration
    def fillControllersConfiguration(self) -> (keyValueSettings, keyValueSettings):
        inputDriver: str = self.recalboxOptions.getString("global.inputdriver", self.recalboxOptions.getString(self.system.Name + ".inputdriver", 'auto'))

        # Cleanup all
        self.cleanUpControllers()
        self.remap.clear()

        # Build configurations
        for player in self.controllers:
            self.buildController(self.controllers[player], player, self.system, self.remap)

        # Snes9x and snes9x2010 remap specific
        if len(self.controllers) > 2 and (self.system.Core == 'snes9x' or self.system.Core == 'snes9x2010'):
            self.remap.setInt("input_libretro_device_p2", 257)

        # Set Hotkey
        self.setHotKey()

        # Dolphin remap specific
        if self.system.Core == 'dolphin':
            from configgen.generators.dolphin.dolphinRomType import DolphinRomType
            import configgen.generators.dolphin.dolphinControllers as DolphinControllers
            gameid = DolphinRomType.extract_gameid(self.rom)

            if gameid and (gameid not in DolphinControllers.sidewaysWiiGames or gameid[0].upper() not in DolphinControllers.wiiVirtualConsoles):
                # We should define to use the controller type 'WiiMote (sideways)' but RetroArch inverted them
                # For now, if a game is NOT in those list, define to use 'WiiMote (sideways)' even if those games are not sideways
                # Configure for all users currently, not sure if it can be made more granular (specific controller for specific player)
                for player in self.controllers:
                    self.remap.setInt("input_libretro_device_p{}".format(player), 769)

        # Set driver
        if inputDriver == "auto": inputDriver = self.getInputDriver()
        self.settings.setString("input_joypad_driver", inputDriver)

        return self.settings, self.remap

    # Find the best driver for controllers
    def getInputDriver(self) -> str:
        for controller in self.controllers:
            for controllerName in self.sdl2driverControllers:
                if controllerName in self.controllers[controller].DeviceName:
                    return "sdl2"
        return "udev"

    # Remove all controller configurations
    def cleanUpControllers(self):
        self.settings.removeOptionStartingWith("input_player")
        self.settings.removeOption("input_enable_hotkey_mbtn")
        self.settings.removeOption("input_exit_emulator_mbtn")
        for specialkey in self.retroarchspecials.values():
            self.settings.removeOptionStartingWith("input_" + specialkey)

    # Write the hotkey for player 1
    def setHotKey(self):
        if 1 in self.controllers:
            if self.controllers[1].HasHotkey:
                if self.getInputDriver() == "sdl2":
                    self.settings.setInt("input_enable_hotkey_btn", self.getConfigValue(self.controllers[1].SdlIndex, self.controllers[1].Hotkey))
                else:
                    self.settings.setInt("input_enable_hotkey_btn", self.getConfigValue(self.controllers[1].UdevIndex, self.controllers[1].Hotkey))

    # Return the retroarch analog_dpad_mode
    @staticmethod
    def getAnalogMode(controller: Controller, system: Emulator) -> int:
        # if system.Name != 'psx':
        for dirkey in LibretroControllers.retroarchdirs:
            if controller.HasInput(dirkey):
                if controller.Input(dirkey).Type in (InputItem.TypeButton, InputItem.TypeHat):
                    if system.Name in ('atomiswave', 'naomi2', 'naomigd'):
                        return 3
                    return 1
        return 0

    # Return the playstation analog mode for a controller
    @staticmethod
    def getAnalogCoreMode(controller: Controller) -> str:
        for dirkey in LibretroControllers.retroarchdirs:
            if controller.HasInput(dirkey):
                if controller.Input(dirkey).Type in (InputItem.TypeButton, InputItem.TypeHat):
                    return 'analog'
        return 'standard'

    # Returns the value to write in retroarch config file, depending on the type
    def getConfigValue(self, controller_id: int, inp: InputItem) -> str:
        if inp.IsButton or inp.IsKey:
            if controller_id not in self.button_mapping or inp.Code not in self.button_mapping[controller_id]:
                print(f"warning: wrong controller_id ({controller_id}) or inp.Code ({inp.Code})")
                print("usually the reason is a bad button mapping")
                return 99
            print(f"controller {controller_id} code evdev {inp.Code} mapped to ra id {self.button_mapping[controller_id][inp.Code]}")
            return self.button_mapping[controller_id][inp.Code]
        if inp.IsAxis: return ('-' if inp.Value < 0 else '+') + str(inp.Id)
        if inp.IsHat : return 'h' + str(inp.Id) + LibretroControllers.hatstoname[inp.Value]
        raise TypeError

    # May change the sign of a joystick axis if rotated
    @staticmethod
    def getJoystickSignRotated(rotate: bool, standardAxis: str, jskey: int) -> str:
        if rotate:
            if jskey == InputItem.ItemJoy2Up and standardAxis == "-":
                return "+"
            if jskey == InputItem.ItemJoy2Up and standardAxis == "+":
                return "-"
        return standardAxis

    # Write a configuration for a specified controller
    def buildController(self, controller: Controller, playerIndex: int, system: Emulator, remapConfig: keyValueSettings):
        settings = self.settings

        is_jamma = controller.DeviceName.startswith("JammaController")

        if self.getInputDriver == "sdl2":
            inputIndex = controller.SdlIndex
        else:
            inputIndex = controller.UdevIndex
        # Get specials string or default
        specials = self.system.SpecialKeys
        print(f"player n°{playerIndex} controller n°{inputIndex} ({controller.DeviceName} at {controller.DevicePath})")
        # config['input_device'] = '"%s"' % controller.RealName
        btnmap = self.retroarchbtns
        # Special case for megadrive and jamma
        if is_jamma and system.Name == "megadrive":
            if system.JammaLayoutP1 == JammaLayout.SixBtn:
                btnmap = self.retroarchmegadrive6btns
            else:
                btnmap = self.retroarchmegadrivebtns

        # Define L2/R2 on L1/R1 if controllers don't have L2/R2 for Dreamcast
        if not controller.HasL2 and not controller.HasR2 and system.Name == "dreamcast" and not is_jamma:
            btnmap = self.retroarchdreamcastbtns

        if system.RotateControls:
            btnmap = self.retroarchbtnsTate
        for btnkey in btnmap:
            btnvalue = btnmap[btnkey]
            if controller.HasInput(btnkey):
                inp: InputItem = controller.Input(btnkey)
                settings.setString("input_player%s_%s%s" % (controller.PlayerIndex, btnvalue, self.typetoname[inp.Type]),
                                   self.getConfigValue(inputIndex, inp))
        for dirkey in self.retroarchdirs:
            dirvalue = self.retroarchdirs[dirkey]
            if controller.HasInput(dirkey):
                inp = controller.Input(dirkey)
                settings.setString("input_player%s_%s%s" % (controller.PlayerIndex, dirvalue, self.typetoname[inp.Type]),
                                   self.getConfigValue(inputIndex, inp))
        for jskey in self.retroarchjoysticks:
            jsvalue = self.retroarchjoysticks[jskey] if not system.RotateControls else self.retroarchjoysticksTate[jskey]
            if controller.HasInput(jskey):
                inp = controller.Input(jskey)
                # handle inverted axis
                minus = "-" if inp.Value < 0 else "+"
                plus = "+" if inp.Value < 0 else "-"
                settings.setString("input_player%s_%s_minus_axis" % (controller.PlayerIndex, jsvalue), self.getJoystickSignRotated(system.RotateControls, minus, jskey) + str(inp.Id))
                settings.setString("input_player%s_%s_plus_axis" % (controller.PlayerIndex, jsvalue),  self.getJoystickSignRotated(system.RotateControls, plus, jskey) + str(inp.Id))

        if controller.PlayerIndex == 1:
            specialMap: Dict[int, str] = {}
            # No menu always priority
            if specials == "nomenu":
                specialMap = self.retroarchspecialsnomenu
            elif specials == "default":
                    specialMap = self.jammaspecials if is_jamma else self.retroarchspecials
            for item in specialMap:
                if controller.HasInput(item):
                    value: str = specialMap[item]
                    inp = controller.Input(item)
                    settings.setString("input_%s%s" % (value, self.typetoname[inp.Type]), self.getConfigValue(inputIndex, inp))
            if controller.HasStart:
                specialvalue = self.retroarchspecials[InputItem.ItemStart]
                inp = controller.Start
                settings.setString("input_%s%s" % (specialvalue, self.typetoname[inp.Type]), self.getConfigValue(inputIndex, inp))

        # No default keymap?
        if self.nodefaultkeymap:
            for btnkey in self.retroarchbtns:
                settings.setString("input_player%s_%s" % (controller.PlayerIndex, self.retroarchbtns[btnkey]), '"nul"')
            for dirkey in self.retroarchdirs:
                settings.setString("input_player%s_%s" % (controller.PlayerIndex, self.retroarchdirs[dirkey]), '"nul"')
        else:
            for btnkey in self.retroarchbtns:
                settings.removeOption("input_player%s_%s" % (controller.PlayerIndex, self.retroarchbtns[btnkey]))
            for dirkey in self.retroarchdirs:
                settings.removeOption("input_player%s_%s" % (controller.PlayerIndex, self.retroarchdirs[dirkey]))
        settings.setString("input_enable_hotkey", '"f12"')
        settings.setString("input_exit_emulator", '"escape"')

        # Assign pad to player
        settings.setInt("input_player{}_joypad_index".format(playerIndex), inputIndex)

        # Specific configuration for Nintendo Switch N64 Controller
        if controller.DeviceName in ['Nintendo Switch N64 Controller', 'N64 Controller']:
            settings.setString("input_player{}_r_x_plus_btn".format(playerIndex), '"2"')    # C-Right
            settings.setString("input_player{}_r_x_minus_btn".format(playerIndex), '"4"')   # C-Left
            settings.setString("input_player{}_r_y_plus_btn".format(playerIndex), '"3"')    # C-Down
            settings.setString("input_player{}_r_y_minus_btn".format(playerIndex), '"10"')  # C-Up

        # remap config
        print("[libretroControllers] setting input_libretro_device_p{}={}".format(playerIndex, 1))
        remapConfig.setInt("input_libretro_device_p{}".format(playerIndex), 1)
        remapConfig.setString("input_player{}_analog_dpad_mode".format(playerIndex),
                              '"{}"'.format(self.getAnalogMode(controller, system)))
        remapConfig.setString("input_remap_port_p{}".format(playerIndex), '"{}"'.format(playerIndex-1))

        if is_jamma:
            remapConfig.setString("input_libretro_device_p{}".format(playerIndex), system.JammaLayoutP2.toRetroarchDeviceType(system.Name) if playerIndex == 2 else system.JammaLayoutP1.toRetroarchDeviceType(system.Name) )
        else:
            if playerIndex in self.CORE_TO_PLAYER_DEVICE and system.Core in self.CORE_TO_PLAYER_DEVICE[playerIndex]:
                remapConfig.setInt("input_libretro_device_p{}".format(playerIndex),
                                   self.CORE_TO_PLAYER_DEVICE[playerIndex][system.Core])
            # Rumble
            if not is_jamma:
                rumbleRemapConfig = {
                    "pcsx_rearmed"   : ['517', '1'],
                    "mednafen_psx_hw": ['517', '1'],
                    "mednafen_psx"   : ['517', '1'],
                    "swanstation"    : ['261', '1']
                }

                rumbleCoreConfig = {
                    "dolphin":
                        [
                            {"dolphin_enable_rumble": '"enabled"'},
                            {"dolphin_enable_rumble": '"disabled"'}
                        ],
                    "flycast":
                        [
                            {"reicast_enable_purupuru": '"enabled"'},
                            {"reicast_enable_purupuru": '"disabled"'}
                        ],
                    "flycast-next":
                        [
                            {"reicast_enable_purupuru": '"enabled"'},
                            {"reicast_enable_purupuru": '"disabled"'}
                        ],
                    "mupen64plus_next":
                        [
                            {"mupen64plus-pak1": '"rumble"'},
                            {"mupen64plus-pak1": '"memory"'}
                        ],
                    "parallel_n64":
                        [
                            {"parallel-n64-pak1": '"rumble"'},
                            {"parallel-n64-pak1": '"memory"'}
                        ],
                    "pcsx_rearmed":
                        [
                            {"pcsx_rearmed_vibration": '"enabled"'},
                            {"pcsx_rearmed_vibration": '"disabled"'}
                        ],
                    "pcsx2":
                        [
                            {"pcsx2_rumble_enable": '"enabled"'},
                            {"pcsx2_rumble_enable": '"disabled"'}
                        ],
                    "swanstation":
                        [
                            {"swanstation_Controller_EnableRumble": '"true"'},
                            {"swanstation_Controller_EnableRumble": '"false"'}
                        ]
                }

                if system.Core in rumbleRemapConfig:
                    remapConfig.setInt("input_libretro_device_p{}".format(playerIndex),
                                       rumbleRemapConfig[system.Core][0 if system.Rumble else 1])

                if system.Core in rumbleCoreConfig:
                    for config in rumbleCoreConfig[system.Core][0 if system.Rumble else 1].items():
                        self.coreSettings.setString(config[0], config[1])

        # Configure Spinner and Trackball
        if self.system.Core == "fbneo" or self.system.Name == "mame":
            import os
            import re
            mouse_controllers = list()
            pattern = re.compile("mouse[0-9]*")
            dir = "/dev/input/"
            if os.path.isdir(dir):
                for filepath in os.listdir(dir):
                    if pattern.match(filepath):
                        mouse_controllers.append(filepath)

            if len(mouse_controllers) >= playerIndex:
                import pyudev
                context = pyudev.Context()
                mouse = pyudev.Devices.from_name(context, 'input', mouse_controllers[(playerIndex - 1)])
                if mouse.get("ID_MODEL") in self.SPINNER:
                    if self.system.Core == "fbneo":
                        remapConfig.setInt("input_libretro_device_p{}".format(playerIndex), 773)
                        self.coreSettings.setString("fbneo-analog-speed", '"25%"')
                elif mouse.get("ID_MODEL") in self.SPINNER_WITH_FIVE_BTN:
                    if self.system.Core == "fbneo":
                        remapConfig.setInt("input_libretro_device_p{}".format(playerIndex), 514)
                        self.coreSettings.setString("fbneo-analog-speed", '"25%"')
                    settings.setString("input_player{}_start_mbtn".format(playerIndex), '"5"')
                    settings.setString("input_player{}_select_mbtn".format(playerIndex), '"4"')
                    settings.setString("input_enable_hotkey_mbtn".format(playerIndex), '"3"')
                    settings.setString("input_exit_emulator_mbtn".format(playerIndex), '"5"')

    @staticmethod
    def _MapSdl2ControllerButtons(controller: Controller) -> dict:
        button_mapping = {}
        # scan BTN_MISC to KEY_MAX then
        for BTN in controller.AvailableInput:
            if BTN.IsButton:
                button_mapping[BTN.Code] = BTN.Id

        print(f"SDL2 button mapping summary for {controller.DeviceName} ({controller.DevicePath}):")
        for k in button_mapping:
            print(f"- input evdev code {k} mapped to SDL2 button id {button_mapping[k]}")
        return button_mapping

    @staticmethod
    def _MapUdevControllerButtons(controller: Controller) -> dict:
        button_id = 0
        button_mapping = {}
        # scan KEY_UP to KEY_DOWN first
        for KEY in list(range(103, 108 + 1)):
            if controller.HasKey(KEY):
                button_mapping[KEY] = button_id
                button_id += 1
        # scan BTN_MISC to KEY_MAX then
        for BTN in list(range(256, 767 + 1)):
            if controller.HasKey(BTN):
                button_mapping[BTN] = button_id
                button_id += 1
        # The following two ranges are scanned and added after the above
        # ranges to maintain compatibility with existing key maps.
        for MSC in list(range(0, 103)):
            if controller.HasKey(MSC):
                button_mapping[MSC] = button_id
                button_id += 1
        for MSC in list(range(109, 256)):
            if controller.HasKey(MSC):
                button_mapping[MSC] = button_id
                button_id += 1
        print(f"Udev button mapping summary for {controller.DeviceName} ({controller.DevicePath}):")
        for k in button_mapping:
            print(f"- input evdev code {k} mapped to retroarch button id {button_mapping[k]}")
        return button_mapping
