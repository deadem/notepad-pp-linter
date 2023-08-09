#pragma once
#include <string>
#include "optional.hpp"

class File
{
  public:
    File(const std::wstring &fileName, const std::wstring &directory);
    ~File();
    std::string exec(std::wstring commandLine, const nonstd::optional<std::string> &str);
    bool write(const std::string &data);

  private:
    std::wstring m_fileName;
    std::wstring m_directory;
    std::wstring m_file;
};
