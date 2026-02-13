from unittest.mock import patch, PropertyMock

import pytest
from configgen.Emulator import Emulator, ExtraArguments
from configgen.crt.RGBDual2 import RGBDual2
from configgen.crtswitchres.CRTTypes import RetroarchScreenTypeIndex
from configgen.generators.libretro.crtswitchres.LibretroConfigCRTSwitchres import LibretroConfigCRTSwitchres
from configgen.generators.libretro.crtswitchres.LibretroCoreConfigCRTSwitchres import LibretroCoreConfigCRTSwitchres
from configgen.settings.keyValueSettings import keyValueSettings


@pytest.fixture(autouse=True)
def before(mocker):
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=False, new_callable=mocker.PropertyMock)

def configureForCrt(emulator: Emulator, crtvideostandard="auto", crtresolutiontype="auto", crtscreentype="15kHz",
                    crtadaptor="recalboxrgbdual", crtregion="auto", crtscanlines="", rotation=0, verticalgame=False,
                    crtsuperrez="1", crtsignaltype="RGB", widescreen=False
                    ):
    emulator.configure(keyValueSettings(""),
                       ExtraArguments("", "", "", "", "", "", "", "", "", crtvideostandard, crtresolutiontype,
                                      crtscreentype,
                                      crtadaptor, crtregion,
                                      crtscanlines, rotation=rotation, verticalgame=verticalgame,
                                      crtsuperrez=crtsuperrez, crtsignaltype=crtsignaltype))
    emulator.WideScreenMode = widescreen
    return emulator


def test_given_genesis_core_then_disable_overscan_crop():
    emulator = configureForCrt(
        Emulator(name='mastersystem', videoMode='720x240', ratio='auto', emulator='libretro', core='genesisplusgx'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtvideostandard="auto",
        crtadaptor="recalboxrgbdual2")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)

    assert config["genesis_plus_gx_overscan"] == '"full"'


def test_given_gambatte_core_then_set_colorization():
    emulator = configureForCrt(
        Emulator(name='gb', videoMode='720x240', ratio='auto', emulator='libretro', core='gambatte'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtvideostandard="auto",
        crtadaptor="recalboxrgbdual2")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)

    assert config["gambatte_gb_colorization"] == '"internal"'


def test_given_gamecube_then_force_480px_fb():
    emulator = configureForCrt(
        Emulator(name='gamecube', videoMode='1920x1080', ratio='auto', emulator='libretro', core='dolphin'),
        crtresolutiontype="480i", crtscreentype="15kHz", crtadaptor="recalboxrgbdual2", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)

    assert config["dolphin_efb_height"] == '480'

def test_given_cpc_game_on_31kHz_remove_overscan():
    emulator = configureForCrt(
        Emulator(name='amstradcpc', videoMode='1920x1080', ratio='auto', emulator='libretro', core='cap32'),
        crtresolutiontype="480", crtscreentype="kHz31", crtadaptor="recalboxrgbdual2", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["cap32_scr_crop"] == '"enabled"'

    emulator = configureForCrt(
        Emulator(name='amstradcpc', videoMode='1920x1080', ratio='auto', emulator='libretro', core='cap32'),
        crtresolutiontype="auto", crtscreentype="kHz31", crtadaptor="recalboxrgbdual2", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["cap32_scr_crop"] == '"enabled"'

    emulator = configureForCrt(
        Emulator(name='amstradcpc', videoMode='1920x1080', ratio='auto', emulator='libretro', core='cap32'),
        crtresolutiontype="auto", crtscreentype="kHzTriFreq", crtadaptor="recalboxrgbdual2", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["cap32_scr_crop"] == '"enabled"'

    emulator = configureForCrt(
        Emulator(name='amstradcpc', videoMode='1920x1080', ratio='auto', emulator='libretro', core='cap32'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["cap32_scr_crop"] == '"disabled"'

def test_given_dosbox_pure_then_set_refresh_rate():
    emulator = configureForCrt(
        Emulator(name='dos', videoMode='1920x1080', ratio='auto', emulator='libretro', core='dosbox_pure'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtvideostandard="ntsc", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["dosbox_pure_force60fps"] == '"true"'

    emulator = configureForCrt(
        Emulator(name='dos', videoMode='1920x1080', ratio='auto', emulator='libretro', core='dosbox_pure'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtvideostandard="pal", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["dosbox_pure_force60fps"] == '"50"'

def test_given_scummvm_then_set_refresh_rate():
    emulator = configureForCrt(
        Emulator(name='scummvm', videoMode='1920x1080', ratio='auto', emulator='libretro', core='scummvm'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtvideostandard="auto", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["scummvm_framerate"] == '"60 Hz"'

    emulator = configureForCrt(
        Emulator(name='scummvm', videoMode='1920x1080', ratio='auto', emulator='libretro', core='scummvm'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtvideostandard="ntsc", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["scummvm_framerate"] == '"60 Hz"'

    emulator = configureForCrt(
        Emulator(name='scummvm', videoMode='1920x1080', ratio='auto', emulator='libretro', core='scummvm'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtvideostandard="pal", crtsuperrez="0")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["scummvm_framerate"] == '"50 Hz"'

def test_given_gearsystem_then_set_overscan():
    emulator = configureForCrt(
        Emulator(name='mastersystem', videoMode='1920x1080', ratio='auto', emulator='libretro', core='gearsystem'),
        crtresolutiontype="auto", crtscreentype="kHz15", crtadaptor="recalboxrgbdual2", crtvideostandard="auto", crtsuperrez="1")
    config = LibretroCoreConfigCRTSwitchres().createConfigFor(emulator)
    assert config["gearsystem_overscan"] == '"Full (284 width)"'
