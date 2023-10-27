#include "stdafx.h"
#include "XmlParser.h"

#include "DomDocument.h"
#include "SystemError.h"
#include "encoding.h"

#include <memory>
#include <sstream>

#include <msxml6.h>

using Linter::SystemError;

std::vector<XmlParser::Error> XmlParser::getErrors(const std::string &xml)
{
    ::Linter::DomDocument XMLDocument(xml);
    // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />

    std::vector<XmlParser::Error> errors;

    CComPtr<IXMLDOMNodeList> XMLNodeList{XMLDocument.getNodeList("//error")};

    LONG uLength;
    HRESULT hr = XMLNodeList->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw SystemError(hr, "Can't get XPath //error length");
    }
    for (int iIndex = 0; iIndex < uLength; iIndex++)
    {
        CComPtr<IXMLDOMNode> node;
        hr = XMLNodeList->nextNode(&node);
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't get next XPath element");
        }

        CComQIPtr<IXMLDOMElement> element(node);
        Error error;
        CComVariant value;

        element->getAttribute(L"line", &value);
        error.m_line = std::stoi(value.bstrVal);

        element->getAttribute(L"column", &value);
        error.m_column = std::stoi(value.bstrVal);

        element->getAttribute(L"message", &value);
        error.m_message = value.bstrVal;

        errors.push_back(error);
    }

    return errors;
}

XmlParser::Settings XmlParser::getLinters(std::wstring const &file)
{
    XmlParser::Settings settings;
    ::Linter::DomDocument XMLDocument(file);
    CComPtr<IXMLDOMNodeList> styleNode{XMLDocument.getNodeList("//style")};

    LONG uLength;
    HRESULT hr = styleNode->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw SystemError(hr, "Can't get XPath style length");
    }

    if (uLength != 0)
    {
        CComPtr<IXMLDOMNode> node;
        hr = styleNode->nextNode(&node);
        if (!SUCCEEDED(hr))
        {
            throw SystemError(hr, "Can't read style node");
        }
        CComQIPtr<IXMLDOMElement> element(node);
        CComVariant value;
        if (element->getAttribute(L"alpha", &value) == S_OK)
        {
            int alpha = 0;
            if (value.bstrVal)
            {
                std::wstringstream data{std::wstring(value.bstrVal, SysStringLen(value.bstrVal))};
                data >> alpha;
            }
            settings.m_alpha = alpha;
        }

        if (element->getAttribute(L"color", &value) == S_OK)
        {
            unsigned int color(0);
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

    // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
    CComPtr<IXMLDOMNodeList> XMLNodeList{XMLDocument.getNodeList("//linter")};

    hr = XMLNodeList->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw SystemError(hr, "Can't get XPath length");
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

            element->getAttribute(L"extension", &extension);
            linter.m_extension = extension.bstrVal;

            element->getAttribute(L"command", &extension);
            linter.m_command = extension.bstrVal;

            element->getAttribute(L"stdin", &extension);
            linter.m_useStdin = !!extension.boolVal;

            settings.m_linters.push_back(linter);
        }
    }

    return settings;
}
