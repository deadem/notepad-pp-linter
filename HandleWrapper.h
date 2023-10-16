#pragma once

#include <string>

namespace Linter
{
    class HandleWrapper
    {
      public:
        explicit HandleWrapper(HANDLE h);
        HandleWrapper(HandleWrapper const &) = delete;
        HandleWrapper(HandleWrapper &&other) noexcept;
        HandleWrapper &operator=(HandleWrapper const &) = delete;
        HandleWrapper &operator=(HandleWrapper &&other) = delete;
        ~HandleWrapper();

        void close() const;

        operator HANDLE() const noexcept;

        /** Write a string to the handle
        *
        * @param str - string to write
        */
        void writeFile(std::string const &str) const;

        /** Read the entire file */
        std::string readFile() const;

      private:
        mutable HANDLE m_handle;
    };
}    // namespace Linter
