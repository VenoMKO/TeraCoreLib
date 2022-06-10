#include "Tera/FPackage.h"
#include "Tera/FStream.h"
#include "Tera/FObjectResource.h"
#include "Tera/UObject.h"
#include "Tera/UObjectRedirector.h"
#include "Tera/UClass.h"
#include "Tera/UMetaData.h"
#include "Tera/UPersistentCookerData.h"
#include "Tera/UProperty.h"
#include "Tera/UTexture.h"
#include "Tera/UObjectReferencer.h"
#include "Tera/Cast.h"
#include "Tera/CoreMath.h"
#include "Tera/CoreTMM.h"
#include "Tera/CoreCompression.h"

#include "Tera/Utils/APerfSamples.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <array>
#include <ppl.h>
#include <cwctype>

const char* PackageListName = "DirCache.re";
const char* PersistentDataName = "GlobalPersistentCookerData";

FString FPackage::RootDir;
std::recursive_mutex FPackage::PackagesMutex;
std::vector<std::shared_ptr<FPackage>> FPackage::LoadedPackages;
std::vector<std::shared_ptr<FPackage>> FPackage::LoadedSourcelessPackages;
std::vector<std::shared_ptr<FPackage>> FPackage::DefaultClassPackages;
std::vector<FString> FPackage::DirCache;
std::vector<FString> FPackage::FilePackageNames;
std::unordered_map<FString, FString> FPackage::TfcCache;
std::unordered_map<FString, FString> FPackage::PkgMap;
std::unordered_map<FString, FString> FPackage::ObjectRedirectorMap;
std::unordered_map<FString, FCompositePackageMapEntry> FPackage::CompositPackageMap;
std::unordered_map<FString, std::vector<FString>> FPackage::CompositPackageList;
std::unordered_map<FString, FBulkDataInfo> FPackage::BulkDataMap;
std::unordered_map<FString, FTextureFileCacheInfo> FPackage::TextureCacheMap;
std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>> FPackage::MetaData;
std::mutex FPackage::ClassMapMutex;
std::unordered_map<FString, UObject*> FPackage::ClassMap;
std::unordered_set<FString> FPackage::MissingClasses;
std::mutex FPackage::MissingPackagesMutex;
std::vector<FString> FPackage::MissingPackages;
MTransStream FPackage::TranscationStream;

uint16 FPackage::CoreVersion = 0;

void BuildPackageList(const FString& path, std::vector<FString>& dirCache, std::unordered_map<FString, FString>& tfcCache)
{
  PERF_START(S1GameIterator);
  
  dirCache.clear();
  FPackage::FilePackageNames.clear();

  std::filesystem::path fspath(path.WString());

  std::vector<std::wstring> tmpPaths;
  std::unordered_map<std::string, std::wstring> tmpTfcPaths;
  
  for (auto& p : std::filesystem::recursive_directory_iterator(fspath))
  {
    if (!p.is_regular_file() || !p.file_size())
    {
      continue;
    }
    const std::filesystem::path& itemPath = p.path();
    if (!itemPath.has_extension())
    {
      continue;
    }
#if IS_TERA_BUILD
    if (FString(itemPath.parent_path().wstring()).EndsWith("S1Game\\Script"))
    {
      continue;
    }
#endif
    std::string ext = itemPath.extension().string();
    if (((ext[1] == 'g' || ext[1] == 'G') && ((ext[2] == 'p' || ext[2] == 'P') || (ext[2] == 'm' || ext[2] == 'M'))) || ext[1] == 'u' || ext[1] == 'U')
    {
      size_t pos = path.Size();
      if (path[pos - 1] != '\\')
      {
        pos++;
      }
      tmpPaths.emplace_back(itemPath.wstring().substr(pos));
      FPackage::FilePackageNames.emplace_back(itemPath.filename().replace_extension());
    }
    else if (ext[1] == 't' || ext[1] == 'T')
    {
      size_t pos = path.Size();
      if (path[pos - 1] != '\\')
      {
        pos++;
      }
      tmpTfcPaths[itemPath.filename().replace_extension().string()] = itemPath.wstring().substr(pos);
    }
  }
  
  std::sort(tmpPaths.begin(), tmpPaths.end(), [](const std::wstring& a, const std::wstring& b) {
    std::wstring_view tA(&a[a.find_last_of('\\')]);
    std::wstring_view tB(&b[b.find_last_of('\\')]);
    return tA < tB;
  });
  
  dirCache.reserve(tmpPaths.size());
  for (const auto& path : tmpPaths)
  {
    dirCache.emplace_back(W2A(path));
  }
  
  // Must copy the dirCache coz it might be freed
  std::thread([=]() mutable {
    std::filesystem::path listPath = fspath / PackageListName;
    FWriteStream s(listPath.wstring());
    s << dirCache;
  }).detach();

  tfcCache.reserve(tmpTfcPaths.size());
  for (const auto& item : tmpTfcPaths)
  {
    tfcCache[item.first] = W2A(item.second);
  }

  PERF_END(S1GameIterator);
}

void FPackage::SetRootPath(const FString& path)
{
  RootDir = path;
#if CACHE_S1GAME_CONTENTS
  std::filesystem::path listPath = std::filesystem::path(path.WString()) / PackageListName;
  FReadStream s(listPath.wstring());
  if (s.IsGood())
  {
    s << DirCache;
    s << TfcCache;
    return;
  }
#endif
  LogI("Building directory cache: \"%s\"", path.C_str());
  BuildPackageList(path, DirCache, TfcCache);
  LogI("Done. Found %ld packages", DirCache.size());
}

FString FPackage::GetRootPath()
{
  return FPackage::RootDir;
}

FString FPackage::GetDcPath(const FString& s1data)
{
#if !IS_TERA_BUILD
  return {};
#else
  FString data = s1data.Empty() ? FPackage::RootDir : s1data;
  data = data.FStringByAppendingPath("S1Data");
  FString tmp = data.FStringByAppendingPath("DataCenter_Final.dat");
  if (std::filesystem::exists(tmp.WString()))
  {
    return tmp;
  }
  tmp = data.FStringByAppendingPath("DataCenter_Final_EUR.dat");
  if (std::filesystem::exists(tmp.WString()))
  {
    return tmp;
  }
  tmp = data.FStringByAppendingPath("DataCenter_Final_TW.dat");
  if (std::filesystem::exists(tmp.WString()))
  {
    return tmp;
  }
  tmp = data.FStringByAppendingPath("DataCenter_Final_JP.dat");
  if (std::filesystem::exists(tmp.WString()))
  {
    return tmp;
  }
  return {};
#endif
}

void FPackage::SetMetaData(const std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>>& meta)
{
  MetaData = meta;
}

FBulkDataInfo* FPackage::GetBulkDataInfo(const FString& bulkDataName)
{
  if (BulkDataMap.count(bulkDataName))
  {
    return &BulkDataMap[bulkDataName];
  }
  return nullptr;
}

FString FPackage::GetTextureFileCachePath(const FString& tfcName)
{
  if (TfcCache.count(tfcName))
  {
    return RootDir.FStringByAppendingPath(TfcCache[tfcName]);
  }
  return {};
}

const std::unordered_map<FString, FCompositePackageMapEntry>& FPackage::GetCompositePackageMap()
{
  return CompositPackageMap;
}

const std::unordered_map<FString, std::vector<FString>>& FPackage::GetCompositePackageList()
{
  return CompositPackageList;
}

FString FPackage::GetCompositePackageMapPath()
{
#if !IS_TERA_BUILD
  return {};
#else
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / CompositePackageMapperName;
  encryptedPath.replace_extension(".dat");
  return encryptedPath.wstring();
#endif
}

FString FPackage::GetObjectCompositePath(const FString& path)
{
  if (path.Size() && PkgMap.count(path.ToUpper()))
  {
    return PkgMap.at(path.ToUpper());
  }
  return {};
}

void FPackage::UpdateDirCache(const FString& s1game)
{
  FString dir = RootDir;
  if (s1game.Size())
  {
    dir = s1game;
  }
  LogI("Building directory cache: \"%s\"", dir.C_str());
  if (dir.Empty())
  {
    LogE("Can't build directory cache. The path is empty!");
    return;
  }
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.clear();
  }
  BuildPackageList(dir, DirCache, TfcCache);
  LogI("Done. Found %ld packages", DirCache.size());
}

std::vector<FString> FPackage::GetCachedDirCache(const FString& s1game)
{
  FString dir = RootDir;
  if (s1game.Size())
  {
    dir = s1game;
  }
  std::filesystem::path listPath = std::filesystem::path(dir.WString()) / PackageListName;
  FReadStream s(listPath.wstring());
  std::vector<FString> dirCache;
  if (s.IsGood())
  {
    s << dirCache;
  }
  return dirCache;
}

bool FPackage::CreateCompositeMod(CompositeModContext& ctx)
{
  std::vector<FString> objects;
  std::vector<std::pair<FString, int32>> tfcs;

  std::vector<FString> packages;
  for (const FString& path : ctx.Items)
  {
    if (path.FileExtension().ToUpper() == "TFC")
    {
      FString fname = path.Filename();
      if (!fname.StartsWith(NAME_WorldTextures) || fname.Size() < strlen(NAME_WorldTextures) + 3)
      {
        ctx.Error = FString::Sprintf("Incorrect TFC name: %s!", path.Filename().UTF8().c_str());
        return false;
      }
      int32 idx = 0;
      try
      {
        idx = std::stoi(fname.Substr(strlen(NAME_WorldTextures)).C_str());
      }
      catch (...)
      {
        idx = -1;
      }
      if (idx <= 0)
      {
        ctx.Error = FString::Sprintf("Incorrect TFC name: %s!", path.Filename().UTF8().c_str());
        return false;
      }
      tfcs.emplace_back(std::make_pair(path, idx));
      continue;
    }

    FReadStream s(path);
    FPackageSummary sum;
    try
    {
      s << sum;
    }
    catch (const std::exception& e)
    {
      ctx.Error = FString::Sprintf("Failed to read %s:\n%s", path.Filename().C_str(), e.what());
      return false;
    }
    if (sum.GetFileVersion() == VER_TERA_CLASSIC)
    {
      ctx.Error = FString::Sprintf("The package %s is from an old 32-bit game and is not supported by the modern game.", path.Filename().C_str());
      return false;
    }
    if (!s.IsGood())
    {
      ctx.Error = FString::Sprintf("Failed to read package %s.", path.Filename().C_str());
      return false;
    }
    if (!sum.FolderName.StartsWith(TMM_ModPrefix))
    {
      ctx.Error = FString::Sprintf("Package %s has no composite info! Try to resave it from the original.", sum.PackageName.C_str());
      return false;
    }
    FString objName = sum.FolderName.Substr(4);
    if (objName.Empty())
    {
      ctx.Error = FString::Sprintf("Package %s has no composite info! Try to resave it from the original.", sum.PackageName.C_str());
      return false;
    }
    for (int32 idx = 0; idx < objects.size(); ++idx)
    {
      if (objects[idx] == objName)
      {
        ctx.Error = FString::Sprintf("%s and %s are modifying the same composite package!", path.Filename().C_str(), ctx.Items[idx].Filename().C_str());
        return false;
      }
    }
    objects.emplace_back(objName);
    packages.emplace_back(path);
  }

  if (objects.empty())
  {
    ctx.Error = FString::Sprintf("You did not select any valid GPK file!");
    return false;
  }

  FCompositeMeta meta(VER_TERA_FILEMOD, ctx.Container);
  meta.Name = ctx.Name;
  meta.Author = ctx.Author;
  FWriteStream write(ctx.Path);

#if USE_LEGACY_FILEMOD_VER == 0
  if (std::shared_ptr<FPackage> desc = FPackage::CreateModDescriptor(meta.Name, meta.Author))
  {
    PackageSaveContext sctx;
    sctx.EmbedObjectPath = false;
    sctx.PreserveOffsets = false;
    sctx.DisableTextureCaching = false;
    sctx.FullRecook = false;
    sctx.Path = GetTempFilePath();
    sctx.Compression = COMPRESS_LZO;
    try
    {
      desc->Save(sctx);
    }
    catch (...)
    {
      desc = nullptr;
    }
    if (desc)
    {
      FReadStream rs(sctx.Path);
      FILE_OFFSET size = rs.GetSize();
      if (size)
      {
        void* data = malloc(size);
        rs.SerializeBytes(data, size);
        meta.DescriptorOffset = write.GetPosition();
        meta.DescriptorSize = size;
        write.SerializeBytes(data, size);
        free(data);
      }
    }
  }
#endif
  
  const bool compress = USE_MOD_COMPRESSION && ctx.Compression != COMPRESS_None && meta.Version >= VER_TERA_FILEMOD_NEW_META;

  if (compress && ctx.ProgressDescriptionCallback)
  {
    ctx.ProgressDescriptionCallback("Compressing...");
  }

  std::vector<void*> dataBlocks(packages.size());
  auto writePackage = [&](size_t idx) {
    FReadStream read(packages[idx]);
    FILE_OFFSET size = read.GetSize();
    FString name = objects[idx];
    void* data = malloc(size);
    read.SerializeBytes(data, size);
    if (compress)
    {
      int32 compressedSize = size * 2;
      void* compressedData = malloc(compressedSize);
      if (CompressMemory(ctx.Compression, compressedData, &compressedSize, data, size) && compressedSize < size)
      {
        dataBlocks[idx] = compressedData;
        meta.Packages[idx].CompressedSize = compressedSize;
        meta.Packages[idx].Compression = ctx.Compression;
        free(data);
      }
      else
      {
        dataBlocks[idx] = data;
        free(compressedData);
      }
    }
    else
    {
      dataBlocks[idx] = data;
    }
    meta.Packages[idx].ObjectPath = name;
    meta.Packages[idx].Size = size;
  };

  meta.Packages.resize(objects.size());
#if CONCURRENT_MOD_COMPRESSION
  concurrency::parallel_for(size_t(0), size_t(packages.size()), [&](size_t index) {
    writePackage(index);
  });
#else
  for (int32 index = 0; index < packages.size(); ++index)
  {
    writePackage(index);
  }
#endif

  if (ctx.ProgressDescriptionCallback)
  {
    ctx.ProgressDescriptionCallback("Packing...");
  }

  for (int32 idx = 0; idx < dataBlocks.size(); ++idx)
  {
    FILE_OFFSET size = meta.Packages[idx].CompressedSize ? meta.Packages[idx].CompressedSize : meta.Packages[idx].Size;
    meta.Packages[idx].Offset = write.GetPosition();
    write.SerializeBytes(dataBlocks[idx], size);
    free(dataBlocks[idx]);
    dataBlocks[idx] = nullptr;
  }
  meta.LegacyPackagesEnd = write.GetPosition();

  for (const auto& tfc : tfcs)
  {
    FReadStream read(tfc.first);
    FILE_OFFSET tfcOffset = write.GetPosition();
    FILE_OFFSET tfcSize = read.GetSize();
    void* data = malloc(tfcSize);
    read.SerializeBytes(data, tfcSize);
    write.SerializeBytes(data, tfcSize);
    free(data);
    meta.AddTfcOffset(tfcOffset, tfcSize, tfc.second);
  }

  meta.LegacyTfcEnd = write.GetPosition();

  write << meta;

  if (meta.PayloadCRCOffset)
  {
    write.Flush();
    {
      FReadStream r(ctx.Path);
      meta.PayloadCRC = FCompositeMeta::ComputePayloadChecksum(r, meta);
    }
    write.SetPosition(meta.PayloadCRCOffset);
    write << meta.PayloadCRC;
  }
  if (meta.MetaCRCOffset)
  {
    write.Flush();
    {
      FReadStream r(ctx.Path);
      meta.MetaCRC = FCompositeMeta::ComputeMetaChecksum(r, meta);
    }
    write.SetPosition(meta.MetaCRCOffset);
    write << meta.MetaCRC;
  }
  return true;
}

