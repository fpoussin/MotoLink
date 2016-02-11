#!/usr/bin/python

import re

PIN_MODE_INPUT = "PIN_MODE_INPUT({0})"
PIN_MODE_OUTPUT = "PIN_MODE_OUTPUT({0})"
PIN_MODE_ALTERNATE = "PIN_MODE_ALTERNATE({0})"
PIN_MODE_ANALOG = "PIN_MODE_ANALOG({0})"
PIN_ODR_LOW = "PIN_ODR_LOW({0})"
PIN_ODR_HIGH = "PIN_ODR_HIGH({0})"
PIN_OTYPE_PUSHPULL = "PIN_OTYPE_PUSHPULL({0})"
PIN_OTYPE_OPENDRAIN = "PIN_OTYPE_OPENDRAIN({0})"
PIN_OSPEED_2M = "PIN_OSPEED_2M({0})"
PIN_OSPEED_25M = "PIN_OSPEED_25M({0})"
PIN_OSPEED_50M = "PIN_OSPEED_50M({0})"
PIN_OSPEED_100M = "PIN_OSPEED_100M({0})"
PIN_PUPDR_FLOATING = "PIN_PUPDR_FLOATING({0})"
PIN_PUPDR_PULLUP = "PIN_PUPDR_PULLUP({0})"
PIN_PUPDR_PULLDOWN = "PIN_PUPDR_PULLDOWN({0})"
PIN_AFIO_AF = "PIN_AFIO_AF({0}, {1})"

PIN_MODER = "VAL_GPIO{0}_MODER"
PIN_OTYPER = "VAL_GPIO{0}_OTYPER"
PIN_OSPEEDR = "VAL_GPIO{0}_OSPEEDR"
PIN_PUPDR = "VAL_GPIO{0}_PUPDR"
PIN_ODR = "VAL_GPIO{0}_ODR"
PIN_AFRL = "VAL_GPIO{0}_AFRL"
PIN_AFRH = "VAL_GPIO{0}_AFRH"

FMT = "{0}"

PIN_CONF_LIST = [PIN_MODER, PIN_OTYPER, PIN_OSPEEDR, PIN_PUPDR, PIN_ODR]
PIN_CONF_LIST_AF = [PIN_AFRL, PIN_AFRH]
PIN_SEP = "| \\"

