#pragma once
#include <random>
#include <type_traits>
#include <optional>

class Random
{
public:
    explicit Random(std::optional<unsigned int> seed = std::nullopt)
        : gen(seed.has_value() ? std::mt19937(seed.value()) : std::mt19937(this->rd()))
    {
    }

    // Generates random number in [min, max] range
    template<typename T>
    T Get(T min, T max)
    {
        static_assert(std::is_arithmetic_v<T>, "RandomGenerator::get only supports arithmetic types");

        if constexpr (std::is_integral_v<T>)
        {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(this->gen);
        }
        else
        {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(this->gen);
        }
    }

    bool GetBool(double Probability = 0.5)
    {
        std::bernoulli_distribution dist(Probability);
        return dist(this->gen);
    }

    template<typename T>
    void ShuffleVector(std::vector<T>& Vec)
    {
        std::shuffle(Vec.begin(), Vec.end(), this->gen);
    }

private:
    std::random_device rd;
    std::mt19937 gen;
};

inline std::unique_ptr<Random> RNG;