FCompositeMeta FPackage::InstallCompositeMod(const FString& modPath, const FString& s1game)
{
  FReadStream read(modPath);
  if (!read.IsGood())
  {
    UThrow("Failed to open mod file!");
  }

  FCompositeMeta meta;
  read << meta;

  if (meta.Version >= VER_TERA_FILEMOD_NEW_META)
  {
    if (FCompositeMeta::ComputeMetaChecksum(read, meta) != meta.MetaCRC ||
        FCompositeMeta::ComputePayloadChecksum(read, meta) != meta.PayloadCRC)
    {
      UThrow("Mod file is corrupted!");
    }
  }

  FCompositeMeta newMeta = meta;

  std::vector<FString> containers;
  containers.emplace_back("TMM_" + meta.Container);
  FString dest = TMMGetModContainerPath(s1game.WString(), containers.back().WString()).wstring();
  std::unique_ptr<FWriteStream> write = std::make_unique<FWriteStream>(dest);
  if (!read.IsGood())
  {
    UThrow("Failed to write to the game folder!");
  }

  void* tmp = nullptr;

  int32 continerIndex = 0;
  bool needsDescriptor = meta.DescriptorSize;
  for (uint32 idx = 0; idx < meta.Packages.size(); ++idx)
  {
    const FCompositeMeta::FPackageEntry& src = meta.Packages[idx];
    FCompositeMeta::FPackageEntry& dst = newMeta.Packages[idx];

    if (!src.Size)
    {
      continue;
    }

    // Split too big GPK into pieces
    if (write->GetPosition() > TMM_MAX_CONTAINER_SIZE)
    {
      write->Flush();
      containers.emplace_back(FString::Sprintf("%s_%d", containers.front().UTF8().c_str(), ++continerIndex));
      dest = TMMGetModContainerPath(s1game.WString(), containers.back().WString()).wstring();
      write = std::make_unique<FWriteStream>(dest);
      needsDescriptor = meta.DescriptorSize;
    }

    // Write descriptor
    if (needsDescriptor)
    {
      needsDescriptor = false;
      tmp = malloc(meta.DescriptorSize);
      read.SetPosition(meta.DescriptorOffset);
      read.SerializeBytes(tmp, meta.DescriptorSize);
      newMeta.DescriptorOffset = write->GetPosition();
      write->SerializeBytes(tmp, newMeta.DescriptorSize);
      free(tmp);
    }

    // Actual payload
    read.SetPosition(src.Offset);
    tmp = malloc(src.Size);
    if (src.Compression && src.CompressedSize)
    {
      void* compressed = malloc(src.CompressedSize);
      read.SerializeBytes(compressed, src.CompressedSize);
      DecompressMemory((ECompressionFlags)src.Compression, tmp, src.Size, compressed, src.CompressedSize);
      free(compressed);
      dst.Compression = COMPRESS_None;
      dst.CompressedSize = 0;
    }
    else
    {
      read.SerializeBytes(tmp, src.Size);
    }

    dst.Continer = containers.back();
    dst.Offset = write->GetPosition();

    write->SerializeBytes(tmp, src.Size);
    free(tmp);
  }

  std::map<FString, FString> remapTfcs;
  for (uint32 idx = 0; idx < meta.Tfcs.size(); ++idx)
  {
    const FCompositeMeta::FTfcEntry& src = meta.Tfcs[idx];
    if (!src.Size)
    {
      DBreak();
      continue;
    }

    int32 tfcIndex = TMMGetAvailableTfcIndex(s1game.WString());
    if (!tfcIndex)
    {
      std::error_code err;
      for (const FString& container : containers)
      {
        std::filesystem::remove(TMMGetModContainerPath(s1game.WString(), container.WString()), err);
      }
      for (const auto& p : remapTfcs)
      {
        std::filesystem::remove(TMMGetModTfcPath(s1game.WString(), p.second.WString()), err);
      }
      UThrow("Can't install the mod because there are no free TFC slots left.\nTry to remove a few mods.");
    }

    read.SetPosition(src.Offset);
    tmp = malloc(src.Size);
    read.SerializeBytes(tmp, src.Size);

    FWriteStream tfcWrite(TMMGetModTfcPath(s1game.WString(), tfcIndex));
    tfcWrite.SerializeBytes(tmp, src.Size);
    free(tmp);

    if (!tfcWrite.IsGood())
    {
      std::error_code err;
      for (const FString& container : containers)
      {
        std::filesystem::remove(TMMGetModContainerPath(s1game.WString(), container.WString()), err);
      }
      for (const auto& p : remapTfcs)
      {
        std::filesystem::remove(TMMGetModTfcPath(s1game.WString(), p.second.WString()), err);
      }
      UThrow("Failed to write \"%s\" to the game folder.", TMMGetTfcName(tfcIndex).c_str());
    }

    remapTfcs[TMMGetTfcName(src.Index)] = TMMGetTfcName(tfcIndex);
    newMeta.Tfcs[idx].Index = tfcIndex;
    newMeta.Tfcs[idx].Size = 0;
  }

  // Update Tfc names
  for (const FCompositeMeta::FPackageEntry& pkg : newMeta.Packages)
  {
    const FString tmpPath = TMMGetModContainerPath(s1game.WString(), pkg.Continer.WString()).wstring();
    FReadStream tmpRead(tmpPath);
    FWriteStream tmpWrite(tmpPath);
    tmpRead.SetPosition(pkg.Offset);
    FPackageSummary sum;
    tmpRead << sum;
    tmpRead.SetPosition(pkg.Offset + sum.NamesOffset);

    for (uint32 nameIndex = 0; nameIndex < sum.NamesCount; ++nameIndex)
    {
      FILE_OFFSET nameOffset = tmpRead.GetPosition();
      FNameEntry name;
      tmpRead << name;
      if (remapTfcs.count(name.GetString()))
      {
        tmpWrite.SetPosition(nameOffset);
        name.SetString(remapTfcs[name.GetString()]);
        tmpWrite << name;

        if (remapTfcs.size() == 1)
        {
          // Nothing to do
          break;
        }
      }
    }
  }
  return newMeta;
}

std::vector<UClass*> FPackage::GetClasses()
{
  std::vector<UClass*> result;
  result.reserve(ClassMap.size());
  for (const auto& p : ClassMap)
  {
    if (UClass* cls = Cast<UClass>(p.second))
    {
      result.push_back(cls);
    }
  }
  return result;
}

void FPackage::RegisterClass(UClass* classObject)
{
  ClassMap[classObject->GetObjectNameString()] = classObject;
}

void FPackage::BuildClassInheritance()
{
  std::vector<UClass*> classes = GetClasses();
  for (UClass* cls : classes)
  {
    if (UClass* spr = cls->GetSuperClass())
    {
      spr->AddInheretedClass(cls);
    }
  }
}

MTransStream& FPackage::GetTransactionStream()
{
  return TranscationStream;
}

FPackage::S1DirError FPackage::ValidateRootDirCandidate(const FString& s1game)
{
  if (s1game.Empty() || !std::filesystem::exists(s1game.WString()))
  {
    return S1DirError::NOT_FOUND;
  }
  std::vector<FString> classPackages = ClassPackages(true, true);
  std::vector<FString> extraClassPacakges = ClassPackages(false, false);
#if IS_ASTELLIA_BUILD
  const std::filesystem::path scriptsDir = std::filesystem::path(s1game.WString()) / "Script";
#else
  const std::filesystem::path scriptsDir = std::filesystem::path(s1game.WString()) / "CookedPC";
#endif
  if (s1game.Filename(false) != GameRootDir)
  {
    LogE("Error! Folder name \"%s\" does not match the \"%s\"", s1game.Filename(false).UTF8(), GameRootDir);
    return S1DirError::NAME_MISSMATCH;
  }
  
  std::filesystem::path root = s1game.WString();
  for (const FString& name : classPackages)
  {
    if (!std::filesystem::exists(scriptsDir / name.WString()))
    {
      LogE("Error! %s was not found in the %s folder.", name.UTF8().c_str(), GameRootDir);
      return S1DirError::CLASSES_NOT_FOUND;
    }
  }
  for (const FString& name : extraClassPacakges)
  {
    if (!std::filesystem::exists(scriptsDir / name.WString()))
    {
      LogE("Warning! %s was not found in the %s folder.", name.UTF8().c_str(), GameRootDir);
    }
  }
  std::filesystem::path testWritePermissionsFile = root / "re.tmp";
  FILE* f = _wfopen(testWritePermissionsFile.wstring().c_str(), L"w");
  if (!f)
  {
    LogE("Error! Not enough permissions to write to the %s folder.", scriptsDir);
    return S1DirError::ACCESS_DENIED;
  }
  fclose(f);
  std::error_code err;
  if (!std::filesystem::remove(testWritePermissionsFile, err))
  {
    LogE("Error! Not enough permissions to write to the %s folder.", scriptsDir);
    return S1DirError::ACCESS_DENIED;
  }
  return S1DirError::OK;
}

UClass* FPackage::FindClass(const FString& name)
{
  return Cast<UClass>(ClassMap[name]);
}

std::vector<FString> FPackage::ClassPackages(bool core, bool minimal)
{
  std::vector<FString> result;
  if (core && minimal)
  {
    result.emplace_back("Core.u");
  }
#if IS_ASTELLIA_BUILD
  const char* required[] = { "Engine.u", "GameFramework.u", "NSGame.u", "GFxUI.u" };
  const char* extra[] = { "UnrealEd.u", "NSEditor.u", "GFxUIEditor.u", "WinDrv.u", "IpDrv.u", "OnlineSubsystemPC.u", "OnlineSubsystemLive.u", "OnlineSubsystemGameSpy.u", "OnlineSubsystemSteamworks.u" };
#else
  const char* required[] = { "Engine.u", "GameFramework.u", "S1Game.u", "GFxUI.u" };
  const char* extra[] = { "UnrealEd.u", "GFxUIEditor.u", "WinDrv.u", "IpDrv.u", "OnlineSubsystemPC.u", "GodHand.u", "UnrealScriptTest.u" };
#endif

  if (minimal)
  {
    for (const char* name : required)
    {
      result.emplace_back(name);
    }
  }
  else
  {
    for (const char* name : extra)
    {
      result.emplace_back(name);
    }
  }
  return result;
}

std::shared_ptr<FPackage> FPackage::CreateNewPackage(const FPackageSummary& summary)
{
  FPackageSummary sum = summary;
  sum.Generations.emplace_back();
  sum.OriginalCompressionFlags = sum.CompressionFlags;
  sum.OriginalPackageFlags = sum.PackageFlags;
  sum.PackageFlags |= PKG_NoSource | PKG_Dirty;
  sum.PackageSource = CalculateStringCRC((const uint8*)sum.PackageName.C_str(), (int32)sum.PackageName.Size());
  std::shared_ptr<FPackage> pkg = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    pkg = LoadedSourcelessPackages.emplace_back(new FPackage(sum));
  }
  pkg->GetNameIndex(NAME_None, true);
  pkg->Ready.store(true);
  return pkg;
}

void FPackage::CleanCacheDir()
{
  std::error_code err;
  for (auto& p : std::filesystem::directory_iterator(GetTempDir().WString()))
  {
    std::filesystem::remove(p, err);
  }
}

uint16 FPackage::GetCoreVersion()
{
  return CoreVersion;
}

void FPackage::LoadPersistentData()
{
  std::shared_ptr<FPackage> package = nullptr;
  try
  {
    package = GetPackageNamed(PersistentDataName);
  }
  catch (...)
  {}
  if (package)
  {
    package->Load();
    for (FObjectExport* exp : package->Exports)
    {
      if (exp->GetClassName() == UPersistentCookerData::StaticClassName())
      {
        if (UPersistentCookerData* data = Cast<UPersistentCookerData>(package->GetObject(exp->ObjectIndex, false)))
        {
          data->GetPersistentData(BulkDataMap, TextureCacheMap);
          break;
        }
      }
    }
    UnloadPackage(package);
  }
}

