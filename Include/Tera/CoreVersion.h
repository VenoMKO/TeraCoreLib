#pragma once
#define VER_TERA_CLASSIC 610
#define VER_TERA_MODERN 897
#define VER_UDK_LATEST 868

#define VER_ASTELLIA 867

// Current TMM version
#if USE_LEGACY_FILEMOD_VER
#define VER_TERA_FILEMOD 2
#else
#define VER_TERA_FILEMOD 3
#endif

// First release
#define VER_TERA_FILEMOD_INIT 1
// Added ability to embed TFCs into a mod GPK
#define VER_TERA_FILEMOD_ADD_TFC 2
// New mod metadata. Dropped previous TMM compatibility. TMM v.1.20 and lower will show a proper message.
#define VER_TERA_FILEMOD_NEW_META 3

FString GetClientVersionString(const FString& s1data);