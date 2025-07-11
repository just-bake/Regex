//
// Created by justi on 2025-07-10.
//

#include "regex.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct regex {
	const char *pattern;
	int         is_word_boundary; // \b
	int         is_start_anchor;  // ^
	int         is_end_anchor;    // $
	char *      charset;          // for [] groups
	size_t      charset_len;
};

static int is_word_char(char c) {
	return isalnum((unsigned char) c) || c == '_';
}

static int check_word_boundary(const char *text, size_t pos) {
	int before_is_word = pos > 0 && is_word_char(text[pos - 1]);
	int after_is_word  = text[pos] && is_word_char(text[pos]);
	return before_is_word != after_is_word;
}

static int match_charset(const char *charset, size_t charset_len, char c) {
	int    negate = 0;
	size_t i      = 0;

	if (charset_len > 0 && charset[0] == '^') {
		negate = 1;
		i      = 1;
	}

	for (; i < charset_len; i++) {
		if (charset[i] == c) {
			return ! negate;
		}
		// Handle ranges like a-z
		if (i + 2 < charset_len && charset[i + 1] == '-') {
			if (c >= charset[i] && c <= charset[i + 2]) {
				return ! negate;
			}
			i += 2;
		}
	}
	return negate;
}

regex_t *regex_compile(const char *pattern) {
	if (! pattern)
		return NULL;

	regex_t *re = calloc(1, sizeof(regex_t));
	if (! re)
		return NULL;

	re->pattern = pattern;
	size_t len  = strlen(pattern);

	// Parse pattern
	size_t i = 0;
	if (pattern[0] == '^') {
		re->is_start_anchor = 1;
		i++;
	}
	if (len > 0 && pattern[len - 1] == '$') {
		re->is_end_anchor = 1;
		len--;
	}

	// Check for word boundary
	if (len >= 2 && pattern[i] == '\\' && pattern[i + 1] == 'b') {
		re->is_word_boundary = 1;
		i += 2;
	}

	// Handle character sets [...]
	if (pattern[i] == '[') {
		size_t start = i + 1;
		i++;
		while (i < len && pattern[i] != ']')
			i++;
		if (i < len) {
			re->charset_len = i - start;
			re->charset     = malloc(re->charset_len);
			if (re->charset) {
				memcpy(re->charset, pattern + start, re->charset_len);
			}
			i++; // skip closing ]
		}
	}

	return re;
}

int regex_match(const regex_t *re, const char *text, regex_match_t *match) {
	if (! re || ! text || ! match)
		return -1;

	size_t text_len = strlen(text);
	size_t pos      = 0;

	// Handle start anchor
	if (re->is_start_anchor && pos != 0) {
		return -1;
	}

	while (text[pos]) {
		// Handle word boundary
		if (re->is_word_boundary && ! check_word_boundary(text, pos)) {
			pos++;
			continue;
		}

		// Handle character sets
		if (re->charset) {
			if (! match_charset(re->charset, re->charset_len, text[pos])) {
				pos++;
				continue;
			}
			match->start  = pos;
			match->length = 1;
			return 0;
		}

		// Handle end anchor
		if (re->is_end_anchor && text[pos] != '\0') {
			pos++;
			continue;
		}

		// Simple literal match (for now)
		size_t pattern_pos = re->is_start_anchor ? 1 : 0;
		if (re->is_word_boundary)
			pattern_pos += 2;

		const char *pattern = re->pattern + pattern_pos;
		size_t      len     = strlen(pattern);
		if (re->is_end_anchor)
			len--;

		if (strncmp(text + pos, pattern, len) == 0) {
			match->start  = pos;
			match->length = len;
			return 0;
		}

		pos++;
	}

	return -1;
}

void regex_free(regex_t *re) {
	if (re) {
		free(re->charset);
		free(re);
	}
}
