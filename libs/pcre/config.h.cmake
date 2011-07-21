/* config.h for CMake builds */

#cmakedefine HAVE_DIRENT_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_INTTYPES_H 1
#cmakedefine HAVE_WINDOWS_H 1

#cmakedefine HAVE_TYPE_TRAITS_H 1
#cmakedefine HAVE_BITS_TYPE_TRAITS_H 1

#cmakedefine HAVE_BCOPY 1
#cmakedefine HAVE_MEMMOVE 1
#cmakedefine HAVE_STRERROR 1
#cmakedefine HAVE_STRTOLL 1
#cmakedefine HAVE_STRTOQ 1
#cmakedefine HAVE__STRTOI64 1

#cmakedefine PCRE_STATIC 1

#cmakedefine SUPPORT_UTF8 1
#cmakedefine SUPPORT_UCP 1
#cmakedefine EBCDIC 1
#cmakedefine BSR_ANYCRLF 1
#cmakedefine NO_RECURSE 1

#cmakedefine HAVE_LONG_LONG 1
#cmakedefine HAVE_UNSIGNED_LONG_LONG 1

#cmakedefine SUPPORT_LIBBZ2 1
#cmakedefine SUPPORT_LIBZ 1
#cmakedefine SUPPORT_LIBREADLINE 1

#define NEWLINE			@NEWLINE@
#define POSIX_MALLOC_THRESHOLD	@PCRE_POSIX_MALLOC_THRESHOLD@
#define LINK_SIZE		@PCRE_LINK_SIZE@
#define MATCH_LIMIT		@PCRE_MATCH_LIMIT@
#define MATCH_LIMIT_RECURSION	@PCRE_MATCH_LIMIT_RECURSION@


#define MAX_NAME_SIZE	32
#define MAX_NAME_COUNT	10000

/* end config.h for CMake builds */
