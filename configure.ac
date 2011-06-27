AC_PREREQ([2.68])
AC_INIT([bmc], [0.0], [http://delysid.org/bmc/])
AC_CONFIG_SRCDIR([brltr.cpp])
BMC_SUMMARY_BEGIN
brltty_source_directory="`dirname "${0}"`"

AC_SUBST([PACKAGE_TITLE], ["`echo "${PACKAGE_NAME}" | sed -e 'y%abcdefghijklmnopqrstuvwxyz%ABCDEFGHIJKLMNOPQRSTUVWXYZ%'`"])
AC_DEFINE_UNQUOTED([PACKAGE_TITLE], ["${PACKAGE_TITLE}"],
                   [Define this to be a string containing the package title.])

AC_SUBST([package_date], ["`date "+%Y/%m/%d"`"])

AC_CONFIG_HEADER([config.h])
AC_PREFIX_DEFAULT([])
AC_CONFIG_AUX_DIR([acdir])
AC_CANONICAL_SYSTEM

BRLTTY_PORTABLE_DIRECTORY([includedir], [/usr])

BMC_SUMMARY_ITEM([sysconfdir], [sysconfdir])

BRLTTY_ARG_REQUIRED(
   [tables-directory], [DIRECTORY],
   [path to directory containing data files],
   [tables_directory], ["${sysconfdir}/brltty"]
)

AC_PROG_MAKE_SET

AC_PROG_CC

if test -z "${CXX}"
then
   if test "${GCC}" = "yes"
   then
      AC_CACHE_CHECK([if the C compiler can compile C++], [brltty_cv_prog_cc_cxx], [dnl
      brltty_save_ac_ext="${ac_ext}"
      ac_ext="cc"
      AC_TRY_LINK([], [class c {};],
                  [brltty_cv_prog_cc_cxx="yes"],
                  [brltty_cv_prog_cc_cxx="no"])
      ac_ext="${brltty_save_ac_ext}"])
      test "${brltty_cv_prog_cc_cxx}" = "yes" && CXX="${CC}"
   fi
fi
AC_PROG_CXX

AC_PROG_INSTALL
BRLTTY_EXECUTABLE_PATH([INSTALL])

case "${host_os}"
in
   mingw*)
      BRLTTY_HAVE_WINDOWS_LIBRARY([ws2_32])
      BRLTTY_HAVE_WINDOWS_LIBRARY([gdi32])
      BRLTTY_HAVE_WINDOWS_LIBRARY([winmm])
      ;;
   cygwin*)
      BRLTTY_HAVE_WINDOWS_LIBRARY([gdi32])
      ;;
   *)
      AC_SEARCH_LIBS([socket], [socket])
      AC_SEARCH_LIBS([inet_ntoa], [nsl])
      ;;
esac

BRLTTY_ARG_DISABLE(
   [i18n],
   [support for internationalization],
   [],
[dnl
   AC_CHECK_HEADER([libintl.h], [dnl
      AC_HAVE_LIBRARY([intl])
      AC_DEFINE([ENABLE_I18N_SUPPORT], [1],
                [Define this if internationalization support is to be included.])
   ])
])

AC_CHECK_FUNC([wcslen], [AC_CHECK_HEADERS([wchar.h], [dnl
   AC_CHECK_FUNCS([wcwidth])
])])

icu_includes=""
icu_libraries=""
BRLTTY_ARG_DISABLE(
   [icu],
   [support for Unicode-based internationalization],
   [],
[dnl
   AC_PATH_PROG([ICU_CONFIG], [icu-config])
   test -n "${ICU_CONFIG}" && ${ICU_CONFIG} --exists && {
      icu_includes="`${ICU_CONFIG} --cppflags-searchpath`"
      icu_libraries="`${ICU_CONFIG} --ldflags`"
      AC_DEFINE_UNQUOTED([HAVE_ICU], [1],
                         [Define this if Unicode-based internationalization support is to be included.])

      if test "${brltty_standalone_programs}" = "yes"
      then
         icu_libraries="`echo ${icu_libraries} | sed -e 's/\(-l\)\(icu\)/\1s\2/g'`"
      fi
   }
])
AC_SUBST([icu_includes])
AC_SUBST([icu_libraries])

BRLTTY_ARG_DISABLE(
   [iconv],
   [support for character encoding conversion],
   [],
[dnl
   AC_CHECK_HEADERS([iconv.h], [
      AC_HAVE_LIBRARY([iconv])
   ])
])


AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/file.h])

AC_CHECK_FUNCS([hstrerror realpath vsyslog])
AC_CHECK_FUNCS([fchdir fchmod])

AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE

AC_CACHE_CHECK(
   [if the compiler understands the __alignof__ operator],
   [brltty_cv_c_operator_alignof],
   [AC_TRY_COMPILE([
int a;
int b = __alignof__(a);
],
[],
[brltty_cv_c_operator_alignof=yes],
[brltty_cv_c_operator_alignof=no])])
if test "${brltty_cv_c_operator_alignof}" = "yes"
then
   AC_DEFINE([HAVE_OPERATOR_ALIGNOF],  [1],
             [Define this if the __alignof__ operator is supported.])
fi

AC_CACHE_CHECK(
   [if the compiler understands __attribute__((format(__printf__)))],
   [brltty_cv_c_attribute_format_printf],
   [AC_TRY_COMPILE([
extern void conf_attribute_format_printf(const char *format, ...)
       __attribute__((format(__printf__, 1, 2)));
],
[],
[brltty_cv_c_attribute_format_printf=yes],
[brltty_cv_c_attribute_format_printf=no])])
if test "${brltty_cv_c_attribute_format_printf}" = "yes"
then
   AC_DEFINE([HAVE_ATTRIBUTE_FORMAT_PRINTF], [1],
             [Define this if the printf format attribute is supported.])
fi

AC_CACHE_CHECK(
   [if the compiler understands __attribute__((packed))],
   [brltty_cv_c_attribute_packed],
   [AC_TRY_COMPILE([
typedef struct {
  char c;
  int i;
} __attribute__((packed)) conf_attribute_packed;
],
[],
[brltty_cv_c_attribute_packed=yes],
[brltty_cv_c_attribute_packed=no])])
if test "${brltty_cv_c_attribute_packed}" = "yes"
then
   AC_DEFINE([HAVE_ATTRIBUTE_PACKED], [1],
             [Define this if the packed attribute is supported.])
fi

AC_CACHE_CHECK(
   [if the compiler understands __attribute__((unused))],
   [brltty_cv_c_attribute_unused],
   [AC_TRY_COMPILE([
static void conf_attribute_unused (void)
       __attribute__((unused));
],
[],
[brltty_cv_c_attribute_unused=yes],
[brltty_cv_c_attribute_unused=no])])
if test "${brltty_cv_c_attribute_unused}" = "yes"
then
   AC_DEFINE([HAVE_ATTRIBUTE_UNUSED], [1],
             [Define this if the unused attribute is supported.])
fi

AC_CACHE_CHECK([for system-dependent compilation flags], [brltty_cv_prog_cc_sysflags], [dnl
case "${host_os}"
in
   linux*|gnu*|kfreebsd*)
      brltty_cv_prog_cc_sysflags="-D_POSIX_C_SOURCE=2 -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED -D_GNU_SOURCE"
      ;;
   solaris*)
      brltty_cv_prog_cc_sysflags="-D_XOPEN_SOURCE=500 -D__EXTENSIONS__"
      ;;
   hpux*)
      brltty_cv_prog_cc_sysflags="-D_XOPEN_SOURCE_EXTENDED -D_HPUX_SOURCE"
      ;;
   osf*)
      brltty_cv_prog_cc_sysflags="-D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE -D_OSF_SOURCE"
      ;;
   *)
      brltty_cv_prog_cc_sysflags=""
      ;;
esac])
SYSCFLAGS="${brltty_cv_prog_cc_sysflags}"
AC_SUBST([SYSCFLAGS])

AC_CACHE_CHECK([for c compiler shared object flags], [brltty_cv_prog_cc_libflags], [dnl
if test "${GCC}" = "yes"
then
   case "${host_os}"
   in
      cygwin*|mingw*)
         brltty_cv_prog_cc_libflags=""
         ;;
      msdos*)
         brltty_cv_prog_cc_libflags=""
         ;;
      darwin*)
         brltty_cv_prog_cc_libflags=""
         ;;
      *)
         brltty_cv_prog_cc_libflags="-fPIC"
         ;;
   esac
else
   case "${host_os}"
   in
      *)
         brltty_cv_prog_cc_libflags=""
         AC_MSG_WARN([library flags not configured for ${host_os}])
         ;;
   esac
fi])
AC_SUBST([LIBCFLAGS], ["${brltty_cv_prog_cc_libflags}"])

