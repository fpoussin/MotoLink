/*
 * median.h
 *
 *  Created on: 22 août 2014
 *      Author: Mobyfab
 */

#ifndef MEDIAN_H_
#define MEDIAN_H_

#include "ch.h"

 typedef struct pair_t
 {
   struct pair_t* point; /* Pointers forming list linked in sorted order */
   uint16_t value;       /* Values to sort */
 } pair_t;

 typedef struct
 {
   uint16_t stopper;    /* Smaller than any datum */
   uint16_t size;       /* 3 or more */
   pair_t* buffer;      /* Buffer of nwidth pairs */
   pair_t* datpoint;    /* Pointer into circular buffer of data */
   pair_t small;        /* Chain stopper */
   pair_t big;          /* Pointer to head (largest) of linked list.*/
 } median_t;

void median_init(median_t* conf);
uint16_t median_filter(median_t* conf, uint16_t datum);
uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c);

#endif /* MEDIAN_H_ */
