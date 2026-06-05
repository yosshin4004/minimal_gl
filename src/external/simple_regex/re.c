/*
 * Tiny C Regex with Captures
 *
 * This is free and unencumbered software released into the public domain.
 * For more information, please refer to <http://unlicense.org/>
 */

#include "re.h"
#include <stddef.h>

typedef struct {
  re_t* re;
  const char* p;
  int ccl_buf_idx;
  int group_depth;
} re_compile_state_t;

typedef struct {
  int head;
  int tail;
} re_frag_t;

typedef struct {
  re_cap_t* caps;
  int n_caps;
  const char* text_start;
  int steps_left;
} re_match_state_t;

typedef enum {
  RE_CONT_SUCCESS,
  RE_CONT_REPEAT,
  RE_CONT_GROUP_END
} re_cont_type_t;

typedef struct re_cont {
  re_cont_type_t type;
  int node_idx;
  int count;
  int cap_idx;
  const char* prev_text;
  const struct re_cont* parent;
} re_cont_t;

static int re_parse_expr(re_compile_state_t* cs, re_frag_t* out);
static int re_parse_seq(re_compile_state_t* cs, re_frag_t* out);
static int re_parse_atom(re_compile_state_t* cs, int* out);
static int re_new_node(re_compile_state_t* cs, re_type_t type);
static void re_patch_next(re_t* re, int node_idx, int next);
static int re_match_sequence_c(re_t* re, int node_idx, const char* text, re_match_state_t* state, const char** out, const re_cont_t* cont);
static int re_match_repeat_c(re_t* re, int node_idx, const char* text, re_match_state_t* state, const char** out, int count, const re_cont_t* cont);
static int re_match_atom_c(re_t* re, int node_idx, const char* text, re_match_state_t* state, const char** out, const re_cont_t* cont);
static int re_apply_cont(re_t* re, const re_cont_t* cont, const char* text, re_match_state_t* state, const char** out);
static int re_match_one(re_node_t* node, char c);

static int re_new_node(re_compile_state_t* cs, re_type_t type) {
  re_t* re = cs->re;
  if (re->num_nodes >= RE_MAX_NODES) return -1;

  int idx = re->num_nodes++;
  re->nodes[idx].type = type;
  re->nodes[idx].child = -1;
  re->nodes[idx].next = -1;
  re->nodes[idx].min = 1;
  re->nodes[idx].max = 1;
  return idx;
}

static void re_patch_next(re_t* re, int node_idx, int next) {
  if (node_idx >= 0) re->nodes[node_idx].next = next;
}

int re_compile(re_t* re, const char* pattern) {
  if (!re || !pattern) return -1;

  re->num_nodes = 0;
  re->num_captures = 1; /* Capture 0 is the whole match. */
  re->root = -1;

  re_compile_state_t cs;
  cs.re = re;
  cs.p = pattern;
  cs.ccl_buf_idx = 0;
  cs.group_depth = 0;

  re_frag_t root;
  if (!re_parse_expr(&cs, &root)) return -1;
  if (*cs.p != '\0') return -1;

  re->root = root.head;
  return 0;
}

static int re_parse_expr(re_compile_state_t* cs, re_frag_t* out) {
  re_frag_t left;
  if (!re_parse_seq(cs, &left)) return 0;

  if (*cs->p != '|') {
    *out = left;
    return 1;
  }

  cs->p++;
  re_frag_t right;
  if (!re_parse_expr(cs, &right)) return 0;

  int idx = re_new_node(cs, RE_BRANCH);
  if (idx < 0) return 0;
  cs->re->nodes[idx].u.branch.left = left.head;
  cs->re->nodes[idx].u.branch.right = right.head;
  out->head = idx;
  out->tail = idx;
  return 1;
}

