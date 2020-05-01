#include <iostream>
#include <chrono>
#include <immintrin.h>

#include "benchmark_util.h"

/**
 * Scalar generated code benchmark.
 *
 * Query:
 *
 *      select count(*) from foo f where foo.x = 3;
 */
void execute_scalar_benchmark(const std::shared_ptr<data_block> &data, const size_t dataset_size) {
  constexpr size_t reps{3U};
  constexpr size_t steps{10U};

  const auto& col = data.get()->values;

  for (size_t rep = 0U; rep < reps; rep++) {
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();

    int aggregate = 0;

    for (size_t step = 0U; step < steps; step++) {
      // Scalar Code for the Query:
      for (size_t i = 0; i < dataset_size; i++) {
        if (col[i] == 3) {
          aggregate++;
        }
      }
    }

    end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed_time = (end - start) / steps;

    if (aggregate < 0) {
      std::cerr << " ..." << std::endl;
    }

    const double seconds(elapsed_time.count());
    std::cerr << " Run " << (rep + 1) << " : " << seconds << "s \n";
  }
}

/**
 * SIMD generated code benchmark (AVX-512).
 *
 * Query:
 *
 *      select count(*) from foo f where foo.x = 3;
 */
void execute_simd_benchmark_using_avx512(const std::shared_ptr<data_block> &data, const size_t &dataset_size) {
  constexpr size_t reps{3U};
  constexpr size_t steps{10U};
  constexpr size_t simd_size{16};

  const auto& col = data.get()->values;

  for (size_t rep = 0U; rep < reps; rep++) {
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();

    int aggregate = 0;

    for (size_t step = 0U; step < steps; step++) {
      // SIMD Code for the Query:
      aggregate = 0;
      size_t i = 0;

      // Calculate the number of elements which can be processed by SIMD.
      size_t data_size_simd = dataset_size - (dataset_size % simd_size);

      // Comparison vector.
      alignas(64) int32_t cmp[16] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
      auto cmp_vec = _mm512_load_epi32(cmp);

      // SIMD PART -----------------------------------------------------------------------------------------------------
      for (; i < data_size_simd; i += simd_size) {
        // Load the data.
        auto xmm0 = _mm512_load_epi32(col + i);

        // Compare with comparison vector.
        auto comp_result = static_cast<uint16_t>(_mm512_cmp_epi32_mask(xmm0, cmp_vec, _MM_CMPINT_EQ));

        while (comp_result > 0) {
          aggregate += comp_result & 0x1;
          comp_result >>= 1;
        }
      }

      // SCALAR PART for rest ------------------------------------------------------------------------------------------
      for (; i < dataset_size; i++) {
        if (col[i] == 3) {
          aggregate++;
        }
      }
    }

    end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed_time = (end - start) / steps;

    if (aggregate < 0) {
      std::cerr << " ..." << std::endl;
    }

    const double seconds(elapsed_time.count());
    std::cerr << " Run " << (rep + 1) << " : " << seconds << "s \n";
  }
}

int main() {
  const auto [data, dataset_size] = generator::generate_column();

  std::cout << "SCALAR BENCHMARK\n";
  execute_scalar_benchmark(data, dataset_size);

  std::cout << "\nSIMD BENCHMARK:\n";
  execute_simd_benchmark_using_avx512(data, dataset_size);

  return EXIT_SUCCESS;
}
