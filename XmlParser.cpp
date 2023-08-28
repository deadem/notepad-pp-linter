#include "stdafx.h"
#include "XmlParser.h"

#include "SystemError.h"
#include "encoding.h"

#include <memory>

#include <msxml6.h>
#pragma comment(lib, "msxml6.lib")

namespace
{

#define RELEASE(iface)    \
    if (iface)            \
    {                     \
        iface->Release(); \
        iface = NULL;     \
    }

    template <class T>
    struct Destroyer
    {
        void operator()(T *iface)
        {
            iface->Release();
        }
    };

    class XML_Document
    {
      public:
        /** Creates an XML document from the supplied filename */
        explicit XML_Document(std::wstring const &filename)
        {
            create_xml_doc_obj();

            BSTR bstrValue{bstr_t(filename.c_str())};
            CComVariant value(bstrValue);

            VARIANT_BOOL resultCode = FALSE;
            HRESULT hr = document_->load(value, &resultCode);

            check_load_results(resultCode, hr);
        }

        /** Creates an XML document from the supplied UTF8 string */
        explicit XML_Document(std::string const &xml)
        {
            create_xml_doc_obj();

            std::wstring string = Encoding::toUnicode(xml);
            BSTR bstrValue{(bstr_t(string.c_str()))};

            VARIANT_BOOL resultCode = FALSE;
            HRESULT hr = document_->loadXML(bstrValue, &resultCode);

            check_load_results(resultCode, hr);
        }

        ~XML_Document()
        {
        }

        //Klunky? Remove it
        IXMLDOMDocument2 *operator->()
        {
            return document_.get();
        }

      private:
        void create_xml_doc_obj()
        {
            IXMLDOMDocument2 *tmp = nullptr;
            HRESULT hr = CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_SERVER, IID_IXMLDOMDocument2, (LPVOID *)&tmp);
            if (!SUCCEEDED(hr))
            {
                throw ::Linter::SystemError(hr, "Linter: Can't create IID_IXMLDOMDocument2");
            }
            document_.reset(tmp);

            hr = document_->put_async(VARIANT_FALSE);
            if (!SUCCEEDED(hr))
            {
                throw ::Linter::SystemError("Linter: Can't XMLDOMDocument2::put_async");
            }
        }

        void check_load_results(VARIANT_BOOL resultcode, HRESULT hr)
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

        //Replace this once I have moved selectnodes into here
        std::unique_ptr<IXMLDOMDocument2, Destroyer<IXMLDOMDocument2>> document_;
        //CComPtr<IXMLDOMDocument2> document_;
    };

}    // namespace

std::vector<XmlParser::Error> XmlParser::getErrors(const std::string &xml)
{
    XML_Document XMLDocument{xml};

    // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
    std::unique_ptr<IXMLDOMNodeList, Destroyer<IXMLDOMNodeList>> XMLNodeList;
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
        XML_Document XMLDocument(file);

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
