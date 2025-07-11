//
// Created by justi on 2025-07-10.
//
#ifndef CREGEX_H
#define CREGEX_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
	#endif

	typedef struct cregex cregex_t;

	/**
	 * Compile a regex pattern.
	 * Returns NULL on error.
	 */
	cregex_t *cregex_compile(const char *pattern);

	/**
	 * Free regex resources.
	 */
	void cregex_free(cregex_t *regex);

	/**
	 * Match regex at start of text.
	 * Returns length of matched substring, or 0 if no match.
	 */
	size_t cregex_match(const cregex_t *regex, const char *text);

	#ifdef __cplusplus
}
#endif

#endif // CREGEX_H
