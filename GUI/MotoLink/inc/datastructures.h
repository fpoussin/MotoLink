#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

/* This will be based on the HRC tool hex files */

#define CBR600RR07_SIGN "07CBR600KEA6KO"

struct cbr600rr_map_t {

    /* All RPM values are divided by 100 to fit in 8 bits */
    char ign[4*16];     /* Ignition table */
    char dummy1[16];    /* Unknown - zeros */
    char ign[7*16];     /* Ignition table */
    char fuel[7*16];    /* Fuel table */
    char dummy2[16];    /* Unknown - zeros */
    char fuel[4*16];    /* Fuel table */
    char rpm_row[16];   /* RPM row values */
    char tps_row[11];   /* TPS row values */
    char dummy3;        /* Unknown */
    char staging[16];   /* Staging table */
    char pit_limiter;   /* Pit limiter value */
    char dummy4;        /* Unknown - zeros */
    char shift_light;   /* Shift light value */
    char dummy5;        /* Unknown - zeros */
    char idle;          /* Idle value */
    char dummy6;        /* Unknown - zeros */
    char signature[14]; /* "07CBR600KEA6KO" */
};


#endif // DATASTRUCTURES_H
