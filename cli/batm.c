/* batm - command-line battery monitor using libbatm */

#define _GNU_SOURCE
#define BATM_CLI_VERSION "0.1.2"

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <getopt.h>
#include <unistd.h>

#include "batm/batm.h"

static void
print_field_str(const char *field_type, const char *field_name, const char *field)
{
    printf("%-11s %-28s %-28s\n", field_type, field_name, field);
}

static void
print_field_i32(const char *field_type, const char *field_name, int32_t field)
{
    printf("%-11s %-28s %-28" PRId32 "\n", field_type, field_name, field);
}

static void
print_field_i64(const char *field_type, const char *field_name, int64_t field)
{
    printf("%-11s %-28s %-28" PRId64 "\n", field_type, field_name, field);
}

static void
print_metric_dbl(const char *metric_type, const char *metric_name, double metric, int precision)
{
    printf("%-11s %-28s %-28.*f\n", metric_type, metric_name, precision >= 1 ? precision : 1, metric);
}

static void
print_all(const struct batm_snap *s)
{

    char buf[26]; // as per $ man 3p time
    ctime_r(&s->meta_snap_time, buf);
    char *newline = strchr(buf, '\n');
    if (newline) *newline = '\0';
    

    printf("%-11s %-28s %-28s\n", "Type", "Name", "Value");
    putchar('\n');
    print_field_str("Meta Field", "meta_name", s->meta_name);
    print_field_str("Meta Field", "meta_snap_time", buf);
    print_field_i32("Meta Field", "meta_scanned_fields_amt", s->meta_scanned_fields_amt);
    putchar('\n');

    /* all available fields */
    #define X_STR(field) \
    do { if (s->field[0] != 0) print_field_str("Field", #field, s->field); } while (0);

    #define X_INT32(field) \
    do { if (s->field != INT32_MIN) print_field_i32("Field", #field, s->field); } while (0);

    #define X_INT64(field) \
    do { if (s->field != INT64_MIN) print_field_i64("Field", #field, s->field); } while (0);

    BATM_SNAP_FIELDS_LIST

    #undef X_STR
    #undef X_INT32
    #undef X_INT64

    putchar('\n');

    /* 0.1.n metrics */
    #define X_DBL(func) \
    do { print_metric_dbl("Metric", #func, func(s), 2); } while (0);

    BATM_METRICS_LIST

    #undef X_DBL

}

static int
print_time_to_buf(double hours, char *buf, size_t bufsiz)
{
    if (isnan(hours)) return -1;
    int64_t h = (int64_t)hours;
    int64_t m = (int64_t)((hours - h) * 60.0);
    if (h == 0)
        snprintf(buf, bufsiz, "(%"PRId64"m left)", m);
    else if (h > 0)
        snprintf(buf, bufsiz, "(%"PRId64"h %"PRId64"m left)", h, m);
    else
        return -1;
    return 0;
}

static void
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
        printf("and %s", status);
        /* "BAT1: 80.0% and Not charging" */
        /* "BAT1: 75.5% and Charging (1h 2m left)" */
    }



    if (strcmp(status, "Charging") == 0)
    {
        char buf[64];
        if (print_time_to_buf(batm_time_to_full_hr(s), buf, sizeof buf) == 0) printf(" %s\n", buf);
        /* "BAT1: 75.5% and Charging (1h 2m left)" */
    }
    else if (strcmp(status, "Discharging") == 0)
    {
        char buf[64];
        if (print_time_to_buf(batm_time_to_empty_hr(s), buf, sizeof buf) == 0) printf(" %s\n", buf);
        /* "BAT1: 75.5% and Discharging (8h 32m left)" */
    }
    else
    {
        putchar('\n');
    }

    return;
}


/* temp main */
/*
int
main(void)
{
    struct batm_snap snap;
    batm_snap_update(NULL, "BAT1", &snap);

    printf("=== SUMMARY ===\n");
    print_summary(&snap);

    putchar(10);
    putchar(10);
    putchar(10);

    printf("===== ALL =====\n");
    print_all(&snap);

    return 0;
}
*/

static void
print_version(void)
{
    printf("batm %s\n", BATM_CLI_VERSION);
}

static void
print_help(void)
{
    printf("Usage: batm [OPTION]... BATTERY\n\n"

           "Report battery status from /sys/class/power_supply/\n\n"

           "Options:\n"
           "  -h, --help     show this help and exit\n"
           "  -V, --version  show version and exit\n"
           "  -a, --all      show all available fields and metrics in a table format\n"
           "  -s, --summary  show a brief one-line summary (default)\n");
}

static struct option longopts[] = 
{
    {"help",    no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {"all",     no_argument, NULL, 'a'},
    {"summary", no_argument, NULL, 's'},
    {0, 0, 0, 0}
};

static const char *shortopts = "hVas";

enum mode { MODE_UNSET, MODE_SUMMARY, MODE_ALL };

int
main(int argc, char **argv)
{
    enum mode mode = MODE_UNSET;

    const char *battery_name = NULL;

    int c;
    while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch (c)
        {
            case '?':                  return 1; 
            case 'h': print_help();    return 0;
            case 'V': print_version(); return 0;
            case 'a':
                if (mode != MODE_UNSET && mode != MODE_ALL)
                {
                    fprintf(stderr, "batm: -s/--summary and -a/--all are mutually exclusive\n");
                    return 1;
                }
                mode = MODE_ALL;
                break;
            case 's':
                if (mode != MODE_UNSET && mode != MODE_SUMMARY)
                {
                    fprintf(stderr, "batm: -a/--all and -s/--summary are mutually exclusive\n");
                    return 1;
                }
                mode = MODE_SUMMARY;
                break;
        }
    }

    if (argc - optind != 1)
    {
        fprintf(stderr, "batm: expected exactly one battery name\n");
        return 1;
    }
    battery_name = argv[optind];

    struct batm_snap snap;
    if (batm_snap_update(NULL, battery_name, &snap) == -1)
    {
        fprintf(stderr, "batm: %s: %s\n", battery_name, strerror(errno));
        return 1;
    }

    if (mode == MODE_UNSET || mode == MODE_SUMMARY)
        print_summary(&snap);
    else if (mode == MODE_ALL)
        print_all(&snap);

    return 0;
}