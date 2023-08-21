#include "stdafx.h"
#include "DomDocument.h"

#include "encoding.h"
#include "SystemError.h"

#include <sstream>
#include <stdexcept>

#pragma comment(lib, "msxml6.lib")

namespace Linter
{
    DomDocument::DomDocument()
    {
        HRESULT hr = document_.CoCreateInstance(__uuidof(DOMDocument));
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't create IID_IXMLDOMDocument2");
        }

        hr = document_->put_async(VARIANT_FALSE);
        if (!SUCCEEDED(hr))
        {
            throw SystemError("Can't XMLDOMDocument2::put_async");
        }
    }

    void DomDocument::load_from_file(std::wstring const &filename)
    {
        BSTR bstrValue{bstr_t(filename.c_str())};
        CComVariant value(bstrValue);

        VARIANT_BOOL resultCode = FALSE;
        HRESULT hr = document_->load(value, &resultCode);

        check_load_results(resultCode, hr, Encoding::toUTF(filename));
    }

    void DomDocument::load_from_string(std::string const &xml)
    {
        BSTR bstrValue{(bstr_t(xml.c_str()))};

        VARIANT_BOOL resultCode = FALSE;
        HRESULT hr = document_->loadXML(bstrValue, &resultCode);

        check_load_results(resultCode, hr, "temporary linter output file");
    }

    DomDocument::~DomDocument()
    {
    }

    CComPtr<IXMLDOMNodeList> DomDocument::get_nodelist(std::string const &xpath)
    {
        CComPtr<IXMLDOMNodeList> nodes;
        HRESULT hr = document_->selectNodes(bstr_t(xpath.c_str()), &nodes);
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't execute XPath " + xpath);
        }
        return nodes;
    }

    void DomDocument::check_load_results(VARIANT_BOOL resultcode, HRESULT hr, std::string const & filename)
    {
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr);
        }
        if (resultcode != VARIANT_TRUE)
        {
            CComPtr<IXMLDOMParseError> error;
            (void)document_->get_parseError(&error);

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
                buff << "Invalid XML in " << filename << " at line " << line << " col " << column << " (near "
                     << static_cast<std::string>(static_cast<bstr_t>(text)) << "): " << 
                    static_cast<std::string>(static_cast<bstr_t>(reason));
                throw std::runtime_error(buff.str());
            }
        }
    }

}    // namespace Linter