static int re_parse_seq(re_compile_state_t* cs, re_frag_t* out) {
  out->head = -1;
  out->tail = -1;

  while (*cs->p != '\0' && *cs->p != ')' && *cs->p != '|') {
    int atom;
    if (!re_parse_atom(cs, &atom)) return 0;

    if (*cs->p == '*' || *cs->p == '+' || *cs->p == '?') {
      if (cs->re->nodes[atom].type == RE_BEGIN || cs->re->nodes[atom].type == RE_END) return 0;
      if (*cs->p == '*') {
        cs->re->nodes[atom].min = 0;
        cs->re->nodes[atom].max = -1;
      } else if (*cs->p == '+') {
        cs->re->nodes[atom].min = 1;
        cs->re->nodes[atom].max = -1;
      } else {
        cs->re->nodes[atom].min = 0;
        cs->re->nodes[atom].max = 1;
      }
      cs->p++;
      if (*cs->p == '*' || *cs->p == '+' || *cs->p == '?') return 0;
    }

    if (out->head < 0) {
      out->head = atom;
    } else {
      re_patch_next(cs->re, out->tail, atom);
    }
    out->tail = atom;
  }

  return 1;
}

static int re_parse_atom(re_compile_state_t* cs, int* out) {
  re_t* re = cs->re;
  int idx;

  switch (*cs->p) {
    case '\0':
    case ')':
    case '|':
    case '*':
    case '+':
    case '?':
      return 0;

    case '^':
      idx = re_new_node(cs, RE_BEGIN);
      if (idx < 0) return 0;
      cs->p++;
      break;

    case '$':
      idx = re_new_node(cs, RE_END);
      if (idx < 0) return 0;
      cs->p++;
      break;

    case '.':
      idx = re_new_node(cs, RE_DOT);
      if (idx < 0) return 0;
      cs->p++;
      break;

    case '(': {
      int cap_idx = -1;
      cs->p++;

      if (*cs->p == '?') {
        if (cs->p[1] != ':') return 0;
        cs->p += 2;
      } else {
        if (re->num_captures >= RE_MAX_CAPTURES) return 0;
        cap_idx = re->num_captures++;
      }

      if (cs->group_depth >= RE_MAX_GROUP_DEPTH) return 0;
      cs->group_depth++;

      re_frag_t child;
      if (!re_parse_expr(cs, &child)) return 0;
      if (*cs->p != ')') return 0;
      cs->p++;
      cs->group_depth--;

      idx = re_new_node(cs, RE_GROUP);
      if (idx < 0) return 0;
      re->nodes[idx].u.cap_idx = cap_idx;
      re->nodes[idx].child = child.head;
      break;
    }

    case '\\':
      cs->p++;
      if (*cs->p == '\0') return 0;
      switch (*cs->p) {
        case 'd': idx = re_new_node(cs, RE_DIGIT); break;
        case 'D': idx = re_new_node(cs, RE_NOT_DIGIT); break;
        case 'w': idx = re_new_node(cs, RE_ALPHA); break;
        case 'W': idx = re_new_node(cs, RE_NOT_ALPHA); break;
        case 's': idx = re_new_node(cs, RE_WHITESPACE); break;
        case 'S': idx = re_new_node(cs, RE_NOT_WHITESPACE); break;
        default:
          idx = re_new_node(cs, RE_CHAR);
          if (idx >= 0) re->nodes[idx].u.ch = (unsigned char)*cs->p;
          break;
      }
      if (idx < 0) return 0;
      cs->p++;
      break;

    case '[': {
      cs->p++;
      if (*cs->p == '\0') return 0;

      idx = re_new_node(cs, (*cs->p == '^') ? RE_INV_CHAR_CLASS : RE_CHAR_CLASS);
      if (idx < 0) return 0;
      if (*cs->p == '^') {
        cs->p++;
        if (*cs->p == '\0') return 0;
      }

      re->nodes[idx].u.ccl = &re->char_class_buffer[cs->ccl_buf_idx];
      if (*cs->p == ']') {
        if (cs->ccl_buf_idx >= RE_MAX_CHAR_CLASS_LEN - 1) return 0;
        re->char_class_buffer[cs->ccl_buf_idx++] = (unsigned char)*cs->p++;
      }
      while (*cs->p != ']' && *cs->p != '\0') {
        if (cs->ccl_buf_idx >= RE_MAX_CHAR_CLASS_LEN - 1) return 0;
        re->char_class_buffer[cs->ccl_buf_idx++] = (unsigned char)*cs->p++;
      }
      if (*cs->p != ']') return 0;
      if (cs->ccl_buf_idx >= RE_MAX_CHAR_CLASS_LEN) return 0;
      re->char_class_buffer[cs->ccl_buf_idx++] = '\0';
      cs->p++;
      break;
    }

    default:
      idx = re_new_node(cs, RE_CHAR);
      if (idx < 0) return 0;
      re->nodes[idx].u.ch = (unsigned char)*cs->p++;
      break;
  }

  *out = idx;
  return 1;
}

