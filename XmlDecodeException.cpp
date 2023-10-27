#include "stdafx.h"
#include "XmlDecodeException.h"

#include <MsXml6.h>

#include <cstdio>

Linter::XmlDecodeException::XmlDecodeException(IXMLDOMParseError *error)
{
    // Note that this constructor does allocations. Sorry.

    //Get the error line and column for later.
    error->get_line(&m_line);
    error->get_linepos(&m_column);

    //We use the rest of the information to construct the what message.
    BSTR url;
    error->get_url(&url);

    std::size_t pos = std::snprintf(m_buff,
        sizeof(m_buff),
        "Invalid xml in %s at line %ld col %ld",
        url == nullptr ? "temporary linter output file" : static_cast<char *>(static_cast<_bstr_t>(url)),
        m_line,
        m_column);

    BSTR text;
    error->get_srcText(&text);
    if (text != nullptr)
    {
        pos += std::snprintf(m_buff + pos, sizeof(m_buff) - pos, " (near %s)", static_cast<char *>(static_cast<_bstr_t>(text)));
    }

    long code;
    error->get_errorCode(&code);
    BSTR reason;
    error->get_reason(&reason);
    std::snprintf(
        m_buff + pos, sizeof(m_buff) - pos, ": code %08lx %s", code, static_cast<char *>(static_cast<_bstr_t>(reason)));
}

Linter::XmlDecodeException::~XmlDecodeException() = default;

char const *Linter::XmlDecodeException::what() const noexcept
{
    return &m_buff[0];
}
