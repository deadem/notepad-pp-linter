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

    return {HandleWrapper(parent), HandleWrapper(child)};
}

void Linter::FilePipe::detachFromParent(const HandleWrapper &handle)
{
    if (!SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0))
    {
        throw SystemError(GetLastError());
    }
}
