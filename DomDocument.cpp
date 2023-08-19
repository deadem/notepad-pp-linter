#include "stdafx.h"
#include "DomDocument.h"

#include "SystemError.h"

#include <stdexcept>

#pragma comment(lib, "msxml6.lib")

namespace Linter
{
    DomDocument::DomDocument()
    {
        IXMLDOMDocument2 *tmp = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_SERVER, IID_IXMLDOMDocument2, (LPVOID *)&tmp);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::SystemError(hr, "Linter: Can't create IID_IXMLDOMDocument2");
        }
        document_ = tmp;

        hr = document_->put_async(VARIANT_FALSE);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::SystemError("Linter: Can't XMLDOMDocument2::put_async");
        }
    }

    void DomDocument::load_from_file(std::wstring const &filename)
    {
        BSTR bstrValue{bstr_t(filename.c_str())};
        CComVariant value(bstrValue);

        VARIANT_BOOL resultCode = FALSE;
        HRESULT hr = document_->load(value, &resultCode);

        check_load_results(resultCode, hr);
    }

    void DomDocument::load_from_string(std::string const &xml)
    {
        BSTR bstrValue{(bstr_t(xml.c_str()))};

        VARIANT_BOOL resultCode = FALSE;
        HRESULT hr = document_->loadXML(bstrValue, &resultCode);

        check_load_results(resultCode, hr);
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
            throw ::Linter::SystemError(hr, "Linter: Can't execute XPath " + xpath);
        }
        return nodes;
    }

    void DomDocument::check_load_results(VARIANT_BOOL resultcode, HRESULT hr)
    {
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::SystemError(hr);
        }
        if (resultcode != VARIANT_TRUE)
        {
            CComPtr<IXMLDOMParseError> error;
            (void)document_->get_parseError(&error);

            if (error)
            {
                BSTR reason;
                error->get_reason(&reason);
                long line;
                error->get_line(&line);
                long column;
                error->get_linepos(&column);
                char buff[256];
                std::snprintf(buff,
                    sizeof(buff),
                    "Invalid XML in linter.xml at line %d col %d: %s",
                    line,
                    column,
                    static_cast<std::string>(static_cast<bstr_t>(reason)).c_str());
                throw std::runtime_error(buff);
            }
        }
    }

}    // namespace Linter
