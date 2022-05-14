#pragma once
#include <Tera/CoreTypes.h>

extern const char* TMM_Marker;
extern const char* TMM_ModPrefix;
extern const char* TMM_ConfigName;
extern const char* TMM_GameConfigName;

#define TMM_TFC_INDEX_MIN 500
#define TMM_TFC_INDEX_MAX 899

#define TMM_MAX_CONTAINER_SIZE 204800

// Get TMM version
void GetTmmVersion(int32& major, int32& minor);
// Get TMM version for the latest VER_FILE_MOD
void GetTargetTmmVersion(int32& major, int32& minor);

#include <filesystem>

std::filesystem::path TMMGetGameConfigPath(const std::filesystem::path& s1game);
std::filesystem::path TMMGetMapperPath(const std::filesystem::path& s1game);
std::filesystem::path TMMGetMapperBackupPath(const std::filesystem::path& s1game);
std::filesystem::path TMMGetModContainerPath(const std::filesystem::path& s1game, const std::wstring& container);
std::filesystem::path TMMGetModTfcPath(const std::filesystem::path& s1game, const std::wstring& tfc);
std::filesystem::path TMMGetModTfcPath(const std::filesystem::path& s1game, int32 index);
std::string TMMGetTfcName(int32 index, bool ext = false);
int32 TMMGetAvailableTfcIndex(const std::filesystem::path& s1game, int32 desiredIndex = 0);