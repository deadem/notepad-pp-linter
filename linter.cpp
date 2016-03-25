#include "stdafx.h"
#include "plugin.h"
#include "linter.h"
#include "XmlParser.h"
#include "encoding.h"
#include "file.h"

#include <CommCtrl.h>
#include <vector>
#include <map>

bool isReady = false;
bool isChanged = true;
bool isBufferChanged = false;

HANDLE timer(0);
HANDLE threadHandle(0);
HWND tooltipWindow(0);

std::vector<XmlParser::Error> errors;
std::map<int, std::string> errorText;
std::vector<XmlParser::Linter> linters;

void initLinters()
{
  linters = XmlParser::getLinters(getIniFileName());
}

void ClearErrors()
{
  LRESULT length = SendEditor(SCI_GETLENGTH);
  ShowError(0, length, false);
  SendEditor(SCI_ANNOTATIONCLEARALL);
}

std::wstring GetFilePart(unsigned int part)
{
  LPTSTR buff = new TCHAR[ MAX_PATH + 1 ];
  SendApp(part, MAX_PATH, (LPARAM) buff);
  std::wstring text(buff);
  delete[] buff;
  return text;
}

unsigned int __stdcall AsyncCheck(void *)
{
  CoInitialize(NULL);

  errors.clear();

  std::vector<std::wstring> commands;
  for each (const XmlParser::Linter &linter in linters)
  {
    if (GetFilePart(NPPM_GETEXTPART) == linter.m_extension)
    {
      commands.push_back(linter.m_command);
    }
  }

  std::wstring directory = GetFilePart(NPPM_GETCURRENTDIRECTORY);

  if (!commands.empty())
  {
    std::wstring file = File::write(getDocumentText());
    for each (const std::wstring &command in commands)
    {
      //std::string xml = File::exec(L"C:\\Users\\deadem\\AppData\\Roaming\\npm\\jscs.cmd --reporter=checkstyle ", file);
      std::string xml = File::exec(directory, command, file);
      std::vector<XmlParser::Error> parseError = XmlParser::getErrors(xml);
      errors.insert(errors.end(), parseError.begin(), parseError.end());
    }
    _wunlink(file.c_str());
  }

  return 0;
}

void DrawBoxes()
{
  ClearErrors();
  errorText.clear();
  for each(const XmlParser::Error &error in errors)
  {
    int position = getPositionForLine(error.m_line - 1);
    position += Encoding::utfOffset(getLineText(error.m_line - 1), error.m_column - 1);
    errorText[position] = error.m_message;
    ShowError(position, position + 1);

    //SendEditor(SCI_ANNOTATIONSETSTYLE, error.m_line - 1, SCE_SQUIGGLE_UNDERLINE_RED);
    //SendEditor(SCI_ANNOTATIONSETTEXT, error.m_line - 1, (LPARAM) error.m_message.c_str());
  }
  //SendEditor(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_BOXED);
}

VOID CALLBACK RunThread(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
  if (threadHandle == 0)
  {
    for each (const XmlParser::Linter &linter in linters)
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
    DeleteTimerQueueTimer(timers, timer, NULL);
    CreateTimerQueueTimer(&timer, timers, (WAITORTIMERCALLBACK)RunThread, NULL, 300, 0, 0);
  }
}

void showTooltip()
{
  POINT point;
  if (GetCursorPos(&point))
  {
    ScreenToClient(getScintillaWindow(), &point);
    int position = SendEditor(SCI_POSITIONFROMPOINT, point.x, point.y);

    if (errorText.find(position) != errorText.end())
    {
      if (tooltipWindow == 0)
      {
        tooltipWindow = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
          WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
          getScintillaWindow(), NULL, 0, NULL);

        SetWindowPos(tooltipWindow, HWND_TOPMOST, 0, 0, 0, 0,
          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Set up "tool" information. In this case, the "tool" is the entire parent window.
        std::wstring tip = Encoding::toUnicode(errorText[position]).c_str();;

        TOOLINFO ti = { 0 };
        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_SUBCLASS;
        ti.hwnd     = getScintillaWindow();
        ti.hinst    = 0;
        ti.lpszText = const_cast<wchar_t *>(tip.c_str());

        GetClientRect(getScintillaWindow(), &ti.rect);
        SendMessage(tooltipWindow, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
      }
    }
    else if (tooltipWindow)
    {
      DestroyWindow(tooltipWindow);
      tooltipWindow = 0;

      CStringW debug;
      debug.Format(L"destroy\n");
      OutputDebugString(debug);
    }
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
        } else {
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
    SendEditor(SCI_INDICSETSTYLE ,SCE_SQUIGGLE_UNDERLINE_RED, INDIC_BOX);// INDIC_SQUIGGLE);
    SendEditor(SCI_INDICSETFORE, SCE_SQUIGGLE_UNDERLINE_RED, 0x0000ff);

    initLinters();
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
    if (notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT)){
      isReady = false;
      isChanged = true;
      Check();
      isReady = true;
    }
    break;

  default:
    {
      CStringW debug;
      debug.Format(L"code: %u\n", notifyCode->nmhdr.code);
      OutputDebugString(debug);
    }
  case SCN_PAINTED:
  case SCN_FOCUSIN:
  case SCN_FOCUSOUT:
  case SCN_UPDATEUI:
    showTooltip();
    break;
  }
}
