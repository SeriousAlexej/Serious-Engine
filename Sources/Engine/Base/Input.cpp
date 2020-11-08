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



#include <Engine/Base/Timer.h>
#include <Engine/Base/Input.h>
#include <Engine/Base/Translation.h>
#include <Engine/Base/KeyNames.h>
#include <Engine/Math/Functions.h>
#include <Engine/Graphics/ViewPort.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/Synchronization.h>

#include <Engine/Base/ErrorReporting.h>

#include <SDL2/SDL_keycode.h>

extern INDEX inp_iKeyboardReadingMethod;
extern FLOAT inp_fMouseSensitivity;
extern INDEX inp_bAllowMouseAcceleration;
extern INDEX inp_bMousePrecision;
extern FLOAT inp_fMousePrecisionFactor;
extern FLOAT inp_fMousePrecisionThreshold;
extern FLOAT inp_fMousePrecisionTimeout;
extern FLOAT inp_bInvertMouse;
extern INDEX inp_bFilterMouse;
extern INDEX inp_bAllowPrescan;

INDEX inp_bForceJoystickPolling = 0;
INDEX inp_ctJoysticksAllowed = 8;
INDEX inp_bAutoDisableJoysticks = 0;

static CTString inp_astrAxisTran[MAX_OVERALL_AXES];// translated names for axis

/*

NOTE: Three different types of key codes are used here:
  1) kid - engine internal type - defined in KeyNames.h
  2) scancode - raw PC keyboard scancodes as returned in keydown/keyup messages
  3) virtkey - virtual key codes used by windows

*/

// name that is not translated (international)
#define INTNAME(str) str, ""
// name that is translated
#define TRANAME(str) str, "ETRS" str

