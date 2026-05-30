/*
    batm.c
    implementation of batm.h
*/
#define _POSIX_C_SOURCE 200809L

#include "batm/batm.h" 


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

/* STATIC HELPERS */

/*
    Accept duplication for now,
    Make it an X macro in v 0.2
*/
static
void
batm_snap_clear(struct batm_snap *snap)
{
    #define X_STR(field) \
        snap->field[0] = 0;
    #define X_INT32(field) \
        snap->field = INT32_MIN;
    #define X_INT64(field) \
        snap->field = INT64_MIN;

    BATM_SNAP_FIELDS_LIST

    #undef X_STR
    #undef X_INT32
    #undef X_INT64

    snap->meta_snap_time = -1;
    snap->meta_scanned_fields_amt = 0;
    snap->meta_name[0] = '\0';
}

static
int
batm_read_int64(const char *path, int64_t *out)
{
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    int ret = fscanf(fp, "%" SCNd64, out);
    fclose(fp);

    return ret == 1 ? 0 : -1;
}

static
int
batm_read_int32(const char* path, int32_t *out)
{
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    int ret = fscanf(fp, "%" SCNd32, out);
    fclose(fp);

    return ret == 1 ? 0 : -1;
}

/*
    Comment:
    BATM_STR_MAX is well under INT_MAX, cast is safe
*/
static
int
batm_read_str(const char *path, char *out, size_t outsz)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        out[0] = '\0';
        return -1;
    }

    char *ret = fgets(out, (int)outsz, fp);
    fclose(fp);

    if (!ret)
    {
        out[0] = '\0';
        return -1;
    }

    size_t len = strlen(out);

    /* len-1 is \n by fgets docs if it was in the file */
    if (len > 0 && out[len - 1] == '\n')
    {
        out[len - 1] = '\0';
    }

    return 0;
}

