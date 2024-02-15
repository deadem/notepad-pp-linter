#pragma once
#include <exception>
#include <string>

struct IXMLDOMParseError;

namespace Linter
{
    class XmlDecodeException : public std::exception
    {
      public:
        explicit XmlDecodeException(IXMLDOMParseError *);

        XmlDecodeException(XmlDecodeException const &) = delete;
        XmlDecodeException(XmlDecodeException &&) noexcept;
        XmlDecodeException &operator=(XmlDecodeException const &) = delete;
        XmlDecodeException &operator=(XmlDecodeException &&) = delete;
        ~XmlDecodeException();

        char const *what() const noexcept override;

      private:
        //value of what()
        char m_buff[2048];

        //Line at which error detected
        long m_line;

        //Column at which error detected
        long m_column;
    };
}    // namespace Linter
