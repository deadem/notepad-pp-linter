#include "StdAfx.h"
#include "file.h"

#include <algorithm>
#include <codecvt>
#include <limits>
#include <locale>

void write_file(_In_ HANDLE handle, std::string const& data)
{
    //Write the whole of the file in large blocks
    //FIXME This just assumes everything works.
    std::size_t to_write = data.size();
    char const* buff = data.c_str();
    while (to_write != 0)
    {
        DWORD written;
        WriteFile(
            handle,
            buff,
            static_cast<DWORD>(
                std::min(to_write, static_cast<std::size_t>(std::numeric_limits<DWORD>::max()))
            ),
            &written,
            nullptr);
        buff += written;
        to_write -= written;
    }
}

std::string File::exec(std::wstring commandLine, const std::optional<std::string> &str)
{
  std::string result;

  if (!m_file.empty())
  {
    commandLine += ' ';
    commandLine += '"';
    commandLine += m_file;
    commandLine += '"';
  }

  PROCESS_INFORMATION procInfo = {0};
  STARTUPINFO startInfo = {0};
  BOOL isSuccess = FALSE;

  HANDLE IN_Rd(0), IN_Wr(0), OUT_Rd(0), OUT_Wr(0), ERR_Rd(0), ERR_Wr(0);

  SECURITY_ATTRIBUTES security;

  security.nLength = sizeof(SECURITY_ATTRIBUTES);
  security.bInheritHandle = TRUE;
  security.lpSecurityDescriptor = NULL;

  CreatePipe(&OUT_Rd, &OUT_Wr, &security, 0);
  CreatePipe(&ERR_Rd, &ERR_Wr, &security, 0);
  CreatePipe(&IN_Rd, &IN_Wr, &security, 0);

  SetHandleInformation(OUT_Rd, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(ERR_Rd, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(IN_Wr, HANDLE_FLAG_INHERIT, 0);

  startInfo.cb = sizeof(STARTUPINFO);
  startInfo.hStdError = ERR_Wr;
  startInfo.hStdOutput = OUT_Wr;
  startInfo.hStdInput = IN_Rd;
  startInfo.dwFlags |= STARTF_USESTDHANDLES;

  isSuccess = CreateProcess(NULL,
    const_cast<wchar_t *>(commandLine.c_str()),  // command line
    NULL,                                        // process security attributes
    NULL,                                        // primary thread security attributes
    TRUE,                                        // handles are inherited
    CREATE_NO_WINDOW,                            // creation flags
    NULL,                                        // use parent's environment
    m_directory.c_str(),                         // use parent's current directory
    &startInfo,                                  // STARTUPINFO pointer
    &procInfo);                                  // receives PROCESS_INFORMATION

  if (!isSuccess)
  {
    std::string command = std::wstring_convert<std::codecvt_utf16<wchar_t>>().to_bytes(commandLine);
    throw Linter::Exception("Linter: Can't execute command: " + command);
  }

  if (str.has_value())
  {
    const std::string &value = str.value();
    write_file(IN_Wr, value);
  }

  CloseHandle(procInfo.hProcess);
  CloseHandle(procInfo.hThread);

  CloseHandle(ERR_Wr);
  CloseHandle(OUT_Wr);
  CloseHandle(IN_Wr);

  DWORD readBytes;
  std::string buffer;
  static const DWORD buffsize = 1024 * 16;
  buffer.resize(buffsize);

  for (;;)
  {
    isSuccess = ReadFile(OUT_Rd, &buffer[0], buffsize, &readBytes, NULL);
    if (!isSuccess || readBytes == 0)
    {
      break;
    }

    result += std::string(&buffer[0], readBytes);
  }

  return result;
}

File::File(const std::wstring &fileName, const std::wstring &directory): m_fileName(fileName), m_directory(directory)
{
};


File::~File()
{
  if (!m_file.empty())
  {
    _wunlink(m_file.c_str());
  }
}

bool File::write(const std::string &data)
{
  if (data.empty())
  {
    return false;
  }

  std::wstring tempFileName = m_directory + L"/" + m_fileName + L".temp.linter.file.tmp";

  HANDLE fileHandle = CreateFile(tempFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  write_file(fileHandle, data);
  CloseHandle(fileHandle);

  m_file = tempFileName;

  return true;
}
