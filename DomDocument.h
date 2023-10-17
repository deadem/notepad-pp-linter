#pragma once

#include <memory>
#include <string>

#include <msxml6.h>
#include <wtypes.h>

namespace Linter
{
    class DomDocument
    {
        /** Importamnt note:
         * The wstring constructor takes a filename.
         * The string constructor takes an xml string.
         */
      public:
        /** Creates an XML document from the supplied filename */
        explicit DomDocument(std::wstring const &filename);

        /** Creates an XML document from the supplied UTF8 string */
        explicit DomDocument(std::string const &xml);

        ~DomDocument();

        /** Get list of nodes select by an XPATH */
        CComPtr<IXMLDOMNodeList> getNodeList(std::string const &xpath);

      private:
        /** Set up the dom interface */
        void init();

        /* Check the result of doing a load, die if it didn't complete */
        void checkLoadResults(VARIANT_BOOL resultcode, HRESULT hr, std::string const & filename);

        CComPtr<IXMLDOMDocument2> m_document;
    };

}    // namespace Linter
