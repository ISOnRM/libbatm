/*
    basic_test_02.c
    test if batm_update works
*/
#define _POSIX_C_SOURCE 200809L

#include "../src/batm.c"
#include "../include/batm/batm.h"

#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    const char *name = getenv("BATTERY_NAME");
    if (!name)
    {
        fprintf(stderr, "BATTERY_NAME env var isn't set, using default value\n");
        name = "BAT0";
    }

    printf("battery name = %s\n", name);

//  const char *base = BATM_DEFAULT_BASE;


    struct batm_snap snap;

    int ret = batm_snap_update(NULL, name, &snap);
    printf("ret = %d\n\n", ret);
    if (ret != 0) {
        perror("batm_snap_update");
        return 1;
    }

	printf("meta fields:\n"
           "meta_name:                  %s\n"
		   "meta_scanned_fields_amt:    %" PRId32 "\n"
		   "meta_snap_time:             %s\n" // ctime gives \n
		   "str fields:\n"
		   "manufacturer:               %s\n"
		   "model-name:                 %s\n"
           "technology:                 %s\n\n"
		   "int32 fields:\n"
		   "present:                     %" PRId32 "\n"
		   "capacity                     %" PRId32 "\n"
		   "charge_control_end_threshold %" PRId32 "\n\n",
           snap.meta_name,
           snap.meta_scanned_fields_amt,
           ctime(&snap.meta_snap_time),
           snap.manufacturer,
           snap.model_name,
           snap.technology,
           snap.present,
           snap.capacity,
           snap.charge_control_end_threshold);

    return 0;
}
