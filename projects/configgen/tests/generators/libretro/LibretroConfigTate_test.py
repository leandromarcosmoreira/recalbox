import os
from typing import Optional, Dict
from unittest.mock import mock_open

import pytest

from configgen.Emulator import Emulator, ExtraArguments
import configgen.controllers.controller as controllersConfig
from configgen.crt.CRTConfigParser import CRTConfigParser
from configgen.crt.CRTTypes import CRTSystemMode, CRTArcadeMode
from configgen.crt.CRTModeOffsetter import CRTModeOffsetter
from configgen.generators.libretro.crt.LibretroConfigCRT import LibretroConfigCRT
from configgen.generators.libretro.libretroControllers import LibretroControllers
from configgen.generators.libretro.libretroGenerator import LibretroGenerator
from configgen.settings.keyValueSettings import keyValueSettings


@pytest.fixture
def system_fbneo():
    return Emulator(name='fbneo', videoMode='1920x1080', ratio='auto', emulator='libretro',
                    core='fbneo')
@pytest.fixture
def system_wswanc():
    return Emulator(name='wswanc', videoMode='1920x1080', ratio='auto', emulator='libretro',
                    core='mednafen_wswan')


def configureForTate(system: Emulator, rotation=0, vertical_game=True, rotatecontrols=False, crtscreentype: str="15kHz"):
    opts = keyValueSettings("")
    extraArgs = ExtraArguments('1920x1080', "", "", "", "", "", "", "", "", "", "", crtscreentype, "", rotation=rotation,
                               rotatecontrols=rotatecontrols, verticalgame=vertical_game, )
    system.configure(opts, extraArgs)


def test_given_rotation_0_in_cli_then_dont_rotate(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=0)
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert config["video_allow_rotate"] == '"true"'
    assert config["video_rotation"] == 0


def test_given_rotation_1_in_cli_then_rotate(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=1)
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert config["video_allow_rotate"] == '"true"'
    assert config["video_rotation"] == 1


def test_given_vertical_game_and_rotated_then_set4_3_ratio(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=1, vertical_game=True)
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert config["video_allow_rotate"] == '"true"'
    assert config["aspect_ratio_index"] == 0

def test_given_vertical_game_and_no_rotate_then_dont_set_ratio(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=0, vertical_game=True)
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert "aspect_ratio_index" not in config

def test_given_horizontal_game_and_no_rotate_then_dont_set_ratio(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=0, vertical_game=False)
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert "aspect_ratio_index" not in config

def test_given_horizontal_game_and_rotate_then_set_3_4_ratio(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=1, vertical_game=False)
    config = []
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert config["aspect_ratio_index"] == 8

def test_given_rotated_then_disable_overlays(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=1)
    config = []
    LibretroGenerator().processOverlays(system_fbneo, "anyrom.zip")
    assert config == []

def test_given_rotated_then_disable_integer_scale(mocker, system_fbneo):
    configureForTate(system_fbneo, rotation=1)
    config, core = LibretroGenerator().createTateModeConfiguration(system_fbneo)

    assert config["video_scale_integer"] == '"false"'


controllersConfig.esInputs = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../resources/es_input.cfg"))


@pytest.mark.usefixtures("controller_configuration")
def test_given_rotated_input_then_return_right_joystick_as_left(system_fbneo, controller_configuration):
    lrControllers = LibretroControllers(system_fbneo, keyValueSettings(""), keyValueSettings(""), keyValueSettings(""), keyValueSettings(""), controller_configuration, False, "")
    configureForTate(system_fbneo, rotation=1, rotatecontrols=True)
    config, remap = lrControllers.fillControllersConfiguration()
    assert config.getString('input_player1_l_x_minus_axis', "") == "+3"
    assert config.getString('input_player1_l_x_plus_axis', "") == "-3"
    assert config.getString('input_player1_l_y_minus_axis', "") == "-2"
    assert config.getString('input_player1_l_y_plus_axis', "") == "+2"


def test_given_tate_mode_wonderswan_game_then_configure_core_and_retroarch(system_wswanc):
    configureForTate(system_wswanc, rotation=1, vertical_game=True)
    config, core = LibretroGenerator().createTateModeConfiguration(system_wswanc)

    assert config["video_allow_rotate"] == '"true"'
    assert config["video_rotation"] == 2
    assert core["wswan_rotate_keymap"] == '"enabled"'
    assert core["wswan_rotate_display"] == '"manual"'


def test_given_tate_mode_dreamcast_game_then_configure_retroarch_with_specific_rotation():
    dreamcast = Emulator(name='dreamcast', videoMode='1920x1080', ratio='auto', emulator='libretro',
                    core='flycast')
    configureForTate(dreamcast, rotation=1)
    config, core = LibretroGenerator().createTateModeConfiguration(dreamcast)

    assert config["video_allow_rotate"] == '"true"'
    assert config["video_rotation"] == 2
    assert config["input_player1_analog_dpad_mode"] == 3


def test_given_tate_mode_naomi_game_then_configure_retroarch():
    dreamcast = Emulator(name='naomi', videoMode='1920x1080', ratio='auto', emulator='libretro',
                    core='flycast')
    configureForTate(dreamcast, rotation=1)
    config, core = LibretroGenerator().createTateModeConfiguration(dreamcast)

    assert config["video_allow_rotate"] == '"true"'
    assert config["video_rotation"] == 1
    assert config["input_player1_analog_dpad_mode"] == 3

def test_given_tate_mode_saturn_game_then_configure_retroarch_with_specific_rotation(controller_configuration):
    saturn = Emulator(name='saturn', videoMode='1920x1080', ratio='auto', emulator='libretro',
                         core='yabasanshiro')
    configureForTate(saturn, rotation=1)
    config, core = LibretroGenerator().createTateModeConfiguration(saturn)

    assert config["video_allow_rotate"] == '"true"'
    assert config["video_rotation"] == 2