# Default values for all ports
PIN_FUNC_MAPPING_DEFAULT = {

        "GPIO_Input": (PIN_MODE_INPUT,
                       PIN_OTYPE_PUSHPULL,
                       PIN_OSPEED_2M,
                       PIN_PUPDR_PULLUP,
                       PIN_ODR_HIGH,
                       PIN_AFIO_AF.format("{0}", 0)),

        "GPIO_Output": (PIN_MODE_OUTPUT,
                        PIN_OTYPE_PUSHPULL,
                        PIN_OSPEED_50M,
                        PIN_PUPDR_FLOATING,
                        PIN_ODR_HIGH,
                        PIN_AFIO_AF.format("{0}", 0)),

        "GPIO_Analog": (PIN_MODE_ANALOG,
                        PIN_OTYPE_PUSHPULL,
                        PIN_OSPEED_2M,
                        PIN_PUPDR_FLOATING,
                        PIN_ODR_HIGH,
                        PIN_AFIO_AF.format("{0}", 0)),

        "SWDIO": (PIN_MODE_ALTERNATE,
                  PIN_OTYPE_PUSHPULL,
                  PIN_OSPEED_2M,
                  PIN_PUPDR_FLOATING,
                  PIN_ODR_HIGH,
                  PIN_AFIO_AF.format("{0}", 0)),

        "SWCLK": (PIN_MODE_ALTERNATE,
                  PIN_OTYPE_PUSHPULL,
                  PIN_OSPEED_2M,
                  PIN_PUPDR_FLOATING,
                  PIN_ODR_HIGH,
                  PIN_AFIO_AF.format("{0}", 0)),

        "(RCC|SYS)_OSC": (PIN_MODE_INPUT,
                          PIN_OTYPE_PUSHPULL,
                          PIN_OSPEED_2M,
                          PIN_PUPDR_FLOATING,
                          PIN_ODR_HIGH,
                          PIN_AFIO_AF.format("{0}", 0)),

        "ADC[x1-4]": (PIN_MODE_ANALOG,
                      PIN_OTYPE_PUSHPULL,
                      PIN_OSPEED_2M,
                      PIN_PUPDR_FLOATING,
                      PIN_ODR_HIGH,
                      PIN_AFIO_AF.format("{0}", 0)),

        "DAC": (PIN_MODE_ANALOG,
                PIN_OTYPE_PUSHPULL,
                PIN_OSPEED_2M,
                PIN_PUPDR_FLOATING,
                PIN_ODR_HIGH,
                PIN_AFIO_AF.format("{0}", 0)),

        "CAN_TX": (PIN_MODE_ALTERNATE,
                   PIN_OTYPE_PUSHPULL,
                   PIN_OSPEED_50M,
                   PIN_PUPDR_FLOATING,
                   PIN_ODR_HIGH,
                   PIN_AFIO_AF.format("{0}", 9)),

        "CAN_RX": (PIN_MODE_ALTERNATE,
                   PIN_OTYPE_PUSHPULL,
                   PIN_OSPEED_50M,
                   PIN_PUPDR_FLOATING,
                   PIN_ODR_HIGH,
                   PIN_AFIO_AF.format("{0}", 9)),

        "TIM2_CH[34]": (PIN_MODE_ALTERNATE,
                        PIN_OTYPE_PUSHPULL,
                        PIN_OSPEED_50M,
                        PIN_PUPDR_FLOATING,
                        PIN_ODR_HIGH,
                        PIN_AFIO_AF.format("{0}", 1)),

        "TIM3_CH[1-4]": (PIN_MODE_ALTERNATE,
                         PIN_OTYPE_PUSHPULL,
                         PIN_OSPEED_50M,
                         PIN_PUPDR_FLOATING,
                         PIN_ODR_HIGH,
                         PIN_AFIO_AF.format("{0}", 2)),

        "TIM4_CH[1-4]": (PIN_MODE_ALTERNATE,
                         PIN_OTYPE_PUSHPULL,
                         PIN_OSPEED_50M,
                         PIN_PUPDR_FLOATING,
                         PIN_ODR_HIGH,
                         PIN_AFIO_AF.format("{0}", 10)),

        "USART[1-3]_TX": (PIN_MODE_ALTERNATE,
                          PIN_OTYPE_OPENDRAIN,
                          PIN_OSPEED_50M,
                          PIN_PUPDR_FLOATING,
                          PIN_ODR_HIGH,
                          PIN_AFIO_AF.format("{0}", 7)),

        "USART[1-3]_RX": (PIN_MODE_ALTERNATE,
                          PIN_OTYPE_OPENDRAIN,
                          PIN_OSPEED_50M,
                          PIN_PUPDR_FLOATING,
                          PIN_ODR_HIGH,
                          PIN_AFIO_AF.format("{0}", 7)),

        "USB": (PIN_MODE_ALTERNATE,
                PIN_OTYPE_PUSHPULL,
                PIN_OSPEED_100M,
                PIN_PUPDR_FLOATING,
                PIN_ODR_HIGH,
                PIN_AFIO_AF.format("{0}", 14)),

        "SPI[12]_(MOSI|SCK)": (PIN_MODE_ALTERNATE,
                     PIN_OTYPE_PUSHPULL,
                     PIN_OSPEED_100M,
                     PIN_PUPDR_FLOATING,
                     PIN_ODR_HIGH,
                     PIN_AFIO_AF.format("{0}", 5)),

        "SPI[12]_NSS": (PIN_MODE_OUTPUT,
                     PIN_OTYPE_PUSHPULL,
                     PIN_OSPEED_100M,
                     PIN_PUPDR_FLOATING,
                     PIN_ODR_HIGH,
                     PIN_AFIO_AF.format("{0}", 0)),

        "SPI[12]_MISO": (PIN_MODE_ALTERNATE,
                     PIN_OTYPE_OPENDRAIN,
                     PIN_OSPEED_100M,
                     PIN_PUPDR_FLOATING,
                     PIN_ODR_HIGH,
                     PIN_AFIO_AF.format("{0}", 5)),

        "(RCC|SYS)_MCO": (PIN_MODE_ALTERNATE,
                          PIN_OTYPE_PUSHPULL,
                          PIN_OSPEED_100M,
                          PIN_PUPDR_FLOATING,
                          PIN_ODR_HIGH,
                          PIN_AFIO_AF.format("{0}", 0)),
}

PIN_FUNC_MAPPING = {
        "A": {  # Values for Port A
            "TIM4_CH[1-4]": (PIN_MODE_ALTERNATE,
                             PIN_OTYPE_PUSHPULL,
                             PIN_OSPEED_50M,
                             PIN_PUPDR_FLOATING,
                             PIN_ODR_HIGH,
                             PIN_AFIO_AF.format("{0}", 2)),

            "SPI[12]_": (PIN_MODE_ALTERNATE,
                         PIN_OTYPE_PUSHPULL,
                         PIN_OSPEED_100M,
                         PIN_PUPDR_FLOATING,
                         PIN_ODR_HIGH,
                         PIN_AFIO_AF.format("{0}", 5)),
        },

        "B": {  # Values for Port B
            "TIM4_CH[1-4]": (PIN_MODE_ALTERNATE,
                             PIN_OTYPE_PUSHPULL,
                             PIN_OSPEED_50M,
                             PIN_PUPDR_FLOATING,
                             PIN_ODR_HIGH,
                             PIN_AFIO_AF.format("{0}", 2))
        },
}

