import subprocess
import typing
import pathlib

from configgen.Emulator import Emulator
from configgen.crt.CRTTypes import CRTHandheldFormat
from configgen.crt.RGBDual2 import RGBDual2
from configgen.crtswitchres.CRTTypes import CRTResolutionType, CRTScreenType, CRTScanlines, RetroarchScreenTypeIndex, \
    CRTAdapter, CRTVideoStandard, CRTSignalType
from configgen.generators.libretro.crtswitchres.LibretroCoreConfigCRTSwitchres import LibretroCoreConfigCRTSwitchres
from configgen.utils.architecture import Architecture
import configgen.recalboxFiles as recalboxFiles
from configgen.utils.videoMode import deleteCrtDesktopRes


class LibretroConfigCRTSwitchres:

    def manage_scanlines(self, system: Emulator, config: typing.Dict[str, str]):
        if system.CRTScanlines != CRTScanlines.NONE:
            if (system.CRTScreenType == CRTScreenType.kHz31 and (system.CRTResolutionType == CRTResolutionType.Force480 or system.CRTResolutionType == CRTResolutionType.Auto)) \
                    or (system.CRTScreenType == CRTScreenType.kHzTriFreq or system.CRTScreenType == CRTScreenType.kHzMulti1531) and system.CRTResolutionType == CRTResolutionType.Force480:
                if (system.Rotation.isTate() and system.VerticalGame) or (not system.Rotation.isTate() and not system.VerticalGame):
                    config.update({"video_driver": '"gl"'})
                    config.update({"video_shader_enable": '"true"'})
                    config.update({"video_shader_dir": '"/recalbox/share/shaders/"'})
                    config.update({"video_shader": 'rrgbd-scanlines-{}.glslp'.format(system.CRTScanlines)})

    def manage_lightgun_luminosity(self, system: Emulator, config:typing.Dict[str, str]):
        if system.CRTScreenType != CRTScreenType.kHz31 and system.LighgunLuminosity > 0:
            config.update({"video_driver": '"gl"'})
            config.update({"video_shader_enable": '"true"'})
            config.update({"video_shader_dir": '"/recalbox/share/shaders/"'})
            config.update({"video_shader": 'lightguns-lumi-{}.glslp'.format(system.LighgunLuminosity)})

    def create_switchres_ini(self, system: Emulator, config:typing.Dict[str, str], switchres_config: typing.Dict[str, str]):
        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.ini.value
        with open(recalboxFiles.retroarchSwitchres, "w") as ini_switchres:
            for key, item in switchres_config.items():
                ini_switchres.write("{}     {}\n".format(key, item))

    def createConfigFor(self, system: Emulator, rom: str) -> (typing.Dict[str, any], typing.Dict[str, any], [any], [any]):
        config: typing.Dict[str, any] = {"aspect_ratio_index": "22",
                                         "video_aspect_ratio_auto": 'true',
                                         "crt_switch_timings_pal": "",
                                         "crt_switch_timings_ntsc": "",
                                         "crt_switch_resolution_no_interlaced": 0,
                                         "video_refresh_rate_pal": '',
                                         "video_refresh_rate_ntsc": '',
                                         "crt_switch_timings": "",
                                         "custom_viewport_width": "",
                                         "custom_viewport_width_ntsc": "",
                                         "custom_viewport_width_pal": "",
                                         "custom_viewport_height": "",
                                         "custom_viewport_height_ntsc": "",
                                         "custom_viewport_height_pal": "",
                                         "custom_viewport_x": "",
                                         "custom_viewport_x_ntsc": "",
                                         "custom_viewport_x_pal": "",
                                         "custom_viewport_y": "",
                                         "custom_viewport_y_ntsc": "",
                                         "custom_viewport_y_pal": "",
                                         "video_crop_overscan": '',
                                         "video_fullscreen": '',
                                         "video_fullscreen_x": '',
                                         "video_fullscreen_y": '',
                                         "menu_driver": '"rgui"',
                                         "video_shader_enable": '"false"',
                                         "video_shader": "",
                                         "video_threaded": '"false"',
                                         "video_vsync": '"true"',
                                         "video_black_frame_insertion": '"0"',
                                         "audio_driver": '"pulse"',
                                         "video_smooth": '"false"',
                                         "video_allow_rotate": '"true"',
                                         "video_rotation": 0,
                                         "video_scale_integer": '',
                                         "video_scale_integer_overscale": '',
                                         "video_scale_integer_axis" : '',
                                         "menu_enable_widgets": '"false"',
                                         "video_message_color": '"ffffff"',
                                         "video_msg_bgcolor_enable": '"true"',
                                         "video_msg_bgcolor_red": '"0"',
                                         "video_msg_bgcolor_green": '"0"',
                                         "video_msg_bgcolor_blue": '"0"',
                                         "video_msg_bgcolor_opacity": '"0.8"',
                                         "video_font_size": '"15"',
                                         "video_message_pos_x": '"0.04"',
                                         "video_message_pos_y": '"0.05"',
                                         "quick_menu_show_crt_cali": 'true',
                                         "video_gpu_screenshot": '"true"'
                                         }

        # Remove old switchres.ini file
        pathlib.Path.unlink(recalboxFiles.retroarchSwitchres, missing_ok=True)
        print("[CRTSwitchres]: configuration on {} with {} signal with resolution {}".
              format(system.CRTAdapter.value, system.CRTSignal, system.CRTResolutionType))
        pre = []
        post = []
        if system.CRTAdapter == CRTAdapter.RECALBOXRGBDUAL2:
            if system.WideScreenMode:
                pre.append(RGBDual2.setRatioWide)
            else:
                pre.append(RGBDual2.setRatioStandard)
            post.append(RGBDual2.setRatioDefault)

        # Composite
        if system.CRTAdapter == CRTAdapter.RECALBOXRGBDUAL2 and system.CRTSignal == CRTSignalType.Composite:
            pre.append(RGBDual2.setVideoComposite)
            post.append(RGBDual2.setVideoDefault)
            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.disabled.value
            config["video_fullscreen"] = 1
            if system.CRTResolutionType == CRTResolutionType.Force240 or system.CRTResolutionType == CRTResolutionType.Auto:
                print("[CRTSwitchres]: composite progressive")
                config["video_scale_integer"] = '"true"'
                config["aspect_ratio_index"] = '"20"'
                config["video_aspect_ratio"] = '"2.83"'
                config["video_smooth"] = '"true"'
                if system.CRTVideoStandard == CRTVideoStandard.PAL:
                    print("[CRTSwitchres]: composite PAL")
                    config["video_fullscreen_x"] = 720
                    config["video_fullscreen_y"] = 288
                else:
                    print("[CRTSwitchres]: composite NTSC")
                    config["video_fullscreen_x"] = 720
                    config["video_fullscreen_y"] = 240

            elif system.CRTResolutionType == CRTResolutionType.Force480:
                print("[CRTSwitchres]: composite 480i")
                config["video_fullscreen_x"] = 720
                config["video_fullscreen_y"] = 480
                config["video_scale_integer"] = '"true"'
                config["video_vsync"] = '"false"'

        # RGB
        else:
            if not system.CRTAdapter.supportsInterlaced() or system.CRTAvoidInterlaced or system.Name == "n64":
                print("[CRTSwitchres]: will avoid interlaced")
                config["crt_switch_resolution_no_interlaced"] = 1

            specificHandhelds = ["gb", "gbc", "gba", "ngp", "ngpc", "gamegear", "megaduck", "pico8", "tic80", "wswan", "wswanc"]
            if system.Name in specificHandhelds:
                if system.CrtHandheldFormat == CRTHandheldFormat.Fullscreen or system.SuperGameBoy:
                    print("[CRTSwitchres]: handheld fullscreen")
                    config["video_smooth"] = '"true"'
                    if system.Name == "gamegear":
                        system.Core = "gearsystem"
                else:
                    print("[CRTSwitchres]: handheld original")
                    config["video_scale_integer_axis"] = '"1"'
                    config["video_scale_integer"] = '"true"'
            match system.CRTScreenType:
                case CRTScreenType.kHz15:
                    if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
                        if system.CRTResolutionType == CRTResolutionType.Force240:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Forced240p.value
                        else:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Jamma.value
                    else:
                        if system.CRTVideoStandard == CRTVideoStandard.PAL:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.PAL.value
                        else:
                            if system.CRTResolutionType == CRTResolutionType.Force240:
                                if system.HDGame:
                                    config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Forced240p.value
                                else:
                                    config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15ForcedProgressive.value
                            else:
                                if system.CRTAvoidLowFreqModes and not system.HDGame:
                                    config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15ForcedProgressiveMin57Hz.value
                                else:
                                    config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15.value
                    if system.CRTAvoidInterlacedTateHandhelds:
                        if system.VerticalGame or system.Name in specificHandhelds:
                            print("[CRTSwitchres]: will avoid interlaced on tate game")
                            config["crt_switch_resolution_no_interlaced"] = 1
                            config["video_smooth"] = '"true"'
                    # Disable VSYNC on handhelds (because of switchres and exotic refresh rates)
                    if system.Name in specificHandhelds:
                        if system.CRTVideoStandard == CRTVideoStandard.PAL:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.PAL.value
                        else:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Forced240p.value
                case CRTScreenType.kHz31:
                    if system.CRTResolutionType == CRTResolutionType.DoubleFreq:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz31at120.value
                        config["video_black_frame_insertion"] = '"1"'
                    else:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz31.value
                case CRTScreenType.kHzMulti1525:
                    if system.CRTResolutionType == CRTResolutionType.Force240:
                        if system.HDGame:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Forced240p.value
                        else:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15ForcedProgressive.value
                    elif system.CRTResolutionType == CRTResolutionType.Force384:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz25Forced384p.value
                    else:
                        if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
                            if system.HDGame:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHzMulti1525.value
                            else:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Jamma.value
                        else:
                            if system.HDGame:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHzMulti1525TV.value
                            else:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15.value
                case CRTScreenType.kHzMulti1531:
                    if system.CRTResolutionType == CRTResolutionType.Force480:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz31.value
                    elif system.CRTResolutionType == CRTResolutionType.Force240:
                        if system.HDGame:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Forced240p.value
                        else:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15ForcedProgressive.value
                    elif system.CRTResolutionType == CRTResolutionType.Force480i:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15.value
                    else:
                        if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
                            if system.HDGame:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHzMulti1531.value
                            else:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Jamma.value
                        else:
                            if system.HDGame:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHzMulti1531TV.value
                            else:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15.value
                case CRTScreenType.kHzTriFreq:
                    if system.CRTResolutionType == CRTResolutionType.Force240:
                        if system.HDGame:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Forced240p.value
                        else:
                            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15ForcedProgressive.value
                    elif system.CRTResolutionType == CRTResolutionType.Force480:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz31.value
                    elif system.CRTResolutionType == CRTResolutionType.Force480i:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15.value
                    elif system.CRTResolutionType == CRTResolutionType.Force384:
                        config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz25Forced384p.value
                    else:
                        if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
                            if system.HDGame:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHzTriFreq.value
                            else:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15Jamma.value
                        else:
                            if system.HDGame:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHzTriFreqTV.value
                            else:
                                config["crt_switch_resolution"] = RetroarchScreenTypeIndex.kHz15.value

            if system.CRTSuperrez in ["1920", "2560", "3840"]:
                config["video_font_path"] = '"/usr/share/fonts/truetype/ubuntu_condensed-4x_wide.ttf"'
                config["video_gpu_screenshot"] = '"false"'
            else:
                config["video_font_path"] = '"/usr/share/fonts/truetype/ubuntu_condensed.ttf"'

            config["crt_switch_resolution_super"] = system.CRTSuperrez

            self.manage_scanlines(system, config)
            if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
                system.CRTVideoStandard = CRTVideoStandard.NTSC

        if system.Name == "scummvm" or system.Name == "dos":
            if ( CRTScreenType.kHz15 and system.CRTResolutionType == CRTResolutionType.Force480) or system.CRTResolutionType == CRTResolutionType.Force480i:
                switchres_config = {"monitor": "generic_15", "user_mode": "0x480", "interlace": "1"}
                self.create_switchres_ini(system, config, switchres_config)
            if ( CRTScreenType.kHz15 and system.CRTResolutionType == CRTResolutionType.Force240):
                switchres_config = {"monitor": "generic_15", "user_mode": "0x200", "interlace": "0"}
                self.create_switchres_ini(system, config, switchres_config)

        if system.Name == "dolphin":
            config["crt_switch_resolution"] = RetroarchScreenTypeIndex.disabled.value
            post.append(deleteCrtDesktopRes)

        if system.Rotation.isTate():
            config["video_rotation"] = system.Rotation
        self.manage_lightgun_luminosity(system, config)

        if system.CRTAdapter == CRTAdapter.RECALBOXRGBJAMMA:
            system.CRTVideoStandard = CRTVideoStandard.NTSC
        coreConfig = LibretroCoreConfigCRTSwitchres().createConfigFor(system)


        return config, coreConfig, pre, post
