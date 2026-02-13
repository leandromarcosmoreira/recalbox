import os
import configparser
import shutil
import subprocess
from configgen.utils.architecture import Architecture
from configgen.Command import Command
import configgen.recalboxFiles as recalboxFiles
from configgen.Emulator import Emulator
from configgen.controllers.controller import ControllerPerPlayer
from configgen.generators.Generator import Generator
from configgen.generators.vpinball.vpinballControllers import VpinballControllers
from configgen.generators.vpinball.vpinballConfig import VpinballConfig
from configgen.settings.keyValueSettings import keyValueSettings
from configgen.utils.recallog import recallog
from configgen.generators.vpinball.vpinballOptions import (
    default_player,
    default_standalone,
    default_fss,
    profile_perf,
)


class VpinballGenerator(Generator):
    def get_b2s_size(self, xsize):
        size = [128, 32]
        width = size[0] * xsize
        height = size[1] * xsize
        return width, height

    def get_b2s_x(self, scr1Res, scr2Res, b2sXsize):
        windowx = (int(scr1Res) + (int(scr2Res) / 2)) - (int(b2sXsize / 2))
        return windowx

    def load_settings(self, section, settings, file):
        for x, y in settings.items():
            file.set(section, x, y)

    def get_monitors(self):
        output = [
            l for l in subprocess.check_output(["xrandr"]).decode("utf-8").splitlines()
        ]
        return [l.split()[0] for l in output if " connected " in l]

    def generate(self, system: Emulator, playersControllers: ControllerPerPlayer, recalboxOptions: keyValueSettings, args) -> Command:
        """
        Load, override keys and save back emulator's configuration file
        This way, any modification is kept accross emulator launhes
        """

        recalconf = keyValueSettings(recalboxFiles.recalboxConf)
        recalconf.loadFile(True)

        if not os.path.exists(recalboxFiles.vpinballRootFolder):
            os.makedirs(recalboxFiles.vpinballRootFolder)

        if not os.path.exists(recalboxFiles.vpinballConfigFile):
            shutil.copy(
                "/usr/bin/vpinball/assets/Default_VPinballX.ini",
                recalboxFiles.vpinballConfigFile,
            )

        if os.path.exists(recalboxFiles.vpinballRootFolder + "/VPinballX-custom.ini"):
            recallog("*** Using Custom VPinballX.ini file ***")
            recalboxFiles.vpinballConfigFile = recalboxFiles + "/VPinballX-custom.ini"
        else:
            try:
                vpinballSettings = configparser.ConfigParser(
                    strict=False, allow_no_value=True, empty_lines_in_values=False
                )
                vpinballSettings.optionxform = str
                vpinballSettings.read(recalboxFiles.vpinballConfigFile)
            except configparser.DuplicateOptionError:
                recallog("*** Using default VPinballX.ini file ***")
                shutil.copy(
                    "/usr/bin/vpinball/assets/Default_VPinballX.ini",
                    recalboxFiles.vpinballConfigFile,
                )

            if not vpinballSettings.has_section("Standalone"):
                vpinballSettings.add_section("Standalone")
            if not vpinballSettings.has_section("Player"):
                vpinballSettings.add_section("Player")

            vpinball_mode = "desktop"
            ballTrail = recalconf.getBool("pinball.ball_trail", 0)
            b2sSize = int(recalconf.getString("pinball.score_size", "8"))
            screen1 = recalconf.getString("pinball.screen1", "None")
            screen2 = recalconf.getString("pinball.screen2", "None")
            profile = recalconf.getString("pinball.profile", "1")
            autoConf = recalconf.getBool("pinball.autoconf", "0")
            if Architecture().isX64:
                if len(self.get_monitors()) >= 2:
                    cabinetMode = recalconf.getBool("pinball.cabinet", 0)
                else:
                    cabinetMode = 0
            else:
                cabinetMode = 0

            self.load_settings("Player", default_player, vpinballSettings)
            self.load_settings("Standalone", default_standalone, vpinballSettings)
            self.load_settings("Player", profile_perf[profile], vpinballSettings)

            if ballTrail:
                vpinballSettings.set("Player", "BallTrail", "1")
                vpinballSettings.set("Player", "BallTrailStrength", "0.5")

            if system.Rotation.value > 0 and not cabinetMode:
                vpinball_mode = "FSS"
                vpinballSettings.set("Player", "BGSet", "1")
                self.load_settings("Standalone", default_fss, vpinballSettings)

            if cabinetMode and system.Rotation.value > 0:
                if screen1 != "None":
                    screen1Resolution = recalconf.getString(
                        "pinball.screen1.resolution", "1920x1080"
                    ).split("x")
                if screen2 == "None":
                    os.exit(1)
                else:
                    vpinball_mode = "cabinet"
                    screen2Resolution = recalconf.getString(
                        "pinball.screen2.resolution", "1920x1080"
                    ).split("x")
                    xB2sSize = self.get_b2s_size(b2sSize)
                    B2sWindowX = self.get_b2s_x(
                        screen1Resolution[0], screen2Resolution[0], xB2sSize[0]
                    )
                    vpinballSettings.set("Player", "BGSet", "1")
                    cabinet_settings = {
                        "B2SWindows": "1",
                        "B2SHideGrill": "0",
                        "B2SBackglassX": screen1Resolution[0],
                        "B2SBackglassY": "0",
                        "B2SBackglassWidth": screen2Resolution[0],
                        "B2SBackglassHeight": screen2Resolution[1],
                        "PUPBackglassWindowX": screen1Resolution[0],
                        "PUPBackglassWindowY": "0",
                        "PUPBackglassWindowWidth": screen2Resolution[0],
                        "PUPBackglassWindowHeight": screen2Resolution[1],
                        "PinMAMEWindow": "1",
                        "PinMAMEWindowX": str(int(B2sWindowX)),
                        "PinMAMEWindowY": str(int(screen2Resolution[1]) * 0.8),
                        "PinMAMEWindowWidth": str(xB2sSize[0]),
                        "PinMAMEWindowHeight": str(xB2sSize[1]),
                        "FlexDMDWindowX": str(int(B2sWindowX)),
                        "FlexDMDWindowY": str(int(screen2Resolution[1]) * 0.8),
                        "FlexDMDWindowWidth": str(xB2sSize[0]),
                        "FlexDMDWindowHeight": str(xB2sSize[1]),
                    }
                    self.load_settings("Standalone", cabinet_settings, vpinballSettings)

            if autoConf:
                VpinballConfig(args.rom, vpinball_mode)

            path_rom = args.rom.rpartition("/")[0]
            rom_name = args.rom.rpartition(".")[0]
            if os.path.isdir(path_rom + "/override"):
                if os.path.exists(path_rom + "/override/" + vpinball_mode + ".ini"):
                    recallog("*** Load override settings ***")
                    shutil.copy(
                        path_rom + "/override/" + vpinball_mode + ".ini",
                        rom_name + ".ini",
                    )
                else:
                    if os.path.exists(path_rom + "/override/default.ini"):
                        shutil.copy(
                            path_rom + "/override/default.ini",
                            rom_name + ".ini",
                        )
                    else:
                        if os.path.exists(rom_name + ".ini"):
                            os.remove(rom_name + ".ini")

            with open(recalboxFiles.vpinballConfigFile, "w") as configfile:
                vpinballSettings.write(configfile)


        # Default args
        arguments = [
            "-PrefPath",
            recalboxFiles.vpinballRootFolder,
            "-Ini",
            recalboxFiles.vpinballConfigFile,
            "-play",
        ]

        # Rom
        arguments.append(args.rom)

        commandArray = [recalboxFiles.recalboxBins[system.Emulator]]
        commandArray.extend(arguments)

        if system.HasArgs:
            commandArray.extend(system.Args)

        if Architecture().isPi5:
            longString = "wlr-randr --output '' --mode {}@60Hz &&".format(system.VideoMode)
            for arg in commandArray:
                longString += " '{}'".format(arg)
            commandArray = ["bash", "-c", longString]

        return Command(videomode=system.VideoMode, array=commandArray)
