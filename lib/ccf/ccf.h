/* C Console Formatting
 * @soniachrn <3
 * 30 Sep 2021
 */


#ifndef CCF
#define CCF


// Codes

// Reset code
#define CCF_RESET_CODE              "\033[0m"

// Decoration codes
#define CCF_BOLD_CODE               "\033[1m"
#define CCF_UNDERLINE_CODE          "\033[4m"
#define CCF_REVERSED_CODE           "\033[7m"

// Color codes
#define CCF_BLACK_CODE              "\033[30m"
#define CCF_RED_CODE                "\033[31m"
#define CCF_GREEN_CODE              "\033[32m"
#define CCF_YELLOW_CODE             "\033[33m"
#define CCF_BLUE_CODE               "\033[34m"
#define CCF_MAGENTA_CODE            "\033[35m"
#define CCF_CYAN_CODE               "\033[36m"
#define CCF_WHITE_CODE              "\033[37m"

// Bright color codes
#define CCF_BRIGHT_BLACK_CODE      "\033[30;1m"
#define CCF_BRIGHT_RED_CODE        "\033[31;1m"
#define CCF_BRIGHT_GREEN_CODE      "\033[32;1m"
#define CCF_BRIGHT_YELLOW_CODE     "\033[33;1m"
#define CCF_BRIGHT_BLUE_CODE       "\033[34;1m"
#define CCF_BRIGHT_MAGENTA_CODE    "\033[35;1m"
#define CCF_BRIGHT_CYAN_CODE       "\033[36;1m"
#define CCF_BRIGHT_WHITE_CODE      "\033[37;1m"

// Background color codes
#define CCF_BG_BLACK_CODE           "\033[40m"
#define CCF_BG_RED_CODE             "\033[41m"
#define CCF_BG_GREEN_CODE           "\033[42m"
#define CCF_BG_YELLOW_CODE          "\033[43m"
#define CCF_BG_BLUE_CODE            "\033[44m"
#define CCF_BG_MAGENTA_CODE         "\033[45m"
#define CCF_BG_CYAN_CODE            "\033[46m"
#define CCF_BG_WHITE_CODE           "\033[47m"

// Background bright color codes
#define CCF_BG_BRIGHT_BLACK_CODE    "\033[40;1m"
#define CCF_BG_BRIGHT_RED_CODE      "\033[41;1m"
#define CCF_BG_BRIGHT_GREEN_CODE    "\033[42;1m"
#define CCF_BG_BRIGHT_YELLOW_CODE   "\033[43;1m"
#define CCF_BG_BRIGHT_BLUE_CODE     "\033[44;1m"
#define CCF_BG_BRIGHT_MAGENTA_CODE  "\033[45;1m"
#define CCF_BG_BRIGHT_CYAN_CODE     "\033[46;1m"
#define CCF_BG_BRIGHT_WHITE_CODE    "\033[47;1m"


// Formatting

// Reset
#define CCF_RESET(s)                CCF_RESET_CODE s

// Decoration
#define CCF_BOLD(s)                 CCF_BOLD_CODE      s CCF_RESET_CODE
#define CCF_UNDERLINE(s)            CCF_UNDERLINE_CODE s CCF_RESET_CODE
#define CCF_REVERSED(s)             CCF_REVERSED_CODE  s CCF_RESET_CODE

// Color
#define CCF_BLACK(s)                CCF_BLACK_CODE   s CCF_RESET_CODE
#define CCF_RED(s)                  CCF_RED_CODE     s CCF_RESET_CODE
#define CCF_GREEN(s)                CCF_GREEN_CODE   s CCF_RESET_CODE
#define CCF_YELLOW(s)               CCF_YELLOW_CODE  s CCF_RESET_CODE
#define CCF_BLUE(s)                 CCF_BLUE_CODE    s CCF_RESET_CODE
#define CCF_MAGENTA(s)              CCF_MAGENTA_CODE s CCF_RESET_CODE
#define CCF_CYAN(s)                 CCF_CYAN_CODE    s CCF_RESET_CODE
#define CCF_WHITE(s)                CCF_WHITE_CODE   s CCF_RESET_CODE

// Bright color
#define CCF_BRIGHT_BLACK(s)         CCF_BRIGHT_BLACK_CODE   s CCF_RESET_CODE
#define CCF_BRIGHT_RED(s)           CCF_BRIGHT_RED_CODE     s CCF_RESET_CODE
#define CCF_BRIGHT_GREEN(s)         CCF_BRIGHT_GREEN_CODE   s CCF_RESET_CODE
#define CCF_BRIGHT_YELLOW(s)        CCF_BRIGHT_YELLOW_CODE  s CCF_RESET_CODE
#define CCF_BRIGHT_BLUE(s)          CCF_BRIGHT_BLUE_CODE    s CCF_RESET_CODE
#define CCF_BRIGHT_MAGENTA(s)       CCF_BRIGHT_MAGENTA_CODE s CCF_RESET_CODE
#define CCF_BRIGHT_CYAN(s)          CCF_BRIGHT_CYAN_CODE    s CCF_RESET_CODE
#define CCF_BRIGHT_WHITE(s)         CCF_BRIGHT_WHITE_CODE   s CCF_RESET_CODE

// Background color
#define CCF_BG_BLACK(s)             CCF_BG_BLACK_CODE   s CCF_RESET_CODE
#define CCF_BG_RED(s)               CCF_BG_RED_CODE     s CCF_RESET_CODE
#define CCF_BG_GREEN(s)             CCF_BG_GREEN_CODE   s CCF_RESET_CODE
#define CCF_BG_YELLOW(s)            CCF_BG_YELLOW_CODE  s CCF_RESET_CODE
#define CCF_BG_BLUE(s)              CCF_BG_BLUE_CODE    s CCF_RESET_CODE
#define CCF_BG_MAGENTA(s)           CCF_BG_MAGENTA_CODE s CCF_RESET_CODE
#define CCF_BG_CYAN(s)              CCF_BG_CYAN_CODE    s CCF_RESET_CODE
#define CCF_BG_WHITE(s)             CCF_BG_WHITE_CODE   s CCF_RESET_CODE

// Background bright color
#define CCF_BG_BRIGHT_BLACK(s)      CCF_BG_BRIGHT_BLACK_CODE   s CCF_RESET_CODE
#define CCF_BG_BRIGHT_RED(s)        CCF_BG_BRIGHT_RED_CODE     s CCF_RESET_CODE
#define CCF_BG_BRIGHT_GREEN(s)      CCF_BG_BRIGHT_GREEN_CODE   s CCF_RESET_CODE
#define CCF_BG_BRIGHT_YELLOW(s)     CCF_BG_BRIGHT_YELLOW_CODE  s CCF_RESET_CODE
#define CCF_BG_BRIGHT_BLUE(s)       CCF_BG_BRIGHT_BLUE_CODE    s CCF_RESET_CODE
#define CCF_BG_BRIGHT_MAGENTA(s)    CCF_BG_BRIGHT_MAGENTA_CODE s CCF_RESET_CODE
#define CCF_BG_BRIGHT_CYAN(s)       CCF_BG_BRIGHT_CYAN_CODE    s CCF_RESET_CODE
#define CCF_BG_BRIGHT_WHITE(s)      CCF_BG_BRIGHT_WHITE_CODE   s CCF_RESET_CODE


#endif

