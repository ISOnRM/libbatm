/*
    libbatm - Linux battery metrics
    -std=c99
*/
#ifndef BATM_H
#define BATM_H

#include <stdint.h>
#include <float.h>
#include <time.h>

#define BATM_STR_MAX 64
#define BATM_DEFAULT_BASE "/sys/class/power_supply/"

struct batm_snap
{
    /*
        Notes:
        - Struct's field names are mirroring filenames in sysfs
        - If file is absent, lacks access perms or is unreadable 
          then the field's value is set to sentinel value
        - Some batteries expose charge, some batteries do energy.
          When, for example, battery does not report energy
          respective fields are set to sentinel values
        - Meta fields do not represent info about battery,
          they represent info about this struct when
          data is snapshotted
        - String values are read and written to fields as is
        - All descriptions of fields were partially or fully
          taken from Documentation/ABI/testing/sysfs-class-power
        - Examples are either taken from the same source as descriptions
          or from my own system

        Sentinel rules:
        - "" for strings (s[0] == '\0')
        - INT32_MIN for int32_t
        - INT64_MIN for int64_t
        - -1 for time_t (meta fields)
    */

    /* META FIELDS */

    /*
        Description:
        When the snap is updated this value is updated too

        Example:
        1000213380
        
        Comment:
        Will add meta_prev_snap_time in v 0.2
    */
    time_t meta_snap_time;

    /*
        Description:
        This field is the number of successfully scanned
        fields on snap update.
    */
    int32_t meta_scanned_fields_amt;

    /*
        Description:
        Name of a battery passed in batm_snap_update

        Example:
        "BAT0"
    */
    char meta_name[BATM_STR_MAX];



    /* STRINGS */

    /*
        Description:
        Reports the name of the device manufacturer

        Example:
        "ASUS"
    */
    char manufacturer[BATM_STR_MAX];

    /*
        Description:
        Reports the name of the device model

        Example:
        "A32-K55"
    */
    char model_name[BATM_STR_MAX];

    /*
        Description:
        Reports the serial number of the device
    */
    char serial_number[BATM_STR_MAX];

    /*
        Description:
        Describes the main type of the supply

        Examples:
        "Battery", "UPS", "Mains", "USB"
    */
    char type[BATM_STR_MAX];

    /*
        Description:
        Represents the charging status of the battery

        Examples:
        "Unknown", "Charging", "Discharging", "Not charging", "Full"
    */
    char status[BATM_STR_MAX];

    /*
        Description:
        Represents the charging behaviour

        Examples:
        "auto", "inhibit-charge", "inhibit-charge-awake", "force-discharge"
    */
    char charge_behaviour[BATM_STR_MAX];

    /*
        Description:
        Describes the battery technology supported by the supply

        Examples:
        "Unknown", "NiMH", "Li-ion", "Li-poly", "LiFe", "NiCd", "LiMn"
    */
    char technology[BATM_STR_MAX];

    /*
        Description:
        Coarse representation of battery capacity

        Examples:
        "Unknown", "Critical", "Low", "Normal", "High", "Full"
    */
    char capacity_level[BATM_STR_MAX];

    /*
        Description:
        Reports the health of the battery or battery side of charger
        functionality

        Examples:
        "Unknown", "Good", "Overheat", "Dead",
		"Over voltage", "Under voltage", "Unspecified failure", "Cold",
		"Watchdog timer expire", "Safety timer expire",
		"Over current", "Calibration required", "Warm",
		"Cool", "Hot", "No battery", "Blown fuse", "Cell imbalance"
    */
    char health[BATM_STR_MAX];

    /*
        Description:
        Represents the type of charging currently being applied to the
		battery

        Examples:
        "Unknown", "N/A", "Trickle", "Fast", "Standard",
		"Adaptive", "Custom", "Long Life", "Bypass"
    */
    char charge_type[BATM_STR_MAX];

    /* BOOLEAN-LIKES AND REGULAR INTEGERS */

    /*
        Description:
        Reports whether a battery is present or not in the system

        Examples:
        0, 1
    */
    int32_t present;

