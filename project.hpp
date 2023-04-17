#ifndef project_hpp_
#define project_hpp_

#include <algorithm>
#include <concepts>
#include <iostream>
#include <random>
#include <ranges>
#include <vector>
#include "utils.hpp"

namespace uwindsor_2023w {
namespace comp3400 {
namespace project {

// i.e., returns a
template <typename T>
constexpr T const& min(T const& a) {
    return a;
}

 // i.e., like std::min()
template <typename T>
constexpr T const& min(T const& a, T const& b) {
    return a <= b ? a : b;
}

// Definition of min() with three or more arguments...
template <typename T, typename... Rest>
requires (std::same_as<T,Rest> && ...) // i.e., a C++17 fold expression requiring all Rest... types to be the same as T
constexpr T const& min(T const& a, T const& b, Rest const&... rest) {
    return a <= b ? min(a, rest...) : min(b, rest...); // Recursively calls min with either a or b (based on if a <= b). Should eventually reach one of the other min definitions.
}


/* LEVENSHTEIN */
template <typename StringA, typename StringB>
requires
 std::ranges::sized_range<StringA> &&
 std::ranges::sized_range<StringB> &&
 std::ranges::random_access_range<StringA> &&
 std::ranges::random_access_range<StringB> &&
 std::same_as<
   std::ranges::range_value_t<StringA>,
   std::ranges::range_value_t<StringB>
 >
std::size_t levenshtein(StringA const& a, StringB const& b) {

    // Store sizes for increased readability.
    std::size_t n = std::ranges::size(b);
    std::size_t m = std::ranges::size(a);

    std::vector<std::size_t> v0{};
    std::vector<std::size_t> v1{};
    std::size_t deletionCost = 0, insertionCost = 0, substitutionCost = 0;

    
    std::size_t i = 0, j = 0;
    for(i = 0;i <= n;i++) v0.push_back(i); // Initialize v0 (the previous row of distances).
    for(i = 0;i <= n;i++) v1.push_back(0); // Create n + 1 values for v1.

    // Declare iterators for a and b so they can be indexed relative to the start position.
    auto ait = std::ranges::begin(a);
    auto bit = std::ranges::begin(b);
    
    for(i = 0;i < m;i++) {
        v1.at(0) = i + 1; // Calculate v1 (current row distances) from the previous row v0.

        // Fill in the rest of the row.
        for(j = 0;j < n;j++) {
            // Calculating costs for A[i + 1][j + 1].
            deletionCost = v0.at(j + 1) + 1;
            insertionCost = v1.at(j) + 1;
            substitutionCost = ait[i] == bit[j] ? v0.at(j) : v0.at(j) + 1; // Calculate based on if ait[i] == bit[j].
            v1[j + 1] = min(deletionCost, insertionCost, substitutionCost);
        }

        std::swap(v0, v1); // Efficient swapping.

    }
    // After the final swap, the results of v1 are now in v0.
    return v0[n];
}


class char_mutator {

private:
    std::string str;
    mutable std::uniform_int_distribution<std::size_t> dist;
    mutable std::default_random_engine rand;

    std::string construct_str() {
        std::string retval{};
        for(short c = 0;c <= std::numeric_limits<char>::max();c++) {
            if(std::isalnum(c) || std::ispunct(c) || c == ' ') retval.push_back(c); // Add all valid string values.
        }
        return retval;
    }

public:

    // Default constructor. Constructs the string of all valid characters, a valid uniform distribution and a basic random_engine.
    char_mutator() : str(construct_str()), dist(0, str.size() - 1), rand(std::random_device{}()) {}

    template <typename... Args>
    char operator()(Args&&...) const {
        return str.at(dist(rand)); // Returns a random character from the string.
    }
};


/* MUTATE */
template <std::ranges::range Individual, typename MutateOp, typename URBG>
requires
  std::uniform_random_bit_generator<std::remove_cvref_t<URBG>> &&
  std::invocable<MutateOp,std::ranges::range_value_t<Individual>>

void mutate(Individual& individual, double const rate, MutateOp&& m, URBG&& urbg) {
    std::uniform_real_distribution<double> dist{0.0, 1.0};

    std::ranges::for_each(individual, [&](auto& arg) {
        if(dist(urbg) < rate) arg = m(arg); // Changes the character only if the rate is met.
    });
}


/* CROSSOVER */
template <typename URBG1, typename URBG2, typename Individual>
requires
  std::uniform_random_bit_generator<std::remove_cvref_t<URBG1>> &&
  std::uniform_random_bit_generator<std::remove_cvref_t<URBG2>> &&
  std::ranges::forward_range<Individual> &&
  std::ranges::sized_range<Individual> &&
  smart_insertable<Individual>

auto crossover(
 std::size_t const ncrossover_points,
 URBG1&& urbg_starting_parent,
 URBG2&& urbg_crossover_points,
 Individual const& parent1,
 Individual const& parent2
) -> std::remove_cvref_t<Individual>
{
    const std::size_t psize_truncated = min(std::ranges::size(parent1), std::ranges::size(parent2)); // Get the minimum parent size for calculating the crossover.
    std::bernoulli_distribution bdist{0.5};
    bool which_parent = bdist(urbg_starting_parent); // Sets true for parent1, false for parent2.

    // Can use psize_truncated to check if either parent has a size of 0 (since 0 is obviously the minimum of the two).
    if(ncrossover_points == 0 || parent1.size() == 0 || parent2.size() == 0) {
        return which_parent ? parent1 : parent2; // Return either parent1 or parent2 based on the value of which_parent.
    }

    std::vector<std::size_t> crossover_indices(psize_truncated - 1); // Vector is of size psize_truncated - 1, with all elements initialized to 0.
    std::iota(std::next(crossover_indices.begin()), crossover_indices.end(), 0); // Call std::iota starting on the second index of crossover_indices.

    // Create vector of size psize_truncated - 1.
    std::vector<size_t> crossover_offsets;
    reserve_or_noop(crossover_offsets, ncrossover_points);

    std::ranges::sample(crossover_indices, std::back_inserter(crossover_offsets), ncrossover_points, urbg_crossover_points); // Sample random points to the crossover_offsets vector from crossover_indices.
    std::adjacent_difference(crossover_offsets.begin(), crossover_offsets.end(), crossover_offsets.begin()); // Calculate the per-index differences.

    std::remove_cvref_t<Individual> child{}; // Child to return at the end of the function.

    reserve_or_noop(child, std::max(std::ranges::size(parent1), std::ranges::size(parent2))); // Reserve enough memory for the child.
    auto out = smart_inserter(child);

    // Declare iterators.
    auto p1pos = std::ranges::cbegin(parent1);
    auto p2pos = std::ranges::cbegin(parent2);

    for(auto offset : crossover_offsets) {
        
        // Copy either parent1 or parent2 based on the value of which_parent.
        if(which_parent) out = std::copy_n(p1pos, offset, out);
        else             out = std::copy_n(p2pos, offset, out);

        // Advance the iterators.
        std::advance(p1pos, offset);
        std::advance(p2pos, offset);
        which_parent = !which_parent; // Flip which_parent.
    }

    if(which_parent) std::ranges::copy(p1pos, std::ranges::cend(parent1), out);
    else             std::ranges::copy(p2pos, std::ranges::cend(parent2), out);

    return child;
}

} // namespace project
} // namespace comp3400
} // namespace uwindsor_2023w

#endif // #ifndef project_hpp_