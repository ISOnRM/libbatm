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
  -a, --all      show common fields and metrics in a table format
  -s, --summary  show a brief one-line summary (default)

~ $ ls /sys/class/power_supply/ | grep -i bat
BAT1

~ $ batm BAT1 -s
BAT1: 100.0% and Not charging

~ $ batm BAT1 -a
Type        Name                         Value                       

Meta Field  meta_name                    BAT1                        
Meta Field  meta_scanned_fields_amt      18                          
Meta Field  meta_snap_time               Sun May 24 18:07:03 2026    

Field       present                      1                           
Field       manufacturer                 ASUS                        
Field       model_name                   A32-K55                     
Field       technology                   Li-ion                      
Field       serial_number                                            
Field       type                         Battery                     
Field       status                       Not charging                
Field       charge_control_end_threshold 80                          

Metric (%)  batm_soc_pct                 100.00                      
Metric (%)  batm_health_pct              97.94                       
Metric (W)  batm_energy_rate_w           0.00                        
Metric (Wh) batm_energy_full_wh          88.15                       
Metric (Wh) batm_energy_full_design_wh   90.00                       
Metric (hr) batm_time_to_empty_hr        nan                         
Metric (hr) batm_time_to_full_hr         nan 
```
## Status
v0.1.0. Linux only: reads /sys/class/power_supply/. Tested on glibc.
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
