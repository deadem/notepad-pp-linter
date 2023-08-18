#include "StdAfx.h"

#include "SystemError.h"

#include "encoding.h"

#include <cstdio>
#include <system_error>

#include <comdef.h>
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
        std::snprintf(buff_,
            sizeof(buff_),
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
        std::snprintf(buff_,
            sizeof(buff_),
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

    SystemError::SystemError(HRESULT err
#if __cplusplus >= 202002L
        ,
        std::source_location location
#endif
    )
    {
        IErrorInfo *err_info{nullptr};
        (void)GetErrorInfo(0, &err_info);
        _com_error error{err, err_info};
        std::snprintf(buff_,
            sizeof(buff_),
#if __cplusplus >= 202002L
            "%s %d: "
#endif
            "%s",
#if __cplusplus >= 202002L
            location.file_name(),
            location.line(),
#endif
            Encoding::toUTF(std::wstring(error.ErrorMessage())).c_str());
    }

    SystemError::SystemError(HRESULT err, std::string const &info
#if __cplusplus >= 202002L
        ,
        std::source_location location
#endif
    )
    {
        IErrorInfo *err_info{nullptr};
        (void)GetErrorInfo(0, &err_info);
        _com_error error{err, err_info};
        std::snprintf(buff_,
            sizeof(buff_),
#if __cplusplus >= 202002L
            "%s %d: "
#endif
            "%s - %s",
#if __cplusplus >= 202002L
            location.file_name(),
            location.line(),
#endif
            Encoding::toUTF(std::wstring(error.ErrorMessage())).c_str(),
            info.c_str());
    }

    SystemError::~SystemError()
    {
    }

}    // namespace Linter
