/* See LICENSE file for copyright and license details. */
#include <stdint.h>

extern char buf[1024];

#define LEN(x) (sizeof (x) / sizeof *(x))

extern char *argv0;
extern const char* out_path;  //file path for the hwmon temperature input file

void warn(const char *, ...);
void die(const char *, ...);

int esnprintf(char *str, size_t size, const char *fmt, ...);
const char *bprintf(const char *fmt, ...);
const char *fmt_human(uintmax_t num, int base);
int pscanf(const char *path, const char *fmt, ...);

//----- functions related to finding the hwmon temperature input file
int dir_contains(char* dir_name, const char* file_name, char* start_path);
char* concat(char* dir_name, char* file_name);
const char* find_input_file(char* start_path);
