#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <errno.h>
#include <string.h>

#ifdef WITH_SYSLOG
#include <syslog.h>

#define LOGGER_DEBUG                    LOG_DEBUG
#define LOGGER_INFO                     LOG_INFO
#define LOGGER_NOTICE                   LOG_NOTICE
#define LOGGER_WARNING                  LOG_WARNING
#define LOGGER_ERROR                    LOG_ERR
#define LOGGER_CRITICAL                 LOG_CRIT

#define logger(level, format, args...)  syslog(level, format, ##args)

#else
#include <stdio.h>

#define LOGGER_DEBUG                    stderr
#define LOGGER_INFO                     stdout
#define LOGGER_NOTICE                   stdout
#define LOGGER_WARNING                  stdout
#define LOGGER_ERROR                    stderr
#define LOGGER_CRITICAL                 stderr

#define logger(level, format, args...)  fprintf(level, format "\n", ##args)
#endif

#ifdef WITH_DEBUG
#define debug(format, args...)          logger(LOGGER_DEBUG, "D/ " format, ##args)
void dump(const char *prefix, const void *buf, size_t len);
#else
#define debug(...)
#define dump(...)
#endif

#define info(format, args...)           logger(LOGGER_INFO, "I/ " format, ##args)
#define notice(format, args...)         logger(LOGGER_NOTICE, "N/ " format, ##args)
#define warning(format, args...)        logger(LOGGER_WARNING, "W/ " format, ##args)
#define error(format, args...)          logger(LOGGER_ERROR, "E/ " format, ##args)
#define critical(format, args...)       logger(LOGGER_CRITICAL, "C/ " format, ##args)

#define ewarning(format, args...)       warning(format ": %s", ##args, strerror(errno))
#define eerror(format, args...)         error(format ": %s", ##args, strerror(errno))
#define ecritical(format, args...)      critical(format ": %s", ##args, strerror(errno))

#endif
