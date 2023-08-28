#include "StdAfx.h"
#include "file.h"

#include "HandleWrapper.h"
#include "SystemError.h"

#include <codecvt>

using ::Linter::HandleWrapper;
using ::Linter::SystemError;

std::string File::exec(std::wstring commandLine, const nonstd::optional<std::string> &str)
{
    if (!m_file.empty())
    {
        commandLine += ' ';
        commandLine += '"';
        commandLine += m_file;
        commandLine += '"';
    }

    auto out_handles = HandleWrapper::create_pipe();
    HandleWrapper &OUT_Rd{out_handles.first};
    HandleWrapper &OUT_Wr{out_handles.second};

    auto err_handles = HandleWrapper::create_pipe();
    //HandleWrapper &ERR_Rd{err_handles.first}; //We don't use this?
    HandleWrapper &ERR_Wr{err_handles.second};

    auto in_handles = HandleWrapper::create_pipe();
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
        throw SystemError(error, "Linter: Can't execute command: " + command);
    }

    if (str.has_value())
    {
        IN_Wr.write_string(str.value());
    }

    //We need to close all the handles for this end otherwise strange things happen.
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    OUT_Wr.close();
    ERR_Wr.close();
    IN_Wr.close();

    return OUT_Rd.read_file();
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

    fileHandle.write_string(data);

    m_file = tempFileName;
}
