MotoLink
===================

[![GitHub version](https://badge.fury.io/gh/fpoussin%2Fmotolink.svg)](https://badge.fury.io/gh/fpoussin%2Fmotolink)  
GUI code:  
[![Build Status](http://vps2.ntx.so/jenkins/buildStatus/icon?job=Motolink-GUI)](http://vps2.ntx.so/jenkins/job/Motolink-GUI)  
MCU code:  
[![Build Status](http://vps2.ntx.so/jenkins/buildStatus/icon?job=Motolink-ARM)](http://vps2.ntx.so/jenkins/job/Motolink-ARM)

CAN Bus, K-Line interface for motorcycles.

Primarily intended to communicate with Honda HRC ECUs, as a USB CDC to K-Line connection.

Includes some digital (RPM, VSS) and analog inputs (TPS, Wideband), as well as a knock sensor interface for tuning.

###File tree:###

**├── Board**  *Eagle board files*  
**├── Code**  
**│   ├── App**  *Motolink's MCU code*  
**│   ├── Bootloader**  *STM32 Bootloader*  
**│   ├── ChibiOS-RT**  *ChibiOS/RT submodule*  
**│   ├── ChibiOS-Contrib**  *ChibiOS extra Drivers submodule*  
**│   └── Common**  *Common files for bootloader and Motolink*  
**├── Drivers**  *Windows drivers installer*  
**├── GUI**  *Motolink's graphical user interface*  
**│   └── QtUsb**  *Qt USB submodule*  
**├── LICENSE**  *GPL Licence file*  
**└── README.md**  *This readme file*  

You will need to init and update the git submodules (QtUsb, ChibiOS-RT, ChibiOS-Contrib) to build the projects.
