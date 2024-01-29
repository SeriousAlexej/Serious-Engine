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

#include "StdH.h"
#include "ImportedMesh.h"
#include <SeriousEngineCppAPI/Templates/Stock_CTextureData.h>
#include <SeriousEngineCAPI/Base/Memory.h>

#include <QImageReader>
#include <QImage>

#include <algorithm>
#include <set>

// global engine gui handling object
CEngineGUI _EngineGUI;

/*
 We cannot use dllmain if using MFC.
 See MSDN article "Regular DLLs Dynamically Linked to MFC" if initialization is needed.

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
*/

namespace
{
void AddFilter(std::vector<char>& result, const std::string& descr, const std::string& formatList)
{
  for (char c : descr)
    result.push_back(c);
  result.push_back('\0');
  for (char c : formatList)
    result.push_back(c);
  result.push_back('\0');
}
} // anonymous namespace

void CEngineGUI::CreateTexture_t(const CTFileName& inFileName, MEX inMex, INDEX inMipmaps, BOOL bForce32bit)
{
  CTFileName outFileName = inFileName.FileDir() + inFileName.FileName() + ".TEX";
  this->CreateTexture_t(inFileName, outFileName, inMex, inMipmaps, bForce32bit);
}

void CEngineGUI::CreateTexture_t(const CTFileName& inFileName, const CTFileName& outFileName, MEX inMex, INDEX inMipmaps, BOOL bForce32bit)
{
  if (inFileName.FileExt() == ".SCR")
  {
    // input is a script file
    ProcessScript_t(inFileName);
  }
  else
  {
    // input is a picture file
    CTextureData tex;
    CImageInfo inPic;

    // mex must be specified and valid
    if ((inMex <= 0)) throw(TRANS("Invalid or unspecified mexel units."));

    // load picture
    this->LoadAnyGfxFormat_t(inPic, inFileName);

    // create texture
    tex.Create_t(&inPic, inMex, inMipmaps, bForce32bit);

    // no more need for picture - get out!
    inPic.Clear();

    // save texture to file
    tex.Save_t(outFileName);
  }
}

ImageFormat CEngineGUI::GetGfxFileInfo_t(CImageInfo& ii, const CTFileName& strFileName)
{
  ImageFormat format = (ImageFormat)ii.GetGfxFileInfo_t(strFileName);
  if (format == ImageFormat::Unsupported)
  {
    auto fullFileName = strFileName;
    fullFileName.RemoveApplicationPath_t();
    fullFileName = _fnmApplicationPath + _fnmMod + fullFileName;
    fullFileName.SetAbsolutePath();

    QImage im;
    if (im.load(QString(fullFileName)))
    {
      ii.ii_Width = im.width();
      ii.ii_Height = im.height();
      ii.ii_BitsPerPixel = im.hasAlphaChannel() ? 32 : 24;
      return ImageFormat::Other;
    }
  }
  return format;
}

// check for the supported gfx format file and invokes corresponding routine to load it
void CEngineGUI::LoadAnyGfxFormat_t(CImageInfo& ii, const CTFileName& strFileName) // throw char *
{
  auto iFileFormat = this->GetGfxFileInfo_t(ii, strFileName);
  if (iFileFormat == ImageFormat::Other)
  {
    auto fullFileName = strFileName;
    fullFileName.RemoveApplicationPath_t();
    fullFileName = _fnmApplicationPath + _fnmMod + fullFileName;
    fullFileName.SetAbsolutePath();

    QImage im;
    if (im.load(QString(fullFileName)))
    {
      const auto format = im.format();
      if (im.hasAlphaChannel() && format != QImage::Format_RGBA8888)
        im = im.convertToFormat(QImage::Format_RGBA8888);
      else if (format != QImage::Format_RGB888)
        im = im.convertToFormat(QImage::Format_RGB888);

      const auto bytesPerPixel = ii.ii_BitsPerPixel / 8;
      const auto byteCount = ii.ii_Width * ii.ii_Height * bytesPerPixel;
      ii.ii_Picture = (UBYTE*)AllocMemory_(byteCount);

      if (im.byteCount() == byteCount)
      {
        memcpy(ii.ii_Picture, im.constBits(), byteCount);
      }
      else
      {
        const auto lineSize = ii.ii_Width * bytesPerPixel;
        for (PIX line = 0; line < ii.ii_Height; ++line)
          memcpy(ii.ii_Picture + line * lineSize, im.constBits() + line * im.bytesPerLine(), lineSize);
      }
    }
    return;
  }

  ii.LoadAnyGfxFormat_t(strFileName);
}

