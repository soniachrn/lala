/* C Unit Test
 * @soniachrn <3
 * 1 Oct 2021
 */

#ifndef CUT
#define CUT

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ccf.h"


// Types

typedef void (*__CutTestFunction)(
        bool* __cutTestResult, const char* __cutTestName
);


// Functions

void __cutRegisterTest(const char* testName, __CutTestFunction testFunction);


// Macros

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define TEST(testName) \
    static void testName(bool* __cutTestResult, const char* __cutTestName); \
    static void __attribute__((constructor)) __cutRegister_##testName() { \
        __cutRegisterTest(#testName, testName); \
    } \
    static void testName(bool* __cutTestResult, const char* __cutTestName)

#define EXPECT_INTERNAL(condition, ...) \
    if (!(condition)) { \
        printf(CCF_RED(CCF_BOLD("%s")) CCF_RED(": Expectation failed at %s:%d\n"), \
                __cutTestName, __FILENAME__, __LINE__ \
        ); \
        printf(__VA_ARGS__); \
        printf("\n\n"); \
        *__cutTestResult = false; \
        return; \
    }

#define EXPECT(condition) \
    EXPECT_INTERNAL((condition), \
            "Expected " CCF_BLUE(CCF_BOLD(#condition)) \
            " to be true, got false" \
    )

#define EXPECT_FALSE(condition) \
    EXPECT_INTERNAL(!(condition), \
            "Expected " CCF_BLUE(CCF_BOLD(#condition)) \
            " to be false, got true" \
    )

#define EXPECT_EQUALS(a, b) \
    EXPECT_INTERNAL((a) == (b), \
            "Expected " CCF_YELLOW(CCF_BOLD(#a)) \
            " to be equal to " CCF_BLUE(CCF_BOLD(#b)) ", got unequal" \
    )

#define EXPECT_EQUALS_F(a, b, format) \
    EXPECT_INTERNAL((a) == (b), \
            "Expected " CCF_YELLOW(CCF_BOLD(#a " (" format ")")) \
            " to be equal to " CCF_BLUE(CCF_BOLD(#b " (" format ")")), \
            a, \
            b \
    )

#define EXPECT_EQUALS_F_POSTFIX(a, b, format) \
    EXPECT_INTERNAL((a) == (b), \
            "Expected " CCF_YELLOW(CCF_BOLD(#a " (" format ")")) \
            " to be equal to " CCF_BLUE(CCF_BOLD(#b " (" format ")")), \
            a, \
            b \
    )

#define EXPECT_EQUALS_TOSTR(a, b, to_string) \
    EXPECT_INTERNAL((a) == (b), \
            "Expected " CCF_YELLOW(CCF_BOLD(#a " (%s)")) \
            " to be equal to " CCF_BLUE(CCF_BOLD(#b " (%s)")), \
            to_string((a)), \
            to_string((b)) \
    )

#define EXPECT_NOT_EQUALS(a, b) \
    EXPECT_INTERNAL((a) != (b), \
            "Expected " CCF_YELLOW(CCF_BOLD(#a)) \
            " to not be equal to " CCF_BLUE(CCF_BOLD(#b)) ", got equal" \
    )


#endif