// basic key conversion table
static struct KeyConversion {
  INDEX kc_iKID;
  INDEX kc_iVirtKey;
  INDEX kc_iScanCode;
  const char *kc_strName;
  const char *kc_strNameTrans;
} _akcKeys[] = {

  // reserved for 'no-key-pressed'
  { KID_NONE, -1, -1, TRANAME("None")},
                            
// numbers row              
  { KID_1               , SDLK_1,   SDL_SCANCODE_1, INTNAME("1")},
  { KID_2               , SDLK_2,   SDL_SCANCODE_2, INTNAME("2")},
  { KID_3               , SDLK_3,   SDL_SCANCODE_3, INTNAME("3")},
  { KID_4               , SDLK_4,   SDL_SCANCODE_4, INTNAME("4")},
  { KID_5               , SDLK_5,   SDL_SCANCODE_5, INTNAME("5")},
  { KID_6               , SDLK_6,   SDL_SCANCODE_6, INTNAME("6")},
  { KID_7               , SDLK_7,   SDL_SCANCODE_7, INTNAME("7")},
  { KID_8               , SDLK_8,   SDL_SCANCODE_8, INTNAME("8")},
  { KID_9               , SDLK_9,  SDL_SCANCODE_9, INTNAME("9")},
  { KID_0               , SDLK_0,  SDL_SCANCODE_0, INTNAME("0")},
  { KID_MINUS           , SDLK_MINUS,  SDL_SCANCODE_MINUS, INTNAME("-")},
  { KID_EQUALS          , SDLK_EQUALS,  SDL_SCANCODE_EQUALS, INTNAME("=")},
                            
// 1st alpha row            
  { KID_Q               , SDLK_q,  SDL_SCANCODE_Q, INTNAME("Q")},
  { KID_W               , SDLK_w,  SDL_SCANCODE_W, INTNAME("W")},
  { KID_E               , SDLK_e,  SDL_SCANCODE_E, INTNAME("E")},
  { KID_R               , SDLK_r,  SDL_SCANCODE_R, INTNAME("R")},
  { KID_T               , SDLK_t,  SDL_SCANCODE_T, INTNAME("T")},
  { KID_Y               , SDLK_y,  SDL_SCANCODE_Y, INTNAME("Y")},
  { KID_U               , SDLK_u,  SDL_SCANCODE_U, INTNAME("U")},
  { KID_I               , SDLK_i,  SDL_SCANCODE_I, INTNAME("I")},
  { KID_O               , SDLK_o,  SDL_SCANCODE_O, INTNAME("O")},
  { KID_P               , SDLK_p,  SDL_SCANCODE_P, INTNAME("P")},
  { KID_LBRACKET        , SDLK_LEFTBRACKET,  SDL_SCANCODE_LEFTBRACKET, INTNAME("[")},
  { KID_RBRACKET        , SDLK_RIGHTBRACKET,  SDL_SCANCODE_RIGHTBRACKET, INTNAME("]")},
  { KID_BACKSLASH       , SDLK_BACKSLASH,  SDL_SCANCODE_BACKSLASH, INTNAME("\\")},
                            
// 2nd alpha row            
  { KID_A               , SDLK_a,  SDL_SCANCODE_A, INTNAME("A")},
  { KID_S               , SDLK_s,  SDL_SCANCODE_S, INTNAME("S")},
  { KID_D               , SDLK_d,  SDL_SCANCODE_D, INTNAME("D")},
  { KID_F               , SDLK_f,  SDL_SCANCODE_F, INTNAME("F")},
  { KID_G               , SDLK_g,  SDL_SCANCODE_G, INTNAME("G")},
  { KID_H               , SDLK_h,  SDL_SCANCODE_H, INTNAME("H")},
  { KID_J               , SDLK_j,  SDL_SCANCODE_J, INTNAME("J")},
  { KID_K               , SDLK_k,  SDL_SCANCODE_K, INTNAME("K")},
  { KID_L               , SDLK_l,  SDL_SCANCODE_L, INTNAME("L")},
  { KID_SEMICOLON       , SDLK_SEMICOLON,  SDL_SCANCODE_SEMICOLON, INTNAME(";")},
  { KID_APOSTROPHE      , SDLK_QUOTE,  SDL_SCANCODE_APOSTROPHE, INTNAME("'")},
// 3rd alpha row
  { KID_Z               , SDLK_z,  SDL_SCANCODE_Z, INTNAME("Z")},
  { KID_X               , SDLK_x,  SDL_SCANCODE_X, INTNAME("X")},
  { KID_C               , SDLK_c,  SDL_SCANCODE_C, INTNAME("C")},
  { KID_V               , SDLK_v,  SDL_SCANCODE_V, INTNAME("V")},
  { KID_B               , SDLK_b,  SDL_SCANCODE_B, INTNAME("B")},
  { KID_N               , SDLK_n,  SDL_SCANCODE_N, INTNAME("N")},
  { KID_M               , SDLK_m,  SDL_SCANCODE_M, INTNAME("M")},
  { KID_COMMA           , SDLK_COMMA,  SDL_SCANCODE_COMMA, INTNAME(",")},
  { KID_PERIOD          , SDLK_PERIOD,  SDL_SCANCODE_PERIOD, INTNAME(".")},
  { KID_SLASH           , SDLK_SLASH,  SDL_SCANCODE_SLASH, INTNAME("/")},
                                       
// row with F-keys                     
  { KID_F1              ,  SDLK_F1,  SDL_SCANCODE_F1, INTNAME("F1")},
  { KID_F2              ,  SDLK_F2,  SDL_SCANCODE_F2, INTNAME("F2")},
  { KID_F3              ,  SDLK_F3,  SDL_SCANCODE_F3, INTNAME("F3")},
  { KID_F4              ,  SDLK_F4,  SDL_SCANCODE_F4, INTNAME("F4")},
  { KID_F5              ,  SDLK_F5,  SDL_SCANCODE_F5, INTNAME("F5")},
  { KID_F6              ,  SDLK_F6,  SDL_SCANCODE_F6, INTNAME("F6")},
  { KID_F7              ,  SDLK_F7,  SDL_SCANCODE_F7, INTNAME("F7")},
  { KID_F8              ,  SDLK_F8,  SDL_SCANCODE_F8, INTNAME("F8")},
  { KID_F9              ,  SDLK_F9,  SDL_SCANCODE_F9, INTNAME("F9")},
  { KID_F10             , SDLK_F10,  SDL_SCANCODE_F10, INTNAME("F10")},
  { KID_F11             , SDLK_F11,  SDL_SCANCODE_F11, INTNAME("F11")},
  { KID_F12             , SDLK_F12,  SDL_SCANCODE_F12, INTNAME("F12")},
                            
// extra keys               
  { KID_ESCAPE          , SDLK_ESCAPE,     SDL_SCANCODE_ESCAPE, TRANAME("Escape")},
  { KID_TILDE           , SDLK_BACKQUOTE,  SDL_SCANCODE_GRAVE, TRANAME("Tilde")},
  { KID_BACKSPACE       , SDLK_BACKSPACE,  SDL_SCANCODE_BACKSPACE, TRANAME("Backspace")},
  { KID_TAB             , SDLK_TAB,       SDL_SCANCODE_TAB, TRANAME("Tab")},
  { KID_CAPSLOCK        , SDLK_CAPSLOCK,   SDL_SCANCODE_CAPSLOCK, TRANAME("Caps Lock")},
  { KID_ENTER           , SDLK_RETURN,    SDL_SCANCODE_RETURN, TRANAME("Enter")},
  { KID_SPACE           , SDLK_SPACE,     SDL_SCANCODE_SPACE, TRANAME("Space")},
                                            
// modifier keys                            
  { KID_LSHIFT          , SDLK_LSHIFT  , SDL_SCANCODE_LSHIFT, TRANAME("Left Shift")},
  { KID_RSHIFT          , SDLK_RSHIFT  , SDL_SCANCODE_RSHIFT, TRANAME("Right Shift")},
  { KID_LCONTROL        , SDLK_LCTRL, SDL_SCANCODE_LCTRL, TRANAME("Left Control")},
  { KID_RCONTROL        , SDLK_RCTRL, SDL_SCANCODE_RCTRL, TRANAME("Right Control")},
  { KID_LALT            , SDLK_LALT   , SDL_SCANCODE_LALT, TRANAME("Left Alt")},
  { KID_RALT            , SDLK_RALT   , SDL_SCANCODE_RALT, TRANAME("Right Alt")},
                            
// navigation keys          
  { KID_ARROWUP         , SDLK_UP,        SDL_SCANCODE_UP, TRANAME("Arrow Up")},
  { KID_ARROWDOWN       , SDLK_DOWN,      SDL_SCANCODE_DOWN, TRANAME("Arrow Down")},
  { KID_ARROWLEFT       , SDLK_LEFT,      SDL_SCANCODE_LEFT, TRANAME("Arrow Left")},
  { KID_ARROWRIGHT      , SDLK_RIGHT,     SDL_SCANCODE_RIGHT, TRANAME("Arrow Right")},
  { KID_INSERT          , SDLK_INSERT,    SDL_SCANCODE_INSERT, TRANAME("Insert")},
  { KID_DELETE          , SDLK_DELETE,    SDL_SCANCODE_DELETE, TRANAME("Delete")},
  { KID_HOME            , SDLK_HOME,      SDL_SCANCODE_HOME, TRANAME("Home")},
  { KID_END             , SDLK_END,       SDL_SCANCODE_END, TRANAME("End")},
  { KID_PAGEUP          , SDLK_PAGEUP,     SDL_SCANCODE_PAGEUP, TRANAME("Page Up")},
  { KID_PAGEDOWN        , SDLK_PAGEDOWN,   SDL_SCANCODE_PAGEDOWN, TRANAME("Page Down")},
  { KID_PRINTSCR        , SDLK_PRINTSCREEN,  SDL_SCANCODE_PRINTSCREEN, TRANAME("Print Screen")},
  { KID_SCROLLLOCK      , SDLK_SCROLLLOCK,    SDL_SCANCODE_SCROLLLOCK, TRANAME("Scroll Lock")},
  { KID_PAUSE           , SDLK_PAUSE,     SDL_SCANCODE_PAUSE, TRANAME("Pause")},
                            
// numpad numbers           
  { KID_NUM0            , SDLK_KP_0, SDL_SCANCODE_KP_0, INTNAME("Num 0")},
  { KID_NUM1            , SDLK_KP_1, SDL_SCANCODE_KP_1, INTNAME("Num 1")},
  { KID_NUM2            , SDLK_KP_2, SDL_SCANCODE_KP_2, INTNAME("Num 2")},
  { KID_NUM3            , SDLK_KP_3, SDL_SCANCODE_KP_3, INTNAME("Num 3")},
  { KID_NUM4            , SDLK_KP_4, SDL_SCANCODE_KP_4, INTNAME("Num 4")},
  { KID_NUM5            , SDLK_KP_5, SDL_SCANCODE_KP_5, INTNAME("Num 5")},
  { KID_NUM6            , SDLK_KP_6, SDL_SCANCODE_KP_6, INTNAME("Num 6")},
  { KID_NUM7            , SDLK_KP_7, SDL_SCANCODE_KP_7, INTNAME("Num 7")},
  { KID_NUM8            , SDLK_KP_8, SDL_SCANCODE_KP_8, INTNAME("Num 8")},
  { KID_NUM9            , SDLK_KP_9, SDL_SCANCODE_KP_9, INTNAME("Num 9")},
  { KID_NUMDECIMAL      , SDLK_KP_PERIOD, SDL_SCANCODE_KP_PERIOD, INTNAME("Num .")},
                            
// numpad gray keys         
  { KID_NUMLOCK         , SDLK_NUMLOCKCLEAR,  SDL_SCANCODE_NUMLOCKCLEAR, INTNAME("Num Lock")},
  { KID_NUMSLASH        , SDLK_KP_DIVIDE,    SDL_SCANCODE_KP_DIVIDE, INTNAME("Num /")},
  { KID_NUMMULTIPLY     , SDLK_KP_MULTIPLY,  SDL_SCANCODE_KP_MULTIPLY, INTNAME("Num *")},
  { KID_NUMMINUS        , SDLK_KP_MINUS,  SDL_SCANCODE_KP_MINUS, INTNAME("Num -")},
  { KID_NUMPLUS         , SDLK_KP_PLUS,   SDL_SCANCODE_KP_PLUS, INTNAME("Num +")},
  { KID_NUMENTER        , SDLK_KP_ENTER, SDL_SCANCODE_KP_ENTER, TRANAME("Num Enter")},

// mouse buttons
  { KID_MOUSE1          , -1, -1, TRANAME("Mouse Button 1")},
  { KID_MOUSE2          , -1, -1, TRANAME("Mouse Button 2")},
  { KID_MOUSE3          , -1, -1, TRANAME("Mouse Button 3")},
  { KID_MOUSE4          , -1, -1, TRANAME("Mouse Button 4")},
  { KID_MOUSE5          , -1, -1, TRANAME("Mouse Button 5")},
  { KID_MOUSEWHEELUP    , -1, -1, TRANAME("Mouse Wheel Up")},
  { KID_MOUSEWHEELDOWN  , -1, -1, TRANAME("Mouse Wheel Down")},
};


