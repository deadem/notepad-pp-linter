#include "stdafx.h"
#include "linter.h"

#include "plugin.h"
#include "XmlParser.h"
#include "encoding.h"
#include "file.h"
#include "SystemError.h"

#include <CommCtrl.h>
#include <vector>
#include <map>

bool isReady = false;
bool isChanged = true;
bool isBufferChanged = false;

HANDLE timer(0);
HANDLE threadHandle(0);

std::vector<XmlParser::Error> errors;
std::map<LRESULT, std::wstring> errorText;
XmlParser::Settings settings;

void ClearErrors()
{
    LRESULT length = SendEditor(SCI_GETLENGTH);
    ShowError(0, length, false);
    SendEditor(SCI_ANNOTATIONCLEARALL);
}

void InitErrors()
{
    SendEditor(SCI_INDICSETSTYLE, SCE_SQUIGGLE_UNDERLINE_RED, INDIC_BOX);    // INDIC_SQUIGGLE);
    SendEditor(SCI_INDICSETFORE, SCE_SQUIGGLE_UNDERLINE_RED, 0x0000ff);

    if (!settings.m_linters.empty() && (settings.m_alpha != -1 || settings.m_color != -1))
    {
        SendEditor(SCI_INDICSETSTYLE, SCE_SQUIGGLE_UNDERLINE_RED, INDIC_ROUNDBOX);

        if (settings.m_alpha != -1)
        {
            SendEditor(SCI_INDICSETALPHA, SCE_SQUIGGLE_UNDERLINE_RED, settings.m_alpha);
        }

        if (settings.m_color != -1)
        {
            SendEditor(SCI_INDICSETFORE, SCE_SQUIGGLE_UNDERLINE_RED, settings.m_color);
        }
    }
}

std::wstring GetFilePart(unsigned int part)
{
    LPTSTR buff = new TCHAR[MAX_PATH + 1];
    SendApp(part, MAX_PATH, (LPARAM)buff);
    std::wstring text(buff);
    delete[] buff;
    return text;
}

void showTooltip(std::wstring message = std::wstring())
{
    const LRESULT position = SendEditor(SCI_GETCURRENTPOS);

    HWND main = GetParent(getScintillaWindow());
    HWND childHandle = FindWindowEx(main, NULL, L"msctls_statusbar32", NULL);

    auto error = errorText.find(position);
    if (error != errorText.end())
    {
        SendMessage(childHandle, WM_SETTEXT, 0, reinterpret_cast<LPARAM>((std::wstring(L" - ") + error->second).c_str()));
        //OutputDebugString(error->second.c_str());
    }
    else
    {
        wchar_t title[256] = {0};
        SendMessage(childHandle, WM_GETTEXT, sizeof(title) / sizeof(title[0]) - 1, reinterpret_cast<LPARAM>(title));

        std::wstring str(title);
        if (message.empty() && str.find(L" - ") == 0)
        {
            message = L" - ";
        }

        if (!message.empty())
        {
            SendMessage(childHandle, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(message.c_str()));
        }
    }
}

void apply_linters()
{
    errors.clear();

    std::vector<std::pair<std::wstring, bool>> commands;
    bool useStdin = true;
    for (const XmlParser::Linter &linter : settings.m_linters)
    {
        if (GetFilePart(NPPM_GETEXTPART) == linter.m_extension)
        {
            commands.emplace_back(linter.m_command, linter.m_useStdin);
            useStdin = useStdin && linter.m_useStdin;
        }
    }

    if (commands.empty())
    {
        return;
    }

    const std::string &text = getDocumentText();

    File file(GetFilePart(NPPM_GETFILENAME), GetFilePart(NPPM_GETCURRENTDIRECTORY));
    if (!useStdin)
    {
        file.write(text);
    }

    for (const auto &command : commands)
    {
        //std::string xml = File::exec(L"C:\\Users\\deadem\\AppData\\Roaming\\npm\\jscs.cmd --reporter=checkstyle ", file);
        try
        {
            nonstd::optional<std::string> str;
            if (command.second)
            {
                str = text;
            }
            std::string xml = file.exec(command.first, str);
            std::vector<XmlParser::Error> parseError = XmlParser::getErrors(xml);
            errors.insert(errors.end(), parseError.begin(), parseError.end());
        }
        catch (const std::exception &e)
        {
            //If we get an error running a command, log it, but carry on with the next command.
            std::string str(e.what());
            showTooltip(std::wstring(str.begin(), str.end()));
        }
    }
}

