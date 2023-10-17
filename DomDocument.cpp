#include "stdafx.h"
#include "DomDocument.h"

#include "encoding.h"
#include "SystemError.h"

#include <sstream>
#include <stdexcept>

#pragma comment(lib, "msxml6.lib")

namespace Linter
{
    DomDocument::DomDocument(std::wstring const &filename)
    {
        init();

        BSTR bstrValue{bstr_t(filename.c_str())};
        CComVariant value(bstrValue);

        VARIANT_BOOL resultCode = FALSE;
        HRESULT hr = m_document->load(value, &resultCode);

        checkLoadResults(resultCode, hr, Encoding::toUTF(filename));
    }

    DomDocument::DomDocument(std::string const &xml)
    {
        init();

        BSTR bstrValue{(bstr_t(xml.c_str()))};

        VARIANT_BOOL resultCode = FALSE;
        HRESULT hr = m_document->loadXML(bstrValue, &resultCode);

        checkLoadResults(resultCode, hr, "temporary linter output file");
    }

    DomDocument::~DomDocument() = default;
    
    CComPtr<IXMLDOMNodeList> DomDocument::getNodeList(std::string const &xpath)
    {
        CComPtr<IXMLDOMNodeList> nodes;
        HRESULT hr = m_document->selectNodes(bstr_t(xpath.c_str()), &nodes);
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't execute XPath " + xpath);
        }
        return nodes;
    }

    void DomDocument::init()
    {
        HRESULT hr = m_document.CoCreateInstance(__uuidof(DOMDocument));
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't create IID_IXMLDOMDocument2");
        }

        hr = m_document->put_async(VARIANT_FALSE);
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't XMLDOMDocument2::put_async");
        }
    }

    void DomDocument::checkLoadResults(VARIANT_BOOL resultcode, HRESULT hr, std::string const &filename)
    {
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr);
        }
        if (resultcode != VARIANT_TRUE)
        {
            CComPtr<IXMLDOMParseError> error;
            (void)m_document->get_parseError(&error);

            if (error)
            {
                BSTR text;
                error->get_srcText(&text);
                BSTR reason;
                error->get_reason(&reason);
                long line;
                error->get_line(&line);
                long column;
                error->get_linepos(&column);
                std::ostringstream buff;
                buff << "Invalid XML in " << filename << " at line " << line << " col " << column;
                if (text != nullptr)
                {
                    buff << " (near " << static_cast<std::string>(static_cast<bstr_t>(text)) << ")";
                }
                buff << ": " << static_cast<std::string>(static_cast<bstr_t>(reason));
                throw std::runtime_error(buff.str());
            }
        }
    }

}    // namespace Linter
