#include "stdafx.h"
#include "FilePipe.h"

#include "SystemError.h"

Linter::FilePipe::Pipe Linter::FilePipe::create()
{
    SECURITY_ATTRIBUTES security;

    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    security.bInheritHandle = TRUE;
    security.lpSecurityDescriptor = nullptr;

    HANDLE parent;
    HANDLE child;
    if (!CreatePipe(&parent, &child, &security, 0))
    {
        throw SystemError(GetLastError());
    }

    //Stop my handle being inherited by the child
    if (!SetHandleInformation(parent, HANDLE_FLAG_INHERIT, 0))
    {
        throw SystemError(GetLastError());
    }

    return {HandleWrapper(parent), HandleWrapper(child)};
}
