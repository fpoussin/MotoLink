#!/usr/bin/env python

from subprocess import Popen, PIPE


class app(object):
    def __init__(self):
        pass


motolink = app()
motolink.name = "MotoLink"
motolink.path = "app/build/motolink.elf"
motolink.max_ccm = 8*1024
motolink.max_ram = 40*1024
motolink.max_rom = 128*1024

bootloader = app()
bootloader.name = "BootLoader"
bootloader.path = "bootloader/build/bootloader.elf"
bootloader.max_ccm = 8*1024
bootloader.max_ram = 40*1024
bootloader.max_rom = 20*1024

APPS = [motolink, bootloader]

for app in APPS:

    ccm = 0
    ram = 0
    rom = 0

    p = Popen(["arm-none-eabi-size", "-A", app.path], stdout=PIPE)

    if p.wait() == 0:

        output = p.stdout.read()
        lines = filter(None, output.split("\n"))

        for line in lines:
            columns = filter(None, line.split(" "))
            if ".stacks" in columns[0]:
                ram += int(columns[1])
            elif ".ram4" in columns[0]:
                ccm += int(columns[1])
                rom += int(columns[1])
            elif ".bss" in columns[0]:
                ram += int(columns[1])
            elif ".data" in columns[0]:
                ram += int(columns[1])
                rom += int(columns[1])
            elif ".text" in columns[0]:
                rom += int(columns[1])
            elif ".startup" in columns[0]:
                rom += int(columns[1])

        print ""
        print app.name
        print "CCM used: {}% - {:4.1f}/{}k".format((ccm*100)/app.max_ccm,
                                                   ccm/1024.0,
                                                   app.max_ccm/1024.0)

        print "RAM used: {}% - {:4.1f}/{}k".format((ram*100)/app.max_ram,
                                                   ram/1024.0,
                                                   app.max_ram/1024.0)

        print "ROM used: {}% - {:4.1f}/{}k".format((rom*100)/app.max_rom,
                                                   rom/1024.0,
                                                   app.max_rom/1024.0)
