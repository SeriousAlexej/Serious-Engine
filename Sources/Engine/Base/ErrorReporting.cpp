/* Copyright (c) 2002-2012 Croteam Ltd. 
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/ErrorTable.h>
#include <Engine/Base/Translation.h>

#include <Engine/Base/FileName.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/Console_internal.h>

#include <Engine/Graphics/Adapter.h>

#include <SDL2/SDL_stdinc.h>

INDEX con_bNoWarnings = 0;

// global handle for application window in full-screen mode
extern HWND _hwndMain;
extern BOOL _bFullScreen;


/*
 * Report error and terminate program.
 */
static BOOL _bInFatalError = FALSE;
void FatalError(const char *strFormat, ...)
{
  // disable recursion
  if (_bInFatalError) return;
  _bInFatalError = TRUE;

  // reset default windows display mode first 
  // (this is a low overhead and shouldn't allocate memory)
  CDS_ResetMode();

  // format the message in buffer
  va_list arg;
  va_start(arg, strFormat);
  CTString strBuffer;
  strBuffer.VPrintF(strFormat, arg);

  if (_pConsole!=NULL) {
    // print the buffer to the console
    CPutString(TRANS("FatalError:\n"));
    CPutString(strBuffer);
    // make sure the console log was written safely
    _pConsole->CloseLog();
  }

  use sdl_showsimplemessagebox
#ifdef WIN32
  // create message box with just OK button
  MessageBoxA(NULL, strBuffer, TRANS("Fatal Error"),
    MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
#endif
  _bInFatalError = FALSE;

#ifdef WIN32
  extern void EnableWindowsKeys(void);
  EnableWindowsKeys();
#endif
  // exit program
  exit(EXIT_FAILURE);
}

/*
 * Report warning without terminating program (stops program until user responds).
 */
void WarningMessage(const char *strFormat, ...)
{
  // format the message in buffer
  va_list arg;
  va_start(arg, strFormat);
  CTString strBuffer;
  strBuffer.VPrintF(strFormat, arg);

  // print it to console
  CPrintF("%s\n", strBuffer);
  // if warnings are enabled
  use sdl_showsimplemessagebox
#ifdef WIN32
  if( !con_bNoWarnings) {
    // create message box
    MessageBoxA(NULL, strBuffer, TRANS("Warning"), MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL);
  }
#endif
}


/*
 * Throw an exception of formatted string.
 */
[[noreturn]] void ThrowF_t(const char *strFormat, ...)  // throws char *
{
  const SLONG slBufferSize = 256;
  char strBuffer[slBufferSize+1];
  // format the message in buffer
  va_list arg;
  va_start(arg, strFormat); // variable arguments start after this argument
  SDL_vsnprintf(strBuffer, slBufferSize, strFormat, arg);
  throw strBuffer;
}

/*
 * Get the name string for error code.
 */
 const char *ErrorName(const struct ErrorTable *pet, SLONG ulErrCode)
{
  for (INDEX i=0; i<pet->et_Count; i++) {
    if (pet->et_Errors[i].ec_Code == ulErrCode) {
      return pet->et_Errors[i].ec_Name;
    }
  }
  return TRANS("CROTEAM_UNKNOWN");
}

/*
 * Get the description string for error code.
 */
 const char *ErrorDescription(const struct ErrorTable *pet, SLONG ulErrCode)
{
  for (INDEX i=0; i<pet->et_Count; i++) {
    if (pet->et_Errors[i].ec_Code == ulErrCode) {
      return pet->et_Errors[i].ec_Description;
    }
  }
  return TRANS("Unknown error");
}

/*
 * Get the description string for windows error code.
 */
 extern const CTString GetWindowsError(DWORD dwWindowsErrorCode)
{
   fix this shit and use sdl
#ifdef WIN32
  // buffer to receive error description
  LPVOID lpMsgBuf;
  // call function that will prepare text abount given windows error code
  FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, dwWindowsErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf, 0, NULL );
  // create result CTString from prepared message
  CTString strResultMessage = (char *)lpMsgBuf;
  // Free the buffer.
  LocalFree( lpMsgBuf );
  return strResultMessage;
#elif unix
   return "GetWindowsError: Not implemented on unix platform";
#endif
}
