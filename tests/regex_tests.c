//
// Created by justi on 2025-07-11.
//
#include <assert.h>
#include <stdio.h>
#include "regex.h"

void test_basic_matching() {
	regex_match_t match;
	// Test simple literal matching
	assert(regex_match("hello", "hello", &match) == 1);
	assert(regex_match("hello", "world", &match) == 0);

	// Test dot character
	assert(regex_match("h.llo", "hello", &match) == 1);
	assert(regex_match("h.llo", "hallo", &match) == 1);
	assert(regex_match("h.llo", "hmllo", &match) == 1);
}

void test_character_classes() {
	regex_match_t match;
	// Test digit class
	assert(regex_match("[0-9]+", "123", &match) == 1);
	assert(regex_match("[0-9]+", "abc", &match) == 0);

	// Test word character class
	assert(regex_match("[a-zA-Z]+", "Hello", &match) == 1);
	assert(regex_match("[a-zA-Z]+", "123", &match) == 0);
}

void test_quantifiers() {
	regex_match_t match;
	// Test * quantifier (0 or more)
	assert(regex_match("a*b", "b", &match) == 1);
	assert(regex_match("a*b", "ab", &match) == 1);
	assert(regex_match("a*b", "aaab", &match) == 1);

	// Test + quantifier (1 or more)
	assert(regex_match("a+b", "b", &match) == 0);
	assert(regex_match("a+b", "ab", &match) == 1);
	assert(regex_match("a+b", "aaab", &match) == 1);

	// Test ? quantifier (0 or 1)
	assert(regex_match("a?b", "b", &match) == 1);
	assert(regex_match("a?b", "ab", &match) == 1);
	assert(regex_match("a?b", "aab", &match) == 0);
}

void test_groups() {

	regex_match_t match;
	// Test capturing groups
	assert(regex_match("(ab)+", "ab", &match) == 1);
	assert(regex_match("(ab)+", "abab", &match) == 1);
	assert(regex_match("(ab)+", "abc", &match) == 0);
}

int main() {
	test_basic_matching();
	test_character_classes();
	test_quantifiers();
	test_groups();

	printf("All tests passed!\n");
	return 0;
}