#define BATM_READ_I64(field)                                        \
do                                                                  \
{                                                                   \
    char p[PATH_MAX];                                               \
    int n = snprintf(p, PATH_MAX, "%s/%s", battery_path, #field);   \
    if (n < 0 || n >= PATH_MAX)                                     \
        snap->field = INT64_MIN;                                    \
    else if (batm_read_int64(p, &snap->field) == 0)                 \
        snap->meta_scanned_fields_amt++;                            \
    else                                                            \
        snap->field = INT64_MIN;                                    \
}                                                                   \
while (0)

#define BATM_READ_I32(field)                                        \
do                                                                  \
{                                                                   \
    char p[PATH_MAX];                                               \
    int n = snprintf(p, PATH_MAX, "%s/%s", battery_path, #field);   \
    if (n < 0 || n >= PATH_MAX)                                     \
        snap->field = INT32_MIN;                                    \
    else if (batm_read_int32(p, &snap->field) == 0)                 \
        snap->meta_scanned_fields_amt++;                            \
    else                                                            \
        snap->field = INT32_MIN;                                    \
}                                                                   \
while (0)

#define BATM_READ_S(field)                                          \
do                                                                  \
{                                                                   \
    char p[PATH_MAX];                                               \
    int n = snprintf(p, PATH_MAX, "%s/%s", battery_path, #field);   \
    if (n < 0 || n >= PATH_MAX)                                     \
        snap->field[0] = '\0';                                      \
    else if (batm_read_str(p, snap->field, BATM_STR_MAX) == 0)      \
        snap->meta_scanned_fields_amt++;                            \
    else                                                            \
        snap->field[0] = '\0';                                      \
}                                                                   \
while (0)

int
batm_snap_update(const char *base, const char *name,
                 struct batm_snap *snap)
{
    if (!name || !snap || strlen(name) >= BATM_STR_MAX)
    {
        errno = EINVAL;
        return -1;
    }

    char battery_path[PATH_MAX];
    int n = snprintf(battery_path, PATH_MAX,
             "%s%s", base ? base : BATM_DEFAULT_BASE, name);
    if (n < 0)
    {
        /* errno is set */
        return -1;
    }
    else if (n >= PATH_MAX)
    {
        errno = ENAMETOOLONG;
        return -1;
    }

    if (access(battery_path, R_OK) != 0)
    {
        /* errno is set */
        return -1;
    }

    /* set all fields to sentinel */
    batm_snap_clear(snap);

    /* set meta: meta_name */
    /* size of name is checked above */
    memcpy(snap->meta_name, name, strlen(name) + 1);

    /* scan fields */ 
    #define X_STR(field) \
        BATM_READ_S(field); 
    #define X_INT32(field) \
        BATM_READ_I32(field);
    #define X_INT64(field) \
        BATM_READ_I64(field);
        
    BATM_SNAP_FIELDS_LIST

    #undef X_STR
    #undef X_INT32
    #undef X_INT64

    /* set meta: meta_snap_time */
    snap->meta_snap_time = time(NULL);

    return snap->meta_scanned_fields_amt == 0 ? -1 : 0;
}

/*
    Notes:
    - all formuals are in header file above each
      derived metric func prototype
    - TODO: Make convenient check for isnan
    - compiler would probably smooth some edges
      when optimizing.
      For example: 
      (a) this condition check
      s->current_now != INT64_MIN && s->voltage_now != INT64_MIN
      or (b) this explicit type casting
      result = (double)s->current_now * (double)s->voltage_now * 1e-12;
      or the explicit type conversion in formulas
      Those things are here because
      (a) they are explicit cond checking
      (b) they make me feel safe in formulas
      Might change em later idk
    - Those funcs that check status field do computations if this field is sentinel
      as per theirs contracts
*/

double
batm_energy_rate_w(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->power_now != INT64_MIN)
    {
        result = (double)s->power_now * 1e-6;
    }
    else if (s->current_now != INT64_MIN && s->voltage_now != INT64_MIN)
    {
        result = (double)s->current_now * (double)s->voltage_now * 1e-12;
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}

double
batm_energy_full_wh(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->energy_full != INT64_MIN)
    {
        result = (double)s->energy_full * 1e-6;
    }
    else if (s->charge_full != INT64_MIN && s->voltage_min_design != INT64_MIN)
    {
        result = (double)s->charge_full * (double)s->voltage_min_design * 1e-12;
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}

double
batm_energy_full_design_wh(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->energy_full_design != INT64_MIN)
    {
        result = (double)s->energy_full_design * 1e-6;
    }
    else if (s->charge_full_design != INT64_MIN && s->voltage_min_design != INT64_MIN)
    {
        result = (double)s->charge_full_design * (double)s->voltage_min_design * 1e-12;
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}

double
batm_soc_pct(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->charge_now != INT64_MIN
        && s->charge_full != INT64_MIN
        && s->charge_full != 0)
    {
        result = (double)s->charge_now / (double)s->charge_full * 100.0;
    }
    else if (s->energy_now != INT64_MIN
             && s->energy_full != INT64_MIN
             && s->energy_full != 0)
    {
        result = (double)s->energy_now / (double)s->energy_full * 100.0;
    }
    else if (s->capacity != INT32_MIN)
    {
        result = (double)s->capacity;
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}

double
batm_health_pct(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->charge_full != INT64_MIN
        && s->charge_full_design != INT64_MIN
        && s->charge_full_design != 0)
    {
        result = (double)s->charge_full / (double)s->charge_full_design * 100.0;
    }
    else if (s->energy_full != INT64_MIN
             && s->energy_full_design != INT64_MIN
             && s->energy_full_design != 0)
    {
        result = (double)s->energy_full / (double)s->energy_full_design * 100.0;
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}

double
batm_time_to_full_hr(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->status[0] == '\0' || strcmp(s->status, "Charging") == 0)
    {
        if (s->time_to_full_now != INT64_MIN)
        {
            result = (double)s->time_to_full_now / 3600.0;
        }
        else if (   s->current_now != INT64_MIN
                 && s->current_now != 0
                 && s->charge_now  != INT64_MIN
                 && s->charge_full != INT64_MIN)
        {
            result = (double)(s->charge_full - s->charge_now) / (double)s->current_now;
        }
        else if (   s->power_now   != INT64_MIN
                 && s->power_now   != 0
                 && s->energy_now  != INT64_MIN
                 && s->energy_full != INT64_MIN)
        {
            result = (double)(s->energy_full - s->energy_now) / (double)s->power_now;
        }
        else
        {
            errno = ENODATA;
        }
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}

double
batm_time_to_empty_hr(const struct batm_snap *s)
{
    if (!s)
    {
        errno = EINVAL;
        return NAN;
    }

    double result = NAN;

    if (s->status[0] == '\0' || strcmp(s->status, "Discharging") == 0)
    {
        if (s->time_to_empty_now != INT64_MIN)
        {
            result = (double)s->time_to_empty_now / 3600.0;
        }
        else if (   s->current_now != INT64_MIN
                 && s->current_now != 0
                 && s->charge_now  != INT64_MIN)
                 
        {
            result = (double)s->charge_now / (double)(llabs(s->current_now));
        }
        else if (   s->power_now   != INT64_MIN
                 && s->power_now   != 0
                 && s->energy_now  != INT64_MIN)
        {
            result = (double)s->energy_now / (double)(llabs(s->power_now));
        }
        else
        {
            errno = ENODATA;
        }
    }
    else
    {
        errno = ENODATA;
    }

    return result;
}
