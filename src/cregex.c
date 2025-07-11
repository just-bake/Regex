//
// Created by justi on 2025-07-10.
//
#include "cregex.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef enum {
    RX_CHAR,
    RX_DOT,
    RX_CLASS,
    RX_STAR,
    RX_PLUS,
    RX_QUESTION,
    RX_START_ANCHOR,
    RX_END_ANCHOR
} RxNodeType;

typedef struct RxNode RxNode;

struct RxNode {
    RxNodeType type;
    union {
        char c; // for RX_CHAR
        struct {
            char* chars; // for RX_CLASS
            size_t len;
        } class_data;
        RxNode* child; // for quantifiers
    };
    RxNode* next;
};

struct cregex {
    RxNode* head;
};

// Forward declarations
static RxNode* parse_pattern(const char** pattern_ptr);
static void free_nodes(RxNode* node);
static size_t match_here(RxNode* node, const char* text);

// API implementations

cregex_t* cregex_compile(const char* pattern) {
    if (!pattern) return NULL;

    const char* p = pattern;
    RxNode* head = parse_pattern(&p);

    if (!head) return NULL;

    cregex_t* regex = malloc(sizeof(cregex_t));
    if (!regex) {
        free_nodes(head);
        return NULL;
    }
    regex->head = head;

    return regex;
}

void cregex_free(cregex_t* regex) {
    if (!regex) return;
    free_nodes(regex->head);
    free(regex);
}

size_t cregex_match(const cregex_t* regex, const char* text) {
    if (!regex || !text) return 0;

    return match_here(regex->head, text);
}

// --- Parsing and node creation ---

static RxNode* create_node(RxNodeType type) {
    RxNode* node = malloc(sizeof(RxNode));
    if (!node) return NULL;
    node->type = type;
    node->next = NULL;
    node->child = NULL;
    node->class_data.chars = NULL;
    node->class_data.len = 0;
    return node;
}

static void free_nodes(RxNode* node) {
    while (node) {
        RxNode* next = node->next;
        if (node->type == RX_CLASS && node->class_data.chars) {
            free(node->class_data.chars);
        }
        if ((node->type == RX_STAR || node->type == RX_PLUS || node->type == RX_QUESTION) && node->child) {
            free_nodes(node->child);
        }
        free(node);
        node = next;
    }
}

// Utility: parse char class "[abc]" or "[a-z]"
static RxNode* parse_char_class(const char** pattern_ptr) {
    const char* p = *pattern_ptr;
    if (*p != '[') return NULL;
    p++; // skip '['

    char buf[256];
    size_t idx = 0;

    while (*p && *p != ']') {
        if (*p == '\\' && *(p+1)) {
            buf[idx++] = *(p+1);
            p += 2;
        } else if (*(p+1) == '-' && *(p+2) && *(p+2) != ']') {
            // handle range e.g. a-z
            char start = *p;
            char end = *(p+2);
            if (start > end) return NULL; // invalid range
            for (char c = start; c <= end; c++) {
                if (idx < sizeof(buf)) buf[idx++] = c;
            }
            p += 3;
        } else {
            buf[idx++] = *p++;
        }
        if (idx >= sizeof(buf)) break;
    }

    if (*p != ']') return NULL; // no closing bracket
    p++; // skip ']'

    RxNode* node = create_node(RX_CLASS);
    if (!node) return NULL;
    node->class_data.chars = malloc(idx);
    if (!node->class_data.chars) {
        free(node);
        return NULL;
    }
    memcpy(node->class_data.chars, buf, idx);
    node->class_data.len = idx;

    *pattern_ptr = p;
    return node;
}

// Parse one atom: char, dot, charclass, anchors
static RxNode* parse_atom(const char** pattern_ptr) {
    const char* p = *pattern_ptr;
    RxNode* node = NULL;

    if (*p == '\0') return NULL;

    if (*p == '^') {
        node = create_node(RX_START_ANCHOR);
        p++;
    } else if (*p == '$') {
        node = create_node(RX_END_ANCHOR);
        p++;
    } else if (*p == '.') {
        node = create_node(RX_DOT);
        p++;
    } else if (*p == '[') {
        node = parse_char_class(&p);
        if (!node) return NULL;
    } else if (*p == '\\') {
        p++;
        if (*p == '\0') return NULL;
        node = create_node(RX_CHAR);
        node->c = *p++;
    } else {
        node = create_node(RX_CHAR);
        node->c = *p++;
    }

    *pattern_ptr = p;
    return node;
}

