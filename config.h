#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Version number of package */
#ifndef VERSION
#define VERSION			"1.0-devel"
#endif

/* Define to the full name of this package. */
#define PACKAGE_NAME            "papple"

/* Define to the version of this package. */
#define PACKAGE_VERSION         VERSION

/* Define to the full name and version of this package. */
#define PACKAGE_STRING          PACKAGE_NAME " " PACKAGE_VERSION

/* Define to the home page for this package. */
#define PACKAGE_URL             "http://github.com/jmlemetayer/papple"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT       "http://github.com/jmlemetayer/papple/issues"

/* Define the default service */
#define DEFAULT_SERVICE         "http-alt"

/* Define the wildcard interface */
#define WILDCARD_INTERFACE      "any"

/* Define the default interface */
#define DEFAULT_INTERFACE       WILDCARD_INTERFACE

#endif
