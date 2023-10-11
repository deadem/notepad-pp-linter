#pragma once

#include "HandleWrapper.h"

namespace Linter
{
    class FilePipe
    {
      public:
        struct Pipe
        {
            HandleWrapper m_reader;
            HandleWrapper m_writer;
        };

        static Pipe create();
    };
}    // namespace Linter