// autogenerated fast conversion tables
//static INDEX _aiScanToKid[512];
//static INDEX _aiVirtToKid[256];

// make fast conversion tables from the general table
static void MakeConversionTables(void)
{
  /*
  // clear conversion tables
  memset(_aiScanToKid, -1, sizeof(_aiScanToKid));
  memset(_aiVirtToKid, -1, sizeof(_aiVirtToKid));

  // for each Key
  for (INDEX iKey=0; iKey<ARRAYCOUNT(_akcKeys); iKey++) {
    struct KeyConversion &kc = _akcKeys[iKey];

    // get codes
    INDEX iKID  = kc.kc_iKID;
    INDEX iScan = kc.kc_iScanCode;
    INDEX iVirt = kc.kc_iVirtKey;

    // update the tables
    if (iScan>=0) {
      _aiScanToKid[iScan] = iKID;
    }
    if (iVirt>=0) {
      _aiVirtToKid[iVirt] = iKID;
    }
  }
  */
}

// variables for message interception
static int _iMouseZ = 0;
static BOOL _bWheelUp = FALSE;
static BOOL _bWheelDn = FALSE;

CTCriticalSection csInput;

// which keys are pressed, as recorded by message interception (by KIDs)
static UBYTE _abKeysPressed[256];
/*
// set a key according to a keydown/keyup message
static void SetKeyFromMsg(MSG *pMsg, BOOL bDown)
{
  INDEX iKID = -1;
  // if capturing scan codes
  if (inp_iKeyboardReadingMethod==2) {
    // get scan code
    INDEX iScan = (pMsg->lParam>>16)&0x1FF; // (we use the extended bit too!)
    // convert scan code to kid
    iKID = _aiScanToKid[iScan];
  // if capturing virtual key codes
  } else if (inp_iKeyboardReadingMethod==1) {
    // get virtualkey
    INDEX iVirt = (pMsg->wParam)&0xFF;

    if (iVirt == VK_SHIFT) {
      iVirt = VK_LSHIFT;
    }
    if (iVirt == VK_CONTROL) {
      iVirt = VK_LCONTROL;
    }
    if (iVirt == VK_MENU) {
      iVirt = VK_LMENU;
    }
    // convert virtualkey to kid
    iKID = _aiVirtToKid[iVirt];
  // if not capturing
  }

  if (iKID>=0 && iKID<ARRAYCOUNT(_abKeysPressed)) {
//    CPrintF("%s: %d\n", _pInput->inp_strButtonNames[iKID], bDown);
    _abKeysPressed[iKID] = bDown;
  }
}

static void CheckMessage(MSG *pMsg)
{
  if ( pMsg->message == WM_LBUTTONUP) {
    _abKeysPressed[KID_MOUSE1] = FALSE;
  } else if ( pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK) {
    _abKeysPressed[KID_MOUSE1] = TRUE;
  } else if ( pMsg->message == WM_RBUTTONUP) {
    _abKeysPressed[KID_MOUSE2] = FALSE;
  } else if ( pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_RBUTTONDBLCLK) {
    _abKeysPressed[KID_MOUSE2] = TRUE;
  } else if ( pMsg->message == WM_MBUTTONUP) {
    _abKeysPressed[KID_MOUSE3] = FALSE;
  } else if ( pMsg->message == WM_MBUTTONDOWN || pMsg->message == WM_MBUTTONDBLCLK) {
    _abKeysPressed[KID_MOUSE3] = TRUE;

  } else if ( pMsg->message == inp_iMButton4Dn) {
    _abKeysPressed[KID_MOUSE4] = TRUE;
  } else if ( pMsg->message == inp_iMButton4Up) {
    _abKeysPressed[KID_MOUSE4] = FALSE;

  } else if ( pMsg->message == inp_iMButton5Dn) {
    _abKeysPressed[KID_MOUSE5] = TRUE;
  } else if ( pMsg->message == inp_iMButton5Up) {
    _abKeysPressed[KID_MOUSE5] = FALSE;

  } else if (pMsg->message==WM_KEYUP || pMsg->message==WM_SYSKEYUP) {
    SetKeyFromMsg(pMsg, FALSE);
  } else if (pMsg->message==WM_KEYDOWN || pMsg->message==WM_SYSKEYDOWN) {
    SetKeyFromMsg(pMsg, TRUE);
  }
}
*/

