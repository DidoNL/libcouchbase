#
#     Copyright 2010, 2011 Couchbase, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
AC_PREREQ(2.60)
m4_include([m4/version.m4])
AC_INIT(libcouchbase, VERSION_NUMBER, support@couchbase.com)
AC_CONFIG_SRCDIR([src/internal.h])
AC_CONFIG_AUX_DIR(config)
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE(subdir-objects foreign)
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LN_S
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

# the malloc tests seems to be broken for cross compilation.. ignore them
ac_cv_func_malloc_0_nonnull=yes
ac_cv_func_realloc_0_nonnull=yes

COUCHBASE_GENERIC_COMPILER(c99)
AM_CPPFLAGS="$AM_CPPFLAGS -I\${top_srcdir}/src -I\${top_srcdir}/contrib"
CHANGESET=GIT_CHANGESET
AC_SUBST(CHANGESET)

#
# To ensure that we don't break applications in mysterious ways we need
# to version our library. For a detailed description, check out:
# http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
#
LIBCOUCHBASE_API_CURRENT=6
LIBCOUCHBASE_API_REVISION=0
LIBCOUCHBASE_API_AGE=0
AC_SUBST(LIBCOUCHBASE_API_CURRENT)
AC_SUBST(LIBCOUCHBASE_API_REVISION)
AC_SUBST(LIBCOUCHBASE_API_AGE)

# The actual version numbers for the developers ;)
LIBCOUCHBASE_VERSION="1.1.0dp7"
LIBCOUCHBASE_VERSION_HEX=`echo ${LIBCOUCHBASE_VERSION} | awk -F. '{printf "0x%0.2d%0.2d%0.2d", $1, $2, $3}'`
AC_SUBST(LIBCOUCHBASE_VERSION)
AC_SUBST(LIBCOUCHBASE_VERSION_HEX)


AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(dlopen, dl)

AC_PATH_PROG(JAVA, java, no)
AC_PATH_PROG(WGET, wget, no)
AC_PATH_PROG(CURL, curl, no)

AS_IF([test "x${WGET}" != "xno"],
      [DOWNLOAD="${WGET} -q -O"],
      [AS_IF([test "x${CURL}" != "xno"],
             [DOWNLOAD="${CURL} -s -L -o"],
             [AC_MSG_ERROR([You need wget or curl in path to download the headers.])])])

AC_SUBST(DOWNLOAD)

AC_ARG_WITH([couchbasemock],
    [AC_HELP_STRING([--with-couchbasemock], [Use the specified file as the test mock server])],
    [
        AS_IF([test -f ${withval}],
              [
                    rm -f tests/CouchbaseMock.jar
                    ln -s ${withval} tests/CouchbaseMock.jar
              ],
              [AC_MSG_ERROR([${withval} is not a file I can use as a mock server])])
    ]
)

AC_CHECK_HEADERS_ONCE([mach/mach_time.h sys/socket.h sys/time.h
                       netinet/in.h inttypes.h netdb.h unistd.h
                       ws2tcpip.h winsock2.h libvbucket/vbucket.h
                       event.h stdint.h sys/types.h])

AS_IF([test "x$ac_cv_header_stdint_h" != "xyes"],
      [AC_MSG_ERROR(Failed to locate stdint.h)])

AS_IF([test "x$ac_cv_header_libvbucket_vbucket_h" != "xyes"],
      [AC_MSG_ERROR(Failed to locate libvbucket/vbucket.h)])

dnl Check that we're able to find a usable libvbucket
AC_CACHE_CHECK([for libvbucket >= 1.8.0.2], [ac_cv_have_libvbucket],
  [ saved_libs="$LIBS"
    LIBS=-lvbucket
    AC_TRY_LINK([
      #include <libvbucket/vbucket.h>
            ],[
      VBUCKET_CONFIG_HANDLE config = vbucket_config_create();
            ],[
      ac_cv_have_libvbucket="yes"
            ], [
      ac_cv_have_libvbucket="no"
      ])
    LIBS="$saved_libs"
  ])

AS_IF([test "x${ac_cv_have_libvbucket}" != "xyes" -o "x$ac_cv_header_libvbucket_vbucket_h" != "xyes"],
      [AC_MSG_ERROR(Failed to locate libvbucket >= 1.8.0.2)])

dnl Check that we can use winsock2 plugin
AC_CACHE_CHECK([for winsock2], [ac_cv_have_winsock2],
  [ saved_libs="$LIBS"
    LIBS=-lws2_32
    AC_TRY_LINK([
      #include <winsock2.h>
      #include <ws2tcpip.h>
            ],[
      DWORD error = WSAGetLastError();
            ],[
      ac_cv_have_winsock2="yes"
            ], [
      ac_cv_have_winsock2="no"
      ])
    LIBS="$saved_libs"
  ])
AM_CONDITIONAL(HAVE_WINSOCK2, [test "$ac_cv_have_winsock2" = "yes"])

