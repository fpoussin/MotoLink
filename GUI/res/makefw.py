#!/usr/bin/python

from base64 import b64encode
import xml.etree.cElementTree as ET
from textwrap import fill
import argparse
from datetime import datetime
import sys
import re

FIRMWARE_PATH = "../../code/app/build/motolink.bin"
CHCONF_PATH = "../../code/app/chconf.h"
XML_PATH = "firmware.xml"
encoded = ""

parser = argparse.ArgumentParser(description='Firware file generator')
parser.add_argument('-f', '--file', help='Firmware file path', default=FIRMWARE_PATH)
parser.add_argument('-o', '--output', help='Output file path', default=XML_PATH)
                   
args = parser.parse_args()

def indent(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

try:
    with open(args.file, "rb") as bin_file:
        encoded = b64encode(bin_file.read())
except IOError:
    print "Could not find bin file, skipping..."
    sys.exit(0)

v_major = 0
v_minor = 0
v_patch = 0
sub = '.*VERSION_[A-Z]+ '

for line in open(CHCONF_PATH):
  if "VERSION_MAJOR" in line:
    v_major = re.sub(sub, '', line).strip()
  elif "VERSION_MINOR" in line:
    v_minor = re.sub(sub, '', line).strip()
  elif "VERSION_PATCH" in line:
    v_patch = re.sub(sub, '', line).strip()

top = ET.Element('firmware')
comment = ET.Comment('Generated from makefw.py')
top.append(comment)

info = ET.SubElement(top, 'info')
date = ET.SubElement(info, 'build-date')
date.text = str(datetime.utcnow())

version = ET.SubElement(info, 'version')
version.text = "{0}.{1}.{2}".format(v_major, v_minor, v_patch)

data = ET.SubElement(top, 'data')
data.text = "\n"+fill(encoded, 100, initial_indent="    ", subsequent_indent="    ")+"\n  "

indent(top)

with open(args.output, "wb") as xml_file:
    xml_file.write(ET.tostring(top))


print "Version:", version.text
print "Firmware file:", args.file
print "Output File:", args.output
