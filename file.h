#pragma once
#include <string>
#include <optional>

class File
{
public:
  File(const std::wstring &fileName, const std::wstring &directory);
  ~File();
  std::string exec(std::wstring commandLine, const std::optional<std::string> &str);
  bool write(const std::string &data);

private:
  std::wstring m_fileName;
  std::wstring m_directory;
  std::wstring m_file;
};
