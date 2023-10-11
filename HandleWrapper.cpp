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
        if (!CloseHandle(h))
        {
            if (std::uncaught_exceptions() == 0)
            {
                throw SystemError();
            }
        }
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
    char const *buff = str.c_str();
    char const *end = buff + str.size();
    while (buff != end)
    {
        DWORD to_write = static_cast<DWORD>(std::min(static_cast<std::size_t>(std::numeric_limits<DWORD>::max()), str.size()));
        DWORD written;
        if (!WriteFile(m_handle, buff, to_write, &written, nullptr))
        {
            //Bad things happened
            throw SystemError();
        }
        buff += written;
    }
}

std::string HandleWrapper::readFile() const
{
    std::string result;

    static const DWORD buffer_size = 16384;
    std::string buffer;
    buffer.resize(buffer_size);

    for (;;)
    {
        DWORD readBytes;
        //The API suggests when the other end closes the pipe, you should get 0. What appears to happen
        //is that you get broken pipe.
        if (!ReadFile(m_handle, &buffer[0], buffer_size, &readBytes, nullptr))
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
