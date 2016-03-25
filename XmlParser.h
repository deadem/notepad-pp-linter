#pragma once

#include <string>
#include <vector>

class XmlParser
{
public:
  struct Error
  {
    int m_line;
    int m_column;
    std::string m_message;
  };
  struct Linter {
    std::wstring m_extension;
    std::wstring m_command;
  };
  static std::vector<Error> getErrors(const std::string &xml);
  static std::vector<Linter> getLinters(std::wstring file);
};
