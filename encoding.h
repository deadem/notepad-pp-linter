#pragma once
#include <string>

class Encoding
{
public:
  static std::wstring toUnicode(const std::string &string, UINT encoding = CP_UTF8);
  static std::string toUTF(const std::wstring &string);
  static int utfOffset(const std::string utf8, int unicodeOffset);
};
