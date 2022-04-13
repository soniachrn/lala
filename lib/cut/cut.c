/* C Unit Test
 * @soniachrn <3
 * 1 Oct 2021
 */

#include "cut.h"

#include <stdlib.h>


// TODO: Check compiler
// TODO: Include ccf correctly


// ┌────────┐
// │ Macros │
// └────────┘

#define ASSERT_TEST_QUEUE() { \
        assert((bool)testQueue.first == (bool)testQueue.last); \
    }


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    const char* name;
    __CutTestFunction function;
} Test;

typedef struct TestQueueNode {
    Test value;
    struct TestQueueNode* next;
} TestQueueNode;

typedef struct {
    TestQueueNode* first;
    TestQueueNode* last;
} TestQueue;


// ┌─────────┐
// │ Globals │
// └─────────┘

TestQueue testQueue = {NULL, NULL};


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static void pushTestQueue(const Test test);
static Test popTestQueue();
static bool isTestQueueEmpty();

static void fillStatsColors(
        int succeed, int failed,
        char** totalColorCode, char** succeedColorCode, char** failedColorCode
);


// ┌──────┐
// │ Main │
// └──────┘

int main() {
    int succeed = 0;
    int failed  = 0;

    while (!isTestQueueEmpty()) {
        Test test = popTestQueue();

        bool testResult = true;
        test.function(&testResult, test.name);
        assert(testResult == 1 || testResult == 0);

        succeed +=  testResult;
        failed  += !testResult;
    }

    if (succeed + failed == 0)
        printf("No tests\n");
    else {
        char* totalColorCode   = NULL;
        char* succeedColorCode = NULL;
        char* failedColorCode  = NULL;
        
        fillStatsColors(
                succeed, failed, 
                &totalColorCode, &succeedColorCode, &failedColorCode
        );

        printf("%s" CCF_BOLD("Run %d tests\n")
               "%s  Succeed %d\n"
               "%s  Failed  %d\n"
               CCF_RESET_CODE,
               totalColorCode,   succeed + failed,
               succeedColorCode, succeed,
               failedColorCode,  failed
        );
    }
 
    return 0;
}


// ┌───────────┐
// │ Functions │
// └───────────┘

void __cutRegisterTest(const char* testName, __CutTestFunction testFunction) {
    Test test = {testName, testFunction};
    pushTestQueue(test);
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static void pushTestQueue(const Test test) {
    ASSERT_TEST_QUEUE();

    TestQueueNode* node = calloc(1, sizeof(TestQueueNode));
    node->value = test;
    node->next = NULL;

    if (isTestQueueEmpty()) {
        testQueue.first = node;
        testQueue.last  = node;
    } else {
        testQueue.last->next = node;
        testQueue.last = node;
    }

    ASSERT_TEST_QUEUE();
}

static Test popTestQueue() {
    ASSERT_TEST_QUEUE();
    assert(!isTestQueueEmpty());

    TestQueueNode* node = testQueue.first;
    testQueue.first = node->next;
    if (!testQueue.first)
        testQueue.last = NULL;

    const Test test = node->value;
    free(node);

    ASSERT_TEST_QUEUE();
    return test;
}

static bool isTestQueueEmpty() {
    ASSERT_TEST_QUEUE();
    
    return !testQueue.first;
}

static void fillStatsColors(
        int succeed, int failed, 
        char** totalColorCode, char** succeedColorCode, char** failedColorCode
) {
    if (failed == 0) {
        *totalColorCode   = CCF_GREEN_CODE;
        *succeedColorCode = CCF_GREEN_CODE;
        *failedColorCode  = CCF_GREEN_CODE;
    } else if (succeed == 0) {
        *totalColorCode   = CCF_RED_CODE;
        *succeedColorCode = CCF_RED_CODE;
        *failedColorCode  = CCF_RED_CODE;
    } else {
        *totalColorCode   = CCF_RESET_CODE;
        *succeedColorCode = CCF_GREEN_CODE;
        *failedColorCode  = CCF_RED_CODE;
    }
}

#undef ASSERT_TEST_QUEUE

