/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

char *argv0;
const char* out_path; //file path for the hwmon temperature input file

static void
verr(const char *fmt, va_list ap)
{
	if (argv0 && strncmp(fmt, "usage", sizeof("usage") - 1)) {
		fprintf(stderr, "%s: ", argv0);
	}

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
}

void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
}

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);

	exit(1);
}

static int
evsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
	int ret;

	ret = vsnprintf(str, size, fmt, ap);

	if (ret < 0) {
		warn("vsnprintf:");
		return -1;
	} else if ((size_t)ret >= size) {
		warn("vsnprintf: Output truncated");
		return -1;
	}

	return ret;
}

int
esnprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = evsnprintf(str, size, fmt, ap);
	va_end(ap);

	return ret;
}

const char *
bprintf(const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = evsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return (ret < 0) ? NULL : buf;
}

const char *
fmt_human(uintmax_t num, int base)
{
	double scaled;
	size_t i, prefixlen;
	const char **prefix;
	const char *prefix_1000[] = { "", "k", "M", "G", "T", "P", "E", "Z",
	                              "Y" };
	const char *prefix_1024[] = { "", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei",
	                              "Zi", "Yi" };

	switch (base) {
	case 1000:
		prefix = prefix_1000;
		prefixlen = LEN(prefix_1000);
		break;
	case 1024:
		prefix = prefix_1024;
		prefixlen = LEN(prefix_1024);
		break;
	default:
		warn("fmt_human: Invalid base");
		return NULL;
	}

	scaled = num;
	for (i = 0; i < prefixlen && scaled >= base; i++) {
		scaled /= base;
	}

	return bprintf("%.1f %s", scaled, prefix[i]);
}

int
pscanf(const char *path, const char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	int n;

	if (!(fp = fopen(path, "r"))) {
		warn("fopen '%s':", path);
		return -1;
	}
	va_start(ap, fmt);
	n = vfscanf(fp, fmt, ap);
	va_end(ap);
	fclose(fp);

	return (n == EOF) ? -1 : n;
}


//----- functions related to finding the hwmon temperature input file

int dir_contains(char* dir_name, const char* file_name, char* start_path){
    char* x;
    sprintf(dir_name, "%s/%s", start_path, (x=strdup(dir_name)));
    free(x);
    DIR* c = opendir(dir_name);
    while(1){
        struct dirent* entry;
        const char* name;
        entry = readdir(c);
        if(!entry) break;
        name = entry->d_name;
        if(strcmp(file_name, name) == 0) return 1;
    }
    return 0;
}

char* concat(char* dir_name, char* file_name){
    char* result = malloc(strlen(dir_name) + strlen(file_name));
    strcat(result, dir_name);
    strcat(result, "/");
    strcat(result, file_name);
    return result;
}

const char* find_input_file(char* start_path){
    char* input_file = "temp1_input";
    DIR* d = opendir(start_path);
    while(1){
        struct dirent* entry;
        char* d_name;
        entry = readdir(d);
        if(!entry){
            closedir(d);
            break; //no more entries in this directory
        }
        d_name = entry->d_name;
        if(strcmp(d_name, ".") != 0 && strcmp(d_name, "..") != 0){
            if(dir_contains(d_name, input_file, start_path)){
                out_path = concat(d_name, input_file);
                break;
            }
        }
    }
    return out_path;
}
