#pragma once

#include <bitset>
#include <cstddef>

constexpr std::size_t MAX_COMPONENTS = 64;

constexpr std::size_t COMP_INDEX_CTransform = 0;
constexpr std::size_t COMP_INDEX_CSpecies = 1;
constexpr std::size_t COMP_INDEX_CHealth = 2;
constexpr std::size_t COMP_INDEX_CEnergy = 3;
constexpr std::size_t COMP_INDEX_CReproductive = 4;
constexpr std::size_t COMP_INDEX_CBehavior = 5;

using Signature = std::bitset<MAX_COMPONENTS>;
