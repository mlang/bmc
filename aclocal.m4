AC_DEFUN([BRLTTY_UPPERCASE], [translit([$1], [a-z], [A-Z])])

AC_DEFUN([BRLTTY_VAR_TRIM], [dnl
$1="`echo "${$1}" | sed -e 's/^ *//' -e 's/ *$//'`"
])

AC_DEFUN([BRLTTY_VAR_EXPAND], [dnl
eval '$1="'"$2"'"'])

AC_DEFUN([BRLTTY_DEFINE_EXPANDED], [dnl
BRLTTY_VAR_EXPAND([brltty_expanded], [$2])
AC_DEFINE_UNQUOTED([$1], ["${brltty_expanded}"], [$3])])

AC_DEFUN([BRLTTY_RELATIVE_PATH], [dnl
AC_REQUIRE([AC_PROG_AWK])
eval '$1="`"${AWK}" -v path="'"$2"'" -v reference="'"$3"'" -f "${srcdir}/relpath.awk"`"'])

AC_DEFUN([BRLTTY_DEFINE_DIRECTORY], [dnl
BRLTTY_VAR_EXPAND([$1], [$2])
AC_SUBST([$1])
if test "${brltty_enabled_relocatable_install}" = "yes"
then
   BRLTTY_RELATIVE_PATH([brltty_path], [${$1}], [${brltty_reference_directory}])
else
   brltty_path="${$1}"
fi
BRLTTY_DEFINE_EXPANDED([$1], [${brltty_path}], [$3])])

AC_DEFUN([BRLTTY_ARG_WITH], [dnl
AC_ARG_WITH([$1], BRLTTY_HELP_STRING([--with-$1=$2], [$3]), [$4="${withval}"], [$4=$5])])

AC_DEFUN([BRLTTY_ARG_REQUIRED], [dnl
BRLTTY_ARG_WITH([$1], [$2], [$3], [$4], ["yes"])
if test "${$4}" = "no"
then
   AC_MSG_ERROR([$1 not specified])
elif test "${$4}" = "yes"
then
   $4=$5
fi
AC_SUBST([$4])
BMC_SUMMARY_ITEM([$1], [$4])])

AC_DEFUN([BRLTTY_ARG_TABLE], [dnl
brltty_default_table="$2"
BRLTTY_ARG_WITH(
   [$1-table], [FILE],
   [built-in (fallback) $1 table]brltty_tables_$1,
   [$1_table], ["${brltty_default_table}"]
)
install_$1_tables=install-$1-tables
if test "${$1_table}" = "no"
then
   install_$1_tables=
   $1_table="${brltty_default_table}"
elif test "${$1_table}" = "yes"
then
   $1_table="${brltty_default_table}"
fi
AC_SUBST([install_$1_tables])
BMC_SUMMARY_ITEM([$1-table], [$1_table])
AC_DEFINE_UNQUOTED(BRLTTY_UPPERCASE([$1_table]), ["${$1_table}"],
                   [Define this to be a string containing the path to the default $1 table.])
AC_SUBST([$1_table])])

AC_DEFUN([BRLTTY_ARG_ENABLE], [dnl
BRLTTY_ARG_FEATURE([$1], [$2], [enable], [no], [$3], [$4], [$5])])

AC_DEFUN([BRLTTY_ARG_DISABLE], [dnl
BRLTTY_ARG_FEATURE([$1], [$2], [disable], [yes], [$3], [$4], [$5])])

