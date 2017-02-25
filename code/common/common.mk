# List of all the board related files.
BOARDSRC += ../common/common.c
BOARDSRC += ../common/board.c
BOARDSRC += ../common/usb_config.c
BOARDSRC += ../common/stm32f30x_flash.c
BOARDSRC += ../common/storage.c
BOARDSRC += ../nanopb/pb_common.c
BOARDSRC += ../nanopb/pb_encode.c
BOARDSRC += ../nanopb/pb_decode.c
BOARDSRC += ../common/proto/base.pb.c
BOARDSRC += ../common/proto/bootloader.pb.c
BOARDSRC += ../common/proto/app.pb.c
BOARDSRC += ../common/proto/rpc.c
BOARDSRC += ../common/proto/rpc_commands.c

# Required include directories
BOARDINC += ../common
BOARDINC += ../nanopb
