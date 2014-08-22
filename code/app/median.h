/*
 * median.h
 *
 *  Created on: 22 août 2014
 *      Author: Mobyfab
 */

#ifndef MEDIAN_H_
#define MEDIAN_H_

#include "ch.h"

 typedef struct pair
 {
   struct pair *point;                              /* Pointers forming list linked in sorted order */
   uint16_t value;                                   /* Values to sort */
 } pair;
 typedef struct pair pair;

uint16_t median_filter1(uint16_t datum);
uint16_t median_filter2(uint16_t datum);
uint16_t median_filter3(uint16_t datum);
uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c);

#endif /* MEDIAN_H_ */
