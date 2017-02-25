#include "common.h"
#include "rpc_commands.h"
#include "rpc.h"


CCM_FUNC uint8_t rpcGetVersions(BaseChannel *chn)
{
    rpcSendResponse(chn, CMD_GET_VERSION, MASK_REPLY_OK, (uint8_t*)&versions[0]);
    rpcSendResponse(chn, CMD_GET_VERSION, MASK_REPLY_OK, (uint8_t*)&versions[1]);
    return 0;
}
