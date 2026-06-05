/*
 * Tiny C Regex with Captures
 *
 * This is free and unencumbered software released into the public domain.
 * For more information, please refer to <http://unlicense.org/>
 */

#ifndef RE_H
#define RE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration */
#ifndef RE_MAX_NODES
#define RE_MAX_NODES 512
#endif

#ifndef RE_MAX_CHAR_CLASS_LEN
#define RE_MAX_CHAR_CLASS_LEN 256
#endif

#ifndef RE_MAX_CAPTURES
#define RE_MAX_CAPTURES 10
#endif

#ifndef RE_MAX_GROUP_DEPTH
#define RE_MAX_GROUP_DEPTH RE_MAX_NODES
#endif

#ifndef RE_MAX_MATCH_STEPS
#define RE_MAX_MATCH_STEPS 100000
#endif


/* Capture result structure */
typedef struct {
  const char* start;
  const char* end;
} re_cap_t;

/* Regex types */
typedef enum { 
  RE_UNUSED, RE_DOT, RE_BEGIN, RE_END, RE_QUESTIONMARK, RE_STAR, RE_PLUS, 
  RE_CHAR, RE_CHAR_CLASS, RE_INV_CHAR_CLASS, RE_DIGIT, RE_NOT_DIGIT, 
  RE_ALPHA, RE_NOT_ALPHA, RE_WHITESPACE, RE_NOT_WHITESPACE,
  RE_GROUP, RE_BRANCH
} re_type_t;

/* Compiled regex node structure */
typedef struct {
  re_type_t type;
  union {
    unsigned char ch;    /* RE_CHAR */
    unsigned char* ccl;  /* RE_CHAR_CLASS, RE_INV_CHAR_CLASS */
    int cap_idx;         /* RE_GROUP */
    struct {
      int left;
      int right;
    } branch;            /* RE_BRANCH */
  } u;
  int child;             /* RE_GROUP */
  int next;              /* Next node in sequence, or -1 */
  int min;               /* Quantifier lower bound */
  int max;               /* Quantifier upper bound, -1 for unbounded */
} re_node_t;

/* Main regex structure */
typedef struct {
  re_node_t nodes[RE_MAX_NODES];
  unsigned char char_class_buffer[RE_MAX_CHAR_CLASS_LEN];
  int num_nodes;
  int num_captures;
  int root;
} re_t;


/**
 * Compile pattern to internal representation.
 * Returns 0 on success, -1 on error.
 */
int re_compile(re_t* re, const char* pattern);

/**
 * Match text against compiled regex.
 * Captures are stored in `captures` array (captures[0] is the whole match).
 * If `captures` is NULL and `n_caps` is 0, it performs a simple match check.
 * Returns the number of matched capture groups (including group 0) on success, 
 * or -1 if no match found.
 */
int re_match(re_t* re, const char* text, re_cap_t* captures, int n_caps);

/**
 * Helper to copy a capture to a null-terminated string.
 * Returns the number of characters copied (excluding null-terminator).
 */
int re_cap_copy(const re_cap_t* cap, char* buf, int buf_size);


#ifdef __cplusplus
}
#endif

#endif /* RE_H */
