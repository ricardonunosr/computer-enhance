#include <iostream>
#include <stdio.h>

#include "buffer.cpp"
#include "common.h"
#include "json_parser.cpp"

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: %s [haversine_input.json]\n", argv[0]);
    fprintf(stderr, "       %s [haversine_input.json] [answers.f64]\n",
            argv[0]);
    return 1;
  }

  buffer json = ReadFile(argv[1]);
  haversine_pair *pairs = (haversine_pair *)malloc(1);
  json_parse_haversine(json, pairs);

  return 0;
}