// pointer to global input object
CInput *_pInput = NULL;

// deafult constructor
CInput::CInput(void)
{
  // disable control scaning
  inp_bInputEnabled = FALSE;
  inp_bPollJoysticks = FALSE;
  inp_bLastPrescan = FALSE;
  // clear key buffer
  for( INDEX iButton=0; iButton<MAX_OVERALL_BUTTONS; iButton++)
  {
    inp_ubButtonsBuffer[ iButton] = 0;
  }
  // clear axis relative and absolute values
  for( INDEX iAxis=0; iAxis<MAX_OVERALL_AXES; iAxis++)
  {
    inp_caiAllAxisInfo[ iAxis].cai_fReading  = 0.0f;
    inp_caiAllAxisInfo[ iAxis].cai_bExisting = FALSE;
  }

  MakeConversionTables();
}


// destructor
CInput::~CInput()
{
}

/*
void CInput::SetJoyPolling(BOOL bPoll)
{
  inp_bPollJoysticks = bPoll;
}
*/
/*
 * Sets names of keys on keyboard
 */
void CInput::SetKeyNames( void)
{
  // set name "None" for all keys, known keys will override this default name
  {for( INDEX iKey=0; iKey<ARRAYCOUNT(inp_strButtonNames); iKey++) {
    inp_strButtonNames[iKey] = "None";
    inp_strButtonNamesTra[iKey] = TRANS("None");
  }}

  // for each Key
  {for (INDEX iKey=0; iKey<ARRAYCOUNT(_akcKeys); iKey++) {
    struct KeyConversion &kc = _akcKeys[iKey];
    // set the name
    if (kc.kc_strName!=NULL) {
      inp_strButtonNames[kc.kc_iKID] = kc.kc_strName;
      if (strlen(kc.kc_strNameTrans)==0) {
        inp_strButtonNamesTra[kc.kc_iKID] = kc.kc_strName;
      } else {
        inp_strButtonNamesTra[kc.kc_iKID] = TranslateConst(kc.kc_strNameTrans, 4);
      }
    }
  }}
/*
  // -------- Enumerate known axis -------------
  // no axis as axis type 0
  inp_caiAllAxisInfo[0].cai_strAxisName = "None";
  inp_astrAxisTran[  0] = TRANS("None");
  // mouse axis occupy types from 1 up to 3
  inp_caiAllAxisInfo[1].cai_strAxisName = "mouse X";
  inp_astrAxisTran[  1] = TRANS("mouse X");
  inp_caiAllAxisInfo[2].cai_strAxisName = "mouse Y";
  inp_astrAxisTran[  2] = TRANS("mouse Y");
  inp_caiAllAxisInfo[3].cai_strAxisName = "mouse Z";
  inp_astrAxisTran[  3] = TRANS("mouse Z");
  inp_caiAllAxisInfo[4].cai_strAxisName = "2nd mouse X";
  inp_astrAxisTran[  4] = TRANS("2nd mouse X");
  inp_caiAllAxisInfo[5].cai_strAxisName = "2nd mouse Y";
  inp_astrAxisTran[  5] = TRANS("2nd mouse Y");

  // -------- Get number of joysticks ----------
  // get number of joystics
  INDEX ctJoysticksPresent = joyGetNumDevs();
  CPrintF(TRANS("  joysticks found: %d\n"), ctJoysticksPresent);
  ctJoysticksPresent = Min(ctJoysticksPresent, inp_ctJoysticksAllowed);
  CPrintF(TRANS("  joysticks allowed: %d\n"), ctJoysticksPresent);

  // -------- Enumerate axis and buttons for joysticks ----------
  for (INDEX iJoy=0; iJoy<MAX_JOYSTICKS; iJoy++) {
    inp_abJoystickOn[iJoy] = FALSE;
    if (iJoy<ctJoysticksPresent && CheckJoystick(iJoy)) {
      inp_abJoystickOn[iJoy] = TRUE;
    }
    AddJoystickAbbilities(iJoy);
  }
*/
}
/*
// check if a joystick exists
BOOL CInput::CheckJoystick(INDEX iJoy)
{
  CPrintF(TRANS("  joy %d:"), iJoy+1);

  JOYCAPS jc;
  // seek for capabilities of requested joystick
  MMRESULT mmResult = joyGetDevCaps( JOYSTICKID1+iJoy,	&jc, sizeof(JOYCAPS));
  // report possible errors
  if( mmResult == MMSYSERR_NODRIVER) {
    CPrintF(TRANS(" joystick driver is not present\n"));
    return FALSE;
  } else if( mmResult == MMSYSERR_INVALPARAM) {
    CPrintF(TRANS(" invalid parameter\n"));
    return FALSE;
  } else if( mmResult != JOYERR_NOERROR) {
    CPrintF(TRANS("  error 0x%08x\n"), mmResult);
    return FALSE;
  }
  CPrintF(" '%s'\n", jc.szPname);

  CPrintF(TRANS("    %d axes\n"), jc.wNumAxes);
  CPrintF(TRANS("    %d buttons\n"), jc.wNumButtons);
  if (jc.wCaps&JOYCAPS_HASPOV) {
    CPrintF(TRANS("    POV hat present\n"));
    inp_abJoystickHasPOV[iJoy] = TRUE;
  } else {
    inp_abJoystickHasPOV[iJoy] = FALSE;
  }

  // read joystick state
  JOYINFOEX ji;
  ji.dwFlags = JOY_RETURNBUTTONS|JOY_RETURNCENTERED|JOY_RETURNPOV|JOY_RETURNR|
    JOY_RETURNX|JOY_RETURNY|JOY_RETURNZ|JOY_RETURNU|JOY_RETURNV;
  ji.dwSize = sizeof( JOYINFOEX);
  mmResult = joyGetPosEx( JOYSTICKID1+iJoy, &ji);

  // if some error
  if( mmResult != JOYERR_NOERROR) {
    // fail
    CPrintF(TRANS("    Cannot read the joystick!\n"));
    return FALSE;
  }

  // for each axis
  for(INDEX iAxis=0; iAxis<MAX_AXES_PER_JOYSTICK; iAxis++) {
    ControlAxisInfo &cai= inp_caiAllAxisInfo[ FIRST_JOYAXIS+iJoy*MAX_AXES_PER_JOYSTICK+iAxis];
    // remember min/max info
    switch( iAxis) {
    case 0: 
      cai.cai_slMin = jc.wXmin; cai.cai_slMax = jc.wXmax; 
      cai.cai_bExisting = TRUE;
      break;
    case 1: 
      cai.cai_slMin = jc.wYmin; cai.cai_slMax = jc.wYmax; 
      cai.cai_bExisting = TRUE;
      break;
    case 2: 
      cai.cai_slMin = jc.wZmin; cai.cai_slMax = jc.wZmax; 
      cai.cai_bExisting = jc.wCaps&JOYCAPS_HASZ;
      break;
    case 3: 
      cai.cai_slMin = jc.wRmin; cai.cai_slMax = jc.wRmax; 
      cai.cai_bExisting = jc.wCaps&JOYCAPS_HASR;
      break;
    case 4: 
      cai.cai_slMin = jc.wUmin; cai.cai_slMax = jc.wUmax; 
      cai.cai_bExisting = jc.wCaps&JOYCAPS_HASU;
      break;
    case 5: 
      cai.cai_slMin = jc.wVmin; cai.cai_slMax = jc.wVmax; 
      cai.cai_bExisting = jc.wCaps&JOYCAPS_HASV;
      break;
    }
  }

  return TRUE;
}

// adds axis and buttons for given joystick
void CInput::AddJoystickAbbilities(INDEX iJoy)
{
  CTString strJoystickName;
  strJoystickName.PrintF("Joy %d", iJoy+1);
  CTString strJoystickNameTra;
  strJoystickNameTra.PrintF(TRANS("Joy %d"), iJoy+1);

  // for each axis
  for( UINT iAxis=0; iAxis<6; iAxis++) {
    INDEX iAxisTotal = FIRST_JOYAXIS+iJoy*MAX_AXES_PER_JOYSTICK+iAxis;
    ControlAxisInfo &cai= inp_caiAllAxisInfo[iAxisTotal];
    CTString &strTran = inp_astrAxisTran[iAxisTotal];
    // set axis name
    switch( iAxis) {
    case 0: cai.cai_strAxisName = strJoystickName + " Axis X"; strTran = strJoystickNameTra + TRANS(" Axis X"); break;
    case 1: cai.cai_strAxisName = strJoystickName + " Axis Y"; strTran = strJoystickNameTra + TRANS(" Axis Y"); break;
    case 2: cai.cai_strAxisName = strJoystickName + " Axis Z"; strTran = strJoystickNameTra + TRANS(" Axis Z"); break;
    case 3: cai.cai_strAxisName = strJoystickName + " Axis R"; strTran = strJoystickNameTra + TRANS(" Axis R"); break;
    case 4: cai.cai_strAxisName = strJoystickName + " Axis U"; strTran = strJoystickNameTra + TRANS(" Axis U"); break;
    case 5: cai.cai_strAxisName = strJoystickName + " Axis V"; strTran = strJoystickNameTra + TRANS(" Axis V"); break;
    }
  }

  INDEX iButtonTotal = FIRST_JOYBUTTON+iJoy*MAX_BUTTONS_PER_JOYSTICK;
  // add buttons that the joystick supports
  for( UINT iButton=0; iButton<32; iButton++) {
    CTString strButtonName;
    CTString strButtonNameTra;
    // create name for n-th button
    strButtonName.PrintF( " Button %d", iButton);
    strButtonNameTra.PrintF( TRANS(" Button %d"), iButton);
    // set n-th button name
    inp_strButtonNames[iButtonTotal]    = strJoystickName + strButtonName;
    inp_strButtonNamesTra[iButtonTotal] = strJoystickNameTra + strButtonNameTra;
    iButtonTotal++;
  }

  // add the four POV buttons
  inp_strButtonNames   [ iButtonTotal  ] = strJoystickName    +      (" POV N");
  inp_strButtonNamesTra[ iButtonTotal++] = strJoystickNameTra + TRANS(" POV N");
  inp_strButtonNames   [ iButtonTotal  ] = strJoystickName    +      (" POV E");
  inp_strButtonNamesTra[ iButtonTotal++] = strJoystickNameTra + TRANS(" POV E");
  inp_strButtonNames   [ iButtonTotal  ] = strJoystickName    +      (" POV S");
  inp_strButtonNamesTra[ iButtonTotal++] = strJoystickNameTra + TRANS(" POV S");
  inp_strButtonNames   [ iButtonTotal  ] = strJoystickName    +      (" POV W");
  inp_strButtonNamesTra[ iButtonTotal++] = strJoystickNameTra + TRANS(" POV W");
}
*/
/*
 * Initializes all available devices and enumerates available controls
 */
