/*
    batm.c
    implementation of batm.h
*/

/* it is this way b4 compile_commands for clangd */
#include "../include/batm/batm.h" 


#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

/* STATIC HELPERS */
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

#define BATM_READ_I64(field)                                \
do                                                          \
{                                                           \
    char p[PATH_MAX];                                       \
    snprintf(p, PATH_MAX, "%s%s/%s", base, name, #field);   \
    if (batm_read_int64(p, &snap->field) == 0)              \
        snap->meta_scanned_fields_amt++;                    \
    else                                                    \
        snap->field = INT64_MIN;                            \
}                                                           \
while (0)

#define BATM_READ_I32(field)                                \
do                                                          \
{                                                           \
    char p[PATH_MAX];                                       \
    snprintf(p, PATH_MAX, "%s%s/%s", base, name, #field);   \
    if (batm_read_int32(p, &snap->field) == 0)              \
        snap->meta_scanned_fields_amt++;                    \
    else                                                    \
        snap->field = INT32_MIN;                            \
}                                                           \
while (0)

#define BATM_READ_S(field)                                  \
do                                                          \
{                                                           \
    char p[PATH_MAX];                                       \
    snprintf(p, PATH_MAX, "%s%s/%s", base, name, #field);   \
    if (batm_read_str(p, snap->field, BATM_STR_MAX) == 0)   \
        snap->meta_scanned_fields_amt++;                    \
    else                                                    \
        snap->field[0] = '\0';                              \
}                                                           \
while (0)

