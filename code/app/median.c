/*
 * median.c
 *
 *  Created on: 22 août 2014
 *      Author: Mobyfab
 */

#include "median.h"

#define STOPPER 0                              /* Smaller than any datum */
#define MEDIAN_FILTER_SIZE (64)                /* 3 or more */

uint16_t median_filter1(uint16_t datum)
{
 static pair buffer[MEDIAN_FILTER_SIZE] = {0}; /* Buffer of nwidth pairs */
 static pair *datpoint = buffer;               /* Pointer into circular buffer of data */
 static pair small = {NULL, STOPPER};          /* Chain stopper */
 static pair big = {&small, 0};                /* Pointer to head (largest) of linked list.*/

 pair *successor;                              /* Pointer to successor of replaced data item */
 pair *scan;                                   /* Pointer used to scan down the sorted list */
 pair *scanold;                                /* Previous value of scan */
 pair *median;                                 /* Pointer to median */
 uint16_t i;

 if (datum == STOPPER)
 {
   datum = STOPPER + 1;                             /* No stoppers allowed. */
 }

 if ( (++datpoint - buffer) >= MEDIAN_FILTER_SIZE)
 {
   datpoint = buffer;                               /* Increment and wrap data in pointer.*/
 }

 datpoint->value = datum;                           /* Copy in new datum */
 successor = datpoint->point;                       /* Save pointer to old value's successor */
 median = &big;                                     /* Median initially to first in chain */
 scanold = NULL;                                    /* Scanold initially null. */
 scan = &big;                                       /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint)
 {
   scan->point = successor;
 }
 scanold = scan;                                     /* Save this pointer and   */
 scan = scan->point ;                                /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE; ++i)
 {
   /* Handle odd-numbered item in chain  */
   if (scan->point == datpoint)
   {
     scan->point = successor;                      /* Chain out the old datum.*/
   }

   if (scan->value < datum)                        /* If datum is larger than scanned value,*/
   {
     datpoint->point = scanold->point;             /* Chain it in here.  */
     scanold->point = datpoint;                    /* Mark it chained in. */
     datum = STOPPER;
   };

   /* Step median pointer down chain after doing odd-numbered element */
   median = median->point;                       /* Step median pointer.  */
   if (scan == &small)
   {
     break;                                      /* Break at end of chain  */
   }
   scanold = scan;                               /* Save this pointer and   */
   scan = scan->point;                           /* step down chain */

   /* Handle even-numbered item in chain.  */
   if (scan->point == datpoint)
   {
     scan->point = successor;
   }

   if (scan->value < datum)
   {
     datpoint->point = scanold->point;
     scanold->point = datpoint;
     datum = STOPPER;
   }

   if (scan == &small)
   {
     break;
   }

   scanold = scan;
   scan = scan->point;
 }
 return median->value;
}

uint16_t median_filter2(uint16_t datum)
{
 static pair buffer[MEDIAN_FILTER_SIZE] = {0}; /* Buffer of nwidth pairs */
 static pair *datpoint = buffer;               /* Pointer into circular buffer of data */
 static pair small = {NULL, STOPPER};          /* Chain stopper */
 static pair big = {&small, 0};                /* Pointer to head (largest) of linked list.*/

 pair *successor;                              /* Pointer to successor of replaced data item */
 pair *scan;                                   /* Pointer used to scan down the sorted list */
 pair *scanold;                                /* Previous value of scan */
 pair *median;                                 /* Pointer to median */
 uint16_t i;

 if (datum == STOPPER)
 {
   datum = STOPPER + 1;                             /* No stoppers allowed. */
 }

 if ( (++datpoint - buffer) >= MEDIAN_FILTER_SIZE)
 {
   datpoint = buffer;                               /* Increment and wrap data in pointer.*/
 }

 datpoint->value = datum;                           /* Copy in new datum */
 successor = datpoint->point;                       /* Save pointer to old value's successor */
 median = &big;                                     /* Median initially to first in chain */
 scanold = NULL;                                    /* Scanold initially null. */
 scan = &big;                                       /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint)
 {
   scan->point = successor;
 }
 scanold = scan;                                     /* Save this pointer and   */
 scan = scan->point ;                                /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE; ++i)
 {
   /* Handle odd-numbered item in chain  */
   if (scan->point == datpoint)
   {
     scan->point = successor;                      /* Chain out the old datum.*/
   }

   if (scan->value < datum)                        /* If datum is larger than scanned value,*/
   {
     datpoint->point = scanold->point;             /* Chain it in here.  */
     scanold->point = datpoint;                    /* Mark it chained in. */
     datum = STOPPER;
   };

   /* Step median pointer down chain after doing odd-numbered element */
   median = median->point;                       /* Step median pointer.  */
   if (scan == &small)
   {
     break;                                      /* Break at end of chain  */
   }
   scanold = scan;                               /* Save this pointer and   */
   scan = scan->point;                           /* step down chain */

   /* Handle even-numbered item in chain.  */
   if (scan->point == datpoint)
   {
     scan->point = successor;
   }

   if (scan->value < datum)
   {
     datpoint->point = scanold->point;
     scanold->point = datpoint;
     datum = STOPPER;
   }

   if (scan == &small)
   {
     break;
   }

   scanold = scan;
   scan = scan->point;
 }
 return median->value;
}