void CInput::Initialize( void )
{
  CPrintF(TRANS("Detecting input devices...\n"));
  SetKeyNames();
  CPrintF("\n");
}


/*
 * Enable direct input
 */
void CInput::EnableInput(CViewPort* pvp)
{
  // skip if already enabled
  if( inp_bInputEnabled) return;
/*
  // get window rectangle
  RECT rectClient;
  GetClientRect(hwnd, &rectClient);
  POINT pt;
  pt.x=0;
  pt.y=0;
  ClientToScreen(hwnd, &pt);
  OffsetRect(&rectClient, pt.x, pt.y);

  // remember mouse pos
  GetCursorPos( &inp_ptOldMousePos);
  // set mouse clip region
  ClipCursor(&rectClient);
  // determine screen center position
  inp_slScreenCenterX = (rectClient.left + rectClient.right) / 2;
  inp_slScreenCenterY = (rectClient.top + rectClient.bottom) / 2;

  // clear mouse from screen
  while (ShowCursor(FALSE) >= 0);
  // save system mouse settings
  SystemParametersInfo(SPI_GETMOUSE, 0, &inp_mscMouseSettings, 0);
  // set new mouse speed
  if (!inp_bAllowMouseAcceleration) {
    MouseSpeedControl mscNewSetting = { 0, 0, 0};
    SystemParametersInfo(SPI_SETMOUSE, 0, &mscNewSetting, 0);
  }
  // set cursor position to screen center
  SetCursorPos(inp_slScreenCenterX, inp_slScreenCenterY);

  _hGetMsgHook  = SetWindowsHookEx(WH_GETMESSAGE, &GetMsgProc, NULL, GetCurrentThreadId());
  _hSendMsgHook = SetWindowsHookEx(WH_CALLWNDPROC, &SendMsgProc, NULL, GetCurrentThreadId());

  // clear button's buffer
  memset( _abKeysPressed, 0, sizeof( _abKeysPressed));
  */
  // remember current status
  inp_bInputEnabled = TRUE;
  inp_bPollJoysticks = FALSE;
}


