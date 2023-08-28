#pragma once
#include <exception>
#if __cplusplus >= 202002L
#include <source_location>
#endif
#include <string>

#include <wtypes.h>

namespace Linter
{
    class SystemError : public std::exception
    {
      public:
        /** Creates an exception object from the current system error */
        explicit SystemError(
#if __cplusplus >= 202002L
            std::source_location location = std::source_location::current()
#endif
        );

        /** Creates an exception object from the current system error, appends string */
        explicit SystemError(std::string const &
#if __cplusplus >= 202002L
            ,
            std::source_location location = std::source_location::current()
#endif
        );

        /** Creates an exception object given a system error number */
        explicit SystemError(DWORD err
#if __cplusplus >= 202002L
            ,
            std::source_location location = std::source_location::current()
#endif
        );

        /** Creates an exception object from specified error with addition information string */
        SystemError(DWORD err, std::string const &
#if __cplusplus >= 202002L
            ,
            std::source_location location = std::source_location::current()
#endif
        );

        /** Creates an exception object given an HRESULT */
        explicit SystemError(HRESULT err
#if __cplusplus >= 202002L
            ,
            std::source_location location = std::source_location::current()
#endif
        );

        /** Creates an exception object from specified error with addition information string */
        SystemError(HRESULT err, std::string const &
#if __cplusplus >= 202002L
            ,
            std::source_location location = std::source_location::current()
#endif
        );

        ~SystemError();

        virtual char const *what() const noexcept
        {
            return &buff_[0];
        }

      private:
        char buff_[2048];

#if __cplusplus >= 202002L
        void add_location_to_message(std::source_location const &location);
#endif
    };

}    // namespace Linter
