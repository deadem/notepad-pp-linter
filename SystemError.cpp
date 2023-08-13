#include "StdAfx.h"

#include "SystemError.h"

#include <cstdio>
#include <system_error>

#include <winbase.h>

namespace Linter
{

    SystemError::SystemError(
#if __cplusplus >= 202002L
        std::source_location location
#endif
        )
        : SystemError(GetLastError()
#if __cplusplus >= 202002L
                          ,
              location
#endif
          )
    {
    }

    SystemError::SystemError(std::string const &info
#if __cplusplus >= 202002L
        ,
        std::source_location location
#endif
        )
        : SystemError(GetLastError(), info
#if __cplusplus >= 202002L
              ,
              location
#endif
          )
    {
    }

    SystemError::SystemError(DWORD err
#if __cplusplus >= 202002L
        ,
        std::source_location location
#endif
    )
    {
        // Note: Technically, the message function could fail.
        snprintf(buff,
            sizeof(buff),
#if __cplusplus >= 202002L
            "%s %d: "
#endif
            "%s",
#if __cplusplus >= 202002L
            location.file_name(),
            location.line(),
#endif
            std::system_category().message(err).c_str());
    }

    SystemError::SystemError(DWORD err, std::string const &info
#if __cplusplus >= 202002L
        ,
        std::source_location location
#endif
    )
    {
        // Note: Technically, the message function could fail.
        snprintf(buff,
            sizeof(buff),
#if __cplusplus >= 202002L
            "%s %d: "
#endif
            "%s - %s",
#if __cplusplus >= 202002L
            location.file_name(),
            location.line(),
#endif
            std::system_category().message(err).c_str(),
            info.c_str());
    }

    SystemError::~SystemError()
    {
    }

}    // namespace Linter
