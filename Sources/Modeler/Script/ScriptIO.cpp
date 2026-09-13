/* Copyright (c) 2022 SeriousAlexej (Oleksii Sierov).
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

#include "StdAfx.h"
#include "ScriptIO.h"

#include <fstream>
#include <sstream>

namespace {
double _stod(const std::string& str)
{
  try
  {
    return std::stod(str);
  }
  catch (const std::logic_error& e)
  {
    ThrowF_t("Failed to read number '%s': %s", str.c_str(), e.what());
  }
}

float _stof(const std::string& str)
{
  try
  {
    return std::stof(str);
  }
  catch (const std::logic_error& e)
  {
    ThrowF_t("Failed to read number '%s': %s", str.c_str(), e.what());
  }
}

int _stoi(const std::string& str)
{
  try
  {
    return std::stoi(str);
  }
  catch (const std::logic_error& e)
  {
    ThrowF_t("Failed to read number '%s': %s", str.c_str(), e.what());
  }
}

std::string Trim(std::string str)
{
  size_t i = 0;
  for (; i < str.length(); ++i)
    if (!std::isspace(static_cast<unsigned char>(str.at(i))))
      break;
  return { str.begin() + i, str.end() };
}

bool StartsWith(const std::string_view& str, const std::string_view& what)
{
  if (str.length() < what.length())
    return false;

  for (size_t i = 0; i < what.length(); ++i)
    if (std::tolower(static_cast<unsigned char>(str.at(i))) !=
        std::tolower(static_cast<unsigned char>(what.at(i))))
      return false;

  return true;
}

bool StartsWith(const std::string_view& str, unsigned char c)
{
  if (str.empty())
    return false;

  return std::tolower(static_cast<unsigned char>(str.at(0))) == std::tolower(c);
}
} // anonymous namespace

namespace ScriptIO {

ModelScript ReadFromFile(const CTFileName& filename)
{
  const CTString strFile = _fnmApplicationPath + filename;
  std::ifstream file;
  file.open(strFile, std::ios_base::in);
  if (!file.is_open())
    throw "Failed to open script!";

  enum class ReadState
  {
    Generic,
    Mips,
    Anims,
    Speed,
    Frames,
    SkeletalAnim,
    IgnoreLines
  };

  const char* keywords_to_ignore[] = {
    "REFLECTIONS",
    "MAPPING",
    "TEXTURE_REFLECTION",
    "TEXTURE_SPECULAR",
    "TEXTURE_BUMP",
    "TEXTURE",
    "ORIGIN_TRI",
    "FLAT NO",
    "HALF_FLAT NO",
    "STRETCH_DETAIL NO",
    "HI_QUALITY NO"
  };

  const char* other_state_keywords[] = {
    "ANIMATION",
    "SKELETAL_ANIMATION",
    "ANIM_END",
    "DIRECTORY"
  };

  ReadState state = ReadState::Generic;
  bool endFound = false;
  bool readingComment = false;
  int mips_read = 0;
  int mip_count = 0;
  size_t lines_to_ignore = 0;
  size_t last_read_pos = file.tellg();
  std::string base_dir;
  ModelScript script;
  script.m_highQuality = false;

  for (std::string line; std::getline(file, line); last_read_pos = file.tellg())
  {
    if (Trim(line).empty() || StartsWith(line, ';'))
    {
      continue;
    }
    else if (!readingComment && StartsWith(line, "/*"))
    {
      readingComment = true;
      continue;
    }
    else if (readingComment && StartsWith(line, "*/"))
    {
      readingComment = false;
      continue;
    }
    else if (readingComment)
    {
      continue;
    }

    if (state == ReadState::IgnoreLines)
    {
      if (--lines_to_ignore == 0)
        state = ReadState::Generic;
      continue;
    }
    else if (state == ReadState::Mips)
    {
      const auto mip_file = base_dir + Trim(line);
      script.m_mipModels.emplace_back(CTString(mip_file.c_str()));
      if (mip_count == ++mips_read)
        state = ReadState::Generic;
      continue;
    }
    else if ((state == ReadState::Generic || state == ReadState::Anims) && StartsWith(line, "DIRECTORY "))
    {
      base_dir = Trim({ line.begin() + strlen("DIRECTORY "), line.end() });
      if (base_dir.back() != '\\')
        base_dir += '\\';
      continue;
    }
    else if (state == ReadState::SkeletalAnim)
    {
      if (StartsWith(line, "SOURCE_FILE "))
      {
        auto& anim = script.m_animations.back();
        if (!anim.m_frames.empty())
          ThrowF_t("Animation %s has multiple source files provided!", anim.m_name.c_str());
        const auto src = base_dir + Trim({ line.begin() + strlen("SOURCE_FILE "), line.end() });
        anim.m_frames.emplace_back(CTString(src.c_str()));
      }
      else if (StartsWith(line, "ANIM_NAME_IN_FILE "))
      {
        auto& anim = script.m_animations.back();
        anim.m_customSourceName = Trim({ line.begin() + strlen("ANIM_NAME_IN_FILE "), line.end() });
      }
      else if (StartsWith(line, "ORIG_SKELETON "))
      {
        auto& anim = script.m_animations.back();
        const auto base_skel = base_dir + Trim({ line.begin() + strlen("ORIG_SKELETON "), line.end() });
        anim.m_optRefSkeleton = CTString(base_skel.c_str());
      }
      else if (StartsWith(line, "ORIGIN_BONE "))
      {
        auto& anim = script.m_animations.back();
        anim.m_optOriginBone = { line.begin() + strlen("ORIGIN_BONE "), line.end() };
      }
      else if (StartsWith(line, "INTERPOLATION "))
      {
        auto& anim = script.m_animations.back();
        const std::string interp_line(line.begin() + strlen("INTERPOLATION "), line.end());
        if (interp_line == "Linear")
          anim.m_interpolation = BlenderFCurve::InterpolationMode::Linear;
        else if (interp_line == "Bezier")
          anim.m_interpolation = BlenderFCurve::InterpolationMode::Bezier;
        else
          ThrowF_t("Animation %s has unknown interpolation mode \"%s\". Expected \"Linear\" or \"Bezier\".", anim.m_name.c_str(), interp_line.c_str());
      }
      else if (StartsWith(line, "DURATION "))
      {
        auto& anim = script.m_animations.back();
        anim.m_optDuration = _stod({ line.begin() + strlen("DURATION "), line.end() });
      }
      else if (StartsWith(line, "NUM_FRAMES "))
      {
        auto& anim = script.m_animations.back();
        anim.m_optNumFrames = static_cast<size_t>(_stoi({ line.begin() + strlen("NUM_FRAMES "), line.end()}));
      }
      else if (std::any_of(std::begin(other_state_keywords), std::end(other_state_keywords),
        [&](const char* k) { return StartsWith(line, k); }))
      {
        file.seekg(last_read_pos);
        state = ReadState::Anims;
        const auto& anim = script.m_animations.back();
        if (anim.m_frames.empty())
          ThrowF_t("Animation %s has no source file provided!\n'SOURCE_FILE <filename>' expected!", anim.m_name.c_str());
        continue;
      }
      else
      {
        ThrowF_t("Unrecognizable keyword found in line during skeletal animation parsing: \"%s\".", line.c_str());
      }
    }
    else if (state == ReadState::Frames)
    {
      // deprecated keyword
      if (StartsWith(line, "FRAMES"))
      {
        continue;
      }
      else if (std::any_of(std::begin(other_state_keywords), std::end(other_state_keywords),
        [&](const char* k) { return StartsWith(line, k); }))
      {
        file.seekg(last_read_pos);
        state = ReadState::Anims;
        auto& anim = script.m_animations.back();
        if (anim.m_frames.empty())
          ThrowF_t("Can't find any frames for animation %s.\nThere must be at least 1 frame "
            "per animation.\nList of frames must start at line after line containing key"
            "word SPEED.", anim.m_name.c_str());
        anim.m_optDuration = (*anim.m_optDuration) * anim.m_frames.size();
        continue;
      }
      else if (StartsWith(line, "ANIM "))
      {
        const auto macro_anim = Trim({ line.begin() + strlen("ANIM "), line.end() });
        auto found_pos = std::find_if(script.m_animations.begin(), script.m_animations.end(),
          [&](const ModelScript::Animation& anim) { return stricmp(anim.m_name.c_str(), macro_anim.c_str()) == 0; });
        if (found_pos == script.m_animations.end())
          ThrowF_t("Macro anim \"%s\" should be present before anim \"%s\"!", macro_anim.c_str(), script.m_animations.back().m_name.c_str());
        for (const auto& frame : found_pos->m_frames)
          script.m_animations.back().m_frames.push_back(frame);
      }
      else
      {
        const auto frame = base_dir + Trim(line);
        script.m_animations.back().m_frames.emplace_back(CTString(frame.c_str()));
      }
    }
    else if (state == ReadState::Speed)
    {
      if (StartsWith(line, "SPEED "))
      { // store speed and recompute as duration after all frames are read
        auto& anim = script.m_animations.back();
        anim.m_optDuration = _stof({ line.begin() + strlen("SPEED "), line.end() });
        state = ReadState::Frames;
        continue;
      }
      else
      {
        throw("Expecting key word \"SPEED\" after key word \"ANIMATION\".");
      }
    }
    else if (state == ReadState::Anims)
    {
      if (StartsWith(line, "ANIM_END"))
      {
        state = ReadState::Generic;
        continue;
      }
      else if (StartsWith(line, "ANIMATION "))
      {
        script.m_animations.emplace_back();
        auto& anim = script.m_animations.back();
        anim.m_type = ModelScript::Animation::Type::Vertex;
        anim.m_name = Trim({ line.begin() + strlen("ANIMATION "), line.end() });
        state = ReadState::Speed;
        continue;
      }
      else if (StartsWith(line, "SKELETAL_ANIMATION "))
      {
        script.m_animations.emplace_back();
        auto& anim = script.m_animations.back();
        anim.m_type = ModelScript::Animation::Type::Skeletal;
        anim.m_name = Trim({ line.begin() + strlen("SKELETAL_ANIMATION "), line.end() });
        state = ReadState::SkeletalAnim;
        continue;
      }
      else
      {
        ThrowF_t("Unrecognizable keyword found in line during animation parsing: \"%s\".", line.c_str());
      }
    }
    else if (state == ReadState::Generic)
    {
      if (StartsWith(line, "SIZE "))
      {
        script.m_scale = _stof({ line.begin() + strlen("SIZE "), line.end() });
      }
      else if (StartsWith(line, "TRANSFORM "))
      {
        std::istringstream ss;
        ss.str({ line.begin() + strlen("TRANSFORM "), line.end() });
        for (int row = 1; row <= 3; ++row)
          for (int col = 1; col <= 3; ++col)
            ss >> script.m_transformation(row, col);
      }
      else if (StartsWith(line, "UVCHANNEL "))
      {
        script.m_defaultUVChannel = static_cast<size_t>(_stoi({ line.begin() + strlen("UVCHANNEL "), line.end() }));
      }
      else if (StartsWith(line, "FLAT YES"))
      {
        script.m_flat = ModelScript::Flat::Full;
      }
      else if (StartsWith(line, "HALF_FLAT YES"))
      {
        script.m_flat = ModelScript::Flat::Half;
      }
      else if (StartsWith(line, "STRETCH_DETAIL YES"))
      {
        script.m_stretchDetail = true;
      }
      else if (StartsWith(line, "HI_QUALITY YES"))
      {
        script.m_highQuality = true;
      }
      else if (StartsWith(line, "MAX_SHADOW "))
      {
        script.m_maxShadow = _stoi({ line.begin() + strlen("MAX_SHADOW "), line.end() });
      }
      else if (StartsWith(line, "SKELETON "))
      {
        const auto skeleton = base_dir + Trim({ line.begin() + strlen("SKELETON "), line.end() });
        script.m_skeleton = CTString(skeleton.c_str());
      }
      else if (StartsWith(line, "MIP_MODELS "))
      {
        if (mips_read > 0)
          throw "MIP_MODELS tag should appear only once!";
        mip_count = _stoi({ line.begin() + strlen("MIP_MODELS "), line.end() });
        if (mip_count <= 0 || mip_count >= MAX_MODELMIPS)
          ThrowF_t("Invalid number of mip models. Number must range from 0 to %d.", MAX_MODELMIPS - 1);
        state = ReadState::Mips;
        continue;
      }
      else if (StartsWith(line, "TEXTURE_DIM "))
      {
        std::istringstream ss;
        ss.str({ line.begin() + strlen("TEXTURE_DIM "), line.end() });
        ss >> script.m_textureScale(1) >> script.m_textureScale(2);
      }
      else if (StartsWith(line, "NO_BONE_TRIANGLES"))
      {
        script.m_boneTriangles = false;
      }
      else if (StartsWith(line, "ANIM_START"))
      {
        base_dir = "";
        state = ReadState::Anims;
        continue;
      }
      else if (StartsWith(line, "END"))
      {
        endFound = true;
        break;
      }
      else if (StartsWith(line, "DEFINE_MAPPING"))
      {
        lines_to_ignore = 3;
        state = ReadState::IgnoreLines;
        continue;
      }
      else if (StartsWith(line, "IMPORT_MAPPING"))
      {
        lines_to_ignore = 1;
        state = ReadState::IgnoreLines;
        continue;
      }
      else if (std::none_of(std::begin(keywords_to_ignore), std::end(keywords_to_ignore),
        [&](const char* k) { return StartsWith(line, k); }))
      {
        ThrowF_t("Unrecognizable keyword found in line: \"%s\".", line.c_str());
      }
    }
  }
  if (!endFound)
    throw "Unexpected end of file! Missing END directive";

  if (script.m_mipModels.empty())
    throw "Script contains no mip models!";

  if (script.m_animations.empty())
    throw "Script contains no animations!";

  return script;
}