void FPackage::LoadClassPackage(const FString& name)
{
  LogI("Loading %s", name.C_str());
  if (auto package = GetPackageNamed(name))
  {
    if (name == "Core.u")
    {
      InitCRCTable();
      CoreVersion = package->GetFileVersion();
      LogI("Core version: %u/%u", package->GetFileVersion(), package->GetLicenseeVersion());
    }
    else if (package->GetFileVersion() != CoreVersion)
    {
      UThrow("Package %s has different version %d/%d (32-bit/64-bit mixed files?)", name.C_str(), package->GetFileVersion(), package->GetLicenseeVersion());
    }

    package->AllowEdit = false;
    package->Load();
    DefaultClassPackages.push_back(package);

    // Load package in memory and create a stream
    FILE_OFFSET packageSize = package->Stream->GetSize();
    void* rawPackageData = malloc(packageSize);
    package->Stream->SerializeBytesAt(rawPackageData, 0, packageSize);

    MReadStream packageStream(rawPackageData, true, packageSize);
    packageStream.SetPackage(package.get());

    // Don't load referenced objects
    packageStream.SetLoadSerializedObjects(false);
    package->Stream->SetLoadSerializedObjects(false);

    // List of root classes
    std::vector<UObject*> classes;
    // List of root defaults\templates
    std::vector<UObject*> defaults;
    // Leftovers
    std::vector<UObject*> other;
    // Properties
    std::vector<UProperty*> properties;

    // Prepare object lists
    UMetaData* meta = nullptr;
    for (auto p : package->ExportObjects)
    {
      if (!p.second)
      {
        continue;
      }
      UObject* obj = p.second;
      
      if (obj->IsTemplate(RF_ClassDefaultObject))
      {
        defaults.push_back(obj);
      }
      else if (obj->GetClassName() == NAME_Class)
      {
        classes.push_back(obj);
        std::scoped_lock<std::mutex> l(ClassMapMutex);
        ClassMap[obj->GetObjectNameString()] = obj;
      }
      else if (obj->IsA(UMetaData::StaticClassName()))
      {
        meta = (UMetaData*)obj;
      }
      else
      {
        if (obj->IsA(UProperty::StaticClassName()))
        {
          properties.push_back((UProperty*)obj);
        }
        other.push_back(obj);
      }
    }

    // Load objects from child to parent to prevent possible stack overflow
    std::function<void(UObject*, std::vector<UObject*>&)> loader;
    loader = [&loader](UObject* obj, std::vector<UObject*>& children) {
      auto tmp = obj->GetInner();
      for (UObject* child : tmp)
      {
        loader(child, children);
      }
      children.push_back(obj);
    };

    // Serialize classes
    for (auto & cls : classes)
    {
      std::vector<UObject*> objects;
      loader(cls, objects);
      MReadStream s(packageStream.GetAllocation(), false, packageSize);
      s.SetPackage(package.get());
      s.SetLoadSerializedObjects(false);
      for (UObject* obj : objects)
      {
        obj->Load(s);
      }
    }

    // Link fields
    for (auto& obj : classes)
    {
      if (UClass* cls = Cast<UClass>(obj))
      {
        cls->Link();
      }
    }

    // Serialize defaults
#if LOAD_DEFAULT_CLASSOBJS
    for (UObject* root : defaults)
    {
      MReadStream s(packageStream.GetAllocation(), false, packageSize);
      s.SetPackage(package.get());
      s.SetLoadSerializedObjects(false);
      if (root->GetInner().size())
      {
        std::vector<UObject*> objects;
        loader(root, objects);
        for (UObject* obj : objects)
        {
          obj->Load(s);
        }
      }
      else
      {
        root->Load(s);
      }
    }
#endif

    UClass::CreateBuiltInClasses(package.get());

    // It's safe to load references now
    package->Stream->SetLoadSerializedObjects(true);
    packageStream.SetLoadSerializedObjects(true);

    for (UObject* root : other)
    {
      MReadStream s(packageStream.GetAllocation(), false, packageSize);
      s.SetPackage(package.get());
      s.SetLoadSerializedObjects(false);
      if (root->GetInner().size())
      {
        std::vector<UObject*> objects;
        loader(root, objects);
        for (UObject* obj : objects)
        {
          obj->Load(s);
        }
      }
      else
      {
        root->Load(s);
      }
    }

    if (meta)
    {
      meta->Load();
      const auto& objMap = meta->GetObjectMetaDataMap();
      for (const auto& pair : objMap)
      {
        if (UField* field = Cast<UField>(pair.first))
        {
          for (const auto& info : pair.second)
          {
            if (info.first == "ToolTip")
            {
              field->SetToolTip(info.second);
              break;
            }
          }
        }
      }
    }

    for (UProperty* p : properties)
    {
      const FString propertyName = p->GetObjectNameString();
      UObject* outer = p->GetOuter();
      std::vector<UObject*> parents;

      while (outer->GetOuter())
      {
        parents.push_back(outer);
        outer = outer->GetOuter();
      }
      const FString className = outer->GetObjectNameString();
      if (parents.size() && parents.front()->IsA(UFunction::StaticClassName()))
      {
        continue;
      }

      if (parents.size())
      {
        FString key = className + ":" + parents.back()->GetObjectNameString();
        if (MetaData.count(key))
        {
          if (MetaData.at(key).count(propertyName))
          {
            const AMetaDataEntry& entry = MetaData.at(key).at(propertyName);
            if (entry.Name.Size())
            {
              p->DisplayName = entry.Name;
            }
            if (entry.Tooltip.Size())
            {
              p->SetToolTip(entry.Tooltip);
            }
            continue;
          }
        }
      }

      if (MetaData.count(outer->GetObjectName().String()))
      {
        const FString& key = className;
        if (MetaData[key].count(propertyName))
        {
          const AMetaDataEntry& entry = MetaData.at(key).at(propertyName);
          if (entry.Name.Size())
          {
            p->DisplayName = entry.Name;
          }
          if (entry.Tooltip.Size())
          {
            p->SetToolTip(entry.Tooltip);
          }
        }
      }
    }

#ifdef _DEBUG
    for (FObjectExport* exp : package->Exports)
    {
      UObject* obj = package->GetObject(exp->ObjectIndex, false);
      DBreakIf((!obj || !obj->IsLoaded()) && !obj->IsTemplate(RF_ClassDefaultObject));
    }
#endif
  }
  else
  {
    UThrow("Failed to load: %s!", name.C_str());
  }
}

void FPackage::UnloadDefaultClassPackages()
{
  ClassMap.clear();
  MissingClasses.clear();
  for (const auto& package : DefaultClassPackages)
  {
    UnloadPackage(package);
  }
  DefaultClassPackages.clear();
}

void FPackage::LoadPkgMapper(bool rebuild)
{
  PkgMap.clear();
  std::filesystem::path storagePath = std::filesystem::path(RootDir.WString()) / PackageMapperName;
  storagePath.replace_extension(".re");
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / PackageMapperName;
  encryptedPath.replace_extension(".dat");
  LogI("Reading %s storage...", PackageMapperName);

  uint64 fts = GetFileTime(encryptedPath.wstring());
  if (std::filesystem::exists(storagePath) && !rebuild)
  {
    FReadStream rs(storagePath.wstring());
    uint64 ts = 0;
    rs << ts;
    if (fts == ts || !fts)
    {
      rs << PkgMap;
      LogI("Loaded cached %s storage.", PackageMapperName);
      return;
    }
    LogW("%s storage is outdated! Updating...", PackageMapperName);
  }

  if (!std::filesystem::exists(encryptedPath))
  {
    UThrow("Failed to open %s", encryptedPath.string().c_str());
  }

  DeserializePkgMapper(encryptedPath, PkgMap);

  LogI("Saving %s storage", PackageMapperName);
  FWriteStream ws(storagePath.wstring());
  ws << fts;
  ws << PkgMap;
}

void FPackage::LoadCompositePackageMapper()
{
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.clear();
  }
  CompositPackageMap.clear();
  CompositPackageList.clear();
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / CompositePackageMapperName;
  encryptedPath.replace_extension(".dat");

  DeserializeCompositePackageMapper(encryptedPath, CompositPackageMap, CompositPackageList);

  LogI("Loaded %s storage", CompositePackageMapperName);
}

void FPackage::LoadObjectRedirectorMapper(bool rebuild)
{
  ObjectRedirectorMap.clear();
  std::filesystem::path storagePath = std::filesystem::path(RootDir.WString()) / ObjectRedirectorMapperName;
  storagePath.replace_extension(".re");
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / ObjectRedirectorMapperName;
  encryptedPath.replace_extension(".dat");
  LogI("Reading %s storage...", ObjectRedirectorMapperName);
  uint64 fts = GetFileTime(encryptedPath.wstring());
  if (std::filesystem::exists(storagePath) && !rebuild)
  {
    FReadStream rs(storagePath.wstring());
    uint64 ts = 0;
    rs << ts;
    if (fts == ts || !fts)
    {
      rs << ObjectRedirectorMap;
      LogI("Loaded cached %s storage.", ObjectRedirectorMapperName);
      return;
    }
    LogW("%s storage is outdated! Updating...", ObjectRedirectorMapperName);
  }

  DeserializeObjectRedirectorMapper(encryptedPath, ObjectRedirectorMap);
  
  LogI("Saving %s storage", ObjectRedirectorMapperName);
  FWriteStream ws(storagePath.wstring());
  ws << fts;
  ws << ObjectRedirectorMap;
}

std::shared_ptr<FPackage> FPackage::GetPackage(const FString& path)
{
  std::shared_ptr<FPackage> found = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    for (const auto& package : LoadedPackages)
    {
      if (package->GetSourcePath() == path)
      {
        found = package;
        break;
      }
    }
    if (found)
    {
      LoadedPackages.push_back(found);
      return found;
    }
    for (const auto& package : LoadedPackages)
    {
      for (auto exPackage : package->ExternalPackages)
      {
        if (package->GetSourcePath() == path)
        {
          found = package;
          break;
        }
      }
      if (found)
      {
        LoadedPackages.push_back(found);
        return found;
      }
    }
  }
  if (path.StartsWith(RootDir))
  {
    LogI("Opening package: %s", path.Substr(RootDir.Size()).String().c_str());
  }
  else
  {
    LogI("Opening package: %s", path.String().c_str());
  }
  FStream *stream = new FReadStream(path);
  if (!stream->IsGood())
  {
    delete stream;
    UThrow("Couldn't open the file: %s!", path.FilenameString(true).c_str());
  }
  FPackageSummary sum;
  sum.SourcePath = path;
  sum.DataPath = path;
  sum.PackageName = std::filesystem::path(path.WString()).filename().wstring();
  (*stream) << sum;
  sum.OriginalPackageFlags = sum.PackageFlags;
  sum.OriginalCompressedChunks = sum.CompressedChunks;
  if (CoreVersion && sum.GetFileVersion() != CoreVersion)
  {
    if (sum.GetFileVersion() == VER_TERA_CLASSIC)
    {
      UThrow("Real Editor can't work with 32-bit and 64-bit files simultaneously!\nYour S1Game folder contains 64-bit client, but the %s is a 32-bit file.\nChange your S1Game folder to a 32-bit client and try again.", path.FilenameString(true).c_str());
    }
    else if (sum.GetFileVersion() == VER_TERA_MODERN)
    {
      UThrow("Real Editor can't work with 32-bit and 64-bit files simultaneously!\nYour S1Game folder contains 32-bit client, but the %s is a 64-bit file.\nChange your S1Game folder to a 64-bit client and try again.", path.FilenameString(true).c_str());
    }
    else
    {
      UThrow("%s version %d(L: %d) differs from your S1Game version %d", sum.PackageName.C_str(), sum.GetFileVersion(), sum.GetLicenseeVersion(), CoreVersion);
    }
  }
  if (sum.CompressedChunks.size())
  {
    FILE_OFFSET startOffset = INT_MAX;
    FILE_OFFSET totalDecompressedSize = 0;
    void** compressedChunksData = new void*[sum.CompressedChunks.size()];
    {
      uint32 idx = 0;
      for (FCompressedChunk& chunk : sum.CompressedChunks)
      {
        compressedChunksData[idx] = malloc(chunk.CompressedSize);
        stream->SetPosition(chunk.CompressedOffset);
        stream->SerializeBytes(compressedChunksData[idx], chunk.CompressedSize);
        totalDecompressedSize += chunk.DecompressedSize;
        if (chunk.DecompressedOffset < startOffset)
        {
          startOffset = chunk.DecompressedOffset;
        }
        idx++;
      }
    }

    std::filesystem::path decompressedPath = GetTempFilePath().WString();
    sum.DataPath = W2A(decompressedPath.wstring());
    FStream* tempStream = new FWriteStream(sum.DataPath.WString());
    DBreakIf(!tempStream->IsGood());

    sum.OriginalCompressionFlags = sum.CompressionFlags;
    sum.PackageFlags &= ~PKG_StoreCompressed;
    sum.CompressionFlags = COMPRESS_None;

    std::vector<FCompressedChunk> chunks;
    std::swap(sum.CompressedChunks, chunks);
    auto tmpSize = sum.SourceSize;
    (*tempStream) << sum;
    sum.SourceSize = tmpSize;

    uint8* decompressedData = (uint8*)malloc(totalDecompressedSize);
    try
    {
      auto decompressFunc = [&](size_t idx) {
        const FCompressedChunk& chunk = chunks[idx];
        uint8* dst = decompressedData + chunk.DecompressedOffset - startOffset;
        LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
      };
#if ALLOW_CONCURRENT_LZO
      concurrency::parallel_for(size_t(0), size_t(chunks.size()), [&](size_t idx) {
        decompressFunc(idx);
      });
#else
      for (size_t idx = 0; idx < chunks.size(); ++idx) {
        decompressFunc(idx);
      }
#endif
    }
    catch (const std::exception& e)
    {
      for (size_t idx = 0; idx < chunks.size(); ++idx)
      {
        free(compressedChunksData[idx]);
      }
      delete[] compressedChunksData;
      free(decompressedData);
      delete tempStream;
      delete stream;
      throw e;
    }
    for (size_t idx = 0; idx < chunks.size(); ++idx)
    {
      free(compressedChunksData[idx]);
    }
    delete[] compressedChunksData;
    tempStream->SerializeBytes(decompressedData, totalDecompressedSize);
    free(decompressedData);
    delete tempStream;
    delete stream;

    // Read decompressed header
    stream = new FReadStream(sum.DataPath);
    try
    {
      (*stream) << sum;
    }
    catch (const std::exception& e)
    {
      delete stream;
      throw e;
    }
    sum.PackageName = std::filesystem::path(path.WString()).filename().wstring();
  }
  
  delete stream;
  std::shared_ptr<FPackage> result = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    result = LoadedPackages.emplace_back(new FPackage(sum));
  }
  return result;
}

