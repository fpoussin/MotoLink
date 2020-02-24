/*
 * ipc.c
 *
 *  Created on: 21 oct. 2014
 *      Author: Mobyfab
 */

#include "ipc.h"

static msg_t buf1[MB_SIZE];
static msg_t buf2[MB_SIZE];

MAILBOX_DECL(knockMb, buf1, MB_SIZE);
MAILBOX_DECL(sensorsMb, buf2, MB_SIZE);

static MEMORYPOOL_DECL(samplesPool, POOL_SIZE * sizeof(samples_message_t), PORT_NATURAL_ALIGN, NULL);

void setupIPC(void)
{
  chMBObjectInit(&knockMb, buf1, MB_SIZE);
  chMBObjectInit(&sensorsMb, buf2, MB_SIZE);
}

bool allocSendSamplesI(mailbox_t* mb, void * buffer, size_t size)
{
  if (buffer == NULL) return false;

  samples_message_t* info = chPoolAllocI(&samplesPool);

  if (info == NULL) return false;

  info->location = buffer;
  info->size = size;
  chMBPostI(mb, (msg_t)info);

  return true;
}

bool recvFreeSamples(mailbox_t* mb, void ** buffer, size_t * size)
{
  msg_t msg;
  samples_message_t data;

  if(chMBFetchTimeout(mb, &msg, TIME_IMMEDIATE) != MSG_OK)
    return false;

  if ((samples_message_t*)msg == NULL) return false;

  data = *(samples_message_t*)msg;
  chPoolFree(&samplesPool, (void*)msg);

  if (data.location == NULL) return false;

  *buffer = data.location;
  *size = data.size;

  return true;
}
