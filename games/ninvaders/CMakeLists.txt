cmake_minimum_required(VERSION 3.11)
project(ninvaders C)

set(CMAKE_C_STANDARD 11)

include(GNUInstallDirs)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

set(SOURCES
	aliens.c
	globals.c
	highscore.c
	nInvaders.c
	player.c
	ufo.c
	view.c
)

add_executable(ninvaders ${SOURCES})
target_link_libraries(ninvaders ${CURSES_LIBRARIES})

install(TARGETS ninvaders
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES README.md
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
