#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

#define INITIAL_CAP 4

#define UNEXPECTED_EOL(l, expected_msg)                                        \
  do {                                                                         \
    fprintf(                                                                   \
        stderr,                                                                \
        "%s:%d:[ERROR]: Unexpected end of line at position %zu (line %zu, "    \
        "column %zu). Expected %s\n",                                          \
        __FILE__, __LINE__, (l)->position + 1, (l)->line + 1, (l)->column + 1, \
        (expected_msg));                                                       \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
#define UNEXPECTED_TOKEN(l, expected_msg)                                      \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "%s:%d:[ERROR]: Unexpected token '%c' at position %zu (line %zu, " \
            "column %zu). Expected %s\n",                                      \
            __FILE__, __LINE__, *(l)->content, (l)->position + 1,              \
            (l)->line + 1, (l)->column + 1, (expected_msg));                   \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

static char __hctoh(char hex_ch) {
  if ('0' <= hex_ch && hex_ch <= '9') {
    return hex_ch - '0';
  } else if ('a' <= hex_ch && hex_ch <= 'f') {
    return hex_ch - 'a' + 10;
  } else if ('A' <= hex_ch && hex_ch <= 'F') {
    return hex_ch - 'A' + 10;
  } else {
    BAD_ARG("hex_ch");
  }
}

static unsigned int __atoh(const char *hex_str, size_t hex_str_len) {
  size_t i;
  unsigned int hex_uint = 0;

  for (i = 0; i < hex_str_len; i++) {
    char ch = hex_str[i];
    if (!helpers_ishexdigit(ch)) {
      BAD_ARG("hex_str");
    }
    hex_uint <<= 4;
    hex_uint |= __hctoh(ch);
  }

  return hex_uint;
}

/*
https://en.wikipedia.org/wiki/UTF-8
UTF-8 encodes code points in one to four bytes, depending on the
   value of
the code point. In the following table, the characters u to z, each
   representing
a hexadecimal digit, are replaced by their constituent 4 bits uuuu to
   zzzz, from
the positions U+uvwxyz:
Code point ↔ UTF-8 conversion
First cp	Last cp	 Byte 1		Byte 2		Byte 3   Byte 4
U+0000		U+007F	 0yyyzzzz
U+0080		U+07FF	 110xxxyy	10yyzzzz
U+0800		U+FFFF	 1110wwww	10xxxxyy	10yyzzzz
U+010000	U+10FFFF 11110uvv	10vvwwww	10xxxxyy 10yyzzzz

As an example, the character 桁 has the hexadecimal code point U+6841,
which is 0110 1000 0100 0001 in binary, which makes its UTF-8 encoding 11100110
10100001 10000001.
 */
static int __codepoint_to_utf8(uint32_t cp, array_t *bytes) {
  if (0x0000 <= cp && cp <= 0x007F) {
    return cp & 0x7F; /* Byte 1 */
  }

  if (0x0080 <= cp && cp <= 0x07FF) {
    return ((0xC0 | ((cp >> 0x06) & 0x1F)) << 0x08) | /* Byte 1 */
           ((0x80 | ((cp >> 0x00) & 0x3F)) << 0x00);  /* Byte 2 */
  }

  if (0x0800 <= cp && cp <= 0xFFFF) {
    return ((0xE0 | ((cp >> 0x0C) & 0x0F)) << 0x10) | /* Byte 1 */
           ((0x80 | ((cp >> 0x06) & 0x3F)) << 0x08) | /* Byte 2 */
           ((0x80 | ((cp >> 0x00) & 0x3F)) << 0x00);  /* Byte 3 */
  }

  if (0x010000 <= cp && cp <= 0x10FFFF) {
    return ((0xF0 | ((cp >> 0x12) & 0x07)) << 0x18) | /* Byte 1 */
           ((0x80 | ((cp >> 0x0C) & 0x3F)) << 0x10) | /* Byte 2 */
           ((0x80 | ((cp >> 0x06) & 0x3F)) << 0x08) | /* Byte 3 */
           ((0x80 | ((cp >> 0x00) & 0x3F)) << 0x00);  /* Byte 4 */
  }

  BAD_ARG("cp");
}

typedef struct {
  void *data;
  size_t size;
  size_t length;
  size_t capacity;
} array_t;

void array_init(array_t *array, void *items, size_t size, size_t length) {
  if (array == NULL) {
    BAD_ARG("array");
  }

  array->data = items;
  array->size = size;
  array->length = length;
  array->capacity = length;
}

