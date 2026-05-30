# libbatm
C library for reading battery state from Linux sysfs,
plus a small CLI
## Installation
```bash
git clone https://github.com/ISOnRM/libbatm.git
cd libbatm
MODE=release make
# Use PREFIX=/usr if you are on Arch-based system or packaging
PREFIX=/usr sudo make install clean
```
## Usage (CLI Tool)
```plaintext
~ $ batm -h
Usage: batm [OPTION]... BATTERY

Report battery status from /sys/class/power_supply/

Options:
  -h, --help     show this help and exit
  -V, --version  show version and exit
  -a, --all      show all available fields and metrics in a table format
  -s, --summary  show a brief one-line summary (default)

~ $ ls /sys/class/power_supply/ | grep -i bat
BAT1

~ $ batm -s BAT1
BAT1: 90.5% and Not charging

~ $ batm -a BAT1
Type        Name                         Value                       

Meta Field  meta_name                    BAT1                        
Meta Field  meta_snap_time               Sat May 30 15:32:55 2026    
Meta Field  meta_scanned_fields_amt      18                          

Field       manufacturer                 ASUS                        
Field       model_name                   A32-K55                     
Field       serial_number                                            
Field       type                         Battery                     
Field       status                       Not charging                
Field       technology                   Li-ion                      
Field       capacity_level               Normal                      
Field       present                      1                           
Field       capacity                     90                          
Field       charge_control_end_threshold 80                          
Field       cycle_count                  0                           
Field       voltage_now                  16872000                    
Field       voltage_min_design           15969000                    
Field       current_now                  0                           
Field       charge_now                   5037000                     
Field       charge_full                  5568000                     
Field       charge_full_design           5636000                     
Field       alarm                        556000                      

Metric      batm_energy_rate_w           0.00                        
Metric      batm_energy_full_wh          88.92                       
Metric      batm_energy_full_design_wh   90.00                       
Metric      batm_soc_pct                 90.46                       
Metric      batm_health_pct              98.79                       
Metric      batm_time_to_full_hr         nan                         
Metric      batm_time_to_empty_hr        nan
```
## Status
- v0.1.0. Linux only: reads /sys/class/power_supply/. Tested on glibc.
- v0.1.1. Added X Macro to avoid duplication of code
- v0.1.2. Minor documentation (header, readme) changes
## Library
### API
Header `<batm/batm.h>` exposes `struct batm_snap`, `batm_snap_update` function that populates it and family of the metric functions

### Notes
- batm_snap structure's fields are mirroring possible filenames
which sysfs exposes

- Number of successfully parsed fields is stored in `meta_scanned_fields_amt` field

- Description of every field is in the header, all function contracts are there aswell

- Header is documentation. Read it
### Example
```c
#include <stdio.h>     /* printf function */
#include <math.h>      /* isnan function */
#include <batm/batm.h> /* libbatm header */

int
main(void)
{
    struct batm_snap snap;
    if (batm_snap_update(NULL, "BAT0", &snap) == -1) return 1;

    double battery_health_pct = batm_health_pct(&snap);
    if (isnan(battery_health_pct)) return 1; 
    /* or don't check - printf will print "nan" */

    printf("Battery health: %.1f%%\n", battery_health_pct);
    return 0;
}
```
```bash
cc test.c -lm -lbatm
```
```plaintext
~ $ ./a.out 
Battery health: 97.9%
```
### List of metrics (v 0.1.0)
- batm_energy_rate_w: Instantaneous power drawn from (discharging) or delivered to
(charging) the battery

- batm_energy_full_wh: Energy capacity of the battery when fully charged

- batm_energy_full_design_wh: Energy capacity of the battery as designed by the manufacturer

- batm_soc_pct: This is what most user interfaces label simply "battery percent"

- batm_health_pct: Ratio of current full-charge capacity to design capacity

- batm_time_to_full_hr/s: Estimated hours/seconds remaining until the battery is full

- batm_time_to_empty_hr/s: Estimated hours/seconds remaining until the battery is empty
## License
MIT (See LICENSE)
