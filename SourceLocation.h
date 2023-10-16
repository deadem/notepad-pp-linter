#pragma once
#include <string>

#if __cplusplus >= 202002L
#include <source_location>
#endif

namespace Linter
{
#if __cplusplus >= 202002L
    using SourceLocation = std::source_location;
#else
    struct SourceLocation
    {
        struct Location
        {
            unsigned int line() const noexcept
            {
                return 0;
            }
            const char *file_name() const noexcept
            {
                return "";
            }
            const char *function_name() const noexcept
            {
                return "";
            }
        };
        static Location current()
        {
            return {};
        }
    };
#endif

    using SourceLocationCurrent = decltype(SourceLocation::current());
}    // namespace Linter
