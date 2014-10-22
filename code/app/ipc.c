/*
 * ipc.c
 *
 *  Created on: 21 oct. 2014
 *      Author: Mobyfab
 */

#include "ipc.h"

static msg_t buf1[10];
static msg_t buf2[10];

MAILBOX_DECL(knockMb, buf1, sizeof(buf1)/sizeof(msg_t));
MAILBOX_DECL(sensorsMb, buf2, sizeof(buf2)/sizeof(msg_t));

static samples_message_t samples_messages[POOL_SIZE] __attribute__((aligned(sizeof(stkalign_t))));
static MEMORYPOOL_DECL(samples_pool, sizeof(samples_message_t), NULL);

void setupIPC(void)
{
  size_t i;
  chMBObjectInit(&knockMb, buf1, sizeof(buf1)/sizeof(msg_t));
  chMBObjectInit(&sensorsMb, buf2, sizeof(buf2)/sizeof(msg_t));

  // We need to init and "free" each place.
  chPoolObjectInit(&samples_pool, sizeof(samples_message_t), NULL);
  for(i=0; i < POOL_SIZE; i++)
      chPoolFree(&samples_pool, &samples_messages[i]);
}

bool allocSendSamplesI(mailbox_t* mb, void * buffer, size_t size)
{
  if (buffer == NULL) return false;

  samples_message_t* info = chPoolAllocI(&samples_pool);

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

  if(chMBFetch(mb, &msg, TIME_IMMEDIATE) != MSG_OK)
    return false;

  if ((samples_message_t*)msg == NULL) return false;

  data = *(samples_message_t*)msg;
  chPoolFree(&samples_pool, (void*)msg);

  if (data.location == NULL) return false;

  *buffer = data.location;
  *size = data.size;

  return true;
}
