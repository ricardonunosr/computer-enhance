#include "common.h"

enum json_token_type {
  token_eof,

  token_opening_brace,
  token_opening_paren,
  token_closing_brace,
  token_closing_paren,

  token_comma,
  token_double_quote
};

struct json_parser {
  buffer source;
  u64 at;
  b32 had_error;
};

struct json_element {
  buffer label;
  buffer value;
  json_element *first;
  json_element *next;
};

static json_token_type getToken(json_parser *json_parser) {
  char c = json_parser->source.data[json_parser->at];
  switch (c) {
  case '{':
    return token_opening_brace;
  case '}':
    return token_closing_brace;
  default:
    return token_eof;
  }
}

static json_element *parse_json(buffer json) {
  json_parser json_parser = {};
  json_parser.source = json;

  json_element* root = nullptr;

  return nullptr;
}

static json_element *look_up_element(char *element) { return nullptr; }

void json_parse_haversine(buffer json_input, haversine_pair *pairs) {

  json_element *json = parse_json(json_input);
  json_element *pair_array = look_up_element("pairs");

  return;
}