void SaveToFile(const ModelScript& script, const CTFileName& filename)
{
  const CTString strFile = _fnmApplicationPath + filename;
  std::ofstream file;
  file.open(strFile, std::ios_base::out | std::ios_base::trunc);
  if (!file.is_open())
    throw "Failed to save script!";

  CTString lastDir;
  auto write_dir = [&](const CTFileName& f)
  {
    const CTString dir = static_cast<CTString>(f.FileDir());
    if (lastDir == dir)
      return;
    lastDir = dir;
    file << "DIRECTORY " << dir << '\n';
  };
  auto write_file = [&](const CTFileName& f, const char* tag = "")
  {
    const CTString fnm = static_cast<CTString>(f.FileName() + f.FileExt());
    file << tag << ' ' << fnm << '\n';
  };

  file << "TEXTURE_DIM " << script.m_textureScale(1) << ' ' << script.m_textureScale(2) << '\n';

  file << "TRANSFORM";
  for (int row = 1; row <= 3; ++row)
    for (int col = 1; col <= 3; ++col)
      file << ' ' << script.m_transformation(row, col);
  file << '\n';

  file << "SIZE " << script.m_scale << '\n';

  file << "MAX_SHADOW " << script.m_maxShadow << '\n';

  if (script.m_highQuality)
    file << "HI_QUALITY YES\n";

  if (script.m_flat == ModelScript::Flat::Half)
    file << "HALF_FLAT YES\n";
  else if (script.m_flat == ModelScript::Flat::Full)
    file << "FLAT YES\n";

  if (script.m_stretchDetail)
    file << "STRETCH_DETAIL YES\n";

  if (!script.m_boneTriangles)
    file << "NO_BONE_TRIANGLES\n";

  if (script.m_defaultUVChannel != 0)
    file << "UVCHANNEL " << INDEX(script.m_defaultUVChannel) << '\n';

  file << '\n';

  if (script.m_skeleton.has_value())
  {
    write_dir(*script.m_skeleton);
    write_file(*script.m_skeleton, "SKELETON");
  }

  if (!script.m_mipModels.empty())
    write_dir(script.m_mipModels.front());
  file << "MIP_MODELS " << script.m_mipModels.size() << '\n';
  for (const auto& mip_file : script.m_mipModels)
    write_file(mip_file);

  file << '\n';
  file << "ANIM_START\n";
  file << '\n';
  lastDir = "";

  for (const auto& anim : script.m_animations)
  {
    if (!anim.m_frames.empty())
      write_dir(anim.m_frames.front());

    if (anim.m_type == ModelScript::Animation::Type::Vertex)
    {
      file << "ANIMATION " << anim.m_name << '\n';

      double speed = 0.1;
      if (!anim.m_frames.empty() && anim.m_optDuration.has_value() && *anim.m_optDuration > 0.001)
        speed = (*anim.m_optDuration) / anim.m_frames.size();
      file << "SPEED " << static_cast<float>(speed) << '\n';

      for (const auto& frame : anim.m_frames)
        write_file(frame);
    }
    else
    {
      file << "SKELETAL_ANIMATION " << anim.m_name << '\n';

      if (anim.m_customSourceName.has_value())
        file << "ANIM_NAME_IN_FILE " << *anim.m_customSourceName << '\n';

      if (anim.m_optRefSkeleton.has_value())
        write_file(*anim.m_optRefSkeleton, "ORIG_SKELETON");

      if (anim.m_optOriginBone.has_value())
        file << "ORIGIN_BONE " << *anim.m_optOriginBone << '\n';

      switch (anim.m_interpolation)
      {
      case BlenderFCurve::InterpolationMode::Linear:
        file << "INTERPOLATION Linear\n";
        break;

      case BlenderFCurve::InterpolationMode::Bezier:
        file << "INTERPOLATION Bezier\n";
        break;
      }

      if (anim.m_optDuration.has_value())
        file << "DURATION " << *anim.m_optDuration << '\n';

      if (anim.m_optNumFrames.has_value())
        file << "NUM_FRAMES " << *anim.m_optNumFrames << '\n';

      if (!anim.m_frames.empty())
        write_file(anim.m_frames.front(), "SOURCE_FILE");
    }
    file << '\n';
  }

  file << "ANIM_END\n";
  file << "END\n";
}

} // namespace ScriptIO
