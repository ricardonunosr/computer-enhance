#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <stdio.h>

typedef double f64;

#include "listings/listing_0065_haversine_formula.cpp"

f64 fRand(f64 fMin, f64 fMax) {
  f64 f = (f64)rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

int main(int argc, char *argv[]) {
  if (argc <= 3) {
    printf("Usage: haversine_gen [mode] [seed] [number_of_points]\n");
    return 1;
  }

  uint32_t mode = atoi(argv[1]);
  uint32_t seed = atoi(argv[2]);
  uint32_t number_of_points = atoi(argv[3]);

  std::string file_name = "harv_" + std::to_string(number_of_points) + ".json";

  printf("Mode: %d\n", mode);
  printf("Ramdom seed: %d\n", seed);
  printf("Number Of Points: %d\n", number_of_points);

  srand(seed);

  FILE *fh = fopen(file_name.c_str(), "w");

  f64 expected_sum = 0;

  // Start json file
  fprintf(fh, "{\n\"pairs\":[");

  for (uint32_t index = 0; index < number_of_points; ++index) {
    f64 x0 = fRand(-180, 180);
    f64 x1 = fRand(-180, 180);
    f64 y0 = fRand(-90, 90);
    f64 y1 = fRand(-90, 90);
    expected_sum += ReferenceHaversine(x0, y0, x1, y1, 6372.8);
    fprintf(fh, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f}", x0, y0, x1, y1);
    if (index != number_of_points - 1) {
      fprintf(fh, ",");
    }
  }

  // End json file
  fprintf(fh, "]}");
  printf("Expected sum: %f\n",
         expected_sum / static_cast<double>(number_of_points));
}