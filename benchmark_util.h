#pragma once

#include <memory>
#include <random>
#include <iostream>

constexpr size_t DATASET_SIZE = 1000000;
constexpr size_t ALIGNMENT = 32;

struct alignas(32) data_block {
  int[] values;
};

struct generator_result {
  std::shared_ptr<data_block> data;
  size_t dataset_size;
};

using generator_result_t = generator_result;

struct generator {
  static generator_result_t generate_column() {
    std::shared_ptr<data_block> data(new data_block);

    std::cout << "Generating data...\n";
    for (size_t i = 0; i < DATASET_SIZE; i++) {
      data->values[i] = rand();
    }
    std::cout << "Finished generating data!\n";

    return {data, DATASET_SIZE};
  }
};
