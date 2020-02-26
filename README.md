View this project on [CADLAB.io](https://cadlab.io/node/808). 

MotoLink
===================

[![GitHub version](https://badge.fury.io/gh/fpoussin%2Fmotolink.svg)](https://badge.fury.io/gh/fpoussin%2Fmotolink) 
[![Build Status](https://jenkins.netyxia.net/buildStatus/icon?job=MotoLink%2Fmaster)](https://jenkins.netyxia.net/job/MotoLink/job/master/) 
[![Total alerts](https://img.shields.io/lgtm/alerts/g/fpoussin/MotoLink.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/fpoussin/MotoLink/alerts/) 
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/fpoussin/MotoLink.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/fpoussin/MotoLink/context:cpp)  

CAN Bus, K-Line interface for motorcycles, with a fuel/ignition mapper function.  

Primarily intended to communicate with Honda HRC, Generic OBD, or Yamaha YEC ECUs.  

This device will create a fuel map based on the various sensor inputs. You will then be able to alter the fuel using your prefered application. (HRC, TuneEcu, YEC, etc...)  
This replaces auto tune fuel systems like Bazzaz or Power commander, except you still have to enter the changes manually. (import/export functions are planned)  
Having a wideband exhaust sensor is mandatory. If your bike doesn't have one, you can use an innovate LC2.  

Includes some digital (RPM, Speed), analog (TPS, ECT, Wideband), serial (Wideband), as well as a knock sensor input interface for tuning.

![](http://i.imgur.com/Rat9Znd.jpg)

[![Alt text](https://img.youtube.com/vi/rAnS-8KSQrY/0.jpg)](https://www.youtube.com/watch?v=rAnS-8KSQrY)


### File tree:  
**├── Board**  *Eagle board files*  
**├── Code**  
**│   ├── App**  *Motolink's MCU code*  
**│   ├── Bootloader**  *STM32 Bootloader*  
**│   ├── ChibiOS-RT**  *ChibiOS/RT submodule*  
**│   ├── ChibiOS-Contrib**  *ChibiOS Community drivers submodule*  
**│   └── Common**  *Common files for bootloader and Motolink*  
**├── Drivers**  *Windows drivers installer*  
**├── GUI**  *Motolink's graphical user interface*  
**├── LICENSE**  *GPL Licence file*  
**└── README.md**  *This readme file*  

You will need to init and update the git submodules (QtUsb, ChibiOS-RT, ChibiOS-Contrib) to build the projects.

## Building
### GUI
* Make sure submodules are pulled
* You will need python 2/3 in the path (system path or Qt Creator project setting PATH variable)
* Go to the GUI folder
* Unzip res/oxygen.zip
* You need a binary file of the firmware, an empty one can be created for testing. (../code/app/build/motolink.bin)
* You need to install QtUsb (https://github.com/fpoussin/QtUsb)
* Open the project with Qt Creator
* Compile and launch

### App and Bootloader
* [You will need ARM's GCC toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
* Make sure submodules are pulled
* Go the de code/app or code/bootloader folder
* For the app, you need to compile the DSP lib (just launch make in the dsp_lib folder)
* make
* For the bootloader, use your favorite stlink interface [or my QSTLink2 app](https://github.com/fpoussin/QStlink2)
* Once the bootloader is running, you can update the app through the GUI

