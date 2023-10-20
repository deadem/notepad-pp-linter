#include "stdafx.h"
#include "DomDocument.h"

#include "encoding.h"
#include "SystemError.h"
#include "XmlDecodeException.h"

#include <stdexcept>
#include <sstream>

#pragma comment(lib, "msxml6.lib")

Linter::DomDocument::DomDocument(std::wstring const &filename)
{
    init();

    BSTR bstrValue{bstr_t(filename.c_str())};
    CComVariant value(bstrValue);

    VARIANT_BOOL resultCode = FALSE;
    HRESULT hr = m_document->load(value, &resultCode);

    checkLoadResults(resultCode, hr);
}

Linter::DomDocument::DomDocument(std::string const &xml)
{
    init();

    BSTR bstrValue{(bstr_t(xml.c_str()))};

    VARIANT_BOOL resultCode = FALSE;
    HRESULT hr = m_document->loadXML(bstrValue, &resultCode);

    checkLoadResults(resultCode, hr);
}

Linter::DomDocument::~DomDocument() = default;

CComPtr<IXMLDOMNodeList> Linter::DomDocument::getNodeList(std::string const &xpath)
{
    CComPtr<IXMLDOMNodeList> nodes;
    HRESULT hr = m_document->selectNodes(bstr_t(xpath.c_str()), &nodes);
    if (!SUCCEEDED(hr))
    {
        throw SystemError(hr, "Can't execute XPath " + xpath);
    }
    return nodes;
}

void Linter::DomDocument::init()
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

void Linter::DomDocument::checkLoadResults(VARIANT_BOOL resultcode, HRESULT hr)
{
    if (!SUCCEEDED(hr))
    {
        throw SystemError(hr);
    }
    if (resultcode != VARIANT_TRUE)
    {
        CComPtr<IXMLDOMParseError> error;
        (void)m_document->get_parseError(&error);
        throw XmlDecodeException(error);
    }
}
