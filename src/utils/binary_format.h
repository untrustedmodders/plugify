#pragma once

/*
 * General byte order swapping functions.
 */
#if PLUGIFY_COMPILER_MSVC
#define	bswap16(x)	_byteswap_ushort(x)
#define	bswap32(x)	_byteswap_ulong(x)
#define	bswap64(x)	_byteswap_uint64(x)
#elif PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
#define	bswap16(x)	__builtin_bswap16(x)
#define	bswap32(x)	__builtin_bswap32(x)
#define	bswap64(x)	__builtin_bswap64(x)
#elif PLUGIFY_COMPILER_INTEL
#define	bswap16(x)	((x >> 8) | (x << 8))
#define	bswap32(x)	_bswap(x)
#define	bswap64(x)	_bswap64(x)
#else
#error "Byteswap not supported on this platform!"
#endif

/*
 * Newer Linux and BSD may already define htole* and htobe* as macros.
 */
#undef htobe16
#undef htobe32
#undef htobe64
#undef htole16
#undef htole32
#undef htole64
#undef be16toh
#undef be32toh
#undef be64toh
#undef le16toh
#undef le32toh
#undef le64toh

/*
 * Host to big endian, host to little endian, big endian to host, and little
 * endian to host byte order functions as detailed in byteorder(9).
 */
#if PLUGIFY_IS_BIG_ENDIAN
#define	htobe16(x)	((x))
#define	htobe32(x)	((x))
#define	htobe64(x)	((x))
#define	htole16(x)	bswap16((x))
#define	htole32(x)	bswap32((x))
#define	htole64(x)	bswap64((x))

#define	be16toh(x)	((x))
#define	be32toh(x)	((x))
#define	be64toh(x)	((x))
#define	le16toh(x)	bswap16((x))
#define	le32toh(x)	bswap32((x))
#define	le64toh(x)	bswap64((x))
#else
#define	htobe16(x)	bswap16((x))
#define	htobe32(x)	bswap32((x))
#define	htobe64(x)	bswap64((x))
#define	htole16(x)	((x))
#define	htole32(x)	((x))
#define	htole64(x)	((x))

#define	be16toh(x)	bswap16((x))
#define	be32toh(x)	bswap32((x))
#define	be64toh(x)	bswap64((x))
#define	le16toh(x)	((x))
#define	le32toh(x)	((x))
#define	le64toh(x)	((x))
#endif // PLUGIFY_IS_BIG_ENDIAN