std::shared_ptr<FPackage> FPackage::GetPackageNamed(const FString& name, FGuid guid)
{
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    if (std::find(MissingPackages.begin(), MissingPackages.end(), name) != MissingPackages.end())
    {
      return nullptr;
    }
  }

  bool anyPackageFound = false;
  std::shared_ptr<FPackage> found = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    for (const auto& package : LoadedPackages)
    {
      if (package->GetPackageName().ToUpper() == name.ToUpper())
      {
        anyPackageFound = true;
        if (guid.IsValid())
        {
          if (package->GetGuid() == guid)
          {
            found = package;
            break;
          }
        }
        else
        {
          found = package;
          break;
        }
      }
    }
    if (found)
    {
      return LoadedPackages.emplace_back(found);
    }
  }
  
  // Check and load if the package is a composite package
  if (CoreVersion > VER_TERA_CLASSIC && CompositPackageMap.count(name))
  {
    const FCompositePackageMapEntry& entry = CompositPackageMap[name];
    FString packagePath;
    for (FString& path : DirCache)
    {
      std::wstring filename = path.FilenameWString();
      if (filename.size() < entry.FileName.Size())
      {
        continue;
      }
      std::wstring tmp = entry.FileName.WString();
      if (std::mismatch(tmp.begin(), tmp.end(), filename.begin()).first == tmp.end())
      {
        packagePath = RootDir.FStringByAppendingPath(path);
        break;
      }
    }
    if (packagePath.Size())
    {
      LogI("Reading composite package %s from %s...", name.C_str(), entry.FileName.C_str());
      void* rawData = malloc(entry.Size);
      {
        FReadStream rs(packagePath);
        rs.SetPosition(entry.Offset);
        rs.SerializeBytes(rawData, entry.Size);
        if (!rs.IsGood())
        {
          UThrow("Failed to read %s", name.C_str());
        }
      }
      std::filesystem::path tmpPath = GetTempFilePath().WString();
      {
        FWriteStream ws(tmpPath);
        ws.SerializeBytes(rawData, entry.Size);
        if (!ws.IsGood())
        {
          UThrow("Failed to save composite package %s to %s", name.C_str(), tmpPath.string().c_str());
        }
      }
      free(rawData);
      std::shared_ptr<FPackage> package = GetPackage(tmpPath.wstring());
      package->CompositeDataPath = tmpPath.wstring();
      package->CompositeSourcePath = packagePath.WString();
      package->Summary.PackageName = name;
      package->Composite = true;
      return package;
    }
  }

  std::wstring wname = name.ToUpper().WString();
  // Exact name search
  for (FString& path : DirCache)
  {
    std::wstring filename = path.FilenameWString(false);
    std::transform(filename.begin(), filename.end(), filename.begin(),
      [](std::wint_t c) { return std::towupper(c); }
    );
    if (filename.size() < wname.size())
    {
      continue;
    }
    if (filename == wname)
    {
      if (auto package = GetPackage(RootDir.FStringByAppendingPath(path)))
      {
        if (!guid.IsValid() || guid == package->GetGuid())
        {
          return package;
        }
        UnloadPackage(package);
      }
    }
  }
  // Masked search: Name****
  for (FString& path : DirCache)
  {
    std::wstring filename = path.FilenameWString();
    std::transform(filename.begin(), filename.end(), filename.begin(),
      [](std::wint_t c) { return std::towupper(c); }
    );
    if (filename.size() < wname.size())
    {
      continue;
    }
    if (std::mismatch(wname.begin(), wname.end(), filename.begin()).first == wname.end())
    {
      if (auto package = GetPackage(RootDir.FStringByAppendingPath(path)))
      {
        anyPackageFound = true;
        if (!guid.IsValid() || guid == package->GetGuid())
        {
          return package;
        }
        UnloadPackage(package);
      }
    }
  }

  // If we have not found any packages with similar name regardless guid
  if (!anyPackageFound)
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.push_back(name);
    LogE("Failed to find: %s.gpk", name.UTF8().c_str());
  }
  return nullptr;
}

void FPackage::UnloadPackage(std::shared_ptr<FPackage> package)
{
  if (!package)
  {
    return;
  }
  bool lastPackageRef = false;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    if (package->GetPackageFlag(PKG_NoSource))
    {
      for (size_t idx = 0; idx < LoadedSourcelessPackages.size(); ++idx)
      {
        if (LoadedSourcelessPackages[idx].get() == package.get())
        {
          if (!lastPackageRef)
          {
            LoadedSourcelessPackages.erase(LoadedSourcelessPackages.begin() + idx);
            lastPackageRef = true;
            idx--;
          }
          else
          {
            // There are more refs. No need to continue
            lastPackageRef = false;
            break;
          }
        }
      }
    }
    else
    {
      for (size_t idx = 0; idx < LoadedPackages.size(); ++idx)
      {
        if (LoadedPackages[idx].get() == package.get())
        {
          if (!lastPackageRef)
          {
            LoadedPackages.erase(LoadedPackages.begin() + idx);
            lastPackageRef = true;
            idx--;
          }
          else
          {
            // There are more refs. No need to continue
            lastPackageRef = false;
            break;
          }
        }
      }
    }
  }
  if (lastPackageRef)
  {
    LogI("Package %s unloaded", package->GetPackageName().C_str());
  }
}

bool FPackage::NamedPackageExists(const FString& name, bool updateDirCache)
{
  if (name.Empty())
  {
    return false;
  }
  if (CompositPackageMap.count(name))
  {
    return true;
  }
  std::wstring wname = name.ToUpper().WString();
  for (FString& path : DirCache)
  {
    std::wstring filename = path.FilenameWString(false);
    std::transform(filename.begin(), filename.end(), filename.begin(),
      [](std::wint_t c) { return std::towupper(c); }
    );
    if (filename.size() < wname.size())
    {
      continue;
    }
    if (filename == wname)
    {
      return true;
    }
  }
  if (updateDirCache)
  {
    UpdateDirCache();
    return NamedPackageExists(name, false);
  }
  return false;
}

std::shared_ptr<FPackage> FPackage::CreateModDescriptor(const FString& name, const FString& author, const FString& appVer)
{
  FPackageSummary summary;
  int32 tmm_ver_major = 0, tmm_ver_minor = 0;
  GetTargetTmmVersion(tmm_ver_major, tmm_ver_minor);
  summary.FolderName = FString::Sprintf("%sTMM version %d.%02d", TMM_ModPrefix, tmm_ver_major, tmm_ver_minor);
  summary.FolderName.Terminate();
  summary.Guid = FGuid::Generate();
  summary.SetFileVersion(VER_TERA_MODERN, 17);
  summary.PackageFlags = PKG_AllowDownload | PKG_Cooked | PKG_RequireImportsAlreadyLoaded | PKG_NoExportAllowed | PKG_StrippedSource | PKG_ROAccess;
  summary.EngineVersion = 13249;
  summary.ContentVersion = 142;
  summary.PackageSource = CalculateStringCRC((const uint8*)name.C_str(), (int32)name.Length());
  std::shared_ptr<FPackage> result = FPackage::CreateNewPackage(summary);
  if (UTextBuffer* buffer = Cast<UTextBuffer>(result->GetObject(result->AddExport("ReadMe", NAME_TextBuffer, nullptr))))
  {
    buffer->SetObjectFlags(RF_LoadForEdit | RF_NotForClient | RF_NotForServer | RF_ObjectIsRO);
    FString desc = "Mod: ";
    desc += name;
    desc += "\nAuthor: ";
    desc += author;
    desc += "\nMade by: Real Editor ";
    desc += appVer;
    desc += "\nTarget TMM version: ";
    if (tmm_ver_major)
    {
      desc += FString::Sprintf("v.%d.%02d(or greater)", tmm_ver_major, tmm_ver_minor);
    }
    else
    {
      desc += "Unknown";
    }
    desc += "\n\nTo install this mod:\n * Start Tera Mod Manager(TMM)\n * Press Add and select this file\n\nIf you don't have TMM you can find it here: https://github.com/VenoMKO/TMM/releases";
    buffer->SetText(desc);
    return result;
  }
  return nullptr;
}

FPackage::~FPackage()
{
  if (Stream)
  {
    delete Stream;
  }
  for (auto& obj : ExportObjects)
  {
    delete obj.second;
  }
  for (FObjectExport* exp : Exports)
  {
    delete exp;
  }
  for (FObjectImport* imp : Imports)
  {
    delete imp;
  }
  for (VObjectExport* exp : VExports)
  {
    delete exp;
  }
  for (FObjectThumbnailInfo* info : ThumbnailInfos)
  {
    delete info;
  }
  for (FObjectThumbnail* thumbnail : Thumbnails)
  {
    delete thumbnail;
  }
  
  for (auto& pkg : ExternalPackages)
  {
    UnloadPackage(pkg);
  }
  ExternalPackages.clear();
  if (Summary.DataPath != Summary.SourcePath)
  {
    std::filesystem::remove(std::filesystem::path(Summary.DataPath.WString()));
  }
  if (CompositeDataPath.Size())
  {
    std::filesystem::remove(std::filesystem::path(CompositeDataPath.WString()));
  }
}

void FPackage::Load()
{
  if (Ready.load() || Loading.load())
  {
    return;
  }
  Loading.store(true);
  Stream = new FReadStream(Summary.DataPath);
  Stream->SetPackage(this);
  FStream& s = GetStream();
  Load(s);
  Ready.store(true);
  Loading.store(false);

  if (Referencer)
  {
    bool tmp = s.GetLoadSerializedObjects();
    s.SetLoadSerializedObjects(false);
    Referencer->Load(s);
    s.SetLoadSerializedObjects(tmp);
  }
}

void FPackage::Load(FStream& s)
{
#define CheckCancel() if (Cancelled.load()) { Loading.store(false); return; } //

  PERF_START(LoadPackage);
  if (Summary.NamesOffset != s.GetPosition())
  {
    s.SetPosition(Summary.NamesOffset);
  }
  Names.clear();
  Summary.NamesSize = s.GetPosition();
  for (uint32 idx = 0; idx < Summary.NamesCount; ++idx)
  {
    FNameEntry& e = Names.emplace_back(FNameEntry());
    s << e;
    CheckCancel();
  }
  Summary.NamesSize = s.GetPosition() - Summary.NamesSize;

  if (Summary.ImportsOffset != s.GetPosition())
  {
    s.SetPosition(Summary.ImportsOffset);
  }
  Imports.clear();
  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    FObjectImport* imp = Imports.emplace_back(new FObjectImport(this));
    imp->ObjectIndex = -(PACKAGE_INDEX)idx - 1;
    s << *imp;
    CheckCancel();
  }

  if (Summary.ExportsOffset != s.GetPosition())
  {
    s.SetPosition(Summary.ExportsOffset);
  }
  Exports.clear();
  for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
  {
    FObjectExport* exp = Exports.emplace_back(new FObjectExport(this));
    exp->ObjectIndex = (PACKAGE_INDEX)idx + 1;
    s << *exp;
    CheckCancel();
#ifdef _DEBUG
    exp->ClassNameValue = exp->GetClassNameString();
#endif
  }

  for (FObjectExport* exp : Exports)
  {
    ExportObjects[exp->ObjectIndex] = UObject::Object(exp);
    if (!Referencer && !ExportObjects[exp->ObjectIndex]->HasAnyFlags(RF_ClassDefaultObject) && exp->GetClassName() == UObjectReferencer::StaticClassName())
    {
      Referencer = Cast<UObjectReferencer>(ExportObjects[exp->ObjectIndex]);
    }
  }

  if (Summary.DependsOffset != s.GetPosition())
  {
    s.SetPosition(Summary.DependsOffset);
  }
  Depends.clear();
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
    {
      std::vector<int>& arr = Depends.emplace_back(std::vector<int>());
      s << arr;
      CheckCancel();
    }
  }
  else
  {
    std::vector<int>& arr = Depends.emplace_back(std::vector<int>());
    arr.resize(Summary.ExportsCount);
    for (int& item : arr)
    {
      s << item;
    }
  }

  if (Summary.ThumbnailTableOffset)
  {
    if (Summary.ThumbnailTableOffset != s.GetPosition())
    {
      s.SetPosition(Summary.ThumbnailTableOffset);
    }

    int32 thumbnailCount = 0;
    s << thumbnailCount;

    ThumbnailInfos.resize(thumbnailCount);
    for (int32 idx = 0; idx < thumbnailCount; ++idx)
    {
      ThumbnailInfos[idx] = new FObjectThumbnailInfo;
      s << *ThumbnailInfos[idx];
    }

    Thumbnails.resize(thumbnailCount);
    for (int32 idx = 0; idx < thumbnailCount; ++idx)
    {
      Thumbnails[idx] = new FObjectThumbnail;
      s.SetPosition(ThumbnailInfos[idx]->Offset);
      s << *Thumbnails[idx];
    }
  }

  if (Summary.ImportExportGuidsOffset > 0)
  {
    if (Summary.ImportExportGuidsOffset != s.GetPosition())
    {
      s.SetPosition(Summary.ImportExportGuidsOffset);
    }
#if !IS_ASTELLIA_BUILD
    DBreakIf(Summary.ImportGuidsCount || Summary.ExportGuidsCount);
#endif
    ImportGuids.clear();
    ImportGuids.resize(Summary.ImportGuidsCount);
    for (uint32 idx = 0; idx < Summary.ImportGuidsCount; ++idx)
    {
      s << ImportGuids[idx];
    }

    ExportGuids.clear();
    for (uint32 idx = 0; idx < Summary.ExportGuidsCount; ++idx)
    {
      FGuid objectGuid;
      PACKAGE_INDEX exportIndex = 0;
      s << objectGuid << exportIndex;
      ExportGuids[objectGuid] = GetExportObject(exportIndex);
    }
  }

  for (uint32 index = 0; index < Summary.ExportsCount; ++index)
  {
    FObjectExport* exp = Exports[index];
#ifdef _DEBUG
    exp->Path = exp->GetObjectPath();
#endif
    if (exp->OuterIndex)
    {
      ExportObjects[exp->ObjectIndex]->SetOuter(ExportObjects[exp->OuterIndex]);
      ExportObjects[exp->OuterIndex]->AddInner(ExportObjects[exp->ObjectIndex]);

      FObjectExport* outer = GetExportObject(exp->OuterIndex);
      outer->Inner.push_back(exp);
      exp->Outer = outer;
    }
    else
    {
      RootExports.push_back(exp);
    }
    CheckCancel();
    ObjectNameToExportMap[exp->GetObjectNameString()].push_back(exp);
  }

  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    CheckCancel();
    FObjectImport* imp = Imports[idx];
