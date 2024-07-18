#pragma once

#ifdef __linux__
#include <endian.h>
#endif
#ifdef __FreeBSD__
#include <sys/endian.h>
#endif
#ifdef __NetBSD__
#include <sys/endian.h>
#endif
#ifdef __OpenBSD__
#include <sys/types.h>
#define be16toh(x) betoh16(x)
#define be32toh(x) betoh32(x)
#if PLUGIFY_ARCH_X86 == 64
#define be64toh(x) betoh64(x)
#endif
#endif
#ifdef __APPLE__
#define be16toh(x) ntohs(x)
#define be32toh(x) ntohl(x)
#if PLUGIFY_ARCH_X86 == 64
#define be64toh(x) ntohll(x)
#endif
#define htobe16(x) htons(x)
#define htobe32(x) htonl(x)
#if PLUGIFY_ARCH_X86 == 64
#define htobe64(x) htonll(x)
#endif
#endif
#ifdef _WIN32
#if PLUGIFY_IS_BIG_ENDIAN
#define be16toh(x) (x)
#define be32toh(x) (x)
#if PLUGIFY_ARCH_X86 == 64
#define be64toh(x) (x)
#endif
#define htobe16(x) (x)
#define htobe32(x) (x)
#if PLUGIFY_ARCH_X86 == 64
#define htobe64(x) (x)
#endif
#else
#define be16toh(x) _byteswap_ushort(x)
#define be32toh(x) _byteswap_ulong(x)
#if PLUGIFY_ARCH_X86 == 64
#define be64toh(x) _byteswap_uint64(x)
#endif
#define htobe16(x) _byteswap_ushort(x)
#define htobe32(x) _byteswap_ulong(x)
#if PLUGIFY_ARCH_X86 == 64
#define htobe64(x) _byteswap_uint64(x)
#endif
#endif
#endif