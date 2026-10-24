/* Declarations for the PureC OS mini-snprintf used by wolfSSL. Force-included
 * (-include) in the userspace library build so asn.c call sites compile. */

#ifndef PUREC_SNPRINTF_H
#define PUREC_SNPRINTF_H

#include <stddef.h>
#include <stdarg.h>

int purec_vsnprintf(char *out, size_t size, const char *fmt, va_list ap);
int purec_snprintf(char *out, size_t size, const char *fmt, ...);

#endif /* PUREC_SNPRINTF_H */
