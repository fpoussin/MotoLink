#!/usr/bin/python

import base64
import xml.etree.cElementTree as ET
from textwrap import fill
import argparse

FIRMWARE_PATH = "../../code/app/build/motolink.bin"
XML_PATH = "firmware.xml"
encoded = ""

parser = argparse.ArgumentParser(description='Firware file generator')
parser.add_argument('-v', '--version', help='Firmware version', default="0.1")
parser.add_argument('-r', '--revision', help='Board revision', default="A")
parser.add_argument('-f', '--file', help='Firmware file path', default=FIRMWARE_PATH)
parser.add_argument('-o', '--output', help='Output file path', default=XML_PATH)
                   
args = parser.parse_args()

print "Version:", args.version
print "Revision:", args.revision
print "Firmware file:" ,args.file
print "Output File:", args.output

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


with open(args.file, "rb") as bin_file:
    encoded = base64.b64encode(bin_file.read())

top = ET.Element('firmware')
comment = ET.Comment('Generated from makefw.py')
top.append(comment)

info = ET.SubElement(top, 'info')
rev = ET.SubElement(info, 'board-rev')
rev.text = args.revision

version = ET.SubElement(info, 'version')
version.text = args.version

data = ET.SubElement(top, 'data')
data.text = "\n"+fill(encoded, 100, initial_indent="    ", subsequent_indent="    ")+"\n  "

indent(top)

with open(args.output, "wb") as xml_file:
    xml_file.write(ET.tostring(top))
