include(FindPkgConfig)
include(FindSDL)
pkg_check_modules(PKG_CONFIG_GTK3 gtk+-3.0)
pkg_check_modules(PKG_CONFIG_GTKSOURCEVIEW3 gtksourceview-3.0)
pkg_check_modules(PKG_CONFIG_EVINCE_VIEW3 evince-view-3.0)
FIND_LIBRARY(SDL_MIXER_LIBRARY
  NAMES SDL_mixer SDL_mixer-1.2
  HINTS $ENV{SDLDIR}
  PATH_SUFFIXES lib64 lib)
if(PKG_CONFIG_GTK3_FOUND AND
   PKG_CONFIG_GTKSOURCEVIEW3_FOUND AND
   PKG_CONFIG_EVINCE_VIEW3_FOUND AND
   SDL_FOUND)
  include_directories(${PKG_CONFIG_GTK3_INCLUDE_DIRS})
  include_directories(${PKG_CONFIG_GTKSOURCEVIEW3_INCLUDE_DIRS})
  include_directories(${PKG_CONFIG_EVINCE_VIEW3_INCLUDE_DIRS})
  include_directories(${SDL_INCLUDE_DIRS})
  add_executable(bmc_gtk color.c edit.c goto.c menubar.c open.c save.c toolbar.c
                         window.c compile.c errors.c main.c new.c player.c scoreviewer.c tops.c)
  target_link_libraries(bmc_gtk ${PKG_CONFIG_GTK3_LIBRARIES})
  target_link_libraries(bmc_gtk ${PKG_CONFIG_GTKSOURCEVIEW3_LIBRARIES})
  target_link_libraries(bmc_gtk ${PKG_CONFIG_EVINCE_VIEW3_LIBRARIES})
  target_link_libraries(bmc_gtk ${SDL_MIXER_LIBRARY})
endif()