void *array_at(const array_t *array, size_t i) {
  if (array == NULL) {
    BAD_ARG("array");
  }

  return (char *)array->data + array->size * i;
}

void array_push_n(array_t *array, void *items, size_t n) {
  int needs_realloc = 0;

  if (array == NULL) {
    BAD_ARG("array");
  }

  while (array->length + n > array->capacity) {
    needs_realloc = 1;
    array->capacity = array->capacity == 0 ? INITIAL_CAP : array->capacity * 2;
  }

  if (needs_realloc) {
    void *tmp;
    tmp = realloc(array->data, array->size * array->capacity);
    if (tmp == NULL) {
      ALLOC_FAILURE("Failed to allocate array items");
    }
    array->data = tmp;
  }

  memmove((char *)array->data + (array->size * array->length), items,
          (array->size * n));
}

void array_deinit(array_t *array) {
  if (array == NULL) {
    return;
  }

  if (array->data != NULL) {
    free(array->data);
  }
  array->data = NULL;
  array->size = 0;
  array->length = 0;
  array->capacity = 0;
}

typedef enum {
  JT_INVALID,
  JT_NUMBER,
  JT_STRING,
  JT_NULL,
  JT_BOOLEAN,
  JT_ARRAY,
  JT_OBJECT
} JSON_TYPE;

typedef struct {
  JSON_TYPE type;
  char *key;
  char *value;
  array_t leaves;
} json_node_t;

void json_node_init(json_node_t *node) {
  if (node == NULL) {
    BAD_ARG("node");
  }

  node->type = JT_INVALID;
  node->key = NULL;
  node->value = NULL;
  array_init(&node->leaves, NULL, sizeof(*node), 0);
}

void json_node_append_leaf(json_node_t *node, json_node_t *leaf) {
  if (node == NULL) {
    BAD_ARG("node");
  }

  array_push_n(&node->leaves, leaf, 1);
}

void json_node_print(const json_node_t *const node) {
  if (node == NULL) {
    BAD_ARG("node");
  }

  switch (node->type) {
  case JT_INVALID:
    printf("JT_INVALID\n");
    break;
  case JT_NUMBER:
    if (node->value == NULL) {
      BAD_ARG("node->value");
    }
    printf("JT_NUMBER(%s)\n", node->value);
    break;
  case JT_STRING:
    if (node->value == NULL) {
      BAD_ARG("node->value");
    }
    printf("JT_STRING(%s)\n", node->value);
    break;
  case JT_NULL:
    if (node->value == NULL) {
      BAD_ARG("node->value");
    }
    printf("JT_NULL(%s)\n", node->value);
    break;
  case JT_BOOLEAN:
    if (node->value == NULL) {
      BAD_ARG("node->value");
    }
    printf("JT_BOOLEAN(%s)\n", node->value);
    break;
  case JT_ARRAY: {
    size_t i;

    printf("JT_ARRAY(\n");

    for (i = 0; i < node->leaves.length; i++) {
      json_node_t *n = array_at(&node->leaves, i);
      json_node_print(n);
    }

    printf(")\n");
  } break;
  case JT_OBJECT: {
    size_t i;

    printf("JT_OBJECT(\n");

    for (i = 0; i < node->leaves.length; i++) {
      json_node_t *n = array_at(&node->leaves, i);
      if (n->key == NULL) {
        BAD_ARG("node->key");
      }
      printf("%s:", n->key);
      json_node_print(n);
    }

    printf(")\n");
  } break;
  default:
    UNREACHABLE("JSON_TYPE");
  }
}

void json_node_store_val(json_node_t *node, const char *val, size_t len) {
  if (node == NULL) {
    BAD_ARG("node");
  }

  if (node->value != NULL) {
    free(node->value);
    node->value = NULL;
  }

  node->value = strndup(val, len);
}

void json_node_deinit(json_node_t *node) {
  if (node == NULL) {
    return;
  }

  if (node->value != NULL) {
    free(node->value);
    node->value = NULL;
  }

  if (node->key != NULL) {
    free(node->key);
    node->key = NULL;
  }

  if (node->leaves.data != NULL) {
    size_t i;
    for (i = 0; i < node->leaves.length; i++) {
      json_node_deinit(array_at(&node->leaves, i));
    }
    free(node->leaves.data);
    array_deinit(&node->leaves);
  }
}

