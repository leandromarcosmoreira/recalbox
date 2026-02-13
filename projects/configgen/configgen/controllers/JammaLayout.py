import typing
from enum import Enum

class JammaLayout(str, Enum):
    SixBtn = "6btns",
    NeoLine = "line",
    NeoSquare = "square",
    NeoDefault = "neodefault",
    NoLayout = "",

    @staticmethod
    def fromString(value: str) -> typing.Optional["JammaLayout"]:
        if value == "6btns":
            return JammaLayout.SixBtn
        if value == "line":
            return JammaLayout.NeoLine
        if value == "square":
            return JammaLayout.NeoSquare
        if value == "neodefault":
            return JammaLayout.NeoDefault
        return JammaLayout.NoLayout

    def toRetroarchDeviceType(self, system: str) -> str:
        if self == JammaLayout.NeoLine:
            return "1029"
        if self == JammaLayout.NeoSquare:
            return "1285"
        if self == JammaLayout.NeoDefault:
            return "1541"
        if system == "dreamcast" and self == JammaLayout.SixBtn:
            return "1025"
        return "1"
