/*
    Test for utilizing X macro in output
    $ cc basic_test_05.c -lbatm
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <batm/batm.h>

int
main(int argc, char **argv)
{
    struct batm_snap snap;
    if (batm_snap_update(NULL, argv[1] ? argv[1] : "BAT1", &snap) == -1) 
    {
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
        return 1;
    }
    char buf[26]; // as per $ man 3p time
    ctime_r(&snap.meta_snap_time, buf);
    char *newline = strchr(buf, '\n');
    if (newline) *newline = '\0';

//  #define X_STR(field) \
//      printf("%s: %s\n", #field, snap.field);
//  #define X_INT32(field) \
//      printf("%s: %"PRId32"\n", #field, snap.field);
//  #define X_INT64(field) \
//      printf("%s: %"PRId64"\n", #field, snap.field);

    #define X_STR(field) \
        do \
        { \
            if (snap.field[0] != '\0') \
                printf("%-32s | %s\n", #field, snap.field); \
        } \
        while (0);

    #define X_INT32(field) \
        do \
        { \
            if (snap.field != INT32_MIN) \
                printf("%-32s | %" PRId32 "\n", #field, snap.field); \
        } \
        while (0);

    #define X_INT64(field) \
        do \
        { \
            if (snap.field != INT64_MIN) \
                printf("%-32s | %" PRId64 "\n", #field, snap.field); \
        } \
        while (0);

    printf("%-32s | %s\n", "meta_snap_time", buf);
    X_STR(meta_name);
    X_INT32(meta_scanned_fields_amt);
    BATM_SNAP_FIELDS_LIST

    #undef X_STR
    #undef X_INT32
    #undef X_INT64

    #define X_DBL(fn) printf("%-32s | %.2f\n", #fn, fn(&snap));

    printf("\n\nMetrics:\n\n\n");
    BATM_METRICS_LIST
    
    #undef X_DBL
    return 0;
}