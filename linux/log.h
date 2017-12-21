/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ERROR,
    WARNING,
    INFO,
    DEBUG
} log_t;

#define LOG(l, x)                                                    \
do {                                                                 \
    if (log_print_prefix(l)) log_printf x; \
} while (0)

int log_print_prefix(int level);
void log_printf(const char *fmt, ...);
void log_set_level(int level);

#ifdef __cplusplus
}
#endif

#endif /* _LOG_H_ */