dnl Check that we've got a usable libevent
AC_CACHE_CHECK([for libevent >= 1.4], [ac_cv_have_libevent],
  [ saved_libs="$LIBS"
    LIBS=-levent
    AC_TRY_LINK([
      #include <sys/types.h>
      #include <event.h>
            ],[
      event_base_new();
            ],[
      ac_cv_have_libevent="yes"
            ], [
      ac_cv_have_libevent="no"
      ])
    LIBS="$saved_libs"
  ])
AM_CONDITIONAL(HAVE_LIBEVENT, [test "x${ac_cv_have_libevent}" = "xyes"])

dnl Fail if libevent is the only networking backend accessible
AS_IF([test "x${ac_cv_have_winsock2}" != "xyes" -a \( "x${ac_cv_have_libevent}" != "xyes" -o "x$ac_cv_header_event_h" != "xyes" \)],
      [AC_MSG_ERROR(Failed to locate libevent)])

AC_CACHE_CHECK([for libevent2], [ac_cv_have_libevent2],
  [ saved_libs="$LIBS"
    LIBS=-levent
    AC_TRY_LINK([
      #include <sys/types.h>
      #include <event2/event.h>
            ],[
      event_new(NULL, 0, 0, NULL, NULL);
            ],[
      ac_cv_have_libevent2="yes"
            ], [
      ac_cv_have_libevent2="no"
      ])
    LIBS="$saved_libs"
  ])
AM_CONDITIONAL(HAVE_LIBEVENT2, [test "x${ac_cv_have_libevent2}" = "xyes"])
AS_IF([test "x${ac_cv_have_libevent2}" = "xyes"],
      [AC_DEFINE([HAVE_LIBEVENT2], [1], [We have libevent2])])

dnl Check if we've got libyajl2
AC_CACHE_CHECK([for libyajl], [ac_cv_have_libyajl2],
  [ saved_libs="$LIBS"
    LIBS=-lyajl
    AC_TRY_LINK([
      #include <yajl/yajl_parse.h>
      #include <yajl/yajl_tree.h>
            ],[
      yajl_handle parser = yajl_alloc(0, 0, 0);
      yajl_val json = yajl_tree_parse(0, 0, 0);
            ],[
      ac_cv_have_libyajl2="yes"
            ], [
      ac_cv_have_libyajl2="no"
      ])
    LIBS="$saved_libs"
  ])
AM_CONDITIONAL(HAVE_LIBYAJL2, [test "x${ac_cv_have_libyajl2}" = "xyes"])
AS_IF([test "x${ac_cv_have_libyajl2}" = "xyes"],
      [AC_DEFINE([HAVE_LIBYAJL2], [1], [We have libyajl2])])

AC_ARG_ENABLE([system-libsasl],
    [AS_HELP_STRING([--enable-system-libsasl],
                    [Use libsasl installed on the system
                    @<:@default=off@:>@])],
    [ac_cv_system_libsasl="$enableval"])

AS_IF([test "x${ac_cv_system_libsasl}" = "xyes"],
      [
         LIBSASL=-lsasl
         dnl Check for sasl
         AC_CACHE_CHECK([for libsasl], [ac_cv_have_libsasl],
           [ saved_libs="$LIBS"
             LIBS=-lsasl
             AC_TRY_LINK([
               #include <stdlib.h>
               #include <sasl/sasl.h>
                     ],[
               sasl_client_init(NULL);
                     ],[
               ac_cv_have_libsasl="yes"
                     ], [
               ac_cv_have_libsasl="no"
               ])
             LIBS="$saved_libs"
           ])

         AS_IF([test "$ac_cv_have_libsasl" = no], [
           AC_CACHE_CHECK([for libsasl2], [ac_cv_have_libsasl2],
             [ saved_libs="$LIBS"
               LIBS=-lsasl2
               AC_TRY_LINK([
                 #include <stdlib.h>
                 #include <sasl/sasl.h>
                       ],[
                 sasl_client_init(NULL);
                       ],[
                 ac_cv_have_libsasl2="yes"
                       ], [
                 ac_cv_have_libsasl2="no"
                 ])
               LIBS="$saved_libs"
             ])
           AS_IF([test "x${ac_cv_have_libsasl2}" = "xno"],
               [AC_MSG_ERROR(Failed to locate a usable sasl library)])
           LIBSASL=-lsasl2
         ])
         AC_DEFINE([HAVE_SYSTEM_LIBSASL], [1], [Use system internal sasl])
      ])

AM_CONDITIONAL(HAVE_SYSTEM_LIBSASL,
               [test "x${ac_cv_system_libsasl}" = "xyes"])

AC_SUBST(LIBSASL)

AC_CACHE_CHECK([for htonll], [ac_cv_have_htonll],
    [AC_TRY_LINK([
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
                 ], [
return htonll(0);
                 ],
                 [ ac_cv_have_htonll=yes ],
                 [ ac_cv_have_htonll=no ])
])
AS_IF([test "x$ac_cv_have_htonll" = "xyes"],[
       AC_DEFINE([HAVE_HTONLL], [1], [Have ntohll])])

AC_ARG_ENABLE([couchbasemock],
    [AS_HELP_STRING([--disable-couchbasemock],
            [Build tests that use couchbasemock. @<:@default=on@:>@])],
    [ac_cv_enable_couchbasemock="$enableval"],
    [ac_cv_enable_couchbasemock="yes"])