// Parse quantifiers *, +, ?
static RxNode* parse_quantifier(RxNode* atom, const char** pattern_ptr) {
    const char* p = *pattern_ptr;
    if (!atom) return NULL;

    if (*p == '*') {
        RxNode* node = create_node(RX_STAR);
        node->child = atom;
        p++;
        *pattern_ptr = p;
        return node;
    } else if (*p == '+') {
        RxNode* node = create_node(RX_PLUS);
        node->child = atom;
        p++;
        *pattern_ptr = p;
        return node;
    } else if (*p == '?') {
        RxNode* node = create_node(RX_QUESTION);
        node->child = atom;
        p++;
        *pattern_ptr = p;
        return node;
    }

    return atom;
}

// Parse sequence of atoms and quantifiers
static RxNode* parse_pattern(const char** pattern_ptr) {
    RxNode *head = NULL, *tail = NULL;

    while (**pattern_ptr) {
        RxNode* atom = parse_atom(pattern_ptr);
        if (!atom) break;

        RxNode* node = parse_quantifier(atom, pattern_ptr);
        if (!node) {
            free_nodes(atom);
            break;
        }

        if (!head) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    return head;
}

// --- Matching functions ---

// Check if char is in char class
static int is_in_class(char c, RxNode* class_node) {
    for (size_t i = 0; i < class_node->class_data.len; i++) {
        if (class_node->class_data.chars[i] == c)
            return 1;
    }
    return 0;
}

// Forward declaration
static size_t match_here(RxNode* node, const char* text);

static size_t match_star(RxNode* node, const char* text) {
    size_t max_len = 0;
    size_t len = 0;

    // Match 0 or more occurrences of child pattern
    // Try to consume as much as possible (greedy)
    while ((len = match_here(node->child, text)) > 0) {
        text += len;
        max_len += len;
    }

    // Then match the rest of the pattern after *
    size_t rest = match_here(node->next, text);
    if (rest == 0 && node->next != NULL)
        return 0;

    return max_len + rest;
}

static size_t match_plus(RxNode* node, const char* text) {
    size_t len = match_here(node->child, text);
    if (len == 0) return 0;

    text += len;
    size_t max_len = len;

    while ((len = match_here(node->child, text)) > 0) {
        text += len;
        max_len += len;
    }

    size_t rest = match_here(node->next, text);
    if (rest == 0 && node->next != NULL)
        return 0;

    return max_len + rest;
}

static size_t match_question(RxNode* node, const char* text) {
    size_t len = match_here(node->child, text);
    if (len > 0) {
        size_t rest = match_here(node->next, text + len);
        if (rest > 0)
            return len + rest;
    }
    return match_here(node->next, text);
}

static size_t match_here(RxNode* node, const char* text) {
    if (!node) return 0; // End of pattern matches empty string

    switch (node->type) {
        case RX_START_ANCHOR:
            if (text != NULL && text[0] != '\0') {
                if (text == text) // Always true; start anchor only matches start of string
                    return match_here(node->next, text);
                return 0;
            }
            return 0;

        case RX_END_ANCHOR:
            if (text[0] == '\0')
                return 0;
            else
                return 0;

        case RX_CHAR:
            if (text[0] == node->c)
                return 1 + match_here(node->next, text + 1);
            else
                return 0;

        case RX_DOT:
            if (text[0] != '\0')
                return 1 + match_here(node->next, text + 1);
            else
                return 0;

        case RX_CLASS:
            if (text[0] != '\0' && is_in_class(text[0], node))
                return 1 + match_here(node->next, text + 1);
            else
                return 0;

        case RX_STAR:
            return match_star(node, text);

        case RX_PLUS:
            return match_plus(node, text);

        case RX_QUESTION:
            return match_question(node, text);

        default:
            return 0;
    }
}