std::vector<std::string> CEngineGUI::GetSupportedImportFormats()
{
  std::set<std::string> formats;
  for (const auto& format : QImageReader::supportedImageFormats())
    formats.insert('.' + QString(format).toLower().toStdString());
  formats.insert(".tga");
  formats.insert(".pcx");
  return { formats.begin(), formats.end() };
}

std::vector<std::string> CEngineGUI::GetSupportedExportFormats()
{
  return { ".tga" };
}

std::vector<char> CEngineGUI::GetListOfExportImageFormats()
{
  std::vector<char> result;

  for (const auto& format : GetSupportedExportFormats())
    AddFilter(result, "Picture file (*" + format + ')', '*' + format);

  result.push_back('\0');
  return result;
}

std::vector<char> CEngineGUI::GetListOfImportImageFormats(bool include_scr)
{
  std::vector<char> result;
  auto add_filter = [&result](const std::string& descr, const std::string& formatList)
  {
    AddFilter(result, descr, formatList);
  };

  std::string formatList;
  for (const auto& format : GetSupportedImportFormats())
  {
    if (!formatList.empty())
      formatList += ';';
    formatList += '*' + format;
  }

  if (include_scr)
    add_filter("Picture or script files (" + formatList + ";*.scr)", formatList + ";*.scr");

  add_filter("Picture files (" + formatList + ')', formatList);

  if (include_scr)
    add_filter("Script files (*.scr)", "*.scr");

  add_filter("All files (*.*)", "*.*");

  result.push_back('\0');
  return result;
}

std::vector<char> CEngineGUI::GetListOf3DFormats(bool include_scr)
{
  std::vector<char> result;
  auto add_filter = [&result](const ImportedMesh::TFormatDescr& descr)
  {
    AddFilter(result, descr.first, descr.second);
  };

  const auto& formats = ImportedMesh::GetSupportedFormats();
  ImportedMesh::TFormatDescr all_supported_formats;
  all_supported_formats.first = "All supported formats";
  for (const auto& format : formats)
  {
    if (!all_supported_formats.second.empty())
      all_supported_formats.second += ';';
    all_supported_formats.second += format.second;
  }

  if (include_scr)
  {
    if (!all_supported_formats.second.empty())
      all_supported_formats.second += ';';
    all_supported_formats.second += "*.scr";
  }

  add_filter(all_supported_formats);

  for (const auto& format : formats)
    add_filter(format);

  if (include_scr)
    add_filter({ "SeriousModeler script (*.scr)", "*.scr" });

  add_filter({ "All files (*.*)", "*.*" });

  result.push_back('\0');
  return result;
}

void CEngineGUI::SelectMode( CDisplayMode &dm, GfxAPIType &gat)
{
  // stupid way to change resources, but it must be done
  HANDLE hOldResource = AfxGetResourceHandle();

  // call select mode dialog
  CDlgSelectMode dlgSelectMode( dm, gat);

  // activate CTGfx resources
  AfxSetResourceHandle( GetModuleHandleA(ENGINEGUI_DLL_NAME));
  INDEX iDialogResult = dlgSelectMode.DoModal();

  // restore resources
  AfxSetResourceHandle( (HINSTANCE)hOldResource);

  // if mode dialog ended successfully
  if( iDialogResult == IDOK) SetFullScreenModeToRegistry( "Display modes", dm, gat);
}

