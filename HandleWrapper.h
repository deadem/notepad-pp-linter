#pragma once

#include<string>
#include <utility>

#include <wtypes.h>

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

        void close();

        ~HandleWrapper();

        operator HANDLE() const noexcept
        {
            return handle_;
        }

        /** Write a string to the handle
         *
         * @param str - string to write
         */
        void write_string(std::string const &str);

        /** Read the entire file */
        std::string read_file();

        static std::pair<HandleWrapper, HandleWrapper> create_pipe();

      private:
        HANDLE handle_;
    };

}    // namespace Linter
