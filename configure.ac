# ###### Basic setup ########################################################
AC_INIT(rtpaudio/rtpa-server.cc)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE(rtpaudio, 2.0.0~beta1.3)
AC_PREFIX_DEFAULT(/usr/local)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AM_CONFIG_HEADER(config.h)


# ###### Checks for programs ################################################
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL


# ###### Checks for headers #################################################
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h errno.h)


# ###### Options ############################################################
AC_ARG_ENABLE([qt],
              [  --enable-qt             enable QT-based examples ][default=no]],
              enable_qt=$enableval, enable_qt=no)

AC_ARG_ENABLE([maintainer-mode],
              [  --enable-maintainer-mode enable maintainer mode ][default=yes]],
              enable_maintainer_mode=$enableval, enable_maintainer_mode=yes)


# ###### Check for sin6_scope_id of sockaddr_in6 ############################
if test "$cv_ipv6" = yes; then
   AC_MSG_CHECKING(whether struct sockaddr_in6 has a sin6_scope_id field)
   AC_TRY_COMPILE([#include <sys/types.h>
                   #include <netinet/in.h>],
                  [static struct sockaddr_in6 ac_i;int ac_j = sizeof(ac_i.sin6_scope_id);],
                  [AC_MSG_RESULT(yes)
                   AC_DEFINE(HAVE_SIN6_SCOPE_ID, 1, "Define to 1 if sin6_scope_id is supported.")],
                  AC_MSG_RESULT(no))
fi


# ###### Operating system checks ############################################
case $host_os in
bsdi*)
    CFLAGS="$CFLAGS -DBSDI"
    thread_LIBS="-lpthread"
    CFLAGS="$CFLAGS `glib-config --cflags`"
    LDFLAGS="$LDFLAGS `glib-config --libs`"
    ;;
freebsd*)
    CFLAGS="$CFLAGS -DFreeBSD -D_PTHREADS -pthread -I/usr/local/include -DHAVE_CONNECTX_WITH_ID"
    CXXFLAGS="$CXXFLAGS -DFreeBSD -D_PTHREADS -pthread -I/usr/local/include"
    LDFLAGS="$LDFLAGS -L/usr/local/v6/lib"
    thread_LIBS=""
    ;;
hpux*)
    CFLAGS="$CFLAGS -DSNAKE"
    test -z "$GCC" && TRY_CFLAGS="$TRY_CFLAGS -Wp,-H600000"
    thread_LIBS="-lpthread"
   ;;
linux*)
    CFLAGS="$CFLAGS -DLINUX -std=c99"
    CXXFLAGS="$CXXFLAGS -DLINUX -std=c++98"
    LDFLAGS="$LDFLAGS"
    thread_LIBS="-lpthread"
    ;;
openbsd*)
    LIBS="$LIBS -lcompat"
    thread_LIBS="-lpthread"
    ;;
solaris*)
    CFLAGS="$CFLAGS -D_XPG4_2 -D__EXTENSIONS__ -DSOLARIS -D__sun -DSUN -DSUNOS_5"
    CXXFLAGS="$CXXFLAGS -D_XPG4_2 -D__EXTENSIONS__ -DSOLARIS -D__sun -DSUN -DSUNOS_5"
    thread_LIBS="-lpthread"
    LDFLAGS="$LDFLAGS -lsocket -lnsl -lrt -lresolv"
    ;;
sunos4*)
    CFLAGS="$CFLAGS -DSUN -DSUN4"
    thread_LIBS="-lpthread"
    ;;
darwin*)
    CFLAGS="$CFLAGS -DDARWIN -DUSE_SELECT -DHAVE_CONNECTX_WITH_ID"
    CXXFLAGS="$CXXFLAGS -DDARWIN"
    thread_LIBS="-lpthread"
    ;;
esac
AC_SUBST(thread_LIBS)


# ###### Networking declarations ############################################
AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h)

AC_MSG_CHECKING(for socklen_t)
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <sys/socket.h>],
               [socklen_t x = 1; return 0;],
               [AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(int)
                AC_DEFINE(socklen_t, int, [Define a type for socklen_t.])])