typedef enum {
  TT_OPAREN,
  TT_CPAREN,
  TT_OCURLY,
  TT_CCURLY,
  TT_COLON,
  TT_COMMA,
  TT_KEY,
  TT_VALUE
} TOKEN_TYPE;

typedef struct {
  char *content;
  size_t length;
  size_t position;
  size_t line;
  size_t column;
} json_lexer_t;

void json_lexer_init(json_lexer_t *lexer, char *json_source, size_t length) {
  if (lexer == NULL) {
    BAD_ARG("lexer");
  }
  if (json_source == NULL) {
    BAD_ARG("json_source");
  }

  lexer->content = json_source;
  lexer->length = length;
  lexer->position = 0;
  lexer->line = 0;
  lexer->column = 0;
}

int json_lexer_nextn(json_lexer_t *lexer, size_t n) {
  if (lexer == NULL) {
    BAD_ARG("lexer");
  }

  if (lexer->position + n >= lexer->length) {
    return -1;
  }

  lexer->content += n;
  lexer->position += n;
  lexer->column += n;

  return 0;
}

int json_lexer_next(json_lexer_t *lexer) { return json_lexer_nextn(lexer, 1); }

void json_lexer_skip_space(json_lexer_t *lexer) {
  if (lexer == NULL) {
    BAD_ARG("lexer");
  }

  if (lexer->position >= lexer->length) {
    return;
  }

  while (__isspace(*lexer->content)) {
    if (*lexer->content == '\n') {
      lexer->line++;
      lexer->column = 0;
    }

    if (json_lexer_next(lexer) != 0) {
      return;
    }
  }
}

void json_lexer_deinit(json_lexer_t *lexer) {
  if (lexer == NULL) {
    return;
  }

  lexer->content = NULL;
  lexer->length = 0;
  lexer->position = 0;
  lexer->line = 0;
  lexer->column = 0;
}

typedef struct {
  json_lexer_t *lexer;
  json_node_t *node;
} json_parser_t;

void json_parser_init(json_parser_t *parser, json_lexer_t *lexer,
                      json_node_t *root) {
  if (parser == NULL) {
    BAD_ARG("parser");
  }

  parser->lexer = lexer;
  parser->node = root;
}

void json_parser_parse_value(json_parser_t *parser);

void json_parser_parse_number(json_parser_t *parser) {
  const char *value_start;
  size_t value_length = 0;
  json_node_t *node;
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  node = parser->node;
  lexer = parser->lexer;
  value_start = lexer->content;

  if (*lexer->content == '-') {
    value_length++;
    if (json_lexer_next(lexer) != 0) {
      UNEXPECTED_EOL(lexer, "digit");
    }
  }

  if (!__isdigit(*lexer->content)) {
    UNEXPECTED_TOKEN(lexer, "digit");
  }
  node->type = JT_NUMBER;

  if (*lexer->content == '0') {
    value_length++;
    if (json_lexer_next(lexer) != 0) {
      goto store;
    }

    if (__isdigit(*lexer->content)) {
      UNEXPECTED_TOKEN(lexer, "., e, E or end of number");
    }
  } else {
    while (__isdigit(*lexer->content)) {
      value_length++;
      if (json_lexer_next(lexer) != 0) {
        goto store;
      }
    }
  }

  if (*lexer->content == '.') {
    value_length++;
    if (json_lexer_next(lexer) != 0) {
      UNEXPECTED_EOL(lexer, "digit");
    }

    if (!__isdigit(*lexer->content)) {
      UNEXPECTED_TOKEN(lexer, "digit");
    }

    while (__isdigit(*lexer->content)) {
      value_length++;
      if (json_lexer_next(lexer) != 0) {
        goto store;
      }
    }
  }

  if (*lexer->content == 'e' || *lexer->content == 'E') {
    value_length++;
    if (json_lexer_next(lexer) != 0) {
      UNEXPECTED_EOL(lexer, "digit or sign");
    }

    if (*lexer->content == '-' || *lexer->content == '+') {
      value_length++;
      if (json_lexer_next(lexer) != 0) {
        UNEXPECTED_EOL(lexer, "digit");
      }
    }

    if (!__isdigit(*lexer->content)) {
      UNEXPECTED_TOKEN(lexer, "digit");
    }

    while (__isdigit(*lexer->content)) {
      value_length++;
      if (json_lexer_next(lexer) != 0) {
        goto store;
      }
    }
  }

store:
  json_node_store_val(node, value_start, value_length);
}