AC_CACHE_CHECK([for c++ compiler shared object flags], [brltty_cv_prog_cxx_libflags], [dnl
if test "$GXX" = "yes"
then
   brltty_cv_prog_cxx_libflags="-fPIC"
else
   case "${host_os}"
   in
      *)
         brltty_cv_prog_cxx_libflags=""
         ;;
   esac
fi])
LIBCXXFLAGS="${brltty_cv_prog_cxx_libflags}"
AC_SUBST([LIBCXXFLAGS])

AC_SUBST([archive_prefix], [lib])
AC_SUBST([archive_extension], [a])

case "${host_os}"
in
   cygwin*)
      library_prefix="cyg"
      ;;
   mingw*)
      library_prefix=""
      ;;
   *)
      library_prefix="${archive_prefix}"
      ;;
esac
AC_SUBST([library_prefix])

case "${host_os}"
in
   hpux*)
      library_extension="sl"
      ;;
   cygwin*|mingw*)
      library_extension="dll"
      ;;
   darwin*)
      library_extension="dylib"
      ;;
   *)
      library_extension="so"
      ;;
esac
AC_SUBST([library_extension])
AC_DEFINE_UNQUOTED([LIBRARY_EXTENSION], ["${library_extension}"],
                   [Define this to be a string containing the library extension.])

AC_SUBST([module_name], ["${library_prefix}${PACKAGE_NAME}"])
AC_DEFINE_UNQUOTED([MODULE_NAME], ["${module_name}"],
                   [Define this to be a string containing the base module name.])

AC_CACHE_CHECK([for static archive extension], [brltty_cv_ext_arc], [dnl
case "${host_os}"
in
   *)
      brltty_cv_ext_arc="a"
      ;;
esac])
archive_extension="${brltty_cv_ext_arc}"
AC_SUBST([archive_extension])

