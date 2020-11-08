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



#include <Engine/Graphics/Adapter.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Translation.h>
#include <Engine/Base/Console.h>


extern BOOL _bDedicatedServer;

// list of all modes avaliable through CDS
static CListHead _lhCDSModes;

class CResolution {
public:
  PIX   re_pixSizeI;
  PIX   re_pixSizeJ;
};

static CResolution _areResolutions[] =
{
  {  320,  240 },
  {  400,  300 },
  {  480,  360 },
  {  512,  384 },
  {  640,  480 },
  {  720,  540 },
  {  720,  576 },
  {  800,  600 },
  {  960,  720 },
  { 1024,  768 },
  { 1152,  864 },
  { 1280,  960 },
  { 1280, 1024 },
  { 1600, 1200 },
  { 1792, 1344 },
  { 1856, 1392 },
  { 1920, 1440 },
  { 2048, 1536 },

};
// THIS NUMBER MUST NOT BE OVER 25! (otherwise change it in adapter.h)
static const INDEX MAX_RESOLUTIONS = sizeof(_areResolutions)/sizeof(_areResolutions[0]);



// initialize CDS support (enumerate modes at startup)
void CGfxLibrary::InitAPIs(void)
{
  // no need for gfx when dedicated server is on
  if( _bDedicatedServer) return;

  CDisplayAdapter *pda;
  INDEX iResolution;

  // fill OpenGL adapter info
  gl_gaAPI[GAT_OGL].ga_ctAdapters = 1;
  gl_gaAPI[GAT_OGL].ga_iCurrentAdapter = 0;
  pda = &gl_gaAPI[GAT_OGL].ga_adaAdapter[0];
  pda->da_ulFlags = DAF_USEGDIFUNCTIONS;
  pda->da_strVendor   = TRANS( "unknown");
  pda->da_strRenderer = TRANS( "Default ICD");
  pda->da_strVersion  = "1.1+";

  // detect modes for OpenGL ICD
  pda->da_ctDisplayModes = 0;
  pda->da_iCurrentDisplayMode = -1;

  // enumerate modes thru resolution list
  for( iResolution=0; iResolution<MAX_RESOLUTIONS; iResolution++)
  {
    CResolution &re = _areResolutions[iResolution];

    // make a new display mode
    CDisplayMode &dm = pda->da_admDisplayModes[pda->da_ctDisplayModes];
    dm.dm_pixSizeI = re.re_pixSizeI;
    dm.dm_pixSizeJ = re.re_pixSizeJ;
    dm.dm_ddDepth  = DD_DEFAULT;
    pda->da_ctDisplayModes++;
  }
}

// get list of all modes avaliable through CDS -- do not modify/free the returned list
CListHead &CDS_GetModes(void)
{
  return _lhCDSModes;
}


// set given display mode
BOOL CDS_SetMode( PIX pixSizeI, PIX pixSizeJ)
{
  // no need for gfx when dedicated server is on
  if( _bDedicatedServer) return FALSE;
  // report
  CPrintF(TRANS("  CDS: mode set to %dx%d\n"), pixSizeI, pixSizeJ);
  return TRUE;
}


// reset windows to mode chosen by user within windows diplay properties
void CDS_ResetMode(void)
{
  // no need for gfx when dedicated server is on
  if( _bDedicatedServer) return;

  CPrintF(TRANS("  CDS: mode reset to original desktop settings\n"));
}
