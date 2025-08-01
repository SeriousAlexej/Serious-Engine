// SeriousEngineCAPIGameShell.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SeriousEngineCAPIGameShell.h"
#include <Engine/Engine.h>
#include <Engine/GameShell.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {
  enum GameGUI_Method
  {
    Unspecified,
    OnPlayerSettings,
    OnAudioQuality,
    OnVideoQuality,
    OnSelectPlayerAndControls
  };
}

/////////////////////////////////////////////////////////////////////////////
// CSeriousEngineCAPIGameShellApp

BEGIN_MESSAGE_MAP(CSeriousEngineCAPIGameShellApp, CWinApp)
	//{{AFX_MSG_MAP(CSeriousEngineCAPIGameShellApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSeriousEngineCAPIGameShellApp construction

CSeriousEngineCAPIGameShellApp::CSeriousEngineCAPIGameShellApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSeriousEngineCAPIGameShellApp object

CSeriousEngineCAPIGameShellApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSeriousEngineCAPIGameShellApp initialization

void CSeriousEngineCAPIGameShellApp::SubInitInstance()
{
  int space_pos = -1;
  const size_t len = _tcslen(m_lpCmdLine);
  for (size_t i = 0; i < len && space_pos == -1; ++i)
    if (m_lpCmdLine[i] == _T(' '))
      space_pos = static_cast<int>(i);
  if (space_pos == -1)
    return;
  CString methodstr = CString(m_lpCmdLine).Left(space_pos);
  if (methodstr.IsEmpty())
    return;
  CString gms_file = CString(m_lpCmdLine).Right(len - space_pos - 1);
  gms_file.TrimLeft();
  gms_file.TrimRight();
  if (gms_file.IsEmpty())
    return;

  GameGUI_Method method = Unspecified;
  if (methodstr == _T("OnPlayerSettings"))
    method = OnPlayerSettings;
  else if (methodstr == _T("OnAudioQuality"))
    method = OnAudioQuality;
  else if (methodstr == _T("OnVideoQuality"))
    method = OnVideoQuality;
  else if (methodstr == _T("OnSelectPlayerAndControls"))
    method = OnSelectPlayerAndControls;

  if (method == Unspecified)
    return;

  SE_InitEngine("");
  GameGUI_interface* _pGameGUI = NULL;
  try {
#define GAMEDLL _fnmApplicationExe.FileDir()+"GameGUI"+_strModExt+".dll"
    CTFileName fnmExpanded;
    ExpandFilePath(EFP_READ, CTString(GAMEDLL), fnmExpanded);

    HMODULE hGame = LoadLibraryA(fnmExpanded);
    if (hGame==NULL) {
      ThrowF_t("%s", GetWindowsError(GetLastError()));
    }
    GameGUI_interface* (*GAMEGUI_Create)(void) = (GameGUI_interface* (*)(void))GetProcAddress(hGame, "GAMEGUI_Create");
    if (GAMEGUI_Create==NULL) {
      ThrowF_t("%s", GetWindowsError(GetLastError()));
    }
    _pGameGUI = GAMEGUI_Create();

  } catch (char *strError) {
    FatalError("%s", strError);
  }
  CTString gms_file_string = static_cast<const char*>(gms_file);
  _pGameGUI->Initialize(gms_file_string);
  switch (method)
  {
  case OnPlayerSettings:
    _pGameGUI->OnPlayerSettings();
    break;
  case OnAudioQuality:
    _pGameGUI->OnAudioQuality();
    break;
  case OnVideoQuality:
    _pGameGUI->OnVideoQuality();
    break;
  case OnSelectPlayerAndControls:
    _pGameGUI->OnSelectPlayerAndControls();
    break;
  default:
    break;
  }
  SE_EndEngine();
}

BOOL CSeriousEngineCAPIGameShellApp::InitInstance()
{
	// Standard initialization
  if (m_lpCmdLine[0] == _T('\0'))
    return FALSE;

  CTSTREAM_BEGIN {
    SubInitInstance();
  } CTSTREAM_END;
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