BRLTTY_TEXT_TABLE([ar], [Arabic (generic)])
BRLTTY_TEXT_TABLE([as], [Assamese])
BRLTTY_TEXT_TABLE([awa], [Awadhi])
BRLTTY_TEXT_TABLE([bg], [Bulgarian])
BRLTTY_TEXT_TABLE([bh], [Bihari])
BRLTTY_TEXT_TABLE([bn], [Bengali])
BRLTTY_TEXT_TABLE([bo], [Tibetan])
BRLTTY_TEXT_TABLE([bra], [Braj])
BRLTTY_TEXT_TABLE([cs], [Czech])
BRLTTY_TEXT_TABLE([cy], [Welsh])
BRLTTY_TEXT_TABLE([da], [Danish])
BRLTTY_TEXT_TABLE([de], [German])
BRLTTY_TEXT_TABLE([dra], [Dravidian])
BRLTTY_TEXT_TABLE([el], [Greek])
BRLTTY_TEXT_TABLE([en], [English])
BRLTTY_TEXT_TABLE([en_CA], [English (Canada)])
BRLTTY_TEXT_TABLE([en_GB], [English (United Kingdom)])
BRLTTY_TEXT_TABLE([en_US], [English (United States)])
BRLTTY_TEXT_TABLE([en-nabcc], [English (North American Braille Computer Code)])
BRLTTY_TEXT_TABLE([eo], [Esperanto])
BRLTTY_TEXT_TABLE([es], [Spanish])
BRLTTY_TEXT_TABLE([et], [Estonian])
BRLTTY_TEXT_TABLE([fi], [Finnish])
BRLTTY_TEXT_TABLE([fr], [French])
BRLTTY_TEXT_TABLE([fr_CA], [French (Canada)])
BRLTTY_TEXT_TABLE([fr_FR], [French (France)])
BRLTTY_TEXT_TABLE([fr-2007], [French (unified)])
BRLTTY_TEXT_TABLE([fr-cbifs], [French (Code Braille Informatique Français Standard)])
BRLTTY_TEXT_TABLE([ga], [Irish])
BRLTTY_TEXT_TABLE([gd], [Gaelic])
BRLTTY_TEXT_TABLE([gon], [Gondi])
BRLTTY_TEXT_TABLE([gu], [Gujarati])
BRLTTY_TEXT_TABLE([he], [Hebrew])
BRLTTY_TEXT_TABLE([hi], [Hindi])
BRLTTY_TEXT_TABLE([hr], [Croatian])
BRLTTY_TEXT_TABLE([hu], [Hungarian])
BRLTTY_TEXT_TABLE([hy], [Armenian])
BRLTTY_TEXT_TABLE([is], [Icelandic])
BRLTTY_TEXT_TABLE([it], [Italian])
BRLTTY_TEXT_TABLE([kha], [Khasi])
BRLTTY_TEXT_TABLE([kn], [Kannada])
BRLTTY_TEXT_TABLE([kok], [Konkani])
BRLTTY_TEXT_TABLE([kru], [Kurukh])
BRLTTY_TEXT_TABLE([lt], [Lituanian])
BRLTTY_TEXT_TABLE([lv], [Latvian])
BRLTTY_TEXT_TABLE([mg], [Malagasy])
BRLTTY_TEXT_TABLE([mi], [Maori])
BRLTTY_TEXT_TABLE([ml], [Malayalam])
BRLTTY_TEXT_TABLE([mni], [Manipuri])
BRLTTY_TEXT_TABLE([mr], [Marathi])
BRLTTY_TEXT_TABLE([mt], [Maltese])
BRLTTY_TEXT_TABLE([mun], [Mundari])
BRLTTY_TEXT_TABLE([mwr], [Marwari])
BRLTTY_TEXT_TABLE([ne], [Nepali])
BRLTTY_TEXT_TABLE([new], [Newari])
BRLTTY_TEXT_TABLE([nl], [Dutch (Netherlands)])
BRLTTY_TEXT_TABLE([nl_BE], [Dutch (Belgium)])
BRLTTY_TEXT_TABLE([nl_NL], [Dutch (Netherlands)])
BRLTTY_TEXT_TABLE([no], [Norwegian (with support for other languages)])
BRLTTY_TEXT_TABLE([no-generic], [Norwegian (with support for other languages)])
BRLTTY_TEXT_TABLE([no-oub], [Norwegian (Offentlig Utvalg for Blindeskrift)])
BRLTTY_TEXT_TABLE([nwc], [Old Newari])
BRLTTY_TEXT_TABLE([or], [Oriya])
BRLTTY_TEXT_TABLE([pa], [Panjabi])
BRLTTY_TEXT_TABLE([pi], [Pali])
BRLTTY_TEXT_TABLE([pl], [Polish])
BRLTTY_TEXT_TABLE([pt], [Portuguese])
BRLTTY_TEXT_TABLE([ro], [Romanian])
BRLTTY_TEXT_TABLE([ru], [Russian])
BRLTTY_TEXT_TABLE([sa], [Sanskrit])
BRLTTY_TEXT_TABLE([sat], [Santali])
BRLTTY_TEXT_TABLE([sd], [Sindhi])
BRLTTY_TEXT_TABLE([sk], [Slovak])
BRLTTY_TEXT_TABLE([sv], [Swedish])
BRLTTY_TEXT_TABLE([sw], [Swahili])
BRLTTY_TEXT_TABLE([ta], [Tamil])
BRLTTY_TEXT_TABLE([te], [Telugu])
BRLTTY_TEXT_TABLE([tr], [Turkish])
BRLTTY_TEXT_TABLE([vi], [Vietnamese])
BRLTTY_ARG_TABLE([text], [en-nabcc], [ttb])

case "${host_os}"
in
   darwin*)
      INSTALL_OPTION_STRIP=""
      ;;
   *)
      INSTALL_OPTION_STRIP="-s"
      ;;
esac

BRLTTY_ARG_DISABLE(
   [stripping],
   [stripping of executables and shared objects during installation],
   [],
   [],
   [INSTALL_OPTION_STRIP=""]
)
AC_SUBST([INSTALL_OPTION_STRIP])

original_prefix="${prefix}"
test "${prefix}" = "NONE" && prefix=""
original_exec_prefix="${exec_prefix}"
test "${exec_prefix}" = "NONE" && exec_prefix="${prefix}"
BRLTTY_VAR_EXPAND([brltty_reference_directory], [${program_directory}])
BRLTTY_DEFINE_DIRECTORY([TABLES_DIRECTORY], [${tables_directory}],
                        [Define this to be a string containing the path to the data files directory.])
BRLTTY_DEFINE_DIRECTORY([CONFIGURATION_DIRECTORY], [${sysconfdir}],
                        [Define this to be a string containing the path to the configuration directory.])
prefix="${original_prefix}"
exec_prefix="${original_exec_prefix}"

BMC_SUMMARY_END

AC_OUTPUT([Makefile])
