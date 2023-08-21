#pragma once

#include <memory>
#include <string>

#include <msxml6.h>
#include <wtypes.h>

namespace Linter
{
    class DomDocument
    {
      public:
        DomDocument();

        /** Creates an XML document from the supplied filename */
        void load_from_file(std::wstring const &filename);

        /** Creates an XML document from the supplied UTF8 string */
        void load_from_string(std::string const &xml);

        ~DomDocument();

        /** Get list of nodes select by an XPATH */
        CComPtr<IXMLDOMNodeList> get_nodelist(std::string const &xpath);

      private:
        void check_load_results(VARIANT_BOOL resultcode, HRESULT hr, std::string const & filename);

        CComPtr<IXMLDOMDocument2> document_;
    };

}    // namespace Linter