CTFileName CEngineGUI::CreateTexture(CTFileName fnTexFileToRecreate/*=CTString("")*/,
                         CDynamicArray_CTFileName *pafnCreatedTextures/*=NULL*/)
{
  CTFileName fnResult;
  // stupid way to change resources, but it must be done
  HANDLE hOldResource = AfxGetResourceHandle();
  // activate CTGfx resources
  AfxSetResourceHandle( GetModuleHandleA(ENGINEGUI_DLL_NAME) );

  // if create texture is called with a wish to recreate texture
  if( fnTexFileToRecreate != "")
  {
    fnResult = fnTexFileToRecreate;
    CTextureDataPtr ptdTextureToRecreate;
    CTFileName fnToRecreateNoExt =
      fnTexFileToRecreate.FileDir()+fnTexFileToRecreate.FileName();
    // try to
    try
    {
      // obtain texture to recreate
      ptdTextureToRecreate = _pTextureStock_Obtain_t( fnTexFileToRecreate);
      ptdTextureToRecreate->Reload();
      // if texture is of effect type, call create effect texture dialog
      if( ptdTextureToRecreate->HasEffectTexture())
      {
        // call create effect texture dialog with .tex name
        CDlgCreateEffectTexture dlgCreateEffectTexture( fnTexFileToRecreate);
        dlgCreateEffectTexture.DoModal();
      }
      // else this texture was created from script or from single picture
      else
      {
        // search for script with same name
        CTString strFullNameNoExt = _fnmApplicationPath +
          fnTexFileToRecreate.FileDir() + fnTexFileToRecreate.FileName();

        const auto supportedFormats = GetSupportedImportFormats();

        bool matchingImageFound = false;
        for (const auto& ext : supportedFormats)
          if (GetFileAttributesA(strFullNameNoExt + ext.c_str()) != -1)
          {
            // call create normal texture dialog with proper picture name
            CDlgCreateNormalTexture dlgCreateNormalTexture(fnToRecreateNoExt + ext.c_str());
            if( dlgCreateNormalTexture.m_bSourcePictureValid)
              if( dlgCreateNormalTexture.DoModal() == IDOK)
                fnResult = dlgCreateNormalTexture.m_fnCreatedFileName;
            matchingImageFound = true;
            break;
          }

        // else if script exists
        if(!matchingImageFound && GetFileAttributesA( strFullNameNoExt+".scr") != -1)
        {
          CDynamicArray_CTFileName afnScript;
          CTFileNamePtr pfnScript = afnScript.New();
          *pfnScript = fnToRecreateNoExt+".scr";
          // call create animated texture dialog with script name
          CDlgCreateAnimatedTexture dlgCreateAnimatedTexture( afnScript);
          dlgCreateAnimatedTexture.DoModal();
        }
        else if (!matchingImageFound)
        {
          WarningMessage( "Cannot find source for recreating texture: \"%s\"", fnTexFileToRecreate);
        }
      }
      // reload the texture
      ptdTextureToRecreate->Reload();
      // release obtained texture
      _pTextureStock_Release(*ptdTextureToRecreate);
    }
    catch( char *err_str)
    {
      WarningMessage( err_str);
    }
  }
  else
  {
    // call choose texture type dialog
    CDlgChooseTextureType dlgChooseTextureType;
    int iDlgResult = dlgChooseTextureType.DoModal();
    // if user choosed a texture type
    if( (iDlgResult != -1) && (iDlgResult >= 0) && (iDlgResult <= 2) )
    {
      // if result is 2 we want to create effect texture
      if( iDlgResult == 2)
      {
        // no file requester, just call dialog
        CDlgCreateEffectTexture dlgCreateEffectTexture;
        if( dlgCreateEffectTexture.DoModal() == IDOK)
        {
          fnResult = dlgCreateEffectTexture.m_fnCreatedTextureName;
          if( pafnCreatedTextures != NULL)
          {
            CTFileNamePtr pfnCreatedTexture = pafnCreatedTextures->New();
            *pfnCreatedTexture = fnResult;
          }
        }
      }
      // both normal and animated textures need to call open file requester
      else
      {
        // different filters for different requests
        const auto filters = GetListOfImportImageFormats(iDlgResult == 1);

        // call file requester for opening textures
        CDynamicArray_CTFileName afnCreateTexture;
        FileRequester( "Create texture", filters.data(), KEY_NAME_CREATE_TEXTURE_DIR,
                       "Textures\\", "", &afnCreateTexture);
        if( afnCreateTexture.Count() == 0)
        {
          // restore resources
          AfxSetResourceHandle( (HINSTANCE) hOldResource);
          return CTString("");
        }

        // if requested texture type is 0 we want to create normal texture
        if( iDlgResult == 0)
        {
          const auto supportedFormats = GetSupportedImportFormats();
          auto is_ext_supported = [&supportedFormats](const std::string& ext)
          { return std::find(supportedFormats.begin(), supportedFormats.end(), ext) != supportedFormats.end(); };
          // create textures
          FOREACHINDYNAMICARRAY( afnCreateTexture, CTFileName, itPicture)
          {
            CTFileNamePtr fnSource = itPicture.Current();
            std::string ext = static_cast<const char*>(fnSource->FileExt());
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
            if (is_ext_supported(ext))
            {
              // call create normal texture dialog
              CDlgCreateNormalTexture dlgCreateNormalTexture( *fnSource);
              if( dlgCreateNormalTexture.m_bSourcePictureValid)
              {
                if( dlgCreateNormalTexture.DoModal() == IDOK)
                {
                  fnResult = dlgCreateNormalTexture.m_fnCreatedFileName;
                  if( pafnCreatedTextures != NULL)
                  {
                    CTFileNamePtr pfnCreatedTexture = pafnCreatedTextures->New();
                    *pfnCreatedTexture = fnResult;
                  }
                }
              }
            }
          }
        }
        // if requested texture type is 1 we want to create animated texture
        else if( iDlgResult == 1)
        {
          // call create animated texture dialog
          CDlgCreateAnimatedTexture dlgCreateAnimatedTexture( afnCreateTexture);
          INDEX iDlgResult = dlgCreateAnimatedTexture.DoModal();
          if( iDlgResult == IDOK)
          {
            fnResult = dlgCreateAnimatedTexture.m_fnCreatedFileName;
            if( pafnCreatedTextures != NULL)
            {
              CTFileNamePtr pfnCreatedTexture = pafnCreatedTextures->New();
              *pfnCreatedTexture = fnResult;
            }
          }
        }
      }
    }
  }
  // restore resources
  AfxSetResourceHandle( (HINSTANCE) hOldResource);
  return fnResult;
}