uint16_t median_filter3(uint16_t datum)
{
 static pair buffer[MEDIAN_FILTER_SIZE] = {0}; /* Buffer of nwidth pairs */
 static pair *datpoint = buffer;               /* Pointer into circular buffer of data */
 static pair small = {NULL, STOPPER};          /* Chain stopper */
 static pair big = {&small, 0};                /* Pointer to head (largest) of linked list.*/

 pair *successor;                              /* Pointer to successor of replaced data item */
 pair *scan;                                   /* Pointer used to scan down the sorted list */
 pair *scanold;                                /* Previous value of scan */
 pair *median;                                 /* Pointer to median */
 uint16_t i;

 if (datum == STOPPER)
 {
   datum = STOPPER + 1;                             /* No stoppers allowed. */
 }

 if ( (++datpoint - buffer) >= MEDIAN_FILTER_SIZE)
 {
   datpoint = buffer;                               /* Increment and wrap data in pointer.*/
 }

 datpoint->value = datum;                           /* Copy in new datum */
 successor = datpoint->point;                       /* Save pointer to old value's successor */
 median = &big;                                     /* Median initially to first in chain */
 scanold = NULL;                                    /* Scanold initially null. */
 scan = &big;                                       /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint)
 {
   scan->point = successor;
 }
 scanold = scan;                                     /* Save this pointer and   */
 scan = scan->point ;                                /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE; ++i)
 {
   /* Handle odd-numbered item in chain  */
   if (scan->point == datpoint)
   {
     scan->point = successor;                      /* Chain out the old datum.*/
   }

   if (scan->value < datum)                        /* If datum is larger than scanned value,*/
   {
     datpoint->point = scanold->point;             /* Chain it in here.  */
     scanold->point = datpoint;                    /* Mark it chained in. */
     datum = STOPPER;
   };

   /* Step median pointer down chain after doing odd-numbered element */
   median = median->point;                       /* Step median pointer.  */
   if (scan == &small)
   {
     break;                                      /* Break at end of chain  */
   }
   scanold = scan;                               /* Save this pointer and   */
   scan = scan->point;                           /* step down chain */

   /* Handle even-numbered item in chain.  */
   if (scan->point == datpoint)
   {
     scan->point = successor;
   }

   if (scan->value < datum)
   {
     datpoint->point = scanold->point;
     scanold->point = datpoint;
     datum = STOPPER;
   }

   if (scan == &small)
   {
     break;
   }

   scanold = scan;
   scan = scan->point;
 }
 return median->value;
}

uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c)
{
 uint16_t middle;

 if ((a <= b) && (a <= c))
 {
   middle = (b <= c) ? b : c;
 }
 else if ((b <= a) && (b <= c))
 {
   middle = (a <= c) ? a : c;
 }
 else
 {
   middle = (a <= b) ? a : b;
 }
 return middle;
}