#ifdef _DEBUG
    imp->Path = imp->GetObjectPath();
#endif
    if (imp->OuterIndex)
    {
      if (imp->OuterIndex < 0)
      {
        FObjectImport* outer = GetImportObject(imp->OuterIndex);
        outer->Inner.push_back(imp);
      }
    }
    else
    {
      RootImports.push_back(imp);
    }
  }

  PERF_END(LoadPackage);

#if _DEBUG
  for (FObjectExport* exp : Exports)
  {
    if (exp->ExportFlags & EF_ScriptPatcherExport)
    {
      // If here: probably need to apply stack frames, patch the export table and fix net count
      DBreak();
    }
  }
#endif

  // Not bulletproof. Will work only for exposed packages
  if (CompositPackageList.count(GetPackageName().ToUpper()))
  {
    Composite = true;
  }
}

bool FPackage::Save(PackageSaveContext& context)
{
  if (context.EmbedObjectPath && IsComposite() && GetFolderName() == NAME_None)
  {
    const FString packageName = GetPackageName();
    const auto& map = FPackage::GetCompositePackageMap();
    if (map.count(packageName))
    {
      const FCompositePackageMapEntry& entry = map.at(packageName);
      FString folderName = TMM_ModPrefix;
      folderName += entry.ObjectPath;
      SetFolderName(folderName);
    }
  }

  if (context.DisableTextureCaching)
  {
    if (Summary.TextureAllocations.TextureTypes.size())
    {
      Summary.TextureAllocations.TextureTypes.clear();
      MarkDirty();
    }
    for (FObjectExport* exp : Exports)
    {
      if (exp->GetClassName() == UTexture2D::StaticClassName())
      {
        ((UTexture2D*)GetObject(exp, true))->DisableCaching();
      }
    }
  }

  bool hasSource = true;
  if (Summary.PackageFlags & PKG_NoSource)
  {
    hasSource = false;

    Summary.PackageFlags &= ~PKG_NoSource;
    Summary.OriginalPackageFlags = Summary.PackageFlags;
    Summary.Generations.front().Names = (int32)Names.size();
    Summary.Generations.front().Exports = (int32)Exports.size();
    Summary.Generations.front().NetObjects = 0;
    for (FObjectExport* exp : Exports)
    {
      UObject* obj = GetObject(exp, false);
      if (obj->GetNetIndex() != INDEX_NONE)
      {
        Summary.Generations.front().NetObjects++;
      }
    }
  }
  else if (!IsDirty())
  {
    if (context.Compression == COMPRESS_None)
    {
      if (context.ProgressDescriptionCallback)
      {
        context.ProgressDescriptionCallback("Saving...");
      }
      
      FReadStream readStream(Summary.DataPath);
      FILE_OFFSET size = readStream.GetSize();

      FPackageSummary summary;
      readStream << summary;
      if (summary.CompressionFlags != context.Compression && summary.CompressedChunks.size() && summary.PackageFlags & PKG_StoreCompressed)
      {
        FILE_OFFSET startOffset = INT_MAX;
        FILE_OFFSET totalDecompressedSize = 0;

        std::vector<FCompressedChunk> chunks = summary.CompressedChunks;
        void** compressedChunksData = new void* [chunks.size()];

        for (int32 idx = 0; idx < summary.CompressedChunks.size(); ++idx)
        {
          FCompressedChunk& chunk = chunks[idx];
          compressedChunksData[idx] = malloc(chunk.CompressedSize);
          readStream.SetPosition(chunk.CompressedOffset);
          readStream.SerializeBytes(compressedChunksData[idx], chunk.CompressedSize);
          totalDecompressedSize += chunk.DecompressedSize;
          if (chunk.DecompressedOffset < startOffset)
          {
            startOffset = chunk.DecompressedOffset;
          }
        }
        
        summary.CompressedChunks.clear();
        summary.CompressionFlags = COMPRESS_None;
        summary.PackageFlags &= ~PKG_StoreCompressed;

        void* decompressedPackageData = malloc(readStream.GetPosition() + totalDecompressedSize);
        auto decompressFunc = [&](size_t idx) {
          const FCompressedChunk& chunk = chunks[idx];
          uint8* dst = (uint8*)decompressedPackageData + chunk.DecompressedOffset - startOffset;
          LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
        };
#if ALLOW_CONCURRENT_LZO
        concurrency::parallel_for(size_t(0), size_t(chunks.size()), [&](size_t idx) {
          decompressFunc(idx);
        });
#else
        for (size_t idx = 0; idx < chunks.size(); ++idx) {
          decompressFunc(idx);
        }
#endif

        MWriteStream memStream(decompressedPackageData, readStream.GetPosition() + totalDecompressedSize);
        memStream << summary;

        FWriteStream writeStream(context.Path);
        writeStream.SerializeBytes(memStream.GetAllocation(), memStream.GetSize());

        free(decompressedPackageData);
        return true;
      }

      // Package is already decompressed. Copy the data.

      void* tmp = malloc(size);
      readStream.SetPosition(0);
      readStream.SerializeBytes(tmp, size);
      if (!readStream.IsGood() || !size)
      {
        context.Error = "Failed to read source package.";
        free(tmp);
        return false;
      }
      if (context.ProgressCallback)
      {
        context.ProgressCallback(50);
      }
      if (context.ProgressDescriptionCallback)
      {
        context.ProgressDescriptionCallback("Writing data...");
      }
      FWriteStream writeStream(context.Path);
      writeStream.SerializeBytes(tmp, size);
      if (!writeStream.IsGood())
      {
        context.Error = "Failed to write the data.";
        free(tmp);
        return false;
      }
      free(tmp);
      return true;
    }

    // Compress the package without fancy object serialization

    FReadStream readStream(Summary.DataPath);
    if (!readStream.IsGood() || !readStream.GetSize())
    {
      context.Error = "Failed to read source package.";
      return false;
    }

    FPackageSummary summary;
    readStream << summary;
    FILE_OFFSET dataStart = readStream.GetPosition();
    FILE_OFFSET size = readStream.GetSize() - dataStart;

    // Copy-paste data if the package is already compressed
    if (summary.CompressedChunks.size() && summary.CompressionFlags == context.Compression)
    {
      summary.PackageFlags |= PKG_StoreCompressed;

      FWriteStream writeStream(context.Path);
      writeStream << summary;

      void* pkgData = malloc(size);
      readStream.SerializeBytes(pkgData, size);
      writeStream.SerializeBytes(pkgData, size);
      free(pkgData);

      if (!writeStream.IsGood() || !readStream.IsGood())
      {
        context.Error = "IO error. Check source and destination are available for read and write!";
        return false;
      }
      return true;
    }

    void* uncompressedData = malloc(size);
    readStream.SerializeBytes(uncompressedData, size);

    int32	totalChunkCount = (size + COMPRESSED_BLOCK_SIZE - 1) / COMPRESSED_BLOCK_SIZE;
    summary.CompressedChunks.resize(totalChunkCount);
    summary.PackageFlags |= PKG_StoreCompressed;
    summary.CompressionFlags = context.Compression;

    FWriteStream writeStream(context.Path);
    writeStream << summary;

    const int32 compressedDataSize = 2 * COMPRESSED_BLOCK_SIZE;
    void* compressedData = malloc(compressedDataSize);

    int32 chunkIndex = 0;
    int32 remainingSize = size;
    FILE_OFFSET offset = dataStart;

    while (remainingSize > 0)
    {
      int32 sizeToCompress = std::min(remainingSize, COMPRESSED_BLOCK_SIZE);
      int32 compressedSize = compressedDataSize;

      if (!CompressMemory(COMPRESS_LZO, compressedData, &compressedSize, (uint8*)uncompressedData + offset, sizeToCompress))
      {
        context.Error = "Failed to compress data.";
        free(compressedData);
        free(uncompressedData);
        return false;
      }

      summary.CompressedChunks[chunkIndex].DecompressedSize = sizeToCompress;
      summary.CompressedChunks[chunkIndex].CompressedOffset = writeStream.GetPosition();
      summary.CompressedChunks[chunkIndex].DecompressedOffset = offset;
      offset += sizeToCompress;

      writeStream.SerializeBytes(compressedData, compressedSize);

      summary.CompressedChunks[chunkIndex].CompressedSize = compressedSize;
      chunkIndex++;
      remainingSize -= COMPRESSED_BLOCK_SIZE;
    }

    writeStream.SetPosition(0);
    writeStream << summary;

    free(compressedData);
    free(uncompressedData);

    if (!writeStream.IsGood() || !readStream.IsGood())
    {
      context.Error = "IO error. Check source and destination are available for read and write!";
      return false;
    }
    return true;
  }

  bool needToMoveGpk = true;
  FString ctxPath = GetTempFilePath();
  std::vector<FObjectExport*> marked;

  bool isOk = true;
  FILE_OFFSET payloadSize = 0;
  {
    FWriteStream writer(ctxPath);
    if (!writer.IsGood())
    {
      context.Error = "Failed to write data!";
      return false;
    }
    writer.SetPackage(this);

    FPackageSummary summary;
    std::unique_ptr<FReadStream> reader = nullptr;
    if (hasSource)
    {
      // Stream of the decompressed temporary source.
      reader = std::make_unique<FReadStream>(Summary.DataPath);
      reader->SetPackage(this);
      if (!reader->IsGood())
      {
        context.Error = "Failed to read the source package!";
        return false;
      }
      // Get the original package summary
      *reader << summary;
    }
    else
    {
      summary = Summary;
    }

    if (!Summary.ImportGuidsCount && !Summary.ExportGuidsCount)
    {
      // Non-zero values force linker to seek for Guid map.
      // Set offset to INDEX_NONE to make games linker ignore the map.
      Summary.ImportExportGuidsOffset = INDEX_NONE;
    }
    else
    {
      // Never seen these maps in Tera. Need to implement if BHS used them somewhere
      DBreak();
      context.Error = "Packages with cross level guids are not supported yet!";
      return false;
    }

    Summary.ThumbnailTableOffset = 0; // Cleanup after BHS. They forgot to strip these.

    Summary.NamesCount = (uint32)Names.size();
    Summary.ExportsCount = (uint32)Exports.size();
    Summary.ImportsCount = (uint32)Imports.size();

    Summary.CompressionFlags = context.Compression;
    if (context.Compression == COMPRESS_None)
    {
      Summary.PackageFlags &= ~PKG_StoreCompressed;
    }
    else
    {
      Summary.PackageFlags |= PKG_StoreCompressed;
    }

    Summary.PackageFlags &= ~PKG_Dirty;

    writer << Summary;

    std::vector<FObjectExport*> newExports;
    std::vector<FObjectExport*> sortedExports;
    for (FObjectExport* exp : Exports)
    {
      if (exp->SerialOffset)
      {
        sortedExports.emplace_back(exp);
      }
      else
      {
        newExports.emplace_back(exp);
      }
    }
    std::sort(sortedExports.begin(), sortedExports.end(), [](FObjectExport* a, FObjectExport* b) {
      return a->SerialOffset < b->SerialOffset&& a->SerialOffset;
    });
    std::sort(newExports.begin(), newExports.end(), [](FObjectExport* a, FObjectExport* b) {
      return a->ObjectIndex < b->ObjectIndex;
    });
    if (newExports.size())
    {
      // Ensure all new exports are in the end
      sortedExports.insert(sortedExports.end(), newExports.begin(), newExports.end());
    }

    // Check if header size did change and we honor offsets
    bool moveTables = hasSource && context.PreserveOffsets && (Summary.NamesOffset != writer.GetPosition());

    if (hasSource && !moveTables && context.PreserveOffsets)
    {
      // TODO: exports have non-const size. Table can become larger even if elements count is lower. Fixed-size calculation
      if (Summary.ExportsCount > summary.ExportsCount || Summary.ImportsCount > summary.ImportsCount)
      {
        moveTables = true;
      }
    }

    Summary.NamesOffset = writer.GetPosition();
    for (FNameEntry& e : Names)
    {
      writer << e;
    }

    if (hasSource && context.PreserveOffsets)
    {
      if (!moveTables && writer.GetPosition() - Summary.NamesOffset > Summary.NamesSize)
      {
        // Names table become larger. We need to move tables to preserve offsets
        moveTables = true;
      }
      if (!moveTables)
      {
        // Check if the tables were moved during previous save
        for (FObjectExport* exp : Exports)
        {
          if (exp->SerialOffset < Summary.ImportsOffset)
          {
            moveTables = true;
            break;
          }
        }
      }
    }

    if (moveTables)
    {
      // Prevent the game from buffering whole package if we are moving tables to the end.
      Summary.HeaderSize = writer.GetPosition();
    }
    else
    {
      Summary.ImportsOffset = writer.GetPosition();
      for (FObjectImport* imp : Imports)
      {
        writer << *imp;
      }

      Summary.ExportsOffset = writer.GetPosition();
      for (FObjectExport* exp : Exports)
      {
        writer << *exp;
      }

      Summary.DependsOffset = writer.GetPosition();
      if (Summary.GetFileVersion() > VER_TERA_CLASSIC)
      {
        for (auto& dep : Depends)
        {
          writer << dep;
        }
      }
      else
      {
        for (auto& dep : Depends.front())
        {
          writer << dep;
        }
      }
      Summary.HeaderSize = writer.GetPosition();
    }

    FILE_OFFSET exportsStart = hasSource ? sortedExports.front()->SerialOffset : Summary.HeaderSize;
    auto appendZeroed = [](FStream& s, FILE_OFFSET size) {
      void* tmp = calloc(size, 1);
      s.SerializeBytes(tmp, size);
      free(tmp);
    };

    if (exportsStart > writer.GetPosition())
    {
      appendZeroed(writer, exportsStart - writer.GetPosition());
    }
    else if (exportsStart < writer.GetPosition())
    {
      DBreakIf(context.PreserveOffsets);
      context.Error = "Package has too much changes in the header. Can't save it with the 'Preserve Offsets' flag";
      Summary.PackageFlags |= PKG_Dirty;
      return false;
    }

    if (context.MaxProgressCallback)
    {
      context.MaxProgressCallback((int)Exports.size());
    }
    if (context.ProgressCallback)
    {
      context.ProgressCallback(0);
    }
    if (context.ProgressDescriptionCallback)
    {
      context.ProgressDescriptionCallback("Serializing objects...");
    }

    // List of holes left after moving dirty objects. Offset, Size.
    std::vector<std::pair<FILE_OFFSET, FILE_OFFSET>> holes;
    int32 idx = 0;
    for (FObjectExport* exp : sortedExports)
    {
      if (exp->ObjectFlags & RF_Marked)
      {
        if (context.PreserveOffsets && hasSource)
        {
          holes.emplace_back(writer.GetPosition(), exp->SerialSize);
          appendZeroed(writer, exp->SerialSize);
        }
        else
        {
          exp->SerialOffset = writer.GetPosition();
          UObject* obj = ExportObjects[exp->ObjectIndex];
          obj->Serialize(writer);
          exp->SerialSize = writer.GetPosition() - exp->SerialOffset;
        }
      }
      else
      {
        if (context.PreserveOffsets && writer.GetPosition() != exp->SerialOffset)
        {
          if (writer.GetPosition() < exp->SerialOffset)
          {
            holes.emplace_back(writer.GetPosition(), exp->SerialOffset - writer.GetPosition());
            appendZeroed(writer, exp->SerialOffset - writer.GetPosition());
          }
          else if (exp->SerialOffset)
          {
            DBreak();
            LogE("Failed to preserve offset of %s", exp->GetObjectNameString().C_str());
          }
        }
        if (!context.FullRecook)
        {
          reader->SetPosition(exp->SerialOffset);
          void* data = malloc(exp->SerialSize);
          reader->SerializeBytes(data, exp->SerialSize);
          exp->SerialOffset = writer.GetPosition();
          writer.SerializeBytes(data, exp->SerialSize);
          free(data);
        }
        else
        {
          exp->SerialOffset = writer.GetPosition();
          UObject* obj = ExportObjects[exp->ObjectIndex];
          if (!obj->IsLoaded())
          {
            obj->Load();
          }
          obj->Serialize(writer);
          exp->SerialSize = writer.GetPosition() - exp->SerialOffset;
        }
      }
      if (context.ProgressCallback)
      {
        context.ProgressCallback(++idx);
      }
    }

    if (moveTables)
    {
      Summary.ImportsOffset = writer.GetPosition();
      for (FObjectImport* imp : Imports)
      {
        writer << *imp;
      }

      Summary.ExportsOffset = writer.GetPosition();
      for (FObjectExport* exp : Exports)
      {
        writer << *exp;
      }

      Summary.DependsOffset = writer.GetPosition();
      for (auto& dep : Depends)
      {
        writer << dep;
      }
    }

    if (context.PreserveOffsets && hasSource)
    {
      for (FObjectExport* exp : sortedExports)
      {
        if (exp->ObjectFlags & RF_Marked)
        {
          MWriteStream tmpWriter(nullptr, 1024 * 1024, writer.GetPosition());
          tmpWriter.SetPackage(this);

          exp->SerialOffset = writer.GetPosition();
          UObject* obj = ExportObjects[exp->ObjectIndex];
          DBreakIf(!obj->IsLoaded());
          obj->Serialize(tmpWriter);
          exp->SerialSize = tmpWriter.GetPosition() - exp->SerialOffset;

          // Try to fit data in an existing hole
          int32 foundHole = -1;
          FILE_OFFSET holeSizeDelta = INT32_MAX;
          for (int32 idx = 0; idx < holes.size(); ++idx)
          {
            FILE_OFFSET holeSize = holes[idx].second;
            if (holeSize >= exp->SerialSize && holeSize - exp->SerialSize < holeSizeDelta)
            {
              holeSizeDelta = holeSize - exp->SerialSize;
              foundHole = idx;
            }
          }

          if (foundHole >= 0)
          {
            auto& hole = holes[foundHole];
            exp->SerialOffset = hole.first;

            FILE_OFFSET tmpPos = writer.GetPosition();

            writer.SetPosition(exp->SerialOffset);
            obj->Serialize(writer);

            if (exp->SerialSize != writer.GetPosition() - exp->SerialOffset)
            {
              context.Error = "Failed to save package due to ambiguous object size!";
              LogE("Failed to save package due to ambiguous object size!");
              return false;
            }

            hole.first += exp->SerialSize;
            hole.second -= exp->SerialSize;
            DBreakIf(hole.second < 0);

            writer.SetPosition(tmpPos);
          }
          else
          {
            writer.SerializeBytes(tmpWriter.GetAllocation(), exp->SerialSize);
          }
        }
      }
    }

    payloadSize = writer.GetPosition();

    // Apply header changes
    writer.SetPosition(0);
    writer << Summary;

    // Unmark objects and apply exports table changes 
    writer.SetPosition(Summary.ExportsOffset);
    for (FObjectExport* exp : Exports)
    {
      if (exp->ObjectFlags & RF_Marked)
      {
        marked.push_back(exp);
        exp->ObjectFlags &= ~RF_Marked;
      }
      writer << *exp;
    }
    isOk = writer.IsGood();
  }

  if (isOk)
  {
    if (!hasSource && Summary.CompressionFlags)
    {
      struct TmpChunkData {
        void* UncompressedData = nullptr;
        void* CompressedData = nullptr;
      };

      FILE_OFFSET pos = (Summary.NamesOffset && Summary.NamesOffset < Summary.HeaderSize) ? Summary.NamesOffset : Summary.HeaderSize;

      Summary.CompressedChunks.clear();
      Summary.CompressedChunks.emplace_back();
      Summary.CompressedChunks.back().DecompressedOffset = pos;

      std::vector<TmpChunkData> tmpChunks;
      tmpChunks.emplace_back();

      auto prepareChunk = [this, &tmpChunks](int32 idx) {
        FObjectExport* exp = Exports[idx];
        if (exp->SerialOffset - Summary.CompressedChunks.back().DecompressedOffset >= COMPRESSED_CHUNK_SIZE)
        {
          Summary.CompressedChunks.back().DecompressedSize = exp->SerialOffset - Summary.CompressedChunks.back().DecompressedOffset;
          tmpChunks.back().CompressedData = malloc(Summary.CompressedChunks.back().DecompressedSize * 2);
          tmpChunks.back().UncompressedData = malloc(Summary.CompressedChunks.back().DecompressedSize);

          Summary.CompressedChunks.emplace_back();
          Summary.CompressedChunks.back().DecompressedOffset = exp->SerialOffset;
          tmpChunks.emplace_back();
        }
        else if (idx && ((exp->SerialOffset + exp->SerialSize) - Summary.CompressedChunks.back().DecompressedOffset) >= COMPRESSED_CHUNK_SIZE)
        {
          Summary.CompressedChunks.back().DecompressedSize = (exp->SerialOffset + exp->SerialSize) - Summary.CompressedChunks.back().DecompressedOffset;
          tmpChunks.back().CompressedData = malloc(Summary.CompressedChunks.back().DecompressedSize * 2);
          tmpChunks.back().UncompressedData = malloc(Summary.CompressedChunks.back().DecompressedSize);

          Summary.CompressedChunks.emplace_back();
          Summary.CompressedChunks.back().DecompressedOffset = (exp->SerialOffset + exp->SerialSize);
          tmpChunks.emplace_back();
        }
      };

      for (int32 idx = 0; idx < Exports.size(); ++idx)
      {
        prepareChunk(idx);
      }

      if (!Summary.CompressedChunks.back().DecompressedSize)
      {
        Summary.CompressedChunks.back().DecompressedSize = payloadSize - Summary.CompressedChunks.back().DecompressedOffset;
        tmpChunks.back().CompressedData = malloc(Summary.CompressedChunks.back().DecompressedSize * 2);
        tmpChunks.back().UncompressedData = malloc(Summary.CompressedChunks.back().DecompressedSize);
      }

      {
        FReadStream reader(ctxPath);
        for (size_t idx = 0; idx < tmpChunks.size(); ++idx)
        {
          reader.SetPosition(Summary.CompressedChunks[idx].DecompressedOffset);
          reader.SerializeBytes(tmpChunks[idx].UncompressedData, Summary.CompressedChunks[idx].DecompressedSize);
        }
      }

      std::vector<FCompressedChunk>* compressedChunksPtr = &Summary.CompressedChunks;
      auto compressFunc = [&](size_t idx) {
        const TmpChunkData& chunk = tmpChunks.at(idx);
        int32 compressedSize = compressedChunksPtr->at(idx).DecompressedSize * 2;
        LZO::Compress(chunk.UncompressedData, compressedChunksPtr->at(idx).DecompressedSize, chunk.CompressedData, compressedSize);
        compressedChunksPtr->at(idx).CompressedSize = compressedSize;
      };
#if ALLOW_CONCURRENT_LZO
      concurrency::parallel_for(size_t(0), size_t(tmpChunks.size()), [&](size_t idx) {
        compressFunc(idx);
      });
#else
      for (size_t idx = 0; idx < tmpChunks.size(); ++idx) {
        compressFunc(idx);
      }
#endif

      FWriteStream finalWriter(context.Path);
      finalWriter << Summary;
      for (int32 idx = 0; idx < tmpChunks.size(); ++idx)
      {
        TmpChunkData& chunk = tmpChunks[idx];
        Summary.CompressedChunks[idx].CompressedOffset = finalWriter.GetPosition();
        finalWriter.SerializeBytes(chunk.CompressedData, Summary.CompressedChunks[idx].CompressedSize);
        free(chunk.CompressedData);
        free(chunk.UncompressedData);
        chunk.CompressedData = nullptr;
        chunk.UncompressedData = nullptr;
      }
      finalWriter.SetPosition(0);
      Summary.PackageFlags |= PKG_StoreCompressed;
      finalWriter << Summary;
      Summary.PackageFlags &= ~PKG_StoreCompressed;
      isOk = finalWriter.IsGood();
      needToMoveGpk = false;
    }
  }

  // Since this is a "Save As" we need to restore dirty flags
  for (FObjectExport* exp : marked)
  {
    exp->ObjectFlags |= RF_Marked;
  }
  MarkDirty();
  if (!hasSource)
  {
    Summary.PackageFlags |= PKG_NoSource;
  }
  
  if (!isOk)
  {
    context.Error = "Unknown error! Failed to write data!";
  }
  else if (needToMoveGpk)
  {
    std::error_code err;
    std::filesystem::path dstPath(A2W(context.Path));
    std::filesystem::rename(ctxPath.WString(), dstPath, err);
    if (err)
    {
      context.Error = "Failed to save file!\nCheck if you have permissions to write to the destination and if there is enough free disk space.";
      return false;
    }
  }
  return isOk;
}