/* Functions used by application for getting and setting registry keys concerning modes */
void CEngineGUI::GetFullScreenModeFromRegistry( CTString strSectionName, CDisplayMode &dm, GfxAPIType &gat)
{
  // prepare full screen mode as default
  dm.dm_pixSizeI = 640;
  dm.dm_pixSizeJ = 480;
  dm.dm_ddDepth  = DD_DEFAULT;
  // read FS parameters from registry
  CTString strResult = static_cast<const char*>(CStringA(AfxGetApp()->GetProfileString( CString(strSectionName), L"Full screen mode", L"640 x 480 x 0")));
  strResult.ScanF( "%d x %d x %d", &dm.dm_pixSizeI, &dm.dm_pixSizeJ, &dm.dm_ddDepth);
  if( dm.dm_ddDepth<DD_DEFAULT || dm.dm_ddDepth>DD_32BIT) dm.dm_ddDepth = DD_DEFAULT;
  strResult = static_cast<const char*>(CStringA(AfxGetApp()->GetProfileString( CString(strSectionName), L"Full screen API", L"OpenGL")));
  gat = (strResult=="Direct3D") ? GAT_D3D : GAT_OGL;
}

void CEngineGUI::SetFullScreenModeToRegistry( CTString strSectionName, const CDisplayMode& dm, GfxAPIType gat)
{
  CTString strDM( 0, "%d x %d x %d", dm.dm_pixSizeI, dm.dm_pixSizeJ, dm.dm_ddDepth);
  CTString strGAT = (gat==GAT_D3D) ? "Direct3D" : "OpenGL";
  AfxGetApp()->WriteProfileString(CString(strSectionName), L"Full screen mode", CString(strDM));
  AfxGetApp()->WriteProfileString(CString(strSectionName), L"Full screen API", CString(strGAT));
}
