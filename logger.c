#include "logger.h"

#if defined(WITH_DEBUG)
#include <stdio.h>

#define DUMPSIZE                32
#define val(buf, i)             (((unsigned char *)buf)[i])

void dump(const char *prefix, const void *buf, size_t len)
{
	char hex[3 * DUMPSIZE + 1], *h;
	char str[DUMPSIZE + 1], *s;
	size_t i;

	for (i = 0; i < len;) {
		for (h = hex, s = str;
		     h + 1 < hex + sizeof(hex) && i < len;
		     h += 3, s++, i++) {
			snprintf(h, 4, "%.2X ", val(buf, i));

			if (val(buf, i) < 0x20 || val(buf, i) > 0x7E) {
				snprintf(s, 2, ".");

			} else {
				snprintf(s, 2, "%c", val(buf, i));
			}
		}

		debug("%10s: %-*s |%s|", prefix, 3 * DUMPSIZE, hex, str);
	}
}
#endif
