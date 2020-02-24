/*
 * ipc.h
 *
 *  Created on: 21 oct. 2014
 *      Author: Mobyfab
 */

#ifndef IPC_H_
#define IPC_H_

#include "ch.h"

#define MSG_GO 0x1234ABCD
#define POOL_SIZE 10
#define MB_SIZE 10

extern mailbox_t knockMb;
extern mailbox_t sensorsMb;

typedef struct {
    void* location;
    size_t size;
} samples_message_t;

void setupIPC(void);
bool allocSendSamplesI(mailbox_t* mb, void* buffer, size_t size);
bool recvFreeSamples(mailbox_t* mb, void** buffer, size_t* size);

#endif /* IPC_H_ */
