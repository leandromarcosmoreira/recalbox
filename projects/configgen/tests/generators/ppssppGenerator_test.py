#!/usr/bin/env python
import pytest
import shutil
from configgen.Emulator import Emulator
import configgen.generators.ppsspp.ppssppControllers as ppssppControllers
import configgen.generators.ppsspp.ppssppConfig as ppssppConfig
import configgen.generators.ppsspp.ppssppGenerator as ppssppGenerator
from configgen.generators.ppsspp.ppssppGenerator import PPSSPPGenerator
from configgen.settings.keyValueSettings import keyValueSettings

import configgen.controllers.controller as controllersConfig

from tests.generators.FakeArguments import Arguments


@pytest.fixture
def emulator():
    ppssppGenerator.PPSSPPGenerator.controlsFile = 'tests/tmp/ppsspp-gamecontrollerdb.cfg'
    ppssppConfig.configFile = 'tests/tmp/ppsspp.ini'
    ppssppControllers.configFileName = 'tests/tmp/ppsspp-controls.ini'
    shutil.copyfile('tests/resources/ppsspp-controls.ini', 'tests/tmp/ppsspp-controls.ini')
    return PPSSPPGenerator()


@pytest.fixture
def system_gliden64():
    return Emulator(name='psp', videoMode='1920x1080', ratio='auto', emulator='ppsspp', core='ppsspp')


@pytest.mark.usefixtures("controller_configuration")
def test_simple_generate_ppsspp(emulator, system_gliden64, controller_configuration):
    command = emulator.generate(system_gliden64, controller_configuration, keyValueSettings("", False), Arguments('path/to/test'))
    assert command.videomode == '1920x1080'
    assert command.array == ['/usr/bin/PPSSPPSDL', 'path/to/test',
                             ]
