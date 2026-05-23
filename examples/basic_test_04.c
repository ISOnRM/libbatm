/*
    Basic overlook on how CLI util's output might look like
    $ cc basic_test_04.c -lbatm
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include "batm/batm.h"

#define NL putc(10, stdout)

static void
print_field_str(const char *field_type, const char *field_name, char *field)
{
    if (field_type || field_name || field)
    {
        int n = strlen(field);
        if (field[n-1] == '\n') field[n-1] = '\0';
        printf("%-17s %-28s %-28s\n", field_type, field_name, field[0] == 0 ? "(sentinel)" : field);
    }
}

void
print_field_i32(const char *field_type, const char *field_name, int32_t field)
{
    if (field_type || field_name)
    {
        if (field == INT32_MIN)
        {
            printf("%-17s %-28s %-28s\n", field_type, field_name, "(sentinel)");
            return;
        }
        printf("%-17s %-28s %-28" PRId32 "\n", field_type, field_name, field);
    }
}

void
print_field_i64(const char *field_type, const char *field_name, int64_t field)
{
    if (field_type || field_name)
    {
        if (field == INT64_MIN)
        {
            printf("%-17s %-28s %-28s\n", field_type, field_name, "(sentinel)");
            return;
        }
        printf("%-17s %-28s %-28" PRId64 "\n", field_type, field_name, field);
    }
}

void
print_metric_dbl(const char *metric_type, const char *metric_name, double metric, int precision)
{
    if (metric_type || metric_name)
    {
        if (isnan(metric) != 0)
        {
            printf("%-17s %-28s %-28s\n", metric_type, metric_name, "(NAN)");
            return;
        }
        printf("%-17s %-28s %-28.*f\n", metric_type, metric_name, precision >= 1 ? precision : 1, metric);
    }
}

/*
    Notes/TODOs:
     - write helpers to avoid duplication in strcmp section
*/
void
print_summary(const struct batm_snap *s)
{
    printf("%s: ", s->meta_name);
    /* "BAT1: " */

    double pct = batm_soc_pct(s);
    if (isnan(pct) != 0)
        printf("??? ");
    /* "BAT1: ??? " */
    else
        printf("%.1f%% ", pct);
    /* "BAT1: 80.0% " */
   
    const char *status = s->status;
    if (status[0] == '\0')
    {
        printf("(status missing)\n");
        return;
        /* "BAT1: 80.0% (status missing)" */
    }
    else
    {
        printf("and %s ", status);
        /* "BAT1: 80.0% and Not charging" */
        /* "BAT1: 75.5% and Charging (1h 2m left)" */
    }

    if (strcmp(status, "Charging") == 0)
    {
        double hrs = batm_time_to_full_hr(s);
        if (isnan(hrs) != 0)
        {
            printf("\n");
            return;
        }

        int64_t h, m;
        h = (int64_t)hrs;

        if (h == 0)
        {
            m = (int64_t)(hrs * 60.0);
            printf("(%"PRId64"m left)\n", m);
            /* "BAT1: 95.1% and Charging (15m left)" */
            return;
        }
        else
        {
            m = (int64_t)((hrs - h) * 60.0);
            printf("(%"PRId64"h %"PRId64"m left)\n", h, m);
            /* "BAT1: 65.3% and Charging (2h 54m left)" */
            return;
        }
    }
    else if (strcmp(status, "Discharging") == 0)
    {
        double hrs = batm_time_to_empty_hr(s);
        if (isnan(hrs) != 0)
        {
            printf("\n");
            return;
        }

        int64_t h, m;
        h = (int64_t)hrs;

        if (h == 0)
        {
            m = (int64_t)(hrs * 60.0);
            printf("(%"PRId64"m left)\n", m);
            /* "BAT1: 3.7% and Discharging (4m left)" */
            return;
        }
        else
        {
            m = (int64_t)((hrs - h) * 60.0);
            printf("(%"PRId64"h %"PRId64"m left)\n", h, m);
            /* "BAT1: 30.6% and Discharging (1h 55m left)" */
            return;
        }
    }
    else
    {
        printf("\n");
        return;
    }
    
}

int
main(void)
{
    const char *battery_name = getenv("BATTERY_NAME");
    const char *battery_base = getenv("BATTERY_BASE");
    if (!battery_name)
    {
        fprintf(stderr, "Using default battery name\n");
        battery_name = "BAT0";
    }
    if (!battery_base)
    {
        fprintf(stderr, "Using default battery base\n");
        battery_base = BATM_DEFAULT_BASE;
    }

    struct batm_snap snap;
    int ret = batm_snap_update(battery_base, battery_name, &snap);
    if (ret == -1)
    {
        printf("ret = %d\n", ret);
        perror("batm_snap_update");
        return 1;
    }

    /*
        Printed when --summary
    */
    printf("Summary: \n");
    print_summary(&snap);
    NL;
    /*
        Printed when --all
    */
    printf("%-17s %-28s %-28s\n", "Type", "Name", "Value");
    NL;
    print_field_str("Meta Field", "meta_name", snap.meta_name);
    print_field_str("Meta Field", "meta_snap_time", ctime(&snap.meta_snap_time));
    print_field_i32("Meta Field", "meta_scanned_fields_amt", snap.meta_scanned_fields_amt);
    print_field_i32("Field", "present", snap.present);
    print_field_str("Field", "manufacturer", snap.manufacturer);
    print_field_str("Field", "model_name", snap.model_name);
    print_field_str("Field", "technology", snap.technology);
    print_field_str("Field", "serial_number", snap.serial_number);
    print_field_str("Field", "type", snap.type);
    print_field_str("Field", "status", snap.status);
    print_field_i32("Field", "charge_control_end_threshold", snap.charge_control_end_threshold);
    NL;
    print_metric_dbl("0.1.n Metric (%)", "batm_soc_pct", batm_soc_pct(&snap), 2);
    print_metric_dbl("0.1.n Metric (%)", "batm_health_pct", batm_health_pct(&snap), 2);
    print_metric_dbl("0.1.n Metric (W)", "batm_energy_rate_w", batm_energy_rate_w(&snap), 2);
    print_metric_dbl("0.1.n Metric (Wh)", "batm_energy_full_wh", batm_energy_full_wh(&snap), 2);
    print_metric_dbl("0.1.n Metric (Wh)", "batm_energy_full_design_wh", batm_energy_full_design_wh(&snap), 2);
    print_metric_dbl("0.1.n Metric (hr)", "batm_time_to_empty_hr", batm_time_to_empty_hr(&snap), 1);
    print_metric_dbl("0.1.n Metric (hr)", "batm_time_to_full_hr", batm_time_to_full_hr(&snap), 1);

    

    return 0;
}