GPIO_SPEED = {"GPIO_SPEED_LOW": PIN_OSPEED_2M,
              "GPIO_SPEED_MEDIUM": PIN_OSPEED_50M,
              "GPIO_SPEED_HIGH": PIN_OSPEED_100M}

GPIO_MODE = {"GPIO_MODE_INPUT": PIN_MODE_INPUT,
             "GPIO_MODE_ANALOG": PIN_MODE_ANALOG,
             "GPIO_MODE_AF_PP": PIN_MODE_ALTERNATE,
             "GPIO_MODE_OUTPUT_PP": PIN_OTYPE_PUSHPULL,
             "GPIO_MODE_OUTPUT_OD": PIN_OTYPE_OPENDRAIN}

GPIO_PUPD = {"GPIO_PULLUP": PIN_PUPDR_PULLUP,
             "GPIO_PULLDOWN": PIN_PUPDR_PULLDOWN,
             "GPIO_NOPULL": PIN_PUPDR_FLOATING}

DEFAULT_PAD = {"signal": "GPIO_Analog",
               "label": None,
               "pupd": None,
               "otype": None,
               "speed": None}

MX_FILE_PATH = "../../board/board.ioc"

error = False

# Sort ports and pads
signals = {}
all_pads = {}

# Default all pads to analog
for p in ["A", "B", "C", "D", "E", "F"]:
    all_pads[p] = {}
    for i in range(16):
        all_pads[p][i] = DEFAULT_PAD.copy()

mx_file = open(MX_FILE_PATH, 'r')
tmp = mx_file.readlines()
mx_file.close()

# Extract signals from IOC
lines = []

for t in tmp:
    if re.search(r"^P[A-L]\d{1,2}(-OSC.+)?\.", t, re.M):
        split = t.split('=')
        pad_name = split[0].split(".")[0]
        pad_port = pad_name[1:2]
        pad_num = int(pad_name[2:4].replace('.', '').replace('-', ''))
        pad_prop = split[0].split(".")[-1]
        prop_value = split[-1].rstrip('\r\n')

        if pad_prop == "Signal":
            all_pads[pad_port][pad_num]["signal"] = prop_value
        if pad_prop == "GPIO_Label":
            all_pads[pad_port][pad_num]["label"] = prop_value
        if pad_prop == "GPIO_PuPd":
            all_pads[pad_port][pad_num]["pupd"] = prop_value
        if pad_prop == "GPIO_ModeDefaultOutputPP":
            all_pads[pad_port][pad_num]["otype"] = prop_value
        if pad_prop == "GPIO_Speed":
            all_pads[pad_port][pad_num]["speed"] = prop_value

    if "Signal" in t:
        lines.append(t.rstrip('\n'))

output = "#ifndef _BOARD_GPIO_H_\n#define _BOARD_GPIO_H_\n\n"
sorted_signals = sorted(signals.keys())

# Add defines for all pins with labels
for port_key in sorted(all_pads.keys()):
    for pad_key in sorted(all_pads[port_key].keys()):
        pad_data = all_pads[port_key][pad_key]
        print "P{0}{1} - {2}".format(port_key, pad_key, pad_data)
        if pad_data['signal'] != "GPIO_Analog":
            if not pad_data['label']:
                pad_data['label'] = pad_data['signal']
            pad_data['label'] = pad_data['label'].replace('-', '_')
            output += "#define PORT_{0} GPIO{1}\n".format(
                    pad_data['label'],
                    port_key)
            output += "#define PAD_{0} {1}\n".format(
                    pad_data['label'],
                    pad_key)
            if "TIM" in pad_data['signal'] and "CH" in pad_data['signal']:
                timer = pad_data['signal'].replace('S_TIM', '').replace('_CH', '')[:-1]
                output += "#define TIM_{0} TIM{1}\n".format(
                        pad_data['label'],
                        timer)
                output += "#define CCR_{0} CCR{1}\n".format(
                        pad_data['label'],
                        int(pad_data['signal'][-1:]))
                output += "#define PWMD_{0} PWMD{1}\n".format(
                        pad_data['label'],
                        timer)
                output += "#define ICUD_{0} ICUD{1}\n".format(
                        pad_data['label'],
                        timer)
                output += "#define CHN_{0} {1}\n\n".format(
                        pad_data['label'],
                        int(pad_data['signal'][-1:])-1)
            else:
                output += "\n"

