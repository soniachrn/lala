#ifndef lala_debug_h
#define lala_debug_h


#include <string.h>


#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   // WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          // *NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif


#endif

