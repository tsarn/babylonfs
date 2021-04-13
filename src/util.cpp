#include "util.h"
#include <random>

static const std::string possibleSymbols = "abcdefghijklmnopqrstuvwxyz.,";

std::string generateStringFromSeed(const std::string &seed, int len) {
    std::mt19937 rng;
    rng.seed(std::hash<std::string>{}(seed));

    std::string res;
    for (int i = 0; i < len; ++i) {
        res += possibleSymbols[std::uniform_int_distribution<int>(0, (int)possibleSymbols.size() - 1)(rng)];
    }

    return res;
}
