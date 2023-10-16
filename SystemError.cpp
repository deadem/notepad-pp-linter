#include "StdAfx.h"

#include "SystemError.h"

#include "encoding.h"

#include <cstdio>
#include <system_error>

#include <comdef.h>
#include <winbase.h>

using namespace Linter;

SystemError::SystemError(const SourceLocationCurrent &location) : SystemError(GetLastError(), location)
{
}

SystemError::SystemError(std::string const &info, const SourceLocationCurrent &location) : SystemError(GetLastError(), info, location)
{
}

SystemError::SystemError(DWORD err, const SourceLocationCurrent &location)
{
    // Note: Technically, the message function could fail.
    std::snprintf(m_buff, sizeof(m_buff), "%s", std::system_category().message(err).c_str());
    addLocationToMessage(location);
}

SystemError::SystemError(DWORD err, std::string const &info, const SourceLocationCurrent &location)
{
    // Note: Technically, the message function could fail.
    std::snprintf(m_buff, sizeof(m_buff), "%s - %s", info.c_str(), std::system_category().message(err).c_str());
    addLocationToMessage(location);
}

SystemError::SystemError(HRESULT err, const SourceLocationCurrent &location)
{
    IErrorInfo *err_info{nullptr};
    (void)GetErrorInfo(0, &err_info);
    _com_error error{err, err_info};
    std::snprintf(m_buff, sizeof(m_buff), "%s", Encoding::toUTF(std::wstring(error.ErrorMessage())).c_str());
    addLocationToMessage(location);
}

SystemError::SystemError(HRESULT err, std::string const &info, const SourceLocationCurrent &location)
{
    IErrorInfo *err_info{nullptr};
    (void)GetErrorInfo(0, &err_info);
    _com_error error{err, err_info};
    std::snprintf(m_buff, sizeof(m_buff), "%s - %s", info.c_str(), Encoding::toUTF(std::wstring(error.ErrorMessage())).c_str());
    addLocationToMessage(location);
}

SystemError::~SystemError() = default;

char const *Linter::SystemError::what() const noexcept
{
    return &m_buff[0];
}

void SystemError::addLocationToMessage(const SourceLocationCurrent &location)
{
    const char *fullPath = location.file_name();
    if (fullPath == nullptr || fullPath[0] == 0)
    {
        return;
    }

    const char *fileName = std::strrchr(fullPath, '\\');
    const std::size_t used{std::strlen(m_buff)};
    std::snprintf(m_buff + used,
        sizeof(m_buff) - used,
        " at %s:%d %s",
        (fileName ? fileName + 1 : fullPath),
        location.line(),
        location.function_name());
}
