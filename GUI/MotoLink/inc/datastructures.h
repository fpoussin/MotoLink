#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

/* This will be based on the HRC tool hex files */

struct cbr600rr_t {

    char idle;
    char pit_limiter;
    char tps_row[11];
    char rpm_row[16];
};


#endif // DATASTRUCTURES_H
