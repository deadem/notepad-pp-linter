#include "stdafx.h"
#include "XmlParser.h"

#include "DomDocument.h"
#include "SystemError.h"
#include "encoding.h"

#include <memory>
#include <sstream>

#include <msxml6.h>
#pragma comment(lib, "msxml6.lib")

std::vector<XmlParser::Error> XmlParser::getErrors(const std::string &xml)
{
    ::Linter::DomDocument XMLDocument;

    XMLDocument.load_from_string(xml);
    // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />

    std::vector<XmlParser::Error> errors;

    CComPtr<IXMLDOMNodeList> XMLNodeList{XMLDocument.get_nodelist("//error")};

    //Why do we need uLength if we're using nextNode?
    LONG uLength;
    HRESULT hr = XMLNodeList->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw ::Linter::SystemError(hr, "Linter: Can't get XPath //error length");
    }
    for (int iIndex = 0; iIndex < uLength; iIndex++)
    {
        CComPtr<IXMLDOMNode> node;
        hr = XMLNodeList->nextNode(&node);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::SystemError(hr, "Linter: Can't get next XPath element");
        }

        CComQIPtr<IXMLDOMElement> element(node);
        Error error;
        CComVariant value;

        element->getAttribute(bstr_t(L"line"), &value);
        error.m_line = _wtoi(value.bstrVal);

        element->getAttribute(bstr_t(L"column"), &value);
        error.m_column = _wtoi(value.bstrVal);

        element->getAttribute(bstr_t(L"message"), &value);
        error.m_message = value.bstrVal;

        errors.push_back(error);
    }

    return errors;
}

XmlParser::Settings XmlParser::getLinters(std::wstring const &file)
{
    XmlParser::Settings settings;
    ::Linter::DomDocument XMLDocument;

    XMLDocument.load_from_file(file);
    CComPtr<IXMLDOMNodeList> styleNode{XMLDocument.get_nodelist("//style")};

    //Why do we need to get the length if we're going to use nextNode?
    LONG uLength;
    HRESULT hr = styleNode->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw ::Linter::SystemError(hr, "Linter: Can't get XPath root length");
    }

    if (uLength != 0)
    {
        CComPtr<IXMLDOMNode> node;
        hr = styleNode->nextNode(&node);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<IXMLDOMElement> element(node);
            CComVariant value;
            if (element->getAttribute(bstr_t(L"alpha"), &value) == S_OK)
            {
                CComQIPtr<IXMLDOMElement> element(node);

                CComVariant alpha;
                if (element->getAttribute(bstr_t(L"alpha"), &alpha) == S_OK)
                {
                    std::wstringstream data{std::wstring(value.bstrVal, SysStringLen(value.bstrVal))};
                    data >> alpha;
                }
                settings.m_alpha = alpha;
            }

            if (element->getAttribute(bstr_t(L"color"), &value) == S_OK)
                if (value.bstrVal)
                {
                    std::wstringstream data{std::wstring(value.bstrVal, SysStringLen(value.bstrVal))};
                    data >> std::hex >> color;
                }

                // reverse colors for scintilla's LE order
                settings.m_color = color & 0xFF;

                settings.m_color <<= 8;
                color >>= 8;
                settings.m_color |= color & 0xFF;

                settings.m_color <<= 8;
                color >>= 8;
                settings.m_color |= color & 0xFF;
            }
        }
    }

    // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
    CComPtr<IXMLDOMNodeList> XMLNodeList{XMLDocument.get_nodelist("//linter")};

    //Why do we need to get the length if we're going to use nextNode?
    hr = XMLNodeList->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw ::Linter::SystemError(hr, "Linter: Can't get XPath length");
    }

    for (int iIndex = 0; iIndex < uLength; iIndex++)
    {
        CComPtr<IXMLDOMNode> node;
        hr = XMLNodeList->nextNode(&node);
        if (SUCCEEDED(hr) && node != nullptr)
        {
            CComQIPtr<IXMLDOMElement> element(node);
            Linter linter;
            CComVariant extension;

            element->getAttribute(bstr_t(L"extension"), &extension);
            linter.m_extension = extension.bstrVal;

            element->getAttribute(bstr_t(L"command"), &extension);
            linter.m_command = extension.bstrVal;

            element->getAttribute(bstr_t(L"stdin"), &extension);
            linter.m_useStdin = !!extension.boolVal;

            settings.m_linters.push_back(linter);
        }
    }

    return settings;
}
