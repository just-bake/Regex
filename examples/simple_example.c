//
// Created by justi on 2025-07-10.
//
#include <stdio.h>
#include "cregex.h"

int main(void) {
	const char* pattern = "[0-9]+";
	const char* text = "120938 hello123 world";

	cregex_t* regex = cregex_compile(pattern);
	if (!regex) {
		printf("Failed to compile regex.\n");
		return 1;
	}

	size_t len = cregex_match(regex, text);
	if (len > 0) {
		printf("Matched '%.*s'\n", (int)len, text);
	} else {
		printf("No match\n");
	}

	cregex_free(regex);
	return 0;
}