FString FPackage::GetCompositePath()
{
  if (IsComposite() && CompositPackageMap.count(GetPackageName()))
  {
    return CompositPackageMap[GetPackageName()].ObjectPath;
  }
  return {};
}

VObjectExport* FPackage::CreateVirtualExport(const char* objName, const char* clsName)
{
  VObjectExport* exp = new VObjectExport(this, objName, clsName);
  VExports.push_back(exp);
  return exp;
}

FString FPackage::GetPackageName(bool extension) const
{
  if (extension)
  {
    return Summary.PackageName;
  }
  FString name = Summary.PackageName;
  if (name.Length())
  {
    size_t start = 0;
    size_t idx = name.FindFirstOf('.', start);
    while (idx == 0)
    {
      start++;
      idx = name.FindFirstOf('.', start);
    }
    if (idx != std::string::npos)
    {
      name = name.Substr(start, idx);
    }
  }
  return name;
}

UObject* FPackage::GetObject(PACKAGE_INDEX index, bool load)
{
  if (index > 0)
  {
    return GetObject(GetExportObject(index), load);
  }
  if (index < 0)
  {
    return GetObject(GetImportObject(index), load);
  }
  return nullptr;
}

UObject* FPackage::GetObject(FObjectImport* imp, bool load)
{
  if (imp->Package == this)
  {
    if (UObject* obj = GetCachedImportObject(imp->ObjectIndex))
    {
      if (load)
      {
        obj->Load();
      }
      return obj;
    }
    FString impPkgName = imp->GetPackageName();

    if (impPkgName != GetPackageName(false))
    {
      // Check already loaded packages
      {
        std::scoped_lock<std::mutex> lock(ExternalPackagesMutex);
        for (std::shared_ptr<FPackage>& external : ExternalPackages)
        {
          if (impPkgName == external->GetPackageName(false))
          {
            UObject* obj = external->GetObject(imp, load);
            ImportObjects[imp->ObjectIndex] = obj;
            return obj;
          }
        }
      }
      // Load external package
      std::shared_ptr<FPackage> package = nullptr;
      try
      {
        package = GetPackageNamed(impPkgName);
      }
      catch (...)
      {}
      if (package)
      {
        package->Load();
        if (UObject* obj = package->GetObject(imp, load))
        {
          std::scoped_lock<std::mutex> lock(ExternalPackagesMutex);
          ExternalPackages.emplace_back(package);
          ImportObjects[imp->ObjectIndex] = obj;
          return obj;
        }
        UnloadPackage(package);
      }
      LogW("%s: Couldn't find import %s(%s)", GetPackageName().C_str(), imp->GetObjectNameString().C_str(), imp->GetClassNameString().C_str());
      return nullptr;
    }
  }
  for (VObjectExport* vexp : VExports)
  {
    if (vexp->GetObjectName() == imp->GetObjectName() && vexp->GetClassName() == imp->GetClassName())
    {
      return vexp->GetObject();
    }
  }
  std::vector<FObjectExport*> exps = GetExportObject(imp->GetObjectName());
  FName className = imp->GetClassName();
  for (FObjectExport* exp : exps)
  {
    if (exp->GetClassName() == className)
    {
      return GetObject(exp);
    }
  }
  return nullptr;
}

