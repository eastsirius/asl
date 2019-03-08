/**
 * @file asldef.hpp
 * @brief 全局定义
 * @author 程行通
 */

#pragma once


#if defined(_WIN32) && !defined(WIN32)
#  define WIN32 _WIN32
#endif

#if defined(_WIN32_WCE) && !defined(WIN32)
#  define WIN32 _WIN32_WCE
#endif

#if defined(__WIN32__) && !defined(WIN32)
#  define WIN32 __WIN32__
#endif

#if defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS _WINDOWS
#endif

#if defined(WIN32) && !defined(WINDOWS)
#  define WINDOWS WIN32
#endif

#if defined(__APPLE__) && !defined(APPLE)
#  define APPLE __APPLE__
#endif

#if defined(__unix__) && !defined(UNIX)
#  define UNIX __unix__
#endif

#if defined(__linux__) && !defined(LINUX)
#  define LINUX __linux__
#endif

#if defined(__ANDROID__) && !defined(ANDROID)
#  define ANDROID __ANDROID__
#endif

#if defined(_DEBUG) && !defined(DEBUG)
#  define DEBUG 1
#endif

#if !defined(DEBUG) && !defined(NDEBUG)
#  define NDEBUG 1
#endif

#if defined(_MSC_VER) && !defined(MSVC)
#  define MSVC _MSC_VER
#endif

#if defined(MSVC) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(MSVC) && !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN	//屏蔽Windows头文件中一些不常用接口，解决Windows.h和WinSock2.h包含顺序导致冲突的问题
#endif

#if defined(MSVC) && !defined(_VARIADIC_MAX)
#  define _VARIADIC_MAX 10
#endif

//----------------------------------------------------------------------
#ifndef WINDOWS
#  define ASLPRINT_INT64 "%lld"
#  define ASLPRINT_INT64_TYPE long long int
#  define ASLPRINT_UINT64 "%llu"
#  define ASLPRINT_UINT64_TYPE long long unsigned int
#else
#  define ASLPRINT_INT64 "%I64d"
#  define ASLPRINT_INT64_TYPE int64_t
#  define ASLPRINT_UINT64 "%I64u"
#  define ASLPRINT_UINT64_TYPE uint64_t
#endif

//----------------------------------------------------------------------
#ifndef ASL_NAMESPACE
#  define ASL_NAMESPACE asl
#endif

#define ASL_SHAREDPTR_DEF2(obj,ptr) typedef std::shared_ptr<obj> ptr
#define ASL_SHAREDPTR_DEF(obj) ASL_SHAREDPTR_DEF2(obj, obj##Ptr_t)
#define ASL_SHAREDPTR_PRE_DEF2(obj,ptr) ASL_SHAREDPTR_DEF2(class obj, ptr)
#define ASL_SHAREDPTR_PRE_DEF(obj) ASL_SHAREDPTR_DEF2(class obj, obj##Ptr_t)

//----------------------------------------------------------------------
#ifndef WINDOWS
#  define ASL_CLASS_EXPORT
#  define ASL_CLASS_IMPORT
#  define ASL_C_API
#  define ASL_CXX_API
#else
#  define ASL_CLASS_EXPORT _declspec(dllexport)
#  define ASL_CLASS_IMPORT _declspec(dllimport)
#  define ASL_C_API extern "C" _declspec(dllexport)
#  define ASL_CXX_API _declspec(dllexport)
#endif