void json_parser_parse_string(json_parser_t *parser) {
  array_t buffer;
  json_node_t *node;
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  node = parser->node;
  lexer = parser->lexer;
  array_init(&buffer, NULL, sizeof(*lexer->content), 0);

  if (*lexer->content != '"') {
    UNEXPECTED_TOKEN(lexer, "\"");
  }
  node->type = JT_STRING;

  if (json_lexer_next(lexer) != 0) {
    UNEXPECTED_EOL(lexer, "any codepoint or \"");
  }
  array_push_n(&buffer, lexer->content, 1);

  while (*lexer->content != '"') {
    if (*lexer->content >= '\x01' && *lexer->content <= '\x1f') {
      UNEXPECTED_TOKEN(lexer, "any codepoint or \"");
    }

    if (*lexer->content == '\\') {
      char *decoded = NULL;

      if (json_lexer_next(lexer) != 0) {
        UNEXPECTED_EOL(lexer, "escape sequence");
      }

      switch (*lexer->content) {
      case 'b':
        decoded = "\x08";
        break;
      case 't':
        decoded = "\x09";
        break;
      case 'n':
        decoded = "\x0a";
        break;
      case 'f':
        decoded = "\x0c";
        break;
      case 'r':
        decoded = "\x0d";
        break;
      case '"':
        decoded = "\x22";
        break;
      case '/':
        decoded = "\x2f";
        break;
      case '\\':
        decoded = "\x5c";
        break;
      case 'u': {
        array_t U;
        size_t hex_idx;
        const size_t unicode_hex_len = 4;
        unsigned int codepoint;

        array_init(&U, NULL, sizeof(char), 0);

        for (hex_idx = 0; hex_idx < unicode_hex_len; hex_idx++) {
          if (json_lexer_next(lexer) != 0) {
            UNEXPECTED_EOL(lexer, "any codepoint or \"");
          }

          if (!__ishexdigit(*lexer->content)) {
            UNEXPECTED_TOKEN(lexer, "hex digit");
          }

          array_push_n(&U, lexer->content, 1);
        }

        codepoint = __atoh(U.data, U.length);

        if (0xD800 <= codepoint && codepoint <= 0xDBFF) {
          TODO("Handle utf surrogates");
        }

        TODO("Handle utf code sequence");

        array_deinit(&U);
      } break;
      default:
        UNEXPECTED_TOKEN(lexer, "escape sequence");
        break;
      }
      array_push_n(&buffer, decoded, 1);
    } else {
      array_push_n(&buffer, lexer->content, 1);
    }

    if (json_lexer_next(lexer) != 0) {
      UNEXPECTED_EOL(lexer, "any codepoint or \"");
    }
  }

  json_node_store_val(node, buffer.data, buffer.length);
  json_lexer_next(lexer);

  array_deinit(&buffer);
}

void json_parser_parse_null(json_parser_t *parser) {
  const char *str = "null";
  size_t str_len = 4;
  json_node_t *node;
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  node = parser->node;
  lexer = parser->lexer;

  if (strncmp(lexer->content, str, str_len) != 0) {
    UNEXPECTED_TOKEN(lexer, "null");
  }

  node->type = JT_NULL;

  json_node_store_val(node, str, str_len);
  json_lexer_nextn(lexer, str_len);
}

void json_parser_parse_bool(json_parser_t *parser) {
  const char *str_true = "true";
  size_t str_true_len = 4;
  const char *str_false = "false";
  size_t str_false_len = 5;
  const char *bool_val;
  size_t bool_val_len;
  json_node_t *node;
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  node = parser->node;
  lexer = parser->lexer;

  if (strncmp(lexer->content, str_true, str_true_len) == 0) {
    bool_val = str_true;
    bool_val_len = str_true_len;
  } else if (strncmp(lexer->content, str_false, str_false_len) == 0) {
    bool_val = str_false;
    bool_val_len = str_false_len;
  } else {
    UNEXPECTED_TOKEN(lexer, "true or false");
  }
  node->type = JT_BOOLEAN;

  json_node_store_val(node, bool_val, bool_val_len);
  json_lexer_nextn(lexer, bool_val_len);
}

