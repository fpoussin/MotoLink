MotoLink
===================

[![GitHub version](https://badge.fury.io/gh/fpoussin%2Fmotolink.svg)](https://badge.fury.io/gh/fpoussin%2Fmotolink)  
GUI code:  
[![Build Status](http://vps2.ntx.so/jenkins/buildStatus/icon?job=Motolink-GUI)](http://vps2.ntx.so/jenkins/job/Motolink-GUI)  
MCU code:  
[![Build Status](http://vps2.ntx.so/jenkins/buildStatus/icon?job=Motolink-ARM)](http://vps2.ntx.so/jenkins/job/Motolink-ARM)

CAN Bus, K-Line interface for motorcycles, with a fuel/ignition mapper function.  

Primarily intended to communicate with Honda HRC, Generic OBD, or Yamaha YEC ECUs.  

This device will create a fuel map based on the various sensor inputs. You will then be able to alter the fuel using your prefered application. (HRC, TuneEcu, YEC, etc...)  
This replaces auto tune fuel systems like Bazzaz or Power commander, except you still have to enter the changes manually. (import/export functions are planned)  
Having a wideband exhaust sensor is mandatory. If your bike doesn't have one, you can use an innovate LC2.  

Includes some digital (RPM, Speed), analog (TPS, ECT, Wideband), serial (Wideband), as well as a knock sensor input interface for tuning.

###File tree:###

**├── Board**  *Eagle board files*  
**├── Code**  
**│   ├── App**  *Motolink's MCU code*  
**│   ├── Bootloader**  *STM32 Bootloader*  
**│   ├── ChibiOS-RT**  *ChibiOS/RT submodule*  
**│   ├── ChibiOS-Contrib**  *ChibiOS Community drivers submodule*  
**│   └── Common**  *Common files for bootloader and Motolink*  
**├── Drivers**  *Windows drivers installer*  
**├── GUI**  *Motolink's graphical user interface*  
**│   └── QtUsb**  *Qt USB submodule*  
**├── LICENSE**  *GPL Licence file*  
**└── README.md**  *This readme file*  

You will need to init and update the git submodules (QtUsb, ChibiOS-RT, ChibiOS-Contrib) to build the projects.
