MotoLink
===================

CAN Bus, K-Line interface for motorcycles.

Primarily intended to communicate with Honda HRC ECUs, as a USB CDC to K-Line connection.

Includes some digital (RPM, VSS) and analog inputs (TPS, Wideband), as well as a knock sensor interface for tuning.

###File tree:###

**├── board**  *Eagle board files*  
**├── code**  
**│   ├── app**  *Motolink code*  
**│   ├── bootloader**  *STM32 Bootloader*  
**│   └── common**  *Common files for bootloader and Motolink*  
**├── GUI**  
**│   └── MotoLink**  *Motolink Graphic User Interface*  
**├── LICENSE**  *GPL Licence file*  
**└── README.md**  *This readme file*  