void json_parser_parse_array(json_parser_t *parser) {
  json_node_t *node;
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  node = parser->node;
  lexer = parser->lexer;

  if (*lexer->content != '[') {
    UNEXPECTED_TOKEN(lexer, "[");
  }
  node->type = JT_ARRAY;

  if (json_lexer_next(lexer) != 0) {
    UNEXPECTED_EOL(lexer, "value or ]");
  }

  while (*lexer->content != ']') {
    json_node_t leaf = {0};
    parser->node = &leaf;

    json_parser_parse_value(parser);
    json_node_append_leaf(node, &leaf);

    if (*lexer->content != ',' && *lexer->content != ']') {
      UNEXPECTED_TOKEN(lexer, ", or ]");
    }

    if (*lexer->content == ',') {
      if (json_lexer_next(lexer) != 0) {
        UNEXPECTED_EOL(lexer, "value");
      }

      if (*lexer->content == ']') {
        UNEXPECTED_TOKEN(lexer, "value");
      }
    }
  }

  parser->node = node;
  json_lexer_next(lexer);
}

void json_parser_parse_object(json_parser_t *parser) {
  json_node_t *node;
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  node = parser->node;
  lexer = parser->lexer;

  if (*lexer->content != '{') {
    UNEXPECTED_TOKEN(lexer, "{");
  }
  node->type = JT_OBJECT;

  if (json_lexer_next(lexer) != 0) {
    UNEXPECTED_EOL(lexer, "\" or }");
  }

  while (*lexer->content != '}') {
    json_node_t leaf = {0};
    parser->node = &leaf;

    json_lexer_skip_space(lexer);

    json_parser_parse_string(parser);
    parser->node->key = strdup(parser->node->value);

    json_lexer_skip_space(lexer);

    if (*lexer->content != ':') {
      UNEXPECTED_TOKEN(lexer, ":");
    }

    if (json_lexer_next(lexer) != 0) {
      UNEXPECTED_EOL(lexer, ":");
    }

    json_parser_parse_value(parser);
    json_node_append_leaf(node, &leaf);

    if (*lexer->content != ',' && *lexer->content != '}') {
      UNEXPECTED_TOKEN(lexer, ", or }");
    }

    if (*lexer->content == ',') {
      if (json_lexer_next(lexer) != 0) {
        UNEXPECTED_EOL(lexer, "\"");
      }

      if (*lexer->content == '}') {
        UNEXPECTED_TOKEN(lexer, "\"");
      }
    }
  }

  parser->node = node;
  json_lexer_next(lexer);
}

void json_parser_parse_value(json_parser_t *parser) {
  json_lexer_t *lexer;

  if (parser == NULL) {
    BAD_ARG("parser");
  }
  lexer = parser->lexer;

  json_lexer_skip_space(lexer);

  if (*lexer->content == '"') {
    json_parser_parse_string(parser);
  } else if (*lexer->content == '-' || __isdigit(*lexer->content)) {
    json_parser_parse_number(parser);
  } else if (*lexer->content == 't' || *lexer->content == 'f') {
    json_parser_parse_bool(parser);
  } else if (*lexer->content == 'n') {
    json_parser_parse_null(parser);
  } else if (*lexer->content == '{') {
    json_parser_parse_object(parser);
  } else if (*lexer->content == '[') {
    json_parser_parse_array(parser);
  } else {
    UNEXPECTED_TOKEN(lexer, "\", -, true, false, null, {, or [");
  }

  json_lexer_skip_space(lexer);
}

void json_parser_deinit(json_parser_t *parser) {
  if (parser == NULL) {
    return;
  }

  parser->lexer = NULL;
  parser->node = NULL;
}

static void test_json(char *json) {
  json_lexer_t lexer;
  json_node_t root;
  json_parser_t parser;

  json_lexer_init(&lexer, json, strlen(json));
  json_node_init(&root);
  json_parser_init(&parser, &lexer, &root);

  json_parser_parse_value(&parser);

  if (json_lexer_next(&lexer) == 0) {
    UNEXPECTED_TOKEN(&lexer, "end of json content");
  }

  json_node_print(&root);

  json_node_deinit(&root);
  json_lexer_deinit(&lexer);
  json_parser_deinit(&parser);
}

int main(void) {
  test_json("{\n"
            "\"number\": [-1, 0.1, 2.1e-1],\n"
            "\"string\": \"🀄⏬0983slsdfkj-asdf\\b\\t\\u0041ɚ\",\n"
            "\"null\": null,\n"
            "\"boolean\": {\n"
            "\"true\": true,\n"
            "\"false\": false\n"
            "}\n"
            "}");

  return EXIT_SUCCESS;
}