unsigned int __stdcall AsyncCheck(void *)
{
    (void)CoInitialize(NULL);
    try
    {
        apply_linters();
    }
    catch (const std::exception &e)
    {
        std::string str(e.what());
        showTooltip(std::wstring(str.begin(), str.end()));
    }

    return 0;
}

void DrawBoxes()
{
    ClearErrors();
    errorText.clear();
    if (!errors.empty())
    {
        InitErrors();
    }

    for (const XmlParser::Error &error : errors)
    {
        auto position = getPositionForLine(error.m_line - 1);
        position += Encoding::utfOffset(getLineText(error.m_line - 1), error.m_column - 1);
        errorText[position] = error.m_message;
        ShowError(position, position + 1);
    }
}

VOID CALLBACK RunThread(PVOID /*lpParam*/, BOOLEAN /*TimerOrWaitFired*/)
{
    if (threadHandle == 0)
    {
        for (const XmlParser::Linter &linter : settings.m_linters)
        {
            if (GetFilePart(NPPM_GETEXTPART) == linter.m_extension)
            {
                unsigned threadID(0);
                threadHandle = (HANDLE)_beginthreadex(NULL, 0, &AsyncCheck, NULL, 0, &threadID);
                isChanged = false;
                break;
            }
        }
    }
}

void Check()
{
    if (isChanged)
    {
        (void)DeleteTimerQueueTimer(timers, timer, NULL);
        CreateTimerQueueTimer(&timer, timers, (WAITORTIMERCALLBACK)RunThread, NULL, 300, 0, 0);
    }
}

void initLinters()
{
    try
    {
        settings = XmlParser::getLinters(getIniFileName());
        if (settings.m_linters.empty())
        {
            showTooltip(L"Linter: Empty linters.xml.");
        }
    }
    catch (std::exception const &e)
    {
        std::string str(e.what());
        showTooltip(std::wstring(str.begin(), str.end()));
    }
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    if (threadHandle)
    {
        DWORD exitCode(0);
        if (GetExitCodeThread(threadHandle, &exitCode))
        {
            if (exitCode != STILL_ACTIVE)
            {
                CloseHandle(threadHandle);
                if (isBufferChanged == false)
                {
                    DrawBoxes();
                }
                else
                {
                    isChanged = true;
                }
                threadHandle = 0;
                isBufferChanged = false;
                Check();
            }
        }
    }
    switch (notifyCode->nmhdr.code)
    {
        case NPPN_READY:
            initLinters();
            InitErrors();

            isReady = true;
            isChanged = true;
            Check();
            break;

        case NPPN_SHUTDOWN:
            commandMenuCleanUp();
            break;

        default:
            break;
    }

    if (!isReady)
    {
        return;
    }

    switch (notifyCode->nmhdr.code)
    {
        case NPPN_BUFFERACTIVATED:
            isChanged = true;
            isBufferChanged = true;
            Check();
            break;

        case SCN_MODIFIED:
            if (notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT))
            {
                isReady = false;
                isChanged = true;
                Check();
                isReady = true;
            }
            break;

        default:
        {
            //CStringW debug;
            //debug.Format(L"code: %u\n", notifyCode->nmhdr.code);
            //OutputDebugString(debug);
        }
        break;

        case SCN_UPDATEUI:
            showTooltip();
            break;

        case SCN_PAINTED:
        case SCN_FOCUSIN:
        case SCN_FOCUSOUT:
            break;
    }
}
