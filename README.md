MotoLink
===================

CAN Bus, K-Line interface for motorcycles.

Primarily intended to communicate with Honda HRC ECUs, as a USB CDC to K-Line connection.

Includes some digital (RPM, VSS) and analog inputs (TPS, Wideband), as well as a knock sensor interface for tuning.

###File tree:###

**├── Board**  *Eagle board files*  
**├── Code**  
**│   ├── App**  *Motolink code*  
**│   ├── Bootloader**  *STM32 Bootloader*  
**│   ├── ChibiOS-RT**  *ChibiOS/RT submodule*  
**│   ├── ChibiOS-Drivers**  *ChibiOS extra Drivers submodule*  
**│   └── Common**  *Common files for bootloader and Motolink*  
**├── Drivers**  *Windows drivers installer*  
**├── GUI**  *Motolink's graphical user interface*  
**│   └── QtUsb**  *Qt USB submodule*  
**├── LICENSE**  *GPL Licence file*  
**└── README.md**  *This readme file*  

You will need to init and update the git submodules (QtUsb, ChibiOS-RT, ChibiOS-Drivers) to build the projects.