    /*
        Description:
        Indicates if VBUS is present for the supply. When the supply is
		online, and the supply allows it, then it's possible to switch
		between online states

        Examples:
        0, 1, 2
    */
    int32_t online;

    /* PERCENTAGES */

    /*
        Description:
        Fine grain representation of battery capacity (Basically a percentage)

        Examples:
        0 - 100
    */
    int32_t capacity;

    /*
        Description:
        Minimum battery capacity trip-wire value where the supply will
		notify user-space of the event

        Examples:
        0 - 100
    */
    int32_t capacity_alert_min;

    /*
        Description:
        Maximum battery capacity trip-wire value where the supply will
		notify user-space of the event

        Examples:
        0 - 100
    */
    int32_t capacity_alert_max;


    /*
        Description:
        Represents a battery percentage level, below which charging will begin

        Examples:
        0 - 100
    */
    int32_t charge_control_start_threshold;

    /*
        Description:
        Represents a battery percentage level, above which charging will stop

        Examples:
        0 - 100
    */
    int32_t charge_control_end_threshold;

    /* COUNTERS */    

    /*
        Description:
        Reports the number of full charge + discharge cycles the
		battery has undergone

        Examples:
        >= 0

        Comment:
        Integer > 0: representing full cycles
		Integer = 0: cycle_count info is not available

    */
    int32_t cycle_count;


    /*
        Description:
        Reports the year when the device has been manufactured

        Examples:
        2020
    */
    int32_t manufacture_year;

    /*
        Description:
        Reports the month when the device has been manufactured

        Examples:
        1 - 12
    */
    int32_t manufacture_month;

    /*
        Description:
        Reports the day of month when the device has been manufactured

        Examples:
        1 - 31
    */
    int32_t manufacture_day;

    /* (19.05.2026) TODO: Change formatting for fields below -Mark */

    int64_t voltage_now, voltage_avg,
            voltage_min, voltage_max, 
            voltage_min_design, voltage_max_design;

    /* integer current vals (µA) */

    int64_t current_now, current_avg,
            current_max, charge_control_limit,
            charge_control_limit_max, charge_term_current, precharge_current;

    /* for charging reporting batteries (µAh) */

    int64_t charge_now, charge_avg,
            charge_full, charge_full_design,
            charge_counter;

    /* for energy reporting batteries (µWh) */

    int64_t energy_now, energy_avg,
            energy_full, energy_full_design;

    /* integers in (µW) */

    int64_t power_now;
    int64_t power_avg;

    /* integers in 1/10 C */

    int64_t temp,
            temp_alert_min, temp_alert_max,
            temp_min, temp_max;
    
    /* integers in seconds */
    int64_t time_to_empty_now, time_to_empty_avg,
            time_to_full_now, time_to_full_avg;

    /* alarm is to be read differently, after discovering     */
    /* whether device's battery is charge or energy reporting */
    int64_t alarm;

    /* µΩ */
    int64_t internal_resistance;
};

