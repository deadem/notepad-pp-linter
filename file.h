#pragma once
#include <string>

class File
{
public:
  static std::string exec(const std::wstring &directory, std::wstring commandLine, const std::wstring &parameters);
  static std::wstring write(const std::wstring &directory, const std::string &data);
};
