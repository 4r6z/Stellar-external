#include <Windows.h>

#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#ifndef _CONSOLE_COLOR
#define _CONSOLE_COLOR 0x0004
#endif

inline void set_console_color(int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

#define CNEWLINE std::cout << "\n";

#define CWHITE set_console_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#define CRED set_console_color(FOREGROUND_RED);
#define CGREEN set_console_color(FOREGROUND_GREEN);
#define CBLUE set_console_color(FOREGROUND_BLUE);
#define CPURPLE set_console_color(FOREGROUND_RED | FOREGROUND_BLUE);
#define CYELLOW set_console_color(FOREGROUND_RED | FOREGROUND_GREEN);
#define CCOLOR set_console_color(_CONSOLE_COLOR);

#define COUT(b)													   \
CCOLOR;									                       \
std::cout << "[stellar leaked XD] ";                                    \
CWHITE;      \
std::cout << b;                                                   \
CNEWLINE;                   \

#endif