/* 29.05.2026-30.05.2026 X macro time */
/*
    Notes:
    - There are three types of fields, they are represented here
      as different types of macros. See basic_test_05.c for reference
    - Arguments are field & file names
    - Meta fields aren't handled by this list
    - When a field is added to a main structure, it should also
      be added here. Duh. Don't forget it cuz the error is silent.
      Static assertion is C11, and this lib is C99 so no check here unfortunately
    - Usage examples are in examples/basic_test_05.c and cli/batm.c
*/
#define BATM_SNAP_FIELDS_LIST \
    X_STR(manufacturer)                       \
    X_STR(model_name)                         \
    X_STR(serial_number)                      \
    X_STR(type)                               \
    X_STR(status)                             \
    X_STR(charge_behaviour)                   \
    X_STR(technology)                         \
    X_STR(capacity_level)                     \
    X_STR(health)                             \
    X_STR(charge_type)                        \
    X_INT32(present)                          \
    X_INT32(online)                           \
    X_INT32(capacity)                         \
    X_INT32(capacity_alert_min)               \
    X_INT32(capacity_alert_max)               \
    X_INT32(charge_control_start_threshold)   \
    X_INT32(charge_control_end_threshold)     \
    X_INT32(cycle_count)                      \
    X_INT32(manufacture_year)                 \
    X_INT32(manufacture_month)                \
    X_INT32(manufacture_day)                  \
    X_INT64(voltage_now)                      \
    X_INT64(voltage_min)                      \
    X_INT64(voltage_avg)                      \
    X_INT64(voltage_max)                      \
    X_INT64(voltage_min_design)               \
    X_INT64(voltage_max_design)               \
    X_INT64(current_now)                      \
    X_INT64(current_avg)                      \
    X_INT64(current_max)                      \
    X_INT64(charge_control_limit)             \
    X_INT64(charge_control_limit_max)         \
    X_INT64(charge_term_current)              \
    X_INT64(precharge_current)                \
    X_INT64(charge_now)                       \
    X_INT64(charge_avg)                       \
    X_INT64(charge_full)                      \
    X_INT64(charge_full_design)               \
    X_INT64(charge_counter)                   \
    X_INT64(energy_now)                       \
    X_INT64(energy_avg)                       \
    X_INT64(energy_full)                      \
    X_INT64(energy_full_design)               \
    X_INT64(power_now)                        \
    X_INT64(power_avg)                        \
    X_INT64(temp)                             \
    X_INT64(temp_alert_min)                   \
    X_INT64(temp_alert_max)                   \
    X_INT64(temp_min)                         \
    X_INT64(temp_max)                         \
    X_INT64(time_to_empty_now)                \
    X_INT64(time_to_empty_avg)                \
    X_INT64(time_to_full_now)                 \
    X_INT64(time_to_full_avg)                 \
    X_INT64(alarm)                            \
    X_INT64(internal_resistance)

/* 20.05.2026 Writing inits and updates */

/*
    Description:
    Init or update battery snapshot.
    
    Arguments:
    base - Base, e.g. /sys/class/power_supply/ (w/ the last '/')
           !base means default path ^
    name - Name of a battery in sysfs, e.g. "BAT0" or "BAT1"
    snap - Pointer to snapshot buffer

    Returns:
    0 on success scan and read. -1 on failure

    Errors:
    EINVAL - !name || !snap || strlen(name) >= BATM_STR_MAX
    ENOENT - if battery was not found in sysfs
    EIO    - error of io/parsing
*/
int
batm_snap_update(const char *base,
                 const char *name,
                 struct batm_snap *snap);

/* 30.05.2026: x macro for metrics */

/*
    Notes:
    - Add metric below, add it here
    - Seconds metrics (batm_time_to_*_s) won't be used
      where I use this list.
    - Usage examples are in examples/basic_test_05.c and cli/batm.c
*/
#define BATM_METRICS_LIST             \
    X_DBL(batm_energy_rate_w)         \
    X_DBL(batm_energy_full_wh)        \
    X_DBL(batm_energy_full_design_wh) \
    X_DBL(batm_soc_pct)               \
    X_DBL(batm_health_pct)            \
    X_DBL(batm_time_to_full_hr)       \
    X_DBL(batm_time_to_empty_hr)      \

/* (19.05.2026) TODO: Write procedure prototypes -Mark*/
/* (20.05.2026) Done for 0.1.0 I think -Mark */

/*
    Notes:
    - Fail vals are NAN. You can check them with isnan() from math.h
*/

/* v 0.1 */

/*
    Description:
    Instantaneous power drawn from (discharging) or delivered to
    (charging) the battery. Sign follows current_now's convention
    on the running kernel.

    Arguments:
    s - pointer to a populated batm_snap

    Returns:
    Power in watts. NAN on failure.

    Errors:
    EINVAL  - s is NULL
    ENODATA - neither power_now nor (current_now AND voltage_now)
            are reported by this battery

    Formula:
    primary:  n W = power_now * 1e-6
    fallback: n W = current_now * voltage_now * 1e-12
 */
double
batm_energy_rate_w(const struct batm_snap *s);