AC_MSG_CHECKING(if stderr is a variable of type FILE* and not a macro)
AC_TRY_COMPILE([#include <stdio.h>],
               [FILE** stdlog = &stderr],
               [AC_DEFINE(HAVE_STDERR_FILEPTR, 1, "Define to 1 if stderr is a pointer.")
                AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(no)])

AC_MSG_CHECKING([whether 'struct sockaddr' has sa_len])
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <sys/socket.h>],
               [static struct sockaddr sa; int i = sizeof(sa.sa_len);],
               [AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_SA_LEN,1,"Define this if you have sa_len in sockaddr struct.")],
                AC_MSG_RESULT(no))

AC_MSG_CHECKING([whether 'struct sockaddr_in' has sin_len])
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <netinet/in.h>],
               [static struct sockaddr_in in; int i = sizeof(in.sin_len);],
               [AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_SIN_LEN,1,"Define this if your IPv4 has sin_len in sockaddr_in struct.")],
                AC_MSG_RESULT(no))

AC_MSG_CHECKING([whether 'struct sockaddr_in6' has sin6_len])
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <netinet/in.h>],
               [static struct sockaddr_in6 in; int i = sizeof(in.sin6_len);],
               [AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_SIN6_LEN,1,"Define this if your IPv6 has sin6_len in sockaddr_in6 struct.")],
                AC_MSG_RESULT(no))

AC_CHECK_FUNCS(socket, , AC_CHECK_LIB(socket, socket))


# ###### Check if kernel SCTP should be used ################################
AC_ARG_ENABLE([kernel-sctp],
              [  --enable-kernel-sctp    use kernel SCTP instead of sctplib][default=yes]],
              enable_kernel_sctp=$enableval, enable_kernel_sctp=yes)


