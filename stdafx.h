// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS    // some CString constructors will be explicit
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <atlbase.h>
#include <atlstr.h>
#include <ATLCom.h>
#pragma comment(lib, "comsuppw.lib")
#include <comutil.h>
#include <exception>
#include <string>
#include <sstream>

namespace Linter
{
    struct Exception : public std::exception
    {
        Exception(const std::string &message) : m_message(message)
        {
        }
        virtual const char *what() const
        {
            return m_message.c_str();
        }
        std::string m_message;
    };
}    // namespace Linter
