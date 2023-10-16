#include "stdafx.h"
#include "HandleWrapper.h"

#include "SystemError.h"

using namespace Linter;

HandleWrapper::HandleWrapper(HANDLE h) : m_handle(h)
{
    if (h == INVALID_HANDLE_VALUE)
    {
        throw SystemError();
    }
}

HandleWrapper::HandleWrapper(HandleWrapper &&other) noexcept : m_handle(std::exchange(other.m_handle, INVALID_HANDLE_VALUE))
{
}

void HandleWrapper::close() const
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        HANDLE h{std::exchange(m_handle, INVALID_HANDLE_VALUE)};
        CloseHandle(h);
    }
}

HandleWrapper::operator HANDLE() const noexcept
{
    return m_handle;
}

HandleWrapper::~HandleWrapper()
{
    close();
}

void HandleWrapper::writeFile(std::string const &str) const
{
    static_assert(sizeof(str[0]) == 1, "Invalid byte size");

    char const *start = str.c_str();
    char const *end = start + str.size();
    while (start != end)
    {
        const auto toWrite = static_cast<DWORD>(end - start);
        DWORD written;
        if (!WriteFile(m_handle, start, toWrite, &written, nullptr))
        {
            const DWORD err = GetLastError();
            throw SystemError(err);
        }
        start += written;
    }
}

std::string HandleWrapper::readFile() const
{
    std::string result;

    std::string buffer;
    buffer.resize(0x4000);

    for (;;)
    {
        DWORD readBytes;
        //The API suggests when the other end closes the pipe, you should get 0. What appears to happen
        //is that you get broken pipe.
        if (!ReadFile(m_handle, &buffer[0], static_cast<DWORD>(buffer.size()), &readBytes, nullptr))
        {
            DWORD err = GetLastError();
            if (err != ERROR_BROKEN_PIPE)
            {
                throw SystemError(err);
            }
        }

        if (readBytes == 0)
        {
            break;
        }

        result += std::string(&buffer[0], readBytes);
    }

    return result;
}
