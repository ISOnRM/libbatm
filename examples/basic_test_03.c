/*
    $ cc basic_test_03.c ../src/batm.c -lm
    Testing of derived metrics
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "../include/batm/batm.h"

#define NEWLINE putc(10, stdout)

#define PRINT_METRIC(metric, snap_ptr) \
do \
{ \
    double n = metric(snap_ptr); \
    if (isnan(n) != 0) printf("%s: (NAN: %s)\n", #metric, strerror(errno)); \
    else               printf("%s: %.1f\n", #metric, n); \
} while (0)

int
main(void)
{
    const char *battery_name = getenv("BATTERY_NAME");
    const char *battery_base = getenv("BATTERY_BASE");
    if (!battery_name)
    {
		fprintf(stderr, "Env. Var. \"BATTERY_NAME\" wasn't found\n"
						"Using default value...\n\n");
		battery_name = "BAT1";
    }
    if (!battery_base)
    {
		fprintf(stderr, "Env. Var. \"BATTERY_BASE\" wasn't found\n"
						"Using default value...\n\n");
		battery_base = BATM_DEFAULT_BASE;
    }

    struct batm_snap snap;

    int ret = batm_snap_update(battery_base, battery_name, &snap);
    printf("ret = %d\n", ret);
    if (ret == -1)
    {
        perror("batm_snap_update");
        return 1;
    }

    printf("Fields scanned: %d\n\n", snap.meta_scanned_fields_amt);

    PRINT_METRIC(batm_energy_rate_w, &snap);
    PRINT_METRIC(batm_energy_full_wh, &snap);
    PRINT_METRIC(batm_energy_full_design_wh, &snap);
    PRINT_METRIC(batm_soc_pct, &snap);
    PRINT_METRIC(batm_health_pct, &snap);
    NEWLINE;
    PRINT_METRIC(batm_time_to_full_hr, &snap);
    PRINT_METRIC(batm_time_to_full_s, &snap);
    NEWLINE;
    PRINT_METRIC(batm_time_to_empty_hr, &snap);
    PRINT_METRIC(batm_time_to_empty_s, &snap);
    
    
    return 0; 
}