AS_IF([test "x${ac_cv_enable_couchbasemock}" = "xyes"],
      [AS_IF([test "x${DOWNLOAD}" = "xno"],
             [AC_MSG_ERROR([You need wget or curl in path to download the mock server. Use --disable-couchbasemock to avoid downloading of the mock server])])
       AS_IF([test "x$JAVA" = "xno"],
             [AC_MSG_ERROR([You need java in path to run the mock server.
Try add --disable-couchbasemock
])])])

AM_CONDITIONAL(HAVE_COUCHBASEMOCK, [test "x${ac_cv_enable_couchbasemock}" = "xyes"])

AC_ARG_ENABLE([tools],
    [AS_HELP_STRING([--disable-tools],
            [Build tools. @<:@default=on@:>@])],
    [ac_cv_enable_tools="$enableval"],
    [ac_cv_enable_tools="yes"])

AS_IF([test "x${ac_cv_enable_tools}" = "xyes"],
      [AS_IF([test "x${ac_cv_prog_cxx_works}" != "xyes"],
             [AC_MSG_ERROR([You need C++ compiler to build tools. Try add --disable-tools])])])

AM_CONDITIONAL(BUILD_TOOLS, [test "x${ac_cv_enable_tools}" = "xyes"])

AC_CHECK_FUNCS_ONCE(gethrtime clock_gettime gettimeofday QueryPerformanceCounter)
AM_CONDITIONAL(HAVE_GETHRTIME, [test "x${ac_cv_func_gethrtime}" = "xyes"])

AC_ARG_ENABLE([embed-libevent-plugin],
    [AS_HELP_STRING([--enable-embed-libevent-plugin],
                    [embed libevent plugin into libcouchbase rather
                     than dynamically loading it at runtime.
                    @<:@default=off@:>@])],
    [ac_cv_embed_libevent_plugin="$enableval"])

AM_CONDITIONAL(LIBCOUCHBASE_LIBEVENT_PLUGIN_EMBED,
               [test "x${ac_cv_embed_libevent_plugin}" = "xyes"])

AC_ARG_ENABLE([trace],
    [AS_HELP_STRING([--enable-trace],
                    [enable trace routines, mainly useful for libcouchbase development
                    @<:@default=off@:>@])],
    [ac_cv_enable_trace="$enableval"])

AM_CONDITIONAL(LIBCOUCHBASE_ENABLE_TRACE,
               [test "x${ac_cv_enable_trace}" = "xyes"])

AS_IF([test "x$GTEST_ROOT" != "x"], [
  AC_CACHE_CHECK([for gtest source], [ac_cv_have_gtest_src], [
    AS_IF([test -f "$GTEST_ROOT/src/gtest-all.cc"],
          [ac_cv_have_gtest_src=yes],
          [ac_cv_have_gtest_src=no]) ])
], [
  ac_cv_have_gtest_src=no
  GTEST_ROOT=.
])

AC_SUBST(GTEST_ROOT)
AS_IF([test "x$ac_cv_have_gtest_src" = "xno"], [
  AC_CACHE_CHECK([for gtest], [ac_cv_have_gtest], [
   AC_LANG_PUSH([C++])
   SAVED_LIBS="$LIBS"
   LIBS="-lgtest"
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
       [
#include "gtest/gtest.h"
       ],
       [
return 0;
       ])],
     [ac_cv_have_gtest=yes],
     [ac_cv_have_gtest=no])
   AC_LANG_POP([C++])
   LIBS="$SAVED_LIBS"
 ])
])

AM_CONDITIONAL(HAVE_GOOGLETEST_SRC, [test "$ac_cv_have_gtest_src" = "yes"])
AM_CONDITIONAL(HAVE_GOOGLETEST, [test "$ac_cv_have_gtest" = "yes" -o \
                                 "$ac_cv_have_gtest_src" = "yes"])

AH_TOP([
#ifndef CONFIG_H
#define CONFIG_H
/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2010, 2011 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*
 * This file is generated by running configure. Any changes you make to this
 * file will be overwritten the next time you run configure. If you want to
 * make permanent changes to the file you should edit configure.ac instead.
 * All platform-specific includes should be placed inside config_static.h
 * to keep the config.h as small as possible. That allows us for easily
 * use another build systems with a poor support for automake (like Windows)
 *
 * @author Trond Norbye
 */
])

AH_BOTTOM([
#include "config_static.h"
#endif
])

dnl ----------------------------------------------------------------------------
AC_CONFIG_FILES([Makefile include/libcouchbase/configuration.h])
AC_OUTPUT
AS_IF([test "$ac_cv_have_gtest" = "no" -a "$ac_cv_have_gtest_src" = "no"],
      [
        echo "*****"
        echo "*"
        echo "*  WARNING: I couldn't find the googletest testsuite framework."
        echo "*           This means you cannot run the test suite to verify"
        echo "*           that the library works as expected."
        echo "*           You should consider installing it from:"
        echo "*           http://code.google.com/p/googletest/"
        echo "*"
        echo "*****"
      ])