/*
 * Disable direct input
 */
void CInput::DisableInput( void)
{
  // skip if allready disabled
  if( !inp_bInputEnabled) return;
  /*
  UnhookWindowsHookEx(_hGetMsgHook);
  UnhookWindowsHookEx(_hSendMsgHook);

  // set mouse clip region to entire screen
  ClipCursor(NULL);
  // restore mouse pos
  SetCursorPos( inp_ptOldMousePos.x, inp_ptOldMousePos.y);

  // show mouse on screen
  while (ShowCursor(TRUE) < 0);
  // set system mouse settings
  SystemParametersInfo(SPI_SETMOUSE, 0, &inp_mscMouseSettings, 0);
*/
  // remember current status
  inp_bInputEnabled = FALSE;
  inp_bPollJoysticks = FALSE;
}


/*
 * Scan states of all available input sources
 */
void CInput::GetInput(BOOL bPreScan)
{
//  CTSingleLock sl(&csInput, TRUE);

  if (!inp_bInputEnabled) {
    return;
  }

  if (bPreScan && !inp_bAllowPrescan) {
    return;
  }
/*
  // if not pre-scanning
  if (!bPreScan) {
    // clear button's buffer
    memset( inp_ubButtonsBuffer, 0, sizeof( inp_ubButtonsBuffer));

    // for each Key
    {for (INDEX iKey=0; iKey<ARRAYCOUNT(_akcKeys); iKey++) {
      struct KeyConversion &kc = _akcKeys[iKey];
      // get codes
      INDEX iKID  = kc.kc_iKID;
      INDEX iScan = kc.kc_iScanCode;
      INDEX iVirt = kc.kc_iVirtKey;

      // if snooped that key is pressed
      if (_abKeysPressed[iKID]) {
        // mark it as pressed
        inp_ubButtonsBuffer[iKID] = 0xFF;
      }
    }}
  }

  // read mouse position
  POINT pntMouse;
  if( GetCursorPos( &pntMouse))
  {
    FLOAT fDX = FLOAT( SLONG(pntMouse.x) - inp_slScreenCenterX);
    FLOAT fDY = FLOAT( SLONG(pntMouse.y) - inp_slScreenCenterY);

    FLOAT fSensitivity = inp_fMouseSensitivity;
    if( inp_bAllowMouseAcceleration) fSensitivity *= 0.25f;

    FLOAT fD = Sqrt(fDX*fDX+fDY*fDY);
    if (inp_bMousePrecision) {
      static FLOAT _tmTime = 0.0f;
      if( fD<inp_fMousePrecisionThreshold) _tmTime += 0.05f;
      else _tmTime = 0.0f;
      if( _tmTime>inp_fMousePrecisionTimeout) fSensitivity /= inp_fMousePrecisionFactor;
    }

    static FLOAT fDXOld;
    static FLOAT fDYOld;
    static TIME tmOldDelta;
    static CTimerValue tvBefore;
    CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();
    TIME tmNowDelta = (tvNow-tvBefore).GetSeconds();
    if (tmNowDelta<0.001f) {
      tmNowDelta = 0.001f;
    }
    tvBefore = tvNow;

    FLOAT fDXSmooth = (fDXOld*tmOldDelta+fDX*tmNowDelta)/(tmOldDelta+tmNowDelta);
    FLOAT fDYSmooth = (fDYOld*tmOldDelta+fDY*tmNowDelta)/(tmOldDelta+tmNowDelta);
    fDXOld = fDX;
    fDYOld = fDY;
    tmOldDelta = tmNowDelta;
    if (inp_bFilterMouse) {
      fDX = fDXSmooth;
      fDY = fDYSmooth;
    }

    // get final mouse values
    FLOAT fMouseRelX = +fDX*fSensitivity;
    FLOAT fMouseRelY = -fDY*fSensitivity;
    if (inp_bInvertMouse) {
      fMouseRelY = -fMouseRelY;
    }
    FLOAT fMouseRelZ = _iMouseZ;

    // just interpret values as normal
    inp_caiAllAxisInfo[1].cai_fReading = fMouseRelX;
    inp_caiAllAxisInfo[2].cai_fReading = fMouseRelY;
    inp_caiAllAxisInfo[3].cai_fReading = fMouseRelZ;

    // if not pre-scanning
    if (!bPreScan) {
      // detect wheel up/down movement
      _bWheelDn = FALSE;
      if (_iMouseZ>0) {
        if (_bWheelUp) {
          inp_ubButtonsBuffer[KID_MOUSEWHEELUP] = 0x00;
        } else {
          inp_ubButtonsBuffer[KID_MOUSEWHEELUP] = 0xFF;
          _iMouseZ = ClampDn(_iMouseZ-120, 0);
        }
      }
      _bWheelUp = inp_ubButtonsBuffer[KID_MOUSEWHEELUP];
      if (_iMouseZ<0) {
        if (_bWheelDn) {
          inp_ubButtonsBuffer[KID_MOUSEWHEELDOWN] = 0x00;
        } else {
          inp_ubButtonsBuffer[KID_MOUSEWHEELDOWN] = 0xFF;
          _iMouseZ = ClampUp(_iMouseZ+120, 0);
        }
      }
      _bWheelDn = inp_ubButtonsBuffer[KID_MOUSEWHEELDOWN];
    }
  }
  inp_bLastPrescan = bPreScan;

  // set cursor position to screen center
  if (pntMouse.x!=inp_slScreenCenterX || pntMouse.y!=inp_slScreenCenterY) {
    SetCursorPos(inp_slScreenCenterX, inp_slScreenCenterY);
  }

  // if joystick polling is enabled
  if (inp_bPollJoysticks || inp_bForceJoystickPolling) {
    // scan all available joysticks
    for( INDEX iJoy=0; iJoy<MAX_JOYSTICKS; iJoy++) {
      if (inp_abJoystickOn[iJoy] && iJoy<inp_ctJoysticksAllowed) {
        // scan joy state
        BOOL bSucceeded = ScanJoystick(iJoy, bPreScan);
        // if joystick reading failed
        if (!bSucceeded && inp_bAutoDisableJoysticks) {
          // kill it, so it doesn't slow down CPU
          CPrintF(TRANS("Joystick %d failed, disabling it!\n"), iJoy+1);
          inp_abJoystickOn[iJoy] = FALSE;
        }
      }
    }
  }
  */
}