UObject* FPackage::GetObject(FObjectExport* exp, bool load)
{
  if (!exp)
  {
    return nullptr;
  }
  if (exp->ObjectIndex == VEXP_INDEX)
  {
    for (VObjectExport* vexp : VExports)
    {
      if (vexp == exp)
      {
        return vexp->GetObject();
      }
    }
  }

  UObject* obj = GetCachedExportObject(exp->ObjectIndex);
  if (obj && load)
  {
    obj->Load();
  }
  return obj;
}

UObject* FPackage::GetObject(const FString& path)
{
  if (path.Empty())
  {
    return nullptr;
  }
  std::vector<FString> components = path.Split('.');
  std::vector<FObjectExport*> inner = GetRootExports();
  if (components.size() > 1)
  {
    FString component = components.front();
    if (component.ToUpper() == GetPackageName(false).ToUpper())
    {
      components = std::vector<FString>(components.begin() + 1, components.end());
    }
  }
  FObjectExport* exp = nullptr;
  for (const FString& component : components)
  {
    exp = nullptr;
    for (FObjectExport* in : inner)
    {
      if (in->GetObjectName() == component)
      {
        exp = in;
        break;
      }
    }

    if (exp)
    {
      inner = exp->Inner;
      continue;
    }

    size_t pos = component.FindLastOf('_');
    if (pos != std::string::npos)
    {
      FString name = component.Substr(0, pos);
      try
      {
        int32 number = std::stoi(component.Substr(pos + 1).String());
        for (FObjectExport* in : inner)
        {
          if (in->ObjectName.String(false) == name && in->ObjectName.GetNumber() == number)
          {
            exp = in;
            break;
          }
        }
      }
      catch (...)
      {}
    }

    if (exp)
    {
      inner = exp->Inner;
      continue;
    }
    // Failed to find the component
    break;
  }
  if (!exp)
  {
    // 99% of calls land in the previous loop. This loop can be slow without affecting overall performance.
    FString component = components.back();
    FString componentUpper = component.ToUpper();
    auto exports = Exports;
    for (FObjectExport* e : exports)
    {
      if (e->ObjectName.String(true).ToUpper() == componentUpper || e->ObjectName.String(false).ToUpper() == componentUpper)
      {
        return GetObject(e, false);
      }
      size_t pos = component.FindLastOf('_');
      if (pos != std::string::npos)
      {
        FString name = component.Substr(0, pos);
        try
        {
          int32 number = std::stoi(component.Substr(pos + 1).String());
          if (e->ObjectName.String(false) == name && e->ObjectName.GetNumber() == number)
          {
            return GetObject(e, false);
          }
          number--;
          if (e->ObjectName.String(false) == name && e->ObjectName.GetNumber() == number)
          {
            return GetObject(e, false);
          }
        }
        catch (...)
        {
        }
      }
    }
  }
  return GetObject(exp, false);
}

FObjectExport* FPackage::DuplicateExportRecursivly(FObjectExport* source, FObjectExport* dest, const FString& objectName)
{
  FObjectExport* exp = new FObjectExport(this, objectName);
  exp->ExportFlags = EF_None;
  exp->ObjectFlags = RF_Public | RF_LoadForServer | RF_LoadForClient | RF_LoadForEdit;
  exp->ClassIndex = source->ClassIndex;
  Exports.emplace_back(exp);
  if (GetFileVersion() > VER_TERA_CLASSIC)
  {
    Depends.emplace_back();
  }
  else
  {
    Depends.front().emplace_back(0);
  }
  exp->ObjectIndex = (PACKAGE_INDEX)Exports.size();

  if (dest)
  {
    dest->Inner.emplace_back(exp);
    exp->Outer = dest;
    exp->OuterIndex = dest->ObjectIndex;
  }
  else
  {
    RootExports.emplace_back(exp);
    exp->OuterIndex = 0;
  }
  exp->SerialOffset = source->SerialOffset;
  exp->SerialSize = source->SerialSize;

  UObject* obj = UObject::Object(exp);
  SetCachedExportObject(exp->ObjectIndex, obj);
  obj = GetObject(exp, true);
  if (obj)
  {
    if (Referencer)
    {
      Referencer->AddObject(obj);
    }
    obj->MarkDirty();
  }
  else
  {
    DBreak();
    RemoveExport(exp);
    return nullptr;
  }
  exp->SerialOffset = 0;
  exp->SerialSize = 0;
  for (FObjectExport* sourceChild : source->Inner)
  {
    DuplicateExportRecursivly(sourceChild, exp, sourceChild->GetObjectNameString());
  }
  return exp;
}

PACKAGE_INDEX FPackage::GetObjectIndex(UObject* object) const
{
  if (!object)
  {
    return 0;
  }
  if (object->GetPackage() != this)
  {
    std::scoped_lock<std::mutex> l(ImportObjectsMutex);
    for (const auto& p : ImportObjects)
    {
      if (p.second == object)
      {
        return p.first;
      }
    }
    return 0;
  }
  return object->GetExportObject()->ObjectIndex;
}

PACKAGE_INDEX FPackage::GetNameIndex(const FString& name, bool insert)
{
  for (PACKAGE_INDEX index = 0; index < Names.size(); ++index)
  {
    if (Names[index].GetString() == name)
    {
      return index;
    }
  }
  if (insert)
  {
    Names.emplace_back(name);
    return (PACKAGE_INDEX)Names.size() - 1;
  }
  return INDEX_NONE;
}

UClass* FPackage::LoadClass(PACKAGE_INDEX index)
{
  if (!index)
  {
    return nullptr;
  }

  // Speedup class load during class packages load
  if (index > 0)
  {
    UObject* obj = GetObject(index);
    FString objName = obj->GetObjectNameString();
    {
      std::scoped_lock<std::mutex> l(ClassMapMutex);
      if (!ClassMap.count(objName))
      {
        ClassMap[objName] = obj;
      }
    }
    return (UClass*)obj;
  }

  if (UObject* obj = GetCachedImportObject(index))
  {
    return (UClass*)obj;
  }

  FObjectImport* imp = GetImportObject(index);
  FString name = imp->GetObjectNameString();
  {
    std::scoped_lock<std::mutex> l(ClassMapMutex);
    if (ClassMap.count(name))
    {
      UObject* obj = ClassMap[name];
      SetCachedImportObject(index, obj);
      return (UClass*)obj;
    }
  }
  if (MissingClasses.count(name))
  {
    return nullptr;
  }
  FString pkgName = imp->GetPackageName();
  if (pkgName == GetPackageName())
  {
    UObject* obj = GetObject(imp);
    if (obj)
    {
      std::scoped_lock<std::mutex> l(ClassMapMutex);
      ClassMap[name] = obj;
      return (UClass*)obj;
    }
  }
  {
    std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
    for (const auto& pkg : ExternalPackages)
    {
      if (pkg->GetPackageName() == pkgName)
      {
        UObject* obj = pkg->GetObject(imp);
        if (obj)
        {
          std::scoped_lock<std::mutex> l(ClassMapMutex);
          ClassMap[name] = obj;
          ExternalPackages.push_back(pkg);
          return (UClass*)obj;
        }
        break;
      }
    }
  }
  {
    std::shared_ptr<FPackage> pkg = nullptr;
    try
    {
      pkg = GetPackageNamed(pkgName);
    }
    catch (...)
    {}
    if (pkg)
    {
      UObject* obj = pkg->GetObject(imp);
      if (obj)
      {
        std::scoped_lock<std::mutex> l(ClassMapMutex);
        ClassMap[name] = obj;
        std::scoped_lock<std::mutex> lock(ExternalPackagesMutex);
        ExternalPackages.push_back(pkg);
        return (UClass*)obj;
      }
      FPackage::UnloadPackage(pkg);
    }
  }
  
  
  if (!MissingClasses.count(name))
  {
    LogE("Failed to load class %s", name.C_str());
    MissingClasses.insert(name);
  }
  return nullptr;
}

UClass* FPackage::GetClass(const FString& className)
{
  if (ClassMap.count(className))
  {
    return Cast<UClass>(ClassMap[className]);
  }
  return nullptr;
}

PACKAGE_INDEX FPackage::ImportClass(UClass* cls)
{
  if (cls->GetPackage() == this)
  {
    return cls->GetExportObject()->ObjectIndex;
  }

  FObjectImport* classPackageImport = nullptr;
  const FString classPackage = cls->GetPackage()->GetPackageName();
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetClassName() == UClass::StaticClassName() && imp->GetObjectName() == cls->GetObjectName())
    {
      return imp->ObjectIndex;
    }
    if (imp->GetClassName() == NAME_Package && imp->GetObjectName() == classPackage)
    {
      classPackageImport = imp;
    }
  }

  if (!classPackageImport)
  {
    classPackageImport = FObjectImport::CreateImport(this, classPackage, (UClass*)ClassMap[NAME_Package]);
    if (!classPackageImport)
    {
      return 0;
    }
    classPackageImport->ObjectIndex = -PACKAGE_INDEX(Imports.size()) - 1;
    Imports.push_back(classPackageImport);
    for (FPackageObserver* observer : Observers)
    {
      observer->OnImportAdded(classPackageImport);
    }
    MarkDirty();
  }

  FObjectImport* classImport = FObjectImport::CreateImport(this, cls->GetObjectNameString(), nullptr);
  if (classImport)
  {
    classImport->ObjectIndex = -PACKAGE_INDEX(Imports.size()) - 1;
    Imports.push_back(classImport);
    if (classPackageImport)
    {
      classImport->OuterIndex = classPackageImport->ObjectIndex;
      if (std::find(classPackageImport->Inner.begin(), classPackageImport->Inner.end(), classImport) == classPackageImport->Inner.end())
      {
        classPackageImport->Inner.emplace_back(classImport);
      }
    }
    for (FPackageObserver* observer : Observers)
    {
      observer->OnImportAdded(classImport);
    }
    LogI("Added class: %s", classImport->GetFullObjectName().UTF8().c_str());
    MarkDirty();
    return classImport->ObjectIndex;
  }
  return 0;
}

void FPackage::AddNetObject(UObject* object)
{
  NetIndexMap[object->GetNetIndex()] = object;
}

UObject* FPackage::GetObject(NET_INDEX netIndex, const FString& name, const FString& className)
{
  UObject* result = nullptr;
  if (netIndex != INDEX_NONE && NetIndexMap.count(netIndex))
  {
    result = NetIndexMap[netIndex];
  }
  else
  {
    std::vector<FObjectExport*> exps = GetExportObject(name);
    UObject* obj = nullptr;
    for (FObjectExport* exp : exps)
    {
      if (exp->GetClassName() == className)
      {
        if (netIndex != INDEX_NONE)
        {
          obj = GetObject(exp);
          if (obj->GetNetIndex() == netIndex)
          {
            result = obj;
            break;
          }
        }
        else if (obj = GetObject(exp))
        {
          result = obj;
          break;
        }
      }
    }
  }
  return result;
}

std::vector<FObjectExport*> FPackage::GetExportObject(const FString& name)
{
  if (ObjectNameToExportMap.count(name))
  {
    return ObjectNameToExportMap[name];
  }
  for (FObjectExport* exp : Exports)
  {
    if (exp->GetObjectName() == name)
    {
      return { exp };
    }
  }
  for (VObjectExport* vexp : VExports)
  {
    if (vexp->GetObjectName() == name)
    {
      return { vexp };
    }
  }
  return {};
}

std::vector<FObjectExport*> FPackage::GetExportObject(const FName& name)
{
  std::vector<FObjectExport*> result;
  std::vector<FObjectExport*> exports = Exports;
  for (FObjectExport* exp : exports)
  {
    if (exp->GetObjectName() == name)
    {
      result.emplace_back(exp);
    }
  }
  for (VObjectExport* vexp : VExports)
  {
    if (vexp->GetObjectName() == name)
    {
      result.emplace_back(vexp);
    }
  }
  return result;
}

void FPackage::MarkDirty(bool dirty)
{
  if (dirty)
  {
    Summary.PackageFlags |= PKG_Dirty;
  }
  else
  {
    Summary.PackageFlags &= ~PKG_Dirty;
  }
}

FObjectImport* FPackage::GetImportObject(const FString& objectName, const FString& className) const
{
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetObjectName() == objectName && imp->GetClassName() == className)
    {
      return imp;
    }
  }
  return nullptr;
}

