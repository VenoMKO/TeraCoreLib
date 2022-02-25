#pragma once
// Memory
#ifdef _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#ifndef DEBUG_CLIENTBLOCK
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif
#endif

// Programmable breakpoints
#if _DEBUG
#include <intrin.h>
#define DBreak() __debugbreak()
#define DBreakIf(test) if (test) __debugbreak()
#else
#define DBreak()
#define DBreakIf(expr)
#endif