# Each Port (A...L)
for port_key in sorted(all_pads.keys()):

    output += "/* PORT "+port_key+" */\n"

    # Each property (mode, output/input...)
    for i in range(len(PIN_CONF_LIST)):
        output += "#define "
        output += PIN_CONF_LIST[i].format(port_key) + " ( \\\n"

        # Each pin (0...15)
        for pad_key in sorted(all_pads[port_key].keys()):
            pad_data = all_pads[port_key][pad_key]
            signal = pad_data["signal"]
            match = False

            # Check per port config
            if port_key in PIN_FUNC_MAPPING:
                for p in PIN_FUNC_MAPPING[port_key]:
                    if re.search(p, signal, re.M):
                        match = True
                        type_str = PIN_FUNC_MAPPING[port_key][p][i].format(pad_key)
                        if "GPIO" in signal:
                            if "OTYPE" in type_str:
                                if pad_data["otype"]:
                                    type_str = GPIO_MODE[pad_data["otype"]].format(pad_key)
                            if "PUPD" in type_str:
                                if pad_data["pupd"]:
                                    type_str = GPIO_PUPD[pad_data["pupd"]].format(pad_key)
                            if "SPEED" in type_str:
                                if pad_data["speed"]:
                                    type_str = GPIO_SPEED[pad_data["speed"]].format(pad_key)

                        output += "    " + type_str + " | \\\n"
                        break

            # Check default config
            if not match:
                for p in PIN_FUNC_MAPPING_DEFAULT:
                    if re.search(p, signal, re.M):
                        match = True
                        type_str = PIN_FUNC_MAPPING_DEFAULT[p][i].format(pad_key)
                        if "GPIO" in signal:
                            if "OTYPE" in type_str:
                                if pad_data["otype"]:
                                    type_str = GPIO_MODE[pad_data["otype"]].format(pad_key)
                            if "PUPD" in type_str:
                                if pad_data["pupd"]:
                                    type_str = GPIO_PUPD[pad_data["pupd"]].format(pad_key)
                            if "SPEED" in type_str:
                                if pad_data["speed"]:
                                    type_str = GPIO_SPEED[pad_data["speed"]].format(pad_key)

                        output += "    " + type_str + " | \\\n"
                        break

            if not match:
                print "Missing Peripheral:", signal, "at", "P" + str(port_key) + str(pad_key)
                error = True
                break
        output = output[:-6]
        output += "))\n"
        output += "\n"

    # AF Low bytes 0-7
    output += "#define "
    output += PIN_CONF_LIST_AF[0].format(port_key) + " ( \\\n"
    for pad_key in range(8):
        pad_data = all_pads[port_key][pad_key]
        signal = pad_data["signal"]
        match = False

        # Check per port config
        if port_key in PIN_FUNC_MAPPING:
            for p in PIN_FUNC_MAPPING[port_key]:
                if re.search(p, signal, re.M):
                    match = True
                    type_str = PIN_FUNC_MAPPING[port_key][p][-1].format(pad_key)
                    output += "    " + type_str + " | \\\n"
                    break

        if not match:
            for p in PIN_FUNC_MAPPING_DEFAULT:
                if re.search(p, signal, re.M):
                    match = True
                    type_str = PIN_FUNC_MAPPING_DEFAULT[p][-1].format(pad_key)
                    output += "    " + type_str + " | \\\n"
                    break

        if not match:
            print "Missing Peripheral:", signal, "at", "P" + str(port_key) + str(pad_key)
            error = True
            break

    output = output[:-6]
    output += "))\n"
    output += "\n"

    # AF High bits 8-15
    output += "#define "
    output += PIN_CONF_LIST_AF[1].format(port_key) + " ( \\\n"
    for pad_key in range(8):
        pad_key += 8
        pad_data = all_pads[port_key][pad_key]
        signal = pad_data["signal"]
        match = False

        # Check per port config
        if port_key in PIN_FUNC_MAPPING:
            for p in PIN_FUNC_MAPPING[port_key]:
                if re.search(p, signal, re.M):
                    match = True
                    type_str = PIN_FUNC_MAPPING[port_key][p][-1].format(pad_key)
                    output += "    " + type_str + " | \\\n"
                    break

        if not match:
            for p in PIN_FUNC_MAPPING_DEFAULT:
                if re.search(p, signal, re.M):
                    match = True
                    type_str = PIN_FUNC_MAPPING_DEFAULT[p][-1].format(pad_key)
                    output += "    " + type_str + " | \\\n"
                    break

        if not match:
            print "Missing Peripheral:", signal, "at", "P" + str(port_key) + str(pad_key)
            error = True
            break

    output = output[:-6]
    output += "))\n"
    output += "\n"


output += "#endif\n"

if not error:
    with open("board_gpio.h", "w") as text_file:
        text_file.write(output)
    print "Success!"
