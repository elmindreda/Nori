
project(PCRE C)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckTypeSize)

check_include_file(dirent.h     HAVE_DIRENT_H)
check_include_file(sys/stat.h   HAVE_SYS_STAT_H)
check_include_file(sys/types.h  HAVE_SYS_TYPES_H)
check_include_file(unistd.h     HAVE_UNISTD_H)
check_include_file(stdint.h     HAVE_STDINT_H)
check_include_file(inttypes.h   HAVE_INTTYPES_H)
check_include_file(windows.h    HAVE_WINDOWS_H)

check_function_exists(bcopy     HAVE_BCOPY)
check_function_exists(memmove   HAVE_MEMMOVE)
check_function_exists(strerror  HAVE_STRERROR)
check_function_exists(strtoll   HAVE_STRTOLL)
check_function_exists(strtoq    HAVE_STRTOQ)
check_function_exists(_strtoi64 HAVE__STRTOI64)

check_type_size("long long"          LONG_LONG)
check_type_size("unsigned long long" UNSIGNED_LONG_LONG)

set(PCRE_LINK_SIZE "2" CACHE STRING
    "Internal link size (2, 3 or 4 allowed).")

set(PCRE_MATCH_LIMIT "10000000" CACHE STRING
    "Default limit on internal looping.")

set(PCRE_MATCH_LIMIT_RECURSION "MATCH_LIMIT" CACHE STRING
    "Default limit on internal recursion.")

set(PCRE_NEWLINE "LF" CACHE STRING
    "What to recognize as a newline (one of CR, LF, CRLF, ANY, ANYCRLF).")

set(PCRE_NO_RECURSE OFF CACHE BOOL
    "If ON, then don't use stack recursion when matching.")

set(PCRE_POSIX_MALLOC_THRESHOLD "10" CACHE STRING
    "Threshold for malloc() usage.")

set(PCRE_SUPPORT_UNICODE_PROPERTIES OFF CACHE BOOL
    "Enable support for Unicode properties. (If set, UTF-8 support will be enabled as well)")

set(PCRE_SUPPORT_UTF8 ON CACHE BOOL
    "Enable support for the Unicode UTF-8 encoding.")

set(NEWLINE "")

if (PCRE_NEWLINE STREQUAL "LF")
  set(NEWLINE "10")
endif()
if (PCRE_NEWLINE STREQUAL "CR")
  set(NEWLINE "13")
endif()
if (PCRE_NEWLINE STREQUAL "CRLF")
  set(NEWLINE "3338")
endif()
if (PCRE_NEWLINE STREQUAL "ANY")
  set(NEWLINE "-1")
endif()
if (PCRE_NEWLINE STREQUAL "ANYCRLF")
  set(NEWLINE "-2")
endif()

if (NEWLINE STREQUAL "")
  message(FATAL_ERROR "The PCRE_NEWLINE variable must be set to one of the following values: \"LF\", \"CR\", \"CRLF\", \"ANY\", \"ANYCRLF\".")
endif()

configure_file(${PCRE_SOURCE_DIR}/config.h.cmake ${PCRE_BINARY_DIR}/config.h)

include_directories(${PCRE_BINARY_DIR})

add_definitions(-DHAVE_CONFIG_H -DPCRE_STATIC)

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif()

add_library(pcre STATIC pcre_chartables.c pcre_compile.c pcre_config.c pcre_dfa_exec.c
                        pcre_exec.c pcre_fullinfo.c pcre_get.c pcre_globals.c
                        pcre_info.c pcre_newline.c pcre_ord2utf8.c pcre_refcount.c
			pcre_study.c pcre_tables.c pcre_try_flipped.c
			pcre_ucp_searchfuncs.c pcre_valid_utf8.c pcre_version.c
                        pcre_xclass.c)

