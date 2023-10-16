#pragma once
#include "SourceLocation.h"

#include <exception>
#include <string>

namespace Linter
{
    class SystemError : public std::exception
    {

      public:
        /** Creates an exception object from the current system error */
        explicit SystemError(const SourceLocationCurrent &location = SourceLocation::current());

        /** Creates an exception object from the current system error, appends string */
        explicit SystemError(std::string const &, const SourceLocationCurrent &location = SourceLocation::current());

        /** Creates an exception object given a system error number */
        explicit SystemError(DWORD err, const SourceLocationCurrent &location = SourceLocation::current());

        /** Creates an exception object from specified error with addition information string */
        SystemError(DWORD err, std::string const &, const SourceLocationCurrent &location = SourceLocation::current());

        /** Creates an exception object given an HRESULT */
        explicit SystemError(HRESULT err, const SourceLocationCurrent &location = SourceLocation::current());

        /** Creates an exception object from specified error with addition information string */
        SystemError(HRESULT err, std::string const &, const SourceLocationCurrent &location = SourceLocation::current());

        ~SystemError();

        char const *what() const noexcept override;

      private:
        char m_buff[2048];

        void addLocationToMessage(const SourceLocationCurrent &location);
    };
}    // namespace Linter