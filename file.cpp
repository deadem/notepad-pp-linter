#include "StdAfx.h"
#include "file.h"

#include "SystemError.h"

#include <codecvt>
#include <exception>
#include <limits>
#include <utility>

namespace
{

    class HandleWrapper
    {
      public:
        HandleWrapper(HANDLE h) : handle_(h)
        {
            if (h == INVALID_HANDLE_VALUE)
            {
                throw Linter::SystemError();
            }
        }

        HandleWrapper(HandleWrapper const &) = delete;

        HandleWrapper(HandleWrapper &&other) noexcept : handle_(std::exchange(other.handle_, INVALID_HANDLE_VALUE))
        {
        }

        HandleWrapper &operator=(HandleWrapper const &) = delete;

        HandleWrapper &operator=(HandleWrapper &&other) = delete;

        void close()
        {
            if (handle_ != INVALID_HANDLE_VALUE)
            {
                HANDLE h{std::exchange(handle_, INVALID_HANDLE_VALUE)};
                if (!CloseHandle(h))
                {
                    if (std::uncaught_exceptions() == 0)
                    {
                        throw Linter::SystemError();
                    }
                }
            }
        }

        ~HandleWrapper()
        {
            close();
        }

        operator HANDLE() const noexcept
        {
            return handle_;
        }

      private:
        HANDLE handle_;
    };

    std::pair<HandleWrapper, HandleWrapper> create_pipe()
    {
        SECURITY_ATTRIBUTES security;

        security.nLength = sizeof(SECURITY_ATTRIBUTES);
        security.bInheritHandle = TRUE;
        security.lpSecurityDescriptor = nullptr;

        HANDLE parent_h;
        HANDLE child_h;
        if (!CreatePipe(&parent_h, &child_h, &security, 0))
        {
            throw Linter::SystemError();
        }

        //Stop my handle being inherited by the child
        if (!SetHandleInformation(parent_h, HANDLE_FLAG_INHERIT, 0))
        {
            throw Linter::SystemError();
        }

        return std::make_pair(HandleWrapper(parent_h), HandleWrapper(child_h));
    }

    /** Write a string to an already opened handle
     *
     * @param str - string to write
     */
    void write_string_to_handle(std::string const &str, HandleWrapper &handle)
    {
        char const *buff = str.c_str();
        char const *end = buff + str.size();
        while (buff != end)
        {
            DWORD to_write = static_cast<DWORD>(std::min(static_cast<std::size_t>(std::numeric_limits<DWORD>::max()), str.size()));
            DWORD written;
            if (!WriteFile(handle, buff, to_write, &written, nullptr))
            {
                //Bad things happened
                throw Linter::SystemError();
            }
            buff += written;
        }
    }

}    // namespace

std::string File::exec(std::wstring commandLine, const nonstd::optional<std::string> &str)
{
    if (!m_file.empty())
    {
        commandLine += ' ';
        commandLine += '"';
        commandLine += m_file;
        commandLine += '"';
    }

    auto out_handles = create_pipe();
    HandleWrapper &OUT_Rd{out_handles.first};
    HandleWrapper &OUT_Wr{out_handles.second};

    auto err_handles = create_pipe();
    //HandleWrapper &ERR_Rd{err_handles.first}; //We don't use this?
    HandleWrapper &ERR_Wr{err_handles.second};

    auto in_handles = create_pipe();
    HandleWrapper &IN_Wr{err_handles.first};
    HandleWrapper &IN_Rd{err_handles.second};

    STARTUPINFO startInfo = {0};
    startInfo.cb = sizeof(STARTUPINFO);
    startInfo.hStdError = ERR_Wr;
    startInfo.hStdOutput = OUT_Wr;
    startInfo.hStdInput = IN_Rd;
    startInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION procInfo = {0};

    BOOL isSuccess = CreateProcess(NULL,
        const_cast<wchar_t *>(commandLine.c_str()),    // command line
        NULL,                                          // process security attributes
        NULL,                                          // primary thread security attributes
        TRUE,                                          // handles are inherited
        CREATE_NO_WINDOW,                              // creation flags
        NULL,                                          // use parent's environment
        m_directory.c_str(),                           // use parent's current directory
        &startInfo,                                    // STARTUPINFO pointer
        &procInfo);                                    // receives PROCESS_INFORMATION

    if (!isSuccess)
    {
        DWORD const error{GetLastError()};
        std::string command{std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(commandLine)};
        throw Linter::SystemError(error, "Linter: Can't execute command: " + command);
    }

    if (str.has_value())
    {
        const std::string &value = str.value();
        write_string_to_handle(value, IN_Wr);
    }

    //We need to close all the handles for this end otherwise strange things happen.
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    OUT_Wr.close();
    ERR_Wr.close();
    IN_Wr.close();

    std::string result;

    static const DWORD buffer_size = 16384;
    std::string buffer;
    buffer.resize(buffer_size);

    for (;;)
    {
        DWORD readBytes;
        //The API suggests when the other end closes the pipe, you should get 0. What appears to happen
        //is that you get broken pipe.
        if (!ReadFile(OUT_Rd, &buffer[0], buffer_size, &readBytes, NULL))
        {
            DWORD err = GetLastError();
            if (err != ERROR_BROKEN_PIPE)
            {
                throw Linter::SystemError(err);
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

File::File(const std::wstring &fileName, const std::wstring &directory) : m_fileName(fileName), m_directory(directory)
{
}

File::~File()
{
    if (!m_file.empty())
    {
        _wunlink(m_file.c_str());
    }
}

void File::write(const std::string &data)
{
    if (data.empty())
    {
        return;
    }

    //FIXME Don't use "/"
    std::wstring tempFileName = m_directory + L"/" + m_fileName + L".temp.linter.file.tmp";

    HandleWrapper fileHandle{
        CreateFile(tempFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY, NULL)};

    write_string_to_handle(data, fileHandle);

    m_file = tempFileName;
}