static void re_save_caps(re_match_state_t* state, re_cap_t* saved) {
  for (int i = 0; i < state->n_caps; i++) saved[i] = state->caps[i];
}

static void re_restore_caps(re_match_state_t* state, const re_cap_t* saved) {
  for (int i = 0; i < state->n_caps; i++) state->caps[i] = saved[i];
}

static int re_match_ccl(const unsigned char* ccl, char c) {
  while (*ccl != '\0') {
    if (*ccl == '\\') {
      ccl++;
      if (*ccl == '\0') break;
      switch (*ccl) {
        case 'd': if (c >= '0' && c <= '9') return 1; break;
        case 'w': if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')) return 1; break;
        case 's': if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v') return 1; break;
        default:  if (*ccl == (unsigned char)c) return 1; break;
      }
    } else if (ccl[1] == '-' && ccl[2] != '\0') {
      if ((unsigned char)c >= ccl[0] && (unsigned char)c <= ccl[2]) return 1;
      ccl += 2;
    } else {
      if (*ccl == (unsigned char)c) return 1;
    }
    ccl++;
  }
  return 0;
}

static int re_match_one(re_node_t* node, char c) {
  switch (node->type) {
    case RE_DOT: return 1;
    case RE_CHAR: return (node->u.ch == (unsigned char)c);
    case RE_DIGIT: return (c >= '0' && c <= '9');
    case RE_NOT_DIGIT: return !(c >= '0' && c <= '9');
    case RE_ALPHA: return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
    case RE_NOT_ALPHA: return !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'));
    case RE_WHITESPACE: return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
    case RE_NOT_WHITESPACE: return !(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
    case RE_CHAR_CLASS:
    case RE_INV_CHAR_CLASS: {
      int match = re_match_ccl(node->u.ccl, c);
      return (node->type == RE_CHAR_CLASS) ? match : !match;
    }
    default: return 0;
  }
}

static int re_match_sequence_c(re_t* re, int node_idx, const char* text, re_match_state_t* state, const char** out, const re_cont_t* cont) {
  if (node_idx < 0) {
    return re_apply_cont(re, cont, text, state, out);
  }
  return re_match_repeat_c(re, node_idx, text, state, out, 0, cont);
}

static int re_match_repeat_c(re_t* re, int node_idx, const char* text, re_match_state_t* state, const char** out, int count, const re_cont_t* cont) {
  re_node_t* node = &re->nodes[node_idx];
  int can_repeat = (node->max < 0 || count < node->max);

  if (can_repeat) {
    re_cap_t saved[RE_MAX_CAPTURES];
    re_cont_t repeat_cont;

    repeat_cont.type = RE_CONT_REPEAT;
    repeat_cont.node_idx = node_idx;
    repeat_cont.count = count + 1;
    repeat_cont.cap_idx = -1;
    repeat_cont.prev_text = text;
    repeat_cont.parent = cont;

    re_save_caps(state, saved);
    if (re_match_atom_c(re, node_idx, text, state, out, &repeat_cont)) return 1;
    re_restore_caps(state, saved);
  }

  if (count >= node->min) {
    return re_match_sequence_c(re, node->next, text, state, out, cont);
  }

  return 0;
}

static int re_apply_cont(re_t* re, const re_cont_t* cont, const char* text, re_match_state_t* state, const char** out) {
  if (cont->type == RE_CONT_SUCCESS) {
    *out = text;
    return 1;
  }

  if (cont->type == RE_CONT_GROUP_END) {
    re_cap_t saved[RE_MAX_CAPTURES];
    re_save_caps(state, saved);
    if (cont->cap_idx >= 0 && cont->cap_idx < state->n_caps) {
      state->caps[cont->cap_idx].end = text;
    }
    if (re_apply_cont(re, cont->parent, text, state, out)) return 1;
    re_restore_caps(state, saved);
    return 0;
  }

  if (cont->type == RE_CONT_REPEAT) {
    re_node_t* node = &re->nodes[cont->node_idx];
    if (text == cont->prev_text && cont->count >= node->min) {
      return re_match_sequence_c(re, node->next, text, state, out, cont->parent);
    }
    return re_match_repeat_c(re, cont->node_idx, text, state, out, cont->count, cont->parent);
  }

  return 0;
}

static int re_match_atom_c(re_t* re, int node_idx, const char* text, re_match_state_t* state, const char** out, const re_cont_t* cont) {
  if (state->steps_left-- <= 0) return 0;

  re_node_t* node = &re->nodes[node_idx];

  switch (node->type) {
    case RE_BEGIN:
      if (text != state->text_start) return 0;
      return re_apply_cont(re, cont, text, state, out);

    case RE_END:
      if (*text != '\0') return 0;
      return re_apply_cont(re, cont, text, state, out);

    case RE_GROUP: {
      re_cap_t saved[RE_MAX_CAPTURES];
      re_cont_t group_cont;

      group_cont.type = RE_CONT_GROUP_END;
      group_cont.node_idx = -1;
      group_cont.count = 0;
      group_cont.cap_idx = node->u.cap_idx;
      group_cont.prev_text = NULL;
      group_cont.parent = cont;

      re_save_caps(state, saved);
      if (node->u.cap_idx >= 0 && node->u.cap_idx < state->n_caps) {
        state->caps[node->u.cap_idx].start = text;
      }
      if (re_match_sequence_c(re, node->child, text, state, out, &group_cont)) return 1;
      re_restore_caps(state, saved);
      return 0;
    }

    case RE_BRANCH: {
      re_cap_t saved[RE_MAX_CAPTURES];
      re_save_caps(state, saved);
      if (re_match_sequence_c(re, node->u.branch.left, text, state, out, cont)) return 1;
      re_restore_caps(state, saved);
      return re_match_sequence_c(re, node->u.branch.right, text, state, out, cont);
    }

    default:
      if (*text != '\0' && re_match_one(node, *text)) {
        return re_apply_cont(re, cont, text + 1, state, out);
      }
      return 0;
  }
}

int re_cap_copy(const re_cap_t* cap, char* buf, int buf_size) {
  if (!cap || !cap->start || !cap->end || !buf || buf_size <= 0) return 0;

  int len = (int)(cap->end - cap->start);
  if (len >= buf_size) len = buf_size - 1;

  for (int i = 0; i < len; i++) {
    buf[i] = cap->start[i];
  }
  buf[len] = '\0';

  return len;
}

int re_match(re_t* re, const char* text, re_cap_t* captures, int n_caps) {
  if (!re || !text) return -1;

  re_cap_t internal_caps[1];
  re_cap_t* p_caps = (captures && n_caps > 0) ? captures : internal_caps;
  int p_n_caps = (captures && n_caps > 0) ? n_caps : 1;

  re_match_state_t state;
  state.caps = p_caps;
  state.n_caps = p_n_caps;
  state.text_start = text;
  state.steps_left = RE_MAX_MATCH_STEPS;

  const char* p = text;
  re_cont_t success_cont;
  success_cont.type = RE_CONT_SUCCESS;
  success_cont.node_idx = -1;
  success_cont.count = 0;
  success_cont.cap_idx = -1;
  success_cont.prev_text = NULL;
  success_cont.parent = NULL;

  do {
    for (int i = 0; i < p_n_caps; i++) {
      p_caps[i].start = NULL;
      p_caps[i].end = NULL;
    }

    p_caps[0].start = p;
    const char* end = p;
    if (re_match_sequence_c(re, re->root, p, &state, &end, &success_cont)) {
      p_caps[0].end = end;
      int count = 0;
      for (int i = 0; i < p_n_caps; i++) {
        if (p_caps[i].start != NULL && p_caps[i].end != NULL) count++;
      }
      return count;
    }
  } while (*p++ != '\0' && (re->root < 0 || re->nodes[re->root].type != RE_BEGIN));

  return -1;
}
