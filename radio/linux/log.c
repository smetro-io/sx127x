/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"

static int _level = 3;

static void now(char *result)
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    //Initializing it fully.
    struct tm r = { 0 };
    localtime_r(&(tv.tv_sec), &r);

    sprintf(result, "%02d:%02d:%02d.%03d",
        r.tm_hour, r.tm_min, r.tm_sec, (int)(tv.tv_usec / 1000));
}

int log_print_prefix(int level)
{
    if (level > _level) {
        return 0;
    } else {
        char result[15] = {0};
        now(result);
        fprintf(stderr, "%s: ", result);
        return 1;
    }
}

void log_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void log_set_level(int level)
{
    _level = level;
}