bool FPackage::AddImport(UObject* object, FObjectImport*& output)
{
  if (!object || !object->GetPackage())
  {
    output = nullptr;
    return false;
  }
  if (object->GetPackage() == this)
  {
    LogE("Can't import resource! Source and destination are the same package.");
    output = nullptr;
    return false;
  }

  FObjectImport* outerPackage = nullptr;
  FString outerPackageName = object->GetPackage()->GetPackageName();
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetObjectName() == object->GetObjectName() &&
      imp->GetClassName() == object->GetClassName() &&
      imp->GetObjectPath() == object->GetObjectPath())
    {
      output = imp;
      if (!ImportObjects[imp->ObjectIndex])
      {
        ImportObjects[imp->ObjectIndex] = object;
        RetainPackage(object->GetPackage()->Ref());
      }
      return false;
    }
    if (imp->GetObjectName() == outerPackageName && imp->GetClassName() == NAME_Package)
    {
      outerPackage = imp;
    }
  }

  std::vector<UObject*> pathChain;
  {
    UObject* outer = object->GetOuter();
    while (outer)
    {
      pathChain.insert(pathChain.begin(), outer);
      outer = outer->GetOuter();
    }
  }

  if (!outerPackage)
  {
    FObjectImport* importObject = new FObjectImport(this, outerPackageName);
    importObject->ClassName.SetPackage(this);
    importObject->ClassName.SetString(NAME_Package);
    importObject->ClassPackage.SetPackage(this);
    importObject->ClassPackage.SetString("Core");
    importObject->OuterIndex = 0;
    Imports.push_back(importObject);
    importObject->ObjectIndex = -(PACKAGE_INDEX)Imports.size();
    RootImports.push_back(importObject);
    outerPackage = importObject;
    MarkDirty();
    LogI("Added import: %s", importObject->GetFullObjectName().UTF8().c_str());
    for (FPackageObserver* observer : Observers)
    {
      observer->OnImportAdded(importObject);
    }

    GetNameIndex(outerPackageName, true);
  }

  FObjectImport* outerImp = outerPackage;
  for (UObject* obj : pathChain)
  {
    obj->Load();
    if (!obj->GetClass())
    {
      LogE("Can't import %s! Outer object %s has no UClass!", object->GetObjectNameString().UTF8().c_str(), obj->GetObjectNameString().UTF8().c_str());
      output = nullptr;
      return false;
    }

    FObjectImport* imp = nullptr;
    bool added = AddImport(obj, imp);
    if (!imp)
    {
      output = nullptr;
      return false;
    }

    if (added && outerImp && std::find(outerImp->Inner.begin(), outerImp->Inner.end(), imp) == outerImp->Inner.end())
    {
      outerImp->Inner.push_back(imp);
      imp->OuterIndex = outerImp->ObjectIndex;
    }
    outerImp = imp;
  }

  FObjectImport* importObject = new FObjectImport(this, object->GetObjectNameString());
  importObject->ClassName.SetPackage(this);
  importObject->ClassName.SetString(object->GetClassNameString());
  importObject->ClassPackage.SetPackage(this);
  importObject->ClassPackage.SetString(object->GetClass() ? object->GetClass()->GetPackage()->GetPackageName() : "Core");

  GetNameIndex(object->GetObjectNameString(), true);
  GetNameIndex(object->GetClassNameString(), true);
  GetNameIndex(importObject->ClassPackage.String(), true);

  if (outerImp)
  {
    outerImp->Inner.push_back(importObject);
    importObject->OuterIndex = outerImp->ObjectIndex;
  }
  
  for (FPackageObserver* observer : Observers)
  {
    observer->OnImportAdded(importObject);
  }
  Imports.push_back(importObject);
  importObject->ObjectIndex = -(PACKAGE_INDEX)Imports.size();
  output = importObject;
  ImportObjects[importObject->ObjectIndex] = object;
  RetainPackage(object->GetPackage()->Ref());
  MarkDirty();
  LogI("Added import: %s", importObject->GetFullObjectName().UTF8().c_str());
  return true;
}

FObjectExport* FPackage::AddExport(const FString& inObjectName, const FString& objectClass, FObjectExport* parent)
{
  if (inObjectName.Empty() || objectClass.Empty())
  {
    return nullptr;
  }
  FString objectName = inObjectName;
  if (objectName.Back())
  {
    // Add null-terminator
    objectName.Resize(objectName.Size() + 1);
  }

  UClass* cls = GetClass(objectClass);
  if (!cls)
  {
    LogE("Failed to create object. Couldn't get the class %s!", objectClass.UTF8().c_str());
    return nullptr;
  }
  FObjectImport* clsImp = nullptr;
  AddImport(cls, clsImp);
  GetNameIndex(objectName, true);
  FObjectExport* exp = new FObjectExport(this, objectName);
  exp->ExportFlags = EF_None;
  exp->ObjectFlags = RF_Public | RF_LoadForServer | RF_LoadForClient | RF_LoadForEdit;
  exp->ClassIndex = clsImp->ObjectIndex;
  NET_INDEX netIndex = INDEX_NONE; 
  Exports.emplace_back(exp);
  Summary.Generations.front().Exports++;
  if (GetFileVersion() > VER_TERA_CLASSIC)
  {
    Depends.emplace_back();
  }
  else
  {
    netIndex = (NET_INDEX)Summary.Generations.front().NetObjects;
    Summary.Generations.front().NetObjects++;
    if (GetPackageFlag(PKG_NoSource) && Depends.empty())
    {
      Depends.emplace_back();
    }
    Depends.front().emplace_back(0);
  }
  exp->ObjectIndex = (PACKAGE_INDEX)Exports.size();

  if (parent)
  {
    parent->Inner.emplace_back(exp);
    exp->Outer = parent;
    exp->OuterIndex = parent->ObjectIndex;
  }
  else
  {
    RootExports.emplace_back(exp);
    exp->OuterIndex = 0;
  }

  for (FPackageObserver* observer : Observers)
  {
    observer->OnExportAdded(exp);
  }

  UObject* object = UObject::Object(exp);
  object->SetClass(cls);
  object->MarkDirty();
  object->SetLoaded(true);
  object->SetNetIndex(netIndex);
  {
    std::scoped_lock<std::mutex> l(ExportObjectsMutex);
    ExportObjects[exp->ObjectIndex] = object;
  }
  LogI("Added export: %s", object->GetFullObjectName().UTF8().c_str());
  if (Referencer)
  {
    Referencer->AddObject(object);
  }
  return exp;
}

FObjectExport* FPackage::DuplicateExport(FObjectExport* source, FObjectExport* dest, const FString& objectName)
{
  if (!source)
  {
    return nullptr;
  }
  return DuplicateExportRecursivly(source, dest, objectName);
}

void FPackage::RemoveExport(FObjectExport* exp)
{
  if (!exp)
  {
    return;
  }
  // TODO: remove Class if necessary
  // TODO: remove Name
  delete ExportObjects[exp->ObjectIndex];
  ExportObjects.erase(exp->ObjectIndex);
  if (Referencer)
  {
    Referencer->RemoveExport(exp);
  }

  if (!exp->OuterIndex)
  {
    RootExports.erase(std::remove(RootExports.begin(), RootExports.end(), exp), RootExports.end());
  }
  else
  {
    exp->Outer->Inner.erase(std::remove(exp->Outer->Inner.begin(), exp->Outer->Inner.end(), exp), exp->Outer->Inner.end());
  }
  Exports.erase(std::remove(Exports.begin(), Exports.end(), exp), Exports.end());
  if (GetFileVersion() > VER_TERA_CLASSIC)
  {
    Depends.erase(Depends.begin() + exp->ObjectIndex - 1);
  }
  else
  {
    Depends.front().erase(Depends.front().begin() + exp->ObjectIndex - 1);
  }
  for (FPackageObserver* observer : Observers)
  {
    observer->OnExportRemoved(exp->ObjectIndex);
  }
  delete exp;
}

void FPackage::ConvertObjectToRedirector(UObject*& source, UObject* targer)
{
  UClass* cls = GetClass(UObjectRedirector::StaticClassName());
  
  FObjectImport* imp = nullptr;
  AddImport(cls, imp);
  
  FObjectExport* exp = source->GetExportObject();
  exp->ClassIndex = imp->ObjectIndex;
  exp->ExportFlags = EF_None;
  exp->ObjectFlags = RF_Public | RF_LoadForServer | RF_LoadForClient | RF_LoadForEdit | RF_Standalone;
  exp->SerialSize = 0x10;

  free(ExportObjects[source->GetExportObject()->ObjectIndex]);
  ExportObjects[exp->ObjectIndex] = UObject::Object(exp);

  source = ExportObjects[exp->ObjectIndex];
  UObjectRedirector* redirector = (UObjectRedirector*)source;
  redirector->Loaded = true;

  imp = nullptr;
  AddImport(targer, imp);
  
  redirector->Object = targer;
  redirector->ObjectRefIndex = imp->ObjectIndex;

  FPropertyTag* tag = new FPropertyTag(redirector);
  tag->Name.SetPackage(this);
  tag->Name = NAME_None;
  redirector->AddProperty(tag);
  
  redirector->MarkDirty();
}

void FPackage::ObjectChanged(UObject* object)
{
  if (!object)
  {
    return;
  }
  if (object->GetPackage() != this)
  {
    object->GetPackage()->ObjectChanged(object);
    return;
  }
  for (FPackageObserver* observer : Observers)
  {
    observer->OnObjectDirty(object->GetExportObject());
  }
}

void FPackage::RetainPackage(std::shared_ptr<FPackage> package)
{
  std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
  for (const auto& test : ExternalPackages)
  {
    if (test.get() == package.get())
    {
      return;
    }
  }
  ExternalPackages.push_back(package);
}

void FPackage::_DebugDump() const
{
}

UObject* FPackage::GetCachedExportObject(PACKAGE_INDEX index) const
{
  std::scoped_lock<std::mutex> l(ExportObjectsMutex);
  if (ExportObjects.count(index))
  {
    return ExportObjects.at(index);
  }
  return nullptr;
}

UObject* FPackage::GetCachedImportObject(PACKAGE_INDEX index) const
{
  std::scoped_lock<std::mutex> l(ImportObjectsMutex);
  if (ImportObjects.count(index))
  {
    return ImportObjects.at(index);
  }
  return nullptr;
}

UObject* FPackage::SetCachedExportObject(PACKAGE_INDEX index, UObject* obj)
{
  std::scoped_lock<std::mutex> l(ExportObjectsMutex);
  ExportObjects[index] = obj;
  return obj;
}

UObject* FPackage::SetCachedImportObject(PACKAGE_INDEX index, UObject* obj)
{
  std::scoped_lock<std::mutex> l(ImportObjectsMutex);
  ImportObjects[index] = obj;
  return obj;
}

std::vector<FString> FPackageDumpHelper::GetGpkPools()
{
  std::vector<FString> result;
  result.reserve(FPackage::CompositPackageList.size());
  for (const auto& p : FPackage::CompositPackageList)
  {
    if (p.first == TMM_Marker)
    {
      continue;
    }
    result.emplace_back(p.first);
  }
  return result;
}

FString FPackageDumpHelper::GetPoolPath(const FString& item)
{
  FString result = FPackage::RootDir.FStringByAppendingPath("CookedPC");
  return result.FStringByAppendingPath(item) + ".gpk";
}

std::vector<FString> FPackageDumpHelper::GetPoolItems(const FString& pool)
{
  return FPackage::CompositPackageList.at(pool);
}

void FPackageDumpHelper::GetPoolItemInfo(const FString& item, bool fast, FStream& s, CompositeDumpEntry& output)
{
  const FCompositePackageMapEntry& streamInfo = FPackage::CompositPackageMap.at(item);
  output.ObjectPath = streamInfo.ObjectPath;
  
  FStream* stream = nullptr;
  {
    void* tmpData = malloc(streamInfo.Size);
    s.SetPosition(streamInfo.Offset);
    s.SerializeBytes(tmpData, streamInfo.Size);
    stream = new MReadStream(tmpData, true, streamInfo.Size);
  }

  FPackageSummary sum;
  *stream << sum;
  sum.PackageName = item;
  FPackage pkg(sum);

  FStream* tempStream = nullptr;
  if (sum.CompressedChunks.size())
  {
    FILE_OFFSET startOffset = INT_MAX;
    FILE_OFFSET totalDecompressedSize = 0;
    void** compressedChunksData = new void* [sum.CompressedChunks.size()];
    {
      uint32 idx = 0;
      for (FCompressedChunk& chunk : sum.CompressedChunks)
      {
        compressedChunksData[idx] = malloc(chunk.CompressedSize);
        stream->SetPosition(chunk.CompressedOffset);
        stream->SerializeBytes(compressedChunksData[idx], chunk.CompressedSize);
        totalDecompressedSize += chunk.DecompressedSize;
        if (chunk.DecompressedOffset < startOffset)
        {
          startOffset = chunk.DecompressedOffset;
        }
        idx++;
      }
    }

    tempStream = new MWriteStream(nullptr, 0);

    sum.OriginalCompressionFlags = sum.CompressionFlags;
    sum.PackageFlags &= ~PKG_StoreCompressed;
    sum.CompressionFlags = COMPRESS_None;

    std::vector<FCompressedChunk> chunks;
    std::swap(sum.CompressedChunks, chunks);
    auto tmpSize = sum.SourceSize;
    (*tempStream) << sum;
    sum.SourceSize = tmpSize;

    uint8* decompressedData = (uint8*)malloc(totalDecompressedSize);
    auto decompressFunc = [&](size_t idx) {
      const FCompressedChunk& chunk = chunks[idx];
      uint8* dst = decompressedData + chunk.DecompressedOffset - startOffset;
      LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
    };
    try
    {
#if ALLOW_CONCURRENT_LZO
      concurrency::parallel_for(size_t(0), size_t(chunks.size()), [&](size_t idx) {
        decompressFunc(idx);
      });
#else
      for (size_t idx = 0; idx < chunks.size(); ++idx) {
        decompressFunc(idx);
      }
#endif
    }
    catch (...)
    {
      for (size_t idx = 0; idx < chunks.size(); ++idx)
      {
        free(compressedChunksData[idx]);
      }
      delete[] compressedChunksData;
      free(decompressedData);
      delete tempStream;
      UThrow("Failed to decompress GPK!");
    }
    for (size_t idx = 0; idx < chunks.size(); ++idx)
    {
      free(compressedChunksData[idx]);
    }
    delete[] compressedChunksData;
    tempStream->SerializeBytes(decompressedData, totalDecompressedSize);
    free(decompressedData);

    delete stream;
    {
      void* tmpData = malloc(((MWriteStream*)tempStream)->GetSize());
      std::memcpy(tmpData, ((MWriteStream*)tempStream)->GetAllocation(), ((MWriteStream*)tempStream)->GetSize());
      stream = new MReadStream(tmpData, true, ((MWriteStream*)tempStream)->GetSize());
      delete tempStream;
    }
  }

  stream->SetPackage(&pkg);
  pkg.Load(*stream);

  for (FObjectExport* exp : pkg.Exports)
  {
    if (fast && ((exp->Outer && exp->Outer->GetClassName() != NAME_Package) || exp->GetClassName() == NAME_Package))
    {
      // Skip all components and packages
      continue;
    }
    CompositeDumpEntry::CompositeDumpExport& e = output.Exports.emplace_back();
    e.Index = exp->ObjectIndex;
    e.ObjectName = exp->GetObjectNameString();
    e.ClassName = exp->GetClassNameString();
    e.Path = exp->GetObjectPath();
  }

  delete stream;
}