AC_DEFUN([BRLTTY_ARG_FEATURE], [dnl
AC_ARG_ENABLE([$1], BRLTTY_HELP_STRING([--$3-$1], [$2]), [], [enableval="$4"])

pushdef([var], brltty_enabled_[]translit([$1], [-], [_]))dnl
var="${enableval}"
BMC_SUMMARY_ITEM([$1], var)dnl
popdef([var])

if test "${enableval}" = "no"
then
   ifelse(len([$7]), 0, [:], [$7])
else
ifelse(len([$5]), 0, [], [dnl
   set -- [$5]
])dnl
   if test "${enableval}" = "yes"
   then
      brltty_ok=true
ifelse(len([$5]), 0, [], [dnl
      test "${#}" -gt 0 && enableval="${1}"
])dnl
   else
      brltty_ok=false
ifelse(len([$5]), 0, [], [dnl
      test "${#}" -gt 0 && {
         for brltty_selection
         do
            test "${brltty_selection}" = "${enableval}" && {
               brltty_ok=true
               break
            }
         done
      }
])dnl
   fi

   if "${brltty_ok}"
   then
ifelse(len([$5]), 0, [], [dnl
      test "${#}" -gt 0 && {
         brltty_uc="`echo "use_$1_${enableval}" | sed -e 'y%abcdefghijklmnopqrstuvwxyz-%ABCDEFGHIJKLMNOPQRSTUVWXYZ_%'`"
         AC_DEFINE_UNQUOTED([${brltty_uc}])
      }
])dnl
      ifelse(len([$6]), 0, [:], [$6])
   else
      AC_MSG_ERROR([invalid selection: --enable-$1=${enableval}])
   fi
fi])

