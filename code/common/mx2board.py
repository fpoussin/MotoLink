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

PIN_CONF_LIST = [PIN_MODER, PIN_OTYPER, PIN_OSPEEDR, PIN_PUPDR, PIN_ODR]
PIN_CONF_LIST_AF = [PIN_AFRL, PIN_AFRH]
PIN_SEP = "| \\"

PERIPHERALS = \
    {
        "GPIO_Input": [PIN_MODE_INPUT, PIN_OTYPE_PUSHPULL, PIN_OSPEED_2M, PIN_PUPDR_PULLUP, PIN_ODR_HIGH,
                       PIN_AFIO_AF.format("{0}", 0)],
        "GPIO_Output": [PIN_MODE_OUTPUT, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                        PIN_AFIO_AF.format("{0}", 0)],
        "GPIO_Analog": [PIN_MODE_ANALOG, PIN_OTYPE_PUSHPULL, PIN_OSPEED_2M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                        PIN_AFIO_AF.format("{0}", 0)],
        "SWDIO": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_2M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                  PIN_AFIO_AF.format("{0}", 0)],
        "SWCLK": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_2M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                  PIN_AFIO_AF.format("{0}", 0)],
        "SYS_OSC": [PIN_MODE_INPUT, PIN_OTYPE_PUSHPULL, PIN_OSPEED_2M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                    PIN_AFIO_AF.format("{0}", 0)],
        "ADC[1-4]": [PIN_MODE_ANALOG, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                PIN_AFIO_AF.format("{0}", 0)],
        "DAC": [PIN_MODE_ANALOG, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                PIN_AFIO_AF.format("{0}", 0)],
        "CAN_TX": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                   PIN_AFIO_AF.format("{0}", 9)],
        "CAN_RX": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                   PIN_AFIO_AF.format("{0}", 9)],
        "TIM2_CH[34]": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                    PIN_AFIO_AF.format("{0}", 1)],
        "TIM3_CH[1-4]": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                    PIN_AFIO_AF.format("{0}", 2)],
        "USART1": [PIN_MODE_ALTERNATE, PIN_OTYPE_OPENDRAIN, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                   PIN_AFIO_AF.format("{0}", 7)],
        "USART3": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                   PIN_AFIO_AF.format("{0}", 7)],
        "USB": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                PIN_AFIO_AF.format("{0}", 14)],
        "SYS_MCO": [PIN_MODE_ALTERNATE, PIN_OTYPE_PUSHPULL, PIN_OSPEED_100M, PIN_PUPDR_FLOATING, PIN_ODR_HIGH,
                PIN_AFIO_AF.format("{0}", 0)],
    }

MX_FILE_PATH = "../../board/board.ioc"

error = False

mx_file = open(MX_FILE_PATH, 'r')
tmp = mx_file.readlines()
mx_file.close()

# Extract signals from IOC
lines = []
for t in tmp:
    if "Signal" in t:
        lines.append(t.rstrip('\n'))

# Sort ports and pads
signals = {}

for p in ["A", "B", "C", "D", "E", "F"]:
    signals[p] = {}
    for i in range(16):
        signals[p][i] = "GPIO_Analog"

for l in lines:
    name = l.split(".")[0].split("-")[0]
    port = name[1:2]
    pad = int(name[2:])
    if port not in signals:
        signals[port] = {}
        for i in range(16):
            signals[port][i] = "GPIO_Analog"
    signals[port][pad] = l.split("=")[-1]

output = "#ifndef _BOARD_GPIO_H_\n#define _BOARD_GPIO_H_\n\n"

sorted_signals = sorted(signals.keys())
for port in sorted_signals:
    pads = signals[port]
    output += "/* PORT "+port+" */\n"

    for i in range(len(PIN_CONF_LIST)):
        output += "#define "
        output += PIN_CONF_LIST[i].format(port) + " ( \\\n"
        for pad, type in pads.items():
            match = False
            for p in PERIPHERALS:
                if re.search(p, type, re.M):
                    match = True
                    type_str = PERIPHERALS[p][i].format(pad)
                    output += "    " + type_str + " | \\\n"
            if not match:
                print "Missing Peripheral:", type, "at", "P" + port + str(pad)
                error = True
                break
        output = output[:-6]
        output += "))\n"
        output += "\n"

    output += "#define "
    output += PIN_CONF_LIST_AF[0].format(port) + " ( \\\n"

    for j in range(8):
        pad = j
        type = pads[pad]
        match = False
        for p in PERIPHERALS:
            if re.search(p, type, re.M):
                match = True
                type_str = PERIPHERALS[p][-1].format(pad)
                output += "    " + type_str + " | \\\n"
        if not match:
            print "Missing Peripheral:", type, "at", "P" + port + str(pad)
            error = True
            break
    output = output[:-6]
    output += "))\n"
    output += "\n"

    output += "#define "
    output += PIN_CONF_LIST_AF[1].format(port) + " ( \\\n"

    for j in range(8):
        pad = j + 8
        type = pads[pad]
        match = False
        for p in PERIPHERALS:
            if re.search(p, type, re.M):
                match = True
                type_str = PERIPHERALS[p][-1].format(pad)
                output += "    " + type_str + " | \\\n"
        if not match:
            print "Missing Peripheral:", type, "at", "P" + port + str(pad)
            error = True
            break
    output = output[:-6]
    output += "))\n"
    
    output += "/* END OF PORT "+port+" */\n\n"

output += "#endif\n"

if not error:
    with open("board_gpio.h", "w") as text_file:
        text_file.write(output)
