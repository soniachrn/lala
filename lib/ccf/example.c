#include "ccf.h"

#include <stdio.h>


int main() { 

    printf("%s %s %s %s %s %s %s %s\n",
            CCF_BLACK(  "B"),
            CCF_RED(    "R"),
            CCF_GREEN(  "G"),
            CCF_YELLOW( "Y"),
            CCF_BLUE(   "B"),
            CCF_MAGENTA("M"),
            CCF_CYAN(   "C"),
            CCF_WHITE(  "W")
    );

    printf(CCF_BG_WHITE("%s %s %s %s %s %s %s %s\n"),
            CCF_BLACK_CODE   "B",
            CCF_RED_CODE     "R",
            CCF_GREEN_CODE   "G",
            CCF_YELLOW_CODE  "Y",
            CCF_BLUE_CODE    "B",
            CCF_MAGENTA_CODE "M",
            CCF_CYAN_CODE    "C",
            CCF_WHITE_CODE   "W"
    );

    return 0;
}

