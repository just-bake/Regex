//
// Created by justi on 2025-07-10.
//

#ifndef REGEX_H
#define REGEX_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

typedef struct regex regex_t;

typedef struct {
	size_t start;
	size_t length;
} regex_match_t;

/**
* Compiles a regular expression pattern.
* @param pattern The regex pattern string
* @return Compiled regex object or NULL on error
*/
regex_t *regex_compile(const char *pattern);

/**
* Attempts to match a string against a compiled regex.
* @param regex Compiled regex object
* @param text Text to match against
* @param match Output match information
* @return 0 on match, non-zero on no match
*/
int regex_match(const regex_t *regex, const char *text, regex_match_t *match);

/**
* Frees resources associated with a compiled regex.
*/
void regex_free(regex_t *regex);

#ifdef __cplusplus
}
#endif

#endif // REGEX_H
