#!/usr/bin/env python
import pytest
from configgen.Emulator import Emulator, ExtraArguments
import configgen.generators.supermodel.supermodelGenerator as supermodelGenerator
from configgen.generators.supermodel.supermodelGenerator import SupermodelGenerator
from configgen.settings.keyValueSettings import keyValueSettings
from tests.generators.FakeArguments import Arguments


@pytest.fixture
def emulator():
    supermodelGenerator.SupermodelGenerator.configFile = 'tests/resources/ConfigModel3.ini'
    supermodelGenerator.SupermodelGenerator.configFile = 'tests/resources/ConfigModel3.ini'
    supermodelGenerator.SupermodelGenerator.controlsFile = 'tests/tmp/supermodel.ini'

    return SupermodelGenerator()


@pytest.fixture
def system():
    return Emulator(name='supermodel', videoMode='1920x1080', ratio='auto', emulator='supermodel', core='supermodel')

def configureForCrt(emulator: Emulator, crtvideostandard="auto", crtresolutiontype="auto", crtscreentype="15kHz",
                    crtadaptor="recalboxrgbdual", crtregion="auto", crtscanlines="", rotation=0, verticalgame=False,
                    crtsuperrez = "1", crtsignaltype = "RGB", lightgunluminosity = 0, widescreen=False, avoid_interlaced = False, avoid_interlaced_on_tate_and_handhelds=False
                    ):
    recalCrtConf = keyValueSettings("")
    recalCrtConf.setBool("options.video.avoidinterlaced", avoid_interlaced)
    recalCrtConf.setBool("options.video.avoidinterlacedontatehandhelds", avoid_interlaced_on_tate_and_handhelds)
    emulator.configure(keyValueSettings(""),
                       ExtraArguments("", "", "", "", "", "", "", "", "", crtvideostandard, crtresolutiontype,
                                      crtscreentype,
                                      crtadaptor, crtregion,
                                      crtscanlines, rotation=rotation, verticalgame=verticalgame,
                                      crtsuperrez=crtsuperrez, crtsignaltype=crtsignaltype, lightgunluminosity=lightgunluminosity),
                       recalCrtConf)
    emulator.WideScreenMode = widescreen
    return emulator


@pytest.mark.usefixtures("controller_configuration")
def test_simple_generate_supermodel_on_pi5_wlr(emulator, system, mocker, controller_configuration, fake_process):
    fake_process.register_subprocess(
        ["tvservice -s"],
        stdout=['[{ "code":1, "width":1920, "height":1080 }]']
    )
    mocker.patch('platform.machine', return_value='arm64')
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=True, new_callable=mocker.PropertyMock)
    command = emulator.generate(system, controller_configuration, keyValueSettings("", False), Arguments('path/to/test'))
    assert command.videomode == '1920x1080'
    assert command.array == ['bash', '-c', "wlr-randr --output '' --mode 1920x1080@60Hz && '/usr/bin/supermodel' 'path/to/test' '-sound-volume=100' '-music-volume=100' '-no-flip-stereo' '-sound' '-dsb' '-new-scsp' '-fullscreen' '-res=1920,1080' '-ss=1' '-no-wide-screen' '-upscalemode=0' '-throttle' '-vsync' '-crosshairs=1' '-no-multi-texture' '-legacy3d' '-gpu-multi-threaded' '-ppc-frequency=100' '-log-output=/recalbox/share/system/configs/model3/Supermodel.log' '-log-level=info'",
]

@pytest.mark.usefixtures("controller_configuration")
def test_simple_generate_supermodel_pc(emulator, system, mocker, controller_configuration, fake_process):
    fake_process.register_subprocess(
        ["tvservice -s"],
        stdout=['[{ "code":1, "width":1920, "height":1080 }]']
    )
    mocker.patch('platform.machine', return_value='x86_64')
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=False, new_callable=mocker.PropertyMock)
    command = emulator.generate(system, controller_configuration, keyValueSettings("", False), Arguments('path/to/test'))
    assert command.videomode == '1920x1080'
    assert command.array == ['/usr/bin/supermodel', 'path/to/test',
                             '-sound-volume=100', '-music-volume=100',
                             '-no-flip-stereo', '-sound',
                             '-dsb', '-new-scsp',
                             '-fullscreen', '-res=1920,1080',
                             '-ss=1', '-no-wide-screen',
                             '-upscalemode=0', '-throttle',
                             '-vsync', '-crosshairs=1',
                             '-no-multi-texture', '-legacy3d',
                             '-gpu-multi-threaded', '-ppc-frequency=100',
                             '-log-output=/recalbox/share/system/configs/model3/Supermodel.log',
                             '-log-level=info']

def test_find_resolution_on_1080p(emulator, system, mocker, controller_configuration, fake_process):
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=True, new_callable=mocker.PropertyMock)
    assert SupermodelGenerator.FindResolution(system) == (1920, 1080, True)

def test_find_resolution_on_640x480p(emulator, system, mocker, controller_configuration, fake_process):
    model3ini = keyValueSettings(supermodelGenerator.SupermodelGenerator.configFile).loadFile()
    model3ini.setString("resolution", "auto").saveFile()
    system =  Emulator(name='supermodel', videoMode='640x480', ratio='auto', emulator='supermodel', core='supermodel')
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=False, new_callable=mocker.PropertyMock)
    assert emulator.FindResolution(system) == (640, 480, False)

def test_find_resolution_on_forced_config(emulator, system, mocker, controller_configuration, fake_process):
    model3ini = keyValueSettings(supermodelGenerator.SupermodelGenerator.configFile).loadFile()
    model3ini.setString("resolution", "1024,768").saveFile()
    system =  Emulator(name='supermodel', videoMode='640x480', ratio='auto', emulator='supermodel', core='supermodel')
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=False, new_callable=mocker.PropertyMock)
    assert emulator.FindResolution(system) == (1024, 768, False)

def test_find_resolution_on_crt(emulator, system, mocker, controller_configuration, fake_process):
    model3ini = keyValueSettings(supermodelGenerator.SupermodelGenerator.configFile).loadFile()
    mocker.patch('configgen.utils.architecture.Architecture.isPi5', return_value=True, new_callable=mocker.PropertyMock)
    model3ini.setString("resolution", "auto").saveFile()
    system = configureForCrt(
        Emulator(name='supermodel', videoMode='320x240', ratio='auto', emulator='supermodel',core='supermodel'),
        crtadaptor="recalboxrgbdual")
    assert emulator.FindResolution(system) == (320, 240, False)