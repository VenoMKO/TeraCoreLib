#pragma once
// --------------------------------------------------------------------
// Build configuration
// --------------------------------------------------------------------

// Enables performance samples in Release builds
#define ENABLE_PERF_SAMPLE 0

// If disabled names won't show FName numbers
// and may cause bugs related to objects search
#define USE_FNAME_NUMBERS 1

// Static DC allows almost instant serialization
// and avoids any unnecessary memory allocations
#define USE_STATIC_DC_4_EXPORT 1

// Cache contents of the S1Game folder
#define CACHE_S1GAME_CONTENTS 0

// Currently gives no benefits. May be used in the future. 
#define ADVANCED_TERRAIN_SERIALIZATION 0

// Use LzoPro instead of MiniLzo
#define USE_LZOPRO 1

// LzoPro compression level 0 - 10(low compression(fast) - high compression(slow))
#define LZOPRO_COMPRESSION_LEVEL 10

// Allow concurrent compression/decompression
#define ALLOW_CONCURRENT_LZO 1

// Compress texture MipMaps
#define TEXTURE2D_COMPRESSION 1

// Add NULL-terminator to new FNameEntries
#define TERMINATE_NEW_NAMES 1

// Add NULL-terminator to existing FNameEntries on edit
#define TERMINATE_EXISTING_NAMES 1

// Load minimum class packages to improve load time.
// Must not be used in Release builds!
#define MINIMAL_CORE _DEBUG

// For testing only.
// GPU buffer has lower quality due to packed positions and half precision UVs,
// so its better to use CPU buffer instead. (Tera uses GPU buffer to render models)
#define USE_GPU_VERTEX_BUFFER 0

// Build and save SkelMesh's raw points index buffers
#define SAVE_RAWINDICES 0

// Allows mod compression
#define USE_MOD_COMPRESSION 1

// Parallel MOD compression
#define CONCURRENT_MOD_COMPRESSION NDEBUG

// Enable concurrent *.u serialization
#define MULTITHREADED_CLASS_SERIALIZATION 1

// For testing and debugging. Allows to turn off properties.
#define SERIALIZE_PROPERTIES 1

// Allows to disable modern TMM file format
#define USE_LEGACY_FILEMOD_VER 1

// Allows UDK files
#define ALLOW_LATEST_UDK 0


// --------------------------------------------------------------------
// Optional components
// --------------------------------------------------------------------

// Enable DC key tool
#ifndef DC_KEY_TOOL
#define DC_KEY_TOOL 1
#endif

// Enable MeshUtils
#ifndef MESH_UTILS
#define MESH_UTILS 1
#endif

// Enable T3DUtils
#ifndef T3D_UTILS
#define T3D_UTILS 1
#endif

// Enable TextureUtils
#ifndef TEXTURE_UTILS
#define TEXTURE_UTILS 1
#endif

// Enable OpenAL Device
#ifndef ALAUDIO_DEVICE
#define ALAUDIO_DEVICE 1
#endif

#include <Tera/CoreDebug.h>
#include <Tera/CoreTypes.h>
#include <Tera/CoreVersion.h>
#include <Tera/CoreStrings.h>

FString ObjectFlagsToString(uint64 flags);
FString ExportFlagsToString(uint32 flags);
FString PixelFormatToString(uint32 pf);
FString PackageFlagsToString(uint32 flags);
FString ClassFlagsToString(uint32 flags);
FString TextureCompressionSettingsToString(uint8 flags);

// Check if the CPU has AVX2 instructions set. Mandatory for TGA and PNG export/import
bool HasAVX2();

// Check if RE needs elevation
bool NeedsElevation();

// Generic runtime errors
void UThrow(const char* fmt, ...);
void UThrow(const wchar* fmt, ...);

FString GetTempDir(const char* appName = nullptr);
FString GetTempFilePath(const char* appName = nullptr);
// Get file's last modification date
uint64 GetFileTime(const std::wstring& path);

#ifndef M_PI
#define _USE_MATH_DEFINES
#include <cmath>
#endif
