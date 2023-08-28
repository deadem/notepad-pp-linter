#include "stdafx.h"
#include "HandleWrapper.h"

#include "SystemError.h"

#include <limits>
#include <utility>

namespace Linter
{
    HandleWrapper::HandleWrapper(HANDLE h) : handle_(h)
    {
        if (h == INVALID_HANDLE_VALUE)
        {
            throw SystemError();
        }
    }

    HandleWrapper::HandleWrapper(HandleWrapper &&other) noexcept
        : handle_(std::exchange(other.handle_, INVALID_HANDLE_VALUE))
    {
    }

    void HandleWrapper::close()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
        {
            HANDLE h{std::exchange(handle_, INVALID_HANDLE_VALUE)};
            if (!CloseHandle(h))
            {
                if (std::uncaught_exceptions() == 0)
                {
                    throw SystemError();
                }
            }
        }
    }

    HandleWrapper::~HandleWrapper()
    {
        close();
    }

    void HandleWrapper::write_string(std::string const &str)
    {
        char const *buff = str.c_str();
        char const *end = buff + str.size();
        while (buff != end)
        {
            DWORD to_write = static_cast<DWORD>(std::min(static_cast<std::size_t>(std::numeric_limits<DWORD>::max()), str.size()));
            DWORD written;
            if (!WriteFile(handle_, buff, to_write, &written, nullptr))
            {
                //Bad things happened
                throw SystemError();
            }
            buff += written;
        }
    }

    std::string HandleWrapper::read_file()
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
            if (!ReadFile(handle_, &buffer[0], buffer_size, &readBytes, NULL))
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

    std::pair<HandleWrapper, HandleWrapper> HandleWrapper::create_pipe()
    {
        SECURITY_ATTRIBUTES security;

        security.nLength = sizeof(SECURITY_ATTRIBUTES);
        security.bInheritHandle = TRUE;
        security.lpSecurityDescriptor = nullptr;

        HANDLE parent_h;
        HANDLE child_h;
        if (!CreatePipe(&parent_h, &child_h, &security, 0))
        {
            throw SystemError();
        }

        //Stop my handle being inherited by the child
        if (!SetHandleInformation(parent_h, HANDLE_FLAG_INHERIT, 0))
        {
            throw SystemError();
        }

        return std::make_pair(HandleWrapper(parent_h), HandleWrapper(child_h));
    }

}    // namespace Linter