/*
    Description:
    Energy capacity of the battery when fully charged

    Arguments:
    s - pointer to a populated batm_snap

    Returns:
    Full-charge energy in Wh. NAN on failure.

    Errors:
    EINVAL  - s is NULL
    ENODATA - neither energy_full nor (charge_full AND voltage_min_design)
              are reported by this battery
    
    Formula:
    primary:  n Wh = energy_full * 1e-6
    fallback: n Wh = charge_full * voltage_min_design * 1e-12
*/
double
batm_energy_full_wh(const struct batm_snap *s);


/*
    Description:
    Energy capacity of the battery as designed by the manufacturer.

    Arguments:
    s - pointer to a populated batm_snap
    
    Returns:
    Design full-charge energy in Wh. NAN on failure.
    
    Errors:
    EINVAL  - s is NULL
    ENODATA - neither energy_full_design nor
              (charge_full_design AND voltage_min_design) are reported
    
    Formula:
    primary:  n Wh = energy_full_design * 1e-6
    fallback: n Wh = charge_full_design * voltage_min_design * 1e-12
*/
double
batm_energy_full_design_wh(const struct batm_snap *s);


/*
    Description:
    Battery state of charge: how full the battery is right now.
    This is what most user interfaces label simply "battery percent"
    
    Arguments:
    s - pointer to a populated batm_snap
    
    Returns:
    State of charge in percent, 0.0 .. 100.0. NAN on failure.
    
    Errors:
    EINVAL  - s is NULL
    ENODATA - no usable source available, or the denominator
                (charge_full or energy_full) is zero
    
    Formula:
    primary:    n % = charge_now / charge_full * 100
    fallback 1: n % = energy_now / energy_full * 100
    fallback 2: n % = (double)capacity

    Explanation:
    fallback 2 gives the least information since it is an integer
*/
double
batm_soc_pct(const struct batm_snap *s);


/*
    Description:
    Battery health: ratio of current full-charge capacity to design
    capacity
    
    Arguments:
    s - pointer to a populated batm_snap
    
    Returns:
    Health in percent, 0.0 .. ~100.0. NAN on failure.
    
    Errors:
    EINVAL  - s is NULL
    ENODATA - no usable source available, or the design capacity
              denominator is zero
    
    Formula:
    primary:  n % = charge_full / charge_full_design * 100
    fallback: n % = energy_full / energy_full_design * 100
*/
double
batm_health_pct(const struct batm_snap *s);

/*
    Description:
    Estimated hours remaining until the battery reaches full
    charge. Only meaningful while status is "Charging".
    
    Arguments:
    s - pointer to a populated batm_snap
    
    Returns:
    Hours to full. NAN on failure.
    
    Errors:
    EINVAL  - s is NULL
    ENODATA - no usable source available, or the rate term
              (current_now or power_now) is zero
    
    Formula:
    primary:    n h = (double)time_to_full_now / 3600
    fallback 1: n h = (charge_full - charge_now) / current_now 
    fallback 2: n h = (energy_full - energy_now) / power_now 
    
    Comment:
    Function checks the status to be Charging.
    If it is not - NAN
    If snapshot's status field is a sentinel - calculate anyway
*/
double
batm_time_to_full_hr(const struct batm_snap *s);

/*
    Note:
    Changes rapidly, use hr
*/
#define batm_time_to_full_s(s) (batm_time_to_full_hr((s)) * 3600.0)

/*
    Description:
    Estimated hours remaining until the battery is 
    empty. Only meaningful while status is "Discharging".
    
    Arguments:
    s - pointer to a populated batm_snap
    
    Returns:
    Hours to empty. NAN on failure.
    
    Errors:
    EINVAL  - s is NULL
    ENODATA - no usable source available, or the rate term
              (current_now or power_now) is zero
    
    Formula:
    primary:    n h = (double)time_to_empty_now / 3600
    fallback 1: n h = charge_now / |current_now|
    fallback 2: n h = energy_now / |power_now|
    
    Comment:
    Function checks the status to be Discharging.
    If it is not - NAN
    If snapshot's status field is a sentinel - calculate anyway
*/
double
batm_time_to_empty_hr(const struct batm_snap *s);

/*
    Note:
    Changes rapidly, use hr
*/
#define batm_time_to_empty_s(s) (batm_time_to_empty_hr((s)) * 3600.0)

#endif /* BATM_H */
