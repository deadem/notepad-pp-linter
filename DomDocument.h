#pragma once

#include <memory>
#include <string>

#include <msxml6.h>
#include <wtypes.h>

namespace Linter
{

    template <class T>
    struct Destroyer
    {
        void operator()(T *iface)
        {
            iface->Release();
        }
    };


    class DomDocument
    {
      public:
        DomDocument();

        /** Creates an XML document from the supplied filename */
        void load_from_file(std::wstring const &filename);

        /** Creates an XML document from the supplied UTF8 string */
        void load_from_string(std::string const &xml);

        ~DomDocument();

        //Klunky? Remove it
        IXMLDOMDocument2 *operator->()
        {
            return document_.get();
        }

      private:
        void check_load_results(VARIANT_BOOL resultcode, HRESULT hr);

        //Replace this once I have moved selectnodes into here
        std::unique_ptr<IXMLDOMDocument2, Destroyer<IXMLDOMDocument2>> document_;
        //CComPtr<IXMLDOMDocument2> document_;
    };

}    // namespace Linter
