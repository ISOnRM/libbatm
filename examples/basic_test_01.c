/*
    Artifact #1
    Work on my laptop only tho
    cc basic_test_01.c -I.. -o basic_test_01
*/
#include "include/batm/batm.h"
#include "src/batm.c"
#include <string.h>
#include <time.h>

void
test_read(struct batm_snap *snap)
{
	const char *base = "/sys/class/power_supply/";
	const char *name = "BAT1";
	
    BATM_READ_I32(present);
	BATM_READ_I64(charge_now);
	BATM_READ_I32(capacity);
	BATM_READ_S(technology);

    /* example of a field that is not properly set */
	BATM_READ_I64(energy_now);
    BATM_READ_I32(online);


    snap->meta_snap_time = time(NULL);
}

#define batm_init_snap(name) \
struct batm_snap name = {0};

int
main(void)
{
    /* issue here. Can try either init func or some kind of macro*/
    /* upd: wrote macro on line 21*/
    /* batm_init_snap(snap); */
    struct batm_snap snap;
//  struct batm_snap snap = {0};
    snap.meta_scanned_fields_amt = 0;

    test_read(&snap);

	printf("regular fields"		           "\n"
		   "present:            %" PRId32  "\n"
           "charge_now:         %" PRId64  "\n"
		   "capacity:           %" PRId32  "\n"
		   "technology:         %s"        "\n"
                                           "\n"
           "meta fields"
                                           "\n"
		   "scanned_fields_amt: %" PRId32  "\n"
           "snap_time:          %s"        "\n",
           snap.present,
		   snap.charge_now,
		   snap.capacity,
		   snap.technology,
		   snap.meta_scanned_fields_amt,
           ctime(&snap.meta_snap_time));
    
    
	printf("failed fields"                 "\n"
		   "INT64_MIN is        %" PRId64  "\n"
		   "INT32_MIN is        %" PRId32  "\n"
		   "energy_now:         %" PRId64  "\n"
		   "online:             %" PRId32  "\n",
           INT64_MIN,
           INT32_MIN,
           snap.energy_now,
           snap.online);
		   

    return 0;
}