// Clear all input states (keys become not pressed, axes are reset to zero)
void CInput::ClearInput( void)
{
  // clear button's buffer
  memset( inp_ubButtonsBuffer, 0, sizeof( inp_ubButtonsBuffer));
  // clear axis values
  for (INDEX i=0; i<MAX_OVERALL_AXES; i++) {
    inp_caiAllAxisInfo[i].cai_fReading = 0;
  }
}

const CTString &CInput::GetAxisTransName( INDEX iAxisNo) const
{
  return inp_astrAxisTran[iAxisNo];
}


/*
 * Scans axis and buttons for given joystick
 */
/*
BOOL CInput::ScanJoystick(INDEX iJoy, BOOL bPreScan)
{
  // read joystick state
  JOYINFOEX ji;
  ji.dwFlags = JOY_RETURNBUTTONS|JOY_RETURNCENTERED|JOY_RETURNPOV|JOY_RETURNR|
    JOY_RETURNX|JOY_RETURNY|JOY_RETURNZ|JOY_RETURNU|JOY_RETURNV;
  ji.dwSize = sizeof( JOYINFOEX);
  MMRESULT mmResult = joyGetPosEx( JOYSTICKID1+iJoy, &ji);

  // if some error
  if( mmResult != JOYERR_NOERROR) {
    // fail
    return FALSE;
  }

  // for each available axis
  for( INDEX iAxis=0; iAxis<MAX_AXES_PER_JOYSTICK; iAxis++) {
    ControlAxisInfo &cai = inp_caiAllAxisInfo[ FIRST_JOYAXIS+iJoy*MAX_AXES_PER_JOYSTICK+iAxis];
    // if the axis is not present
    if (!cai.cai_bExisting) {
      // read as zero
      cai.cai_fReading = 0.0f;
      // skip to next axis
      continue;
    }
    // read its state
    SLONG slAxisReading;
    switch( iAxis) {
    case 0: slAxisReading = ji.dwXpos; break;
    case 1: slAxisReading = ji.dwYpos; break;
    case 2: slAxisReading = ji.dwZpos; break;
    case 3: slAxisReading = ji.dwRpos; break;
    case 4: slAxisReading = ji.dwUpos; break;
    case 5: slAxisReading = ji.dwVpos; break;
    }
    // convert from min..max to -1..+1
    FLOAT fAxisReading = FLOAT(slAxisReading-cai.cai_slMin)/(cai.cai_slMax-cai.cai_slMin)*2.0f-1.0f;

    // set current axis value
    cai.cai_fReading = fAxisReading;
  }

  // if not pre-scanning
  if (!bPreScan) {
    INDEX iButtonTotal = FIRST_JOYBUTTON+iJoy*MAX_BUTTONS_PER_JOYSTICK;
    // for each available button
    for( INDEX iButton=0; iButton<32; iButton++) {
      // test if the button is pressed
      if(ji.dwButtons & (1L<<iButton)) {
        inp_ubButtonsBuffer[ iButtonTotal++] = 128;
      } else {
        inp_ubButtonsBuffer[ iButtonTotal++] = 0;
      }
    }

    // POV hat initially not pressed
  //  CPrintF("%d\n", ji.dwPOV);
    INDEX iStartPOV = iButtonTotal;
    inp_ubButtonsBuffer[ iStartPOV+0] = 0;
    inp_ubButtonsBuffer[ iStartPOV+1] = 0;
    inp_ubButtonsBuffer[ iStartPOV+2] = 0;
    inp_ubButtonsBuffer[ iStartPOV+3] = 0;

    // if we have POV
    if (inp_abJoystickHasPOV[iJoy]) {
      // check the four pov directions
      if (ji.dwPOV==JOY_POVFORWARD) {
        inp_ubButtonsBuffer[ iStartPOV+0] = 128;
      } else if (ji.dwPOV==JOY_POVRIGHT) {
        inp_ubButtonsBuffer[ iStartPOV+1] = 128;
      } else if (ji.dwPOV==JOY_POVBACKWARD) {
        inp_ubButtonsBuffer[ iStartPOV+2] = 128;
      } else if (ji.dwPOV==JOY_POVLEFT) {
        inp_ubButtonsBuffer[ iStartPOV+3] = 128;
      // and four mid-positions
      } else if (ji.dwPOV==JOY_POVFORWARD+4500) {
        inp_ubButtonsBuffer[ iStartPOV+0] = 128;
        inp_ubButtonsBuffer[ iStartPOV+1] = 128;
      } else if (ji.dwPOV==JOY_POVRIGHT+4500) {
        inp_ubButtonsBuffer[ iStartPOV+1] = 128;
        inp_ubButtonsBuffer[ iStartPOV+2] = 128;
      } else if (ji.dwPOV==JOY_POVBACKWARD+4500) {
        inp_ubButtonsBuffer[ iStartPOV+2] = 128;
        inp_ubButtonsBuffer[ iStartPOV+3] = 128;
      } else if (ji.dwPOV==JOY_POVLEFT+4500) {
        inp_ubButtonsBuffer[ iStartPOV+3] = 128;
        inp_ubButtonsBuffer[ iStartPOV+0] = 128;
      }
    }
  }
  // successful
  return TRUE;
}
*/