AC_DEFUN([BRLTTY_HELP_STRING], [dnl
AC_HELP_STRING([$1], patsubst([$2], [
.*$]), m4_defn([brltty_help_prefix]))dnl
patsubst(patsubst([$2], [\`[^
]*]), [
], [\&brltty_help_prefix])[]dnl
])
m4_define([brltty_help_indent], 20)
m4_define([brltty_help_prefix], m4_format([%]brltty_help_indent[s], []))
m4_define([brltty_help_width], m4_eval(79-brltty_help_indent))

AC_DEFUN([BRLTTY_ITEM], [dnl
define([brltty_item_list_$1], ifdef([brltty_item_list_$1], [brltty_item_list_$1])[
m4_text_wrap([$3], [      ], [- $2  ], brltty_help_width)])dnl
brltty_item_entries_$1="${brltty_item_entries_$1} $2-$3"
brltty_item_codes_$1="${brltty_item_codes_$1} $2"
brltty_item_names_$1="${brltty_item_names_$1} $3"
AC_SUBST([$1_libraries_$2], ['$4'])])

AC_DEFUN([BRLTTY_ITEM_RESOLVE], [dnl
brltty_item_unknown=true
brltty_item_length=`expr length "${brltty_item}"`

if test "${brltty_item_length}" -eq 2
then
   [brltty_item_entry=`expr "${brltty_items_left_$1}" : '.* \('"${brltty_item}"'-[^ ]*\)'`]
   if test -n "${brltty_item_entry}"
   then
      brltty_item_code="${brltty_item}"
      [brltty_item_name=`expr "${brltty_item_entry}" : '[^[.-.]]*-\(.*\)$'`]
      brltty_item_unknown=false
   fi
elif test "${brltty_item_length}" -gt 2
then
   [brltty_item_entry=`expr "${brltty_items_left_$1}" : '.* \([^- ]*-'"${brltty_item}"'[^ ]*\)'`]
   if test -z "${brltty_item_entry}"
   then
      brltty_lowercase=`echo "${brltty_items_left_$1}" | sed 'y%ABCDEFGHIJKLMNOPQRSTUVWXYZ%abcdefghijklmnopqrstuvwxyz%'`
      [brltty_item_code=`expr "${brltty_lowercase}" : '.* \([^- ]*\)-'"${brltty_item}"`]
      if test -n "${brltty_item_code}"
      then
         [brltty_item_entry=`expr "${brltty_items_left_$1}" : '.* \('"${brltty_item_code}"'-[^ ]*\)'`]
      fi
   fi

   if test -n "${brltty_item_entry}"
   then
      [brltty_item_code=`expr "${brltty_item_entry}" : '\([^[.-.]]*\)'`]
      [brltty_item_name=`expr "${brltty_item_entry}" : '[^[.-.]]*-\(.*\)$'`]
      brltty_item_unknown=false
   fi
fi
])

AC_DEFUN([BRLTTY_TEXT_TABLE], [dnl
define([brltty_tables_text], ifdef([brltty_tables_text], [brltty_tables_text])[
m4_text_wrap([$2], [           ], [- ]m4_format([%-8s ], [$1]), brltty_help_width)])])

AC_DEFUN([BRLTTY_ATTRIBUTES_TABLE], [dnl
define([brltty_tables_attributes], ifdef([brltty_tables_attributes], [brltty_tables_attributes])[
m4_text_wrap([$2], [             ], [- ]m4_format([%-10s ], [$1]), brltty_help_width)])])

AC_DEFUN([BMC_SUMMARY_BEGIN], [dnl
bmc_summary_lines="Options Summary:"
])

AC_DEFUN([BMC_SUMMARY_END], [dnl
AC_CONFIG_COMMANDS([bmc-summary],
   [AC_MSG_NOTICE([${bmc_summary_lines}])],
   [bmc_summary_lines="${bmc_summary_lines}"]
)])

AC_DEFUN([BMC_SUMMARY_ITEM], [dnl
bmc_summary_lines="${bmc_summary_lines}
   $1: ${$2}"])

AC_DEFUN([BRLTTY_EXECUTABLE_PATH], [dnl
[if test `expr "${$1} " : '[^/ ][^/ ]*/'` -gt 0]
then
   $1="`pwd`/${$1}"
fi])

AC_DEFUN([BRLTTY_HAVE_WINDOWS_LIBRARY], [dnl
AC_CACHE_CHECK(
   [if DLL $1 can be loaded],
   [brltty_cv_dll_$1],
   [AC_TRY_RUN([
#include <windows.h>
int main () {
   return !LoadLibrary("$1.DLL");
}
],
[brltty_cv_dll_$1=yes],
[brltty_cv_dll_$1=no])])
if test "${brltty_cv_dll_$1}" = "yes"
then
   AC_HAVE_LIBRARY([$1])
   $2
else
   :
   $3
fi])

AC_DEFUN([BRLTTY_HAVE_WINDOWS_FUNCTION], [dnl
AC_CACHE_CHECK(
   [if function $1 in DLL $2 exists],
   [brltty_cv_function_$1],
   [AC_TRY_RUN([
#include <windows.h>
#include <stdio.h>
#include <errno.h>

int
main (void) {
  HMODULE module;
  HINSTANCE instance;
  if (!(instance = LoadLibrary("$2.dll"))) return 1;
  if (!(module = GetModuleHandle("$2.dll"))) return 2;
  if (!(GetProcAddress(module, "$1"))) return 3;
  return 0;
}
],
   [brltty_cv_function_$1=yes],
   [brltty_cv_function_$1=no])])
if test "${brltty_cv_function_$1}" = "yes"
then
   AC_DEFINE(BRLTTY_UPPERCASE([HAVE_$1]), [1],
             [Define this if the function $1 is available.])
   $3
else
   :
   $4
fi
])

AC_DEFUN([BRLTTY_BINDINGS], [dnl
ifelse($#, 1, [dnl
BRLTTY_BINDINGS([$1], m4_tolower([$1]), m4_toupper([$1]))dnl
], [dnl
BRLTTY_ARG_DISABLE(
   [$2-bindings],
   [$1 bindings for BrlAPI],
   [],
   [
      m4_include([Bindings/$1/bindings.m4])
      BRLTTY_$3_BINDINGS
      if "${$3_OK}"
      then
         api_bindings="${api_bindings} $1"
      else
         AC_MSG_WARN([$1 BrlAPI bindings not included])
      fi
   ]
)dnl
])])
