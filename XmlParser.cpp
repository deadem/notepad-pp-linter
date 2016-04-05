#include "stdafx.h"
#include "XmlParser.h"
#include "encoding.h"

#include <msxml6.h>
#pragma comment(lib, "msxml6.lib")

#define RELEASE(iface) if (iface) { iface->Release(); iface = NULL; }

std::vector<XmlParser::Error> XmlParser::getErrors(const std::string &xml)
{
  std::vector<XmlParser::Error> errors;
  IXMLDOMNodeList *XMLNodeList(NULL);
  IXMLDOMNode *XMLNode(NULL);
  IXMLDOMDocument2 *XMLDocument(NULL);
  HRESULT hr;

  try
  {
    hr = CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_SERVER, IID_IXMLDOMDocument2, (LPVOID*)(&XMLDocument));
    SUCCEEDED(hr) ? 0 : throw std::exception();

    if (XMLDocument)
    {
      hr = XMLDocument->put_async(VARIANT_FALSE);
      if (SUCCEEDED(hr))
      {
        std::wstring &string = Encoding::toUnicode(xml);
        BSTR bstrValue(const_cast<wchar_t *>(string.c_str()));

        short resultCode = FALSE;
        hr = XMLDocument->loadXML(bstrValue, &resultCode);
        if (SUCCEEDED(hr) && (resultCode == VARIANT_TRUE))
        {

          // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
          hr = XMLDocument->selectNodes(_T("//error"), &XMLNodeList);
          if (SUCCEEDED(hr))
          {
            LONG uLength;

            hr = XMLNodeList->get_length(&uLength);
            if (SUCCEEDED(hr))
            {
              for (int iIndex=0; iIndex < uLength; iIndex++)
              {
                IXMLDOMNode *node;
                hr = XMLNodeList->nextNode(&node);
                if (SUCCEEDED(hr))
                {
                  CComQIPtr<IXMLDOMElement> element(node);
                  if (SUCCEEDED(hr) && element)
                  {
                    Error error;
                    CComVariant value;

                    element->getAttribute(L"line", &value);
                    error.m_line = _wtoi(value.bstrVal);

                    element->getAttribute(L"column", &value);
                    error.m_column = _wtoi(value.bstrVal);

                    element->getAttribute(L"message", &value);
                    error.m_message = value.bstrVal;

                    errors.push_back(error);
                  }
                  RELEASE(node);
                }
              }
            }
            RELEASE(XMLNodeList);
          }
        }
      }
      RELEASE(XMLDocument);
    }
  }
  catch (...)
  {
    RELEASE(XMLNode);
    RELEASE(XMLDocument);
    RELEASE(XMLNodeList);
  }

  return errors;
}


std::vector<XmlParser::Linter> XmlParser::getLinters(std::wstring file)
{
  std::vector<XmlParser::Linter> linters;
  IXMLDOMNodeList *XMLNodeList(NULL);
  IXMLDOMNode *XMLNode(NULL);
  IXMLDOMDocument2 *XMLDocument(NULL);
  HRESULT hr;

  try
  {
    hr = CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_SERVER, IID_IXMLDOMDocument2, (LPVOID*)(&XMLDocument));
    SUCCEEDED(hr) ? 0 : throw std::exception();

    if (XMLDocument)
    {
      hr = XMLDocument->put_async(VARIANT_FALSE);
      if (SUCCEEDED(hr))
      {
        BSTR bstrValue(const_cast<wchar_t *>(file.c_str()));
        CComVariant value(bstrValue);

        short resultCode = FALSE;
        hr = XMLDocument->load(value, &resultCode);
        if (SUCCEEDED(hr) && (resultCode == VARIANT_TRUE))
        {

          // <error line="12" column="19" severity="error" message="Unexpected identifier" source="jscs" />
          hr = XMLDocument->selectNodes(_T("//linter"), &XMLNodeList);
          if (SUCCEEDED(hr))
          {
            LONG uLength;

            hr = XMLNodeList->get_length(&uLength);
            if (SUCCEEDED(hr))
            {
              for (int iIndex=0; iIndex < uLength; iIndex++)
              {
                IXMLDOMNode *node;
                hr = XMLNodeList->nextNode(&node);
                if (SUCCEEDED(hr))
                {
                  CComQIPtr<IXMLDOMElement> element(node);
                  if (SUCCEEDED(hr) && element)
                  {
                    Linter linter;
                    CComVariant value;

                    element->getAttribute(L"extension", &value);
                    linter.m_extension =  value.bstrVal;

                    element->getAttribute(L"command", &value);
                    linter.m_command = value.bstrVal;

                    linters.push_back(linter);
                  }
                  RELEASE(node);
                }
              }
            }
            RELEASE(XMLNodeList);
          }
        }
      }
      RELEASE(XMLDocument);
    }
  }
  catch (...)
  {
    RELEASE(XMLNode);
    RELEASE(XMLDocument);
    RELEASE(XMLNodeList);
  }

  return linters;
}
