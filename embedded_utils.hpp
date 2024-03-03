#pragma once
#include <array>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <ranges>
#include <span>
#include <type_traits>

#if defined(__cpp_exceptions) || (defined(__EXCEPTIONS) && __EXCEPTIONS) ||    \
    (defined(_CPPUNWIND) && _CPPUNWIND)
#include <exception>
#include <string>
#define EXCEPTIONS_ENABLED
#endif

namespace utils
{
    // clang-format off
    template <typename T>
    concept DynamicExtentSpan = requires(std::span<T> s)
    {
        { s.size() } -> std::convertible_to<std::size_t>;
    };

    template<std::size_t Sz>
    concept GreaterThanZero = ( Sz > 0 );

    inline void THROW_OR_PANIC(const std::string_view message)
    {
       #ifdef EXCEPTIONS_ENABLED
           throw std::runtime_error(std::string(message));
       #else 
           std::abort();
       #endif
    }
    // clang-format on

    /**
     * @brief copy_every_nth
     * Copies every n'th element from buffer A to buffer B
     * @tparam N = Copy interval
     * @tparam StartIdx = Defaults to 0
     * @tparam T
     * @tparam SourceExtent
     * @tparam DestExtent
     * @param source
     * @param destination
     */
    template <std::size_t N, std::size_t StartIdx = 0, typename T,
              std::size_t SourceExtent, std::size_t DestExtent>
    inline constexpr void
    copy_every_nth(const std::span<T, SourceExtent> source,
                   std::span<T, DestExtent> destination)
    {
        static_assert(StartIdx < SourceExtent && StartIdx >= 0,
                      "StartIdx must be within the source extent");
        static_assert(N > 0, "N must be greater than 0");
        static_assert(((SourceExtent - StartIdx + N - 1) / N) <= DestExtent,
                      "Destination extent may not be sufficient");

        size_t destIndex = 0;
        for (size_t i = StartIdx; i < SourceExtent && destIndex < DestExtent;
             i += N)
        {
            destination[destIndex++] = source[i];
        }
    }

    template <std::size_t N, std::size_t StartIdx = 0, typename T>
    requires DynamicExtentSpan<T>
    inline constexpr void copy_every_nth(const std::span<T> source,
                                         std::span<T> destination)
    {
        static_assert(N > 0, "N must be greater than 0");

        if (StartIdx > std::size(source) or StartIdx < 0)
        {
            THROW_OR_PANIC("Invalid start index");
        }
        // Calculate the required destination size
        size_t requiredSize = (std::size(source) - StartIdx + N - 1) / N;

        if (requiredSize > std::size(destination))
        {
            THROW_OR_PANIC(
                "Destination container does not have sufficient size, ");
        }

        size_t destIndex = 0;
        for (std::size_t i = StartIdx; i < std::size(source); i += N)
        {
            destination[destIndex++] = source[i];
        }
    }

    /**
     * @brief mean
     *  Calculates mean from source span
     * @tparam T
     * @tparam Extent
     * @param source
     * @return constexpr auto
     */
    template <typename T, std::size_t Extent>
    inline constexpr auto mean(const std::span<T, Extent> source) noexcept
    {
        static_assert(Extent > 0, "Source container must be non-empty");
        return std::accumulate(std::begin(source), std::end(source), 0.0) /
               Extent;
    }

    template <typename T>
    requires DynamicExtentSpan<T>
    inline constexpr auto mean(const std::span<T> source)
    {
        if (source.empty())
        {
            THROW_OR_PANIC("Source container must be non-empty");
        }

        return std::accumulate(std::begin(source), std::end(source), 0.0) /
               source.size();
    }
} // namespace utils