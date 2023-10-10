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
        std::snprintf(m_buff, sizeof(m_buff), "%s", std::system_category().message(err).c_str());
#if __cplusplus >= 202002L
        add_location_to_message(location);
#endif
    }

    SystemError::SystemError(DWORD err, std::string const &info
#if __cplusplus >= 202002L
        ,
        std::source_location location
#endif
    )
    {
        // Note: Technically, the message function could fail.
        std::snprintf(m_buff, sizeof(m_buff), "%s - %s", info.c_str(), std::system_category().message(err).c_str());
#if __cplusplus >= 202002L
        add_location_to_message(location);
#endif
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
        std::snprintf(m_buff, sizeof(m_buff), "%s", Encoding::toUTF(std::wstring(error.ErrorMessage())).c_str());
#if __cplusplus >= 202002L
        add_location_to_message(location);
#endif
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
        std::snprintf(m_buff, sizeof(m_buff), "%s - %s", info.c_str(), Encoding::toUTF(std::wstring(error.ErrorMessage())).c_str());
#if __cplusplus >= 202002L
        add_location_to_message(location);
#endif
    }

    SystemError::~SystemError()
    {
    }

#if __cplusplus >= 202002L
    void SystemError::add_location_to_message(std::source_location const &location)
    {
        std::size_t const used{std::strlen(m_buff)};
        std::snprintf(m_buff + used, sizeof(m_buff) - used, " at %s %d", std::strrchr(location.file_name(), '\\') + 1, location.line());
    }
#endif

}    // namespace Linter
