"""The three layers of the driver: hardware-agnostic core, ADC interface, platform port.

Reads: From top to bottom. Your application talks to one NtcThermistor<AdcType> object, whose public calls (listed
from inc/ntc_thermistor.hpp) read the temperature, the resistance and the voltage, and set the divider, the
sampling, the filter and the calibration. The core turns a node voltage into the thermistor's resistance and
the resistance into a temperature with the Beta equation or a lookup table (inc/ntc_conversion.hpp). The ADC
interface (AdcInterface in inc/ntc_adc_interface.hpp) is a CRTP base that forwards to your class. The platform
port is the only code you write: a class that brings the ADC up and returns a channel's voltage and count.
Below it is the hardware: the divider and the thermistor.
"""

import re
from pathlib import Path

from hardfoc_mkdocs_theme.figkit import Fig

ROOT = Path(__file__).resolve().parents[3]
DRIVER = "inc/ntc_thermistor.hpp"
COMM = "inc/ntc_adc_interface.hpp"
SKIP = {"IsInitialized", "Deinitialize", "GetConfiguration", "GetCalibrationOffset", "ResetCalibration",
        "ReadTemperatureFahrenheit", "ReadTemperatureKelvin"}


def data():
    drv = (ROOT / DRIVER).read_text()
    public = drv.split("public:", 1)[1].split("private:", 1)[0]
    calls = []
    for name in re.findall(r"^  (?:\[\[nodiscard\]\] )?(?:bool|NtcError) (\w+)\(", public, re.M):
        if name not in SKIP and name not in calls:
            calls.append(name)
    comm = (ROOT / COMM).read_text()
    hooks = re.findall(r"static_cast<(?:const )?Derived \*>\(this\)->(\w+)\(", comm)
    return {
        "calls": calls,
        "port_hooks": list(dict.fromkeys(hooks)),
        "sources": [f"{DRIVER}:NtcThermistor public calls", f"{COMM}:AdcInterface"],
    }


def build(d):
    f = Fig(
        "driver-layers",
        "Three layers, one of them yours",
        alt="Layer diagram: the application calls the hardware-agnostic NTC thermistor driver, which uses the CRTP "
        "ADC interface, which calls the platform port you write, which reads the ADC input on the node of the "
        "voltage divider formed by the series resistor and the NTC thermistor.",
        width=1200,
        kicker="Architecture",
        subtitle="The driver and the interface are the same on every MCU. The port is a small class with seven methods.",
        profile="overview",
        legend=[("brand", "this library"), ("teal", "you write"), ("slate", "hardware")],
    )
    W = f.body_w
    app = f.card(0, 0, W, "Your application", ["thermal protection, fan control, logging ..."], tone="slate")
    top = app.b + 40
    chips_h = f.chips(16, f.region_inner_top(top), W - 32, [c + "()" for c in d["calls"]], tone="brand")
    reg = f.region(0, top, W, f.region_inner_top(top) - top + chips_h + 18, "Hardware-agnostic core", tone="brand",
                   sub="NtcThermistor<AdcType>, inc/ntc_thermistor.hpp")
    comm = f.card(0, reg.b + 40, W, "ADC interface",
                  ["ntc::AdcInterface<Derived>: CRTP base, calls resolved at compile time;",
                   "volts to ohms (divider), ohms to degrees (Beta or lookup table) in the core above it"],
                  tone="brand", accent=True)
    port = f.card(0, comm.b + 40, W, "Platform port (your class)", ["  ".join(h + "()" for h in d["port_hooks"][:4]),
                                                                    "  ".join(h + "()" for h in d["port_hooks"][4:])],
                  tone="teal", accent=True, mono=True)
    hw = f.card(0, port.b + 40, W, "ADC input and divider",
                ["supply  /  series resistor  /  ADC node  /  NTC thermistor  /  ground"], tone="slate")
    for a, b in ((app, reg), (reg, comm), (comm, port), (port, hw)):
        f.connect(a, b, "bottom", "top", tone="rail")
    return f
