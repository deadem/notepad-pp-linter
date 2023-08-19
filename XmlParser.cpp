#include "stdafx.h"
#include "XmlParser.h"

#include "DomDocument.h"
#include "SystemError.h"
#include "encoding.h"

#include <memory>

#include <msxml6.h>
#pragma comment(lib, "msxml6.lib")

//namespace
//{

#define RELEASE(iface)    \
    if (iface)            \
    {                     \
        iface->Release(); \
        iface = NULL;     \
    }
//}    // namespace

std::vector<XmlParser::Error> XmlParser::getErrors(const std::string &xml)
{
    ::Linter::DomDocument XMLDocument;

    XMLDocument.load_from_string(xml);

    // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
    std::unique_ptr<IXMLDOMNodeList, ::Linter::Destroyer<IXMLDOMNodeList>> XMLNodeList;
    {
        IXMLDOMNodeList *tmp = nullptr;
        HRESULT hr = XMLDocument->selectNodes(bstr_t(L"//error"), &tmp);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::Exception("Linter: Can't execute XPath //error");
        }
        XMLNodeList.reset(tmp);
    }

    std::vector<XmlParser::Error> errors;

    //Why do we need unlength if we're using nextNode?
    LONG uLength;
    HRESULT hr = XMLNodeList->get_length(&uLength);
    if (!SUCCEEDED(hr))
    {
        throw ::Linter::Exception("Linter: Can't get XPath //error length");
    }
    for (int iIndex = 0; iIndex < uLength; iIndex++)
    {
        IXMLDOMNode *node;
        hr = XMLNodeList->nextNode(&node);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::Exception("Linter: Can't get next XPath element");
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
        RELEASE(node);
    }

    return errors;
}

XmlParser::Settings XmlParser::getLinters(std::wstring file)
{
    XmlParser::Settings settings;
    IXMLDOMNodeList *XMLNodeList(NULL), *styleNode(NULL);

    try
    {
        ::Linter::DomDocument XMLDocument;

        XMLDocument.load_from_file(file);

        HRESULT hr = XMLDocument->selectNodes(bstr_t(L"//style"), &styleNode);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::Exception("Linter: Can't execute XPath //style");
        }

        //Why do we need to get the length if we're going to use nextNode?
        LONG uLength;
        hr = styleNode->get_length(&uLength);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::Exception("Linter: Can't get XPath root length");
        }

        if (uLength)
        {
            IXMLDOMNode *node;
            hr = styleNode->nextNode(&node);
            if (SUCCEEDED(hr))
            {
                CComQIPtr<IXMLDOMElement> element(node);

                CComVariant alpha;
                if (element->getAttribute(bstr_t(L"alpha"), &alpha) == S_OK)
                {
                    int alpha = 0;
                    if (value.bstrVal)
                    {
                        std::wstringstream data{std::wstring(value.bstrVal, SysStringLen(value.bstrVal))};
                        data >> alpha;
                    }
                    settings.m_alpha = 0;
                }

                if (element->getAttribute(bstr_t(L"color"), &alpha) == S_OK)
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
        }

        // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
        hr = XMLDocument->selectNodes(bstr_t(L"//linter"), &XMLNodeList);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::Exception("Linter: Can't execute XPath //linter");
        }

        //Why do we need to get the length if we're going to use nextNode?
        hr = XMLNodeList->get_length(&uLength);
        if (!SUCCEEDED(hr))
        {
            throw ::Linter::Exception("Linter: Can't get XPath length");
        }

        for (int iIndex = 0; iIndex < uLength; iIndex++)
        {
            IXMLDOMNode *node;
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
                RELEASE(node);
            }
        }
        RELEASE(XMLNodeList);
    }
    catch (std::exception const &)
    {
        RELEASE(XMLNodeList);
        throw;
    }

    return settings;
}