# ###### Check for SCTP implementation ######################################
if test "x$enable_kernel_sctp" = "xno"; then

   # ====== sctplib =========================================================
   ac_sctplib_includes=no ac_sctplib_libraries=no ac_sctplib_bindir=no
   sctplib_libraries=""
   sctplib_includes=""
   AC_ARG_WITH(sctplib,
      [  --with-sctplib=DIR      where the root of sctplib is installed ],
      [  ac_sctplib_includes="$withval"/include
         ac_sctplib_libraries="$withval"/lib
         ac_sctplib_bindir="$withval"/bin
      ])

   if test "$ac_sctplib_libraries" = "no"; then
      ac_sctplib_defaultdir="/usr/local"
      ac_sctplib_includes="$ac_sctplib_defaultdir"/include
      ac_sctplib_libraries="$ac_sctplib_defaultdir"/lib
      ac_sctplib_bindir="$ac_sctplib_defaultdir"/bin
   fi

   if test ! -e "$ac_sctplib_includes/sctp.h" ; then
      AC_MSG_ERROR([No sctplib installation found ($ac_sctplib_includes/sctp.h)!])
   fi
   if test ! -e $ac_sctplib_libraries/libsctplib.a ; then
      AC_MSG_ERROR([No sctplib installation found ($ac_sctplib_libraries/libsctplib.a)!])
   fi

   # ====== socketapi =======================================================
   ac_socketapi_includes=no ac_socketapi_libraries=no ac_socketapi_bindir=no
   socketapi_libraries=""
   socketapi_includes=""
   AC_ARG_WITH(socketapi,
      [  --with-socketapi=DIR    where the root of socketapi is installed ],
      [  ac_socketapi_includes="$withval"/include
         ac_socketapi_libraries="$withval"/lib
         ac_socketapi_bindir="$withval"/bin
      ])

   if test "$ac_socketapi_libraries" = "no"; then
      ac_socketapi_defaultdir="/usr/local"
      ac_socketapi_includes="$ac_socketapi_defaultdir"/include
      ac_socketapi_libraries="$ac_socketapi_defaultdir"/lib
      ac_socketapi_bindir="$ac_socketapi_defaultdir"/bin
   fi

   if test ! -e $ac_socketapi_libraries/libsctpsocket.a ; then
      AC_MSG_ERROR([No socketapi installation found ($ac_socketapi_libraries/libsctpsocket.a)!])
   fi

   # In maintainer mode, use static linking for sctplib/socketapi!
   if test "$enable_maintainer_mode" = "yes"; then
      sctplib_LIBS="$ac_sctplib_libraries/libsctplib.a"
      socketapi_LIBS="$ac_socketapi_libraries/libsctpsocket.a -lstdc++"
   else
      sctplib_LIBS="-L$ac_sctplib_libraries -lsctplib"
      socketapi_LIBS="-L$ac_socketapi_libraries -lsctpsocket -lstdc++"
   fi

   AC_DEFINE(HAVE_SPP_FLAGS, 1, [Define this if you support version 10 if the API ID.])

   # ====== Check for glib ==================================================
   AM_PATH_GLIB_2_0(2.4.0, , AC_MSG_ERROR(GLib installation not found.))

   CFLAGS="$CFLAGS $GLIB_CFLAGS"
   CPPFLAGS="$CPPFLAGS $GLIB_CFLAGS"
   glib_LIBS="$GLIB_LIBS"

   # ====== Version compatibility test ======================================
   oldLIBS=$LIBS
   LIBS="$LIBS $socketapi_LIBS $sctplib_LIBS $glib_LIBS $thread_LIBS"
   AC_MSG_CHECKING([socketapi library version])
   AC_TRY_RUN([
#include <stdio.h>
#include "ext_socket.h"

int main(int argc, char** argv)
{
   unsigned int version = socketAPIGetVersion();
   printf("Version is %u.%04x\n", (version >> 16), version & 0xffff);
   if(version >= 0x21100) {
      return(0);
   }
   return(1);
}
], socketapi_version=good, socketapi_version=bad, socketapi_version=cross)

   case $socketapi_version in
   good)
      ;;
   bad|*)
      AC_MSG_ERROR([You need at least version 2.110 of the socketapi library!])
      ;;
   esac

   AC_DEFINE(HAVE_SCTP_SENDMSG, 1, [Define to 1 if you have the `sctp_sendmsg' function.])
   AC_DEFINE(HAVE_SCTP_CONNECTX, 1, [Define to 1 if you have the `sctp_connectx' function.])
   AC_DEFINE(HAVE_SCTP_SEND, 1, [Define to 1 if you have the `sctp_send' function.])
   AC_DEFINE(HAVE_SCTP_SENDX, 1, [Define to 1 if you have the `sctp_sendx' function.])
   AC_DEFINE(HAVE_SPP_FLAGS, 1, [Define this if you support version 10 if the API ID.])
   AC_DEFINE(HAVE_CONNECTX_WITH_ID, 1, [Define to 1 if sctp_connectx has associd parameter.])
   AC_DEFINE(HAVE_SCTP_DELAYED_SACK, 1, [Define to 1 if SCTP_DELAYED_SACK with sctp_sack_info is available.])

   LIBS=$oldLIBS
   # ========================================================================

else
   # ====== Kernel SCTP =====================================================
   oldLIBS=$LIBS
   AC_CHECK_FUNCS(sctp_sendmsg, ,  AC_CHECK_LIB(sctp, sctp_sendmsg))
   AC_CHECK_FUNCS(sctp_connectx, , AC_CHECK_LIB(sctp, sctp_connectx))
   AC_CHECK_FUNCS(sctp_send, ,     AC_CHECK_LIB(sctp, sctp_send))
   AC_CHECK_FUNCS(sctp_sendx, ,    AC_CHECK_LIB(sctp, sctp_sendx))
   AC_CHECK_MEMBER(struct sctp_paddrparams.spp_flags,
                   AC_DEFINE(HAVE_SPP_FLAGS, 1, [Define this if you support version 10 if the API ID.]),,
                   [#ifdef HAVE_SYS_TYPES_H
                   #include <sys/types.h>
                   #endif
                   #ifdef HAVE_NETINET_SCTP_H
                   #include <netinet/sctp.h>
                   #endif])
   AC_MSG_CHECKING(whether sctp_connectx has an associd parameter)
   AC_TRY_COMPILE([#include <sys/types.h>
                   #include <netinet/sctp.h>],
                  [sctp_assoc_t id;
                   int result = sctp_connectx(0, NULL, 0, &id);],
                  [AC_MSG_RESULT(yes)
                   AC_DEFINE(HAVE_CONNECTX_WITH_ID, 1, [Define to 1 if sctp_connectx has associd parameter.])],
                  AC_MSG_RESULT(no))
   AC_MSG_CHECKING(whether SCTP_DELAYED_SACK with sctp_sack_info is available)
   AC_TRY_COMPILE([#include <sys/types.h>
                   #include <netinet/sctp.h>],
                  [struct sctp_sack_info sctpSACKInfo;
                   int opt = SCTP_DELAYED_SACK;],
                  [AC_MSG_RESULT(yes)
                   AC_DEFINE(HAVE_SCTP_DELAYED_SACK, 1, [Define to 1 if SCTP_DELAYED_SACK with sctp_sack_info is available.])],
                  AC_MSG_RESULT(no))
   LIBS=$oldLIBS

   # ====== Check for Valgrind ==============================================
   # Needed to avoid false positives for sctp_getladdrs()/sctp_getpaddrs()
   AC_CHECK_HEADERS([valgrind/valgrind.h])
   AC_CHECK_HEADERS([valgrind/memcheck.h])

   # ====== Check for SCTP library linker parameter =========================
   sctplib_LIBS=""
   socketapi_LIBS=""
   sctp_okay=0
   AC_CHECK_LIB(sctp, sctp_sendmsg, [
      socketapi_LIBS="-lsctp"
      sctp_okay=1])
   if test $sctp_okay -ne 1 ; then
      AC_CHECK_LIB(c, sctp_sendmsg, [
         socketapi_LIBS=""
         sctp_okay=1])
   fi

   AC_MSG_CHECKING([whether library parameter for SCTP functions])
   if test $sctp_okay -ne 1 ; then
      AC_MSG_ERROR([Do not know where to find SCTP functions! Missing libsctp?])
   else
      AC_MSG_RESULT([appending "$socketapi_LIBS"])
   fi

   glib_LIBS=""
   CFLAGS="$CFLAGS -DHAVE_KERNEL_SCTP"
   CXXFLAGS="$CXXFLAGS -DHAVE_KERNEL_SCTP"
fi
AC_SUBST(sctplib_LIBS)
AC_SUBST(socketapi_LIBS)
AC_SUBST(glib_LIBS)


# ###### Check for IEEE-754 complient float-point handling ##################
AC_MSG_CHECKING([for floating point type])
AC_TRY_RUN([
#include <stdio.h>

union DoubleIntUnion
{
   double             Double;
   unsigned long long Integer;
};


/*
void hexdump(const void* ptr, const size_t length)
{
   const unsigned char* cptr = (const unsigned char*)ptr;
   size_t               i;

   for(i = 0;i < length;i++) {
      printf("%02x ", cptr[i]);
   }
   puts("");
}
*/

int main ()
{
   union DoubleIntUnion value;

   value.Double = 0.0;

   if(value.Integer != 0ULL) {
      /* puts("Math is non-IEEE (check 1)!\n"); */
      return(1);
   }

   value.Double = 1000.5;
   if(value.Integer != 0x408f440000000000ULL) {
      /* puts("Math is non-IEEE (check 2)!\n"); */
      return(1);
   }

   value.Double = -1000.5;
   if(value.Integer != 0xc08f440000000000ULL) {
      /* puts("Math is non-IEEE (check 3)!\n"); */
      return(1);
   }

   /* puts("Math is IEEE\n"); */
   return(0);
}
], IEEEFP=ok, IEEEFP=broken, IEEEFP=cross)


case $IEEEFP in
ok)
    AC_MSG_RESULT(IEEE)
    AC_DEFINE(HAVE_IEEE_FP, 1, [IEEE compliant float-point handling])
    ;;
cross)
    AC_MSG_RESULT([cross-compiling, assuming IEEE])
    AC_DEFINE(HAVE_IEEE_FP)
    ;;
*|broken)
    AC_MSG_RESULT([non IEEE])
    ;;
esac


# ###### Check if PulseAudio should be used #################################
AC_ARG_ENABLE([pulseaudio],
              [  --enable-pulseaudio    use PulseAudio instead of OSS for audio output][default=yes]],
              enable_pulseaudio=$enableval, enable_pulseaudio=yes)
if test "${enable_pulseaudio}" = "yes"; then
   PULSEAUDIO_REQUIRED_VERSION=0.7
   AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
   AC_MSG_CHECKING(for PulseAudio $PULSEAUDIO_REQUIRED_VERSION support)
   if test x$PKG_CONFIG != xno; then
      if $PKG_CONFIG --atleast-pkgconfig-version 0.7 && $PKG_CONFIG --atleast-version $PULSEAUDIO_REQUIRED_VERSION libpulse-simple; then
         PULSEAUDIO_CFLAGS=`$PKG_CONFIG --cflags libpulse-simple`
         PULSEAUDIO_LIBS=`$PKG_CONFIG --libs libpulse-simple`
         AC_DEFINE(HAVE_PULSEAUDIO, 1, [Use PulseAudio instead of OSS])
         AC_MSG_RESULT(yes)
      else
         AC_MSG_RESULT(no)
         # Skip PulseAudio here -> just use OSS.
         # AC_MSG_ERROR([No PulseAudio installation found! Try to compile with PulseAudio off (--disable-pulseaudio).])
      fi
   fi
else
   PULSEAUDIO_CFLAGS=""
   PULSEAUDIO_LIBS=""
fi
AC_SUBST(PULSEAUDIO_CFLAGS)
AC_SUBST(PULSEAUDIO_LIBS)


# ###### Check for Qt, if it is requested ###################################
AM_CONDITIONAL(BUILD_QT_PROGRAMS, test x$enable_qt = xyes)
if test "${enable_qt}" = "yes"; then
   TD_CHECK_QT4
   AC_DEFINE(ENABLE_QT, 1, [QT is usable])
else
   echo "Skipping checks for QT, since it is not requested."
fi


# ###### colorgcc ###########################################################
AC_ARG_ENABLE([colorgcc],
              [  --enable-colorgcc       enable colorgcc support ][default=yes]],
              enable_colorgcc=$enableval, enable_colorgcc=yes)
if test "${enable_colorgcc}" = "yes"; then
   AC_CHECK_PROG([COLORGCC], [colorgcc], [yes], [no])
   if test "$COLORGCC" == "yes"
   then
      AC_MSG_NOTICE([Compiling using colorgcc])
      CC="colorgcc"
      CXX="colorgcc -lstdc++"
   fi
fi


# ###### Maintainer mode ####################################################
if test "$enable_maintainer_mode" = "yes"; then
   CFLAGS="$CFLAGS -Wall -g3 -O0"
   CXXFLAGS="$CXXFLAGS -Wall -g3 -O0"
else
   CFLAGS="$CFLAGS -Wall -fstack-protector --param=ssp-buffer-size=4"
   CXXFLAGS="$CXXFLAGS -Wall -fstack-protector --param=ssp-buffer-size=4"
   LDFLAGS="$LDFLAGS -Wl,-z,relro"
fi


# ###### Makefiles ##########################################################
AC_OUTPUT(Makefile libmpegsound/Makefile rtpaudio/Makefile )


# ###### Configuration results ##############################################
echo ""
echo "The rsplib package has been configured with the following options:"
echo ""
echo "   Build with Maintainer Mode      : $enable_maintainer_mode"
echo "   Build with Kernel SCTP          : $enable_kernel_sctp"
echo "   Build with Qt-based Programs    : $enable_qt"
echo ""
echo "   sctplib Libraries               : $sctplib_LIBS"
echo "   socketapi Libraries             : $socketapi_LIBS"
echo "   glib Libraries                  : $glib_LIBS"
echo "   Thread Libraries                : $thread_LIBS"
echo "   PulseAudio Libraries            : $PULSEAUDIO_LIBS"
echo ""
echo "   CFLAGS                          : $CFLAGS"
echo "   CXXFLAGS                        : $CXXFLAGS"
echo "   LDFLAGS                         : $LDFLAGS"
echo "   LIBS                            : $LIBS"
echo "   glib_CFLAGS                     : $glib_CFLAGS"
echo ""
echo "   QTDIR                           : $QTDIR"
echo "   MOC                             : $MOC"
echo "   QT_CXXFLAGS                     : $QT_CXXFLAGS"
echo "   QT_LDFLAGS                      : $QT_LDADD"
