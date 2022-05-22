#include "cut.h"

#include "vm.h"


// Makes array literal to be treated as a single argument when passed to a macro.
#define ARRAY(...)  __VA_ARGS__


#define EXPECT_STACK_STATE(vm, expected_stack)                    \
    {                                                             \
        uint8_t expected[] = expected_stack;                      \
        size_t n = (size_t)(vm.stack.stack_top - vm.stack.stack); \
        for (size_t i = 0; i < n; ++i) {                          \
            EXPECT_EQUALS_F(                                      \
                vm.stack.stack[i],                                \
                expected[i],                                      \
                "0x%02X"                                          \
            );                                                    \
        }                                                         \
    }

#define TEST_VM(name, bytecode, expected_stack)             \
    TEST(name) {                                            \
        uint8_t source_unterminated[] = bytecode;           \
                                                            \
        const size_t length = sizeof(source_unterminated) / \
                              sizeof(uint8_t) + 1;          \
        uint8_t source[length];                             \
        memset(source, 0, length);                          \
        memcpy(source, source_unterminated, length - 1);    \
        source[length - 1] = '\0';                          \
                                                            \
        VM vm;                                              \
        Constants constants;                                \
        constants.count = 0;                                \
        initVM(&vm, source, length - 1, &constants);        \
                                                            \
        interpret(&vm);                                     \
                                                            \
        EXPECT_STACK_STATE(vm, ARRAY(expected_stack));      \
                                                            \
        freeVM(&vm);                                        \
    } static_assert(true, "require semicolon")


#define BINARY_FLOAT_0_5       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F
#define BINARY_FLOAT_MINUS_0_5 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xBF
#define BINARY_FLOAT_2         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40
#define BINARY_FLOAT_4         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40


TEST_VM(PushTrue,
    ARRAY({ OP_PUSH_TRUE }),
    ARRAY({ 0x01 })
);

TEST_VM(PushFalse,
    ARRAY({ OP_PUSH_FALSE }),
    ARRAY({ 0x00 })
);

TEST_VM(PushInt,
    ARRAY({ OP_PUSH_INT, 0x05, 0x00, 0x00, 0x00 }),
    ARRAY({ 0x05, 0x00, 0x00, 0x00 })
);

TEST_VM(PushFloat,
    ARRAY({ OP_PUSH_FLOAT, BINARY_FLOAT_0_5 }),
    ARRAY({ BINARY_FLOAT_0_5 })
);

TEST_VM(NegateBool,
    ARRAY({ OP_PUSH_FALSE, OP_NEGATE_BOOL, OP_PUSH_TRUE, OP_NEGATE_BOOL }),
    ARRAY({ 0x01, 0x00 })
);

TEST_VM(NegateInt,
    ARRAY({ OP_PUSH_INT, 0x05, 0x00, 0x00, 0x00, OP_NEGATE_INT }),
    ARRAY({ 0xFB, 0xFF, 0xFF, 0xFF })
);

TEST_VM(NegateFloat,
    ARRAY({ OP_PUSH_FLOAT, BINARY_FLOAT_0_5, OP_NEGATE_FLOAT }),
    ARRAY({ BINARY_FLOAT_MINUS_0_5 })
);

TEST_VM(MultiplyInt,
    ARRAY({ 
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_MULTIPLY_INT
    }),
    ARRAY({ 0x06, 0x00, 0x00, 0x00 })  // 6
);

TEST_VM(MultiplyFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_4,
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,
        OP_MULTIPLY_FLOAT
    }),
    ARRAY({ BINARY_FLOAT_2 })
);

TEST_VM(DivideInt,
    ARRAY({
        OP_PUSH_INT, 0x0A, 0x00, 0x00, 0x00,  // 10
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_DIVIDE_INT
    }),
    ARRAY({ 0x05, 0x00, 0x00, 0x00 })  // 5
);

TEST_VM(DivideFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_4,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_DIVIDE_FLOAT
    }),
    ARRAY({ BINARY_FLOAT_2 })
);

TEST_VM(ModuloInt,
    ARRAY({
        OP_PUSH_INT, 0x0B, 0x00, 0x00, 0x00,  // 11
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_MODULO_INT
    }),
    ARRAY({ 0x02, 0x00, 0x00, 0x00 })  // 2
);

TEST_VM(AddInt,
    ARRAY({ 
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_ADD_INT
    }),
    ARRAY({ 0x05, 0x00, 0x00, 0x00 })  // 5
);

TEST_VM(AddFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_ADD_FLOAT
    }),
    ARRAY({ BINARY_FLOAT_4 })
);

TEST_VM(SubtractInt,
    ARRAY({ 
        OP_PUSH_INT, 0x0A, 0x00, 0x00, 0x00,  // 10
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_NEGATE_INT,
        OP_ADD_INT
    }),
    ARRAY({ 0x07, 0x00, 0x00, 0x00 })  // 7
);

TEST_VM(SubtractFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_4,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_NEGATE_FLOAT,
        OP_ADD_FLOAT
    }),
    ARRAY({ BINARY_FLOAT_2 })
);

TEST_VM(EqualsInt,
    ARRAY({ 
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_EQUALS_INT,
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_EQUALS_INT
    }),
    ARRAY({ 0x00, 0x01 })  // false true
);

TEST_VM(EqualsFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_EQUALS_FLOAT,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,
        OP_EQUALS_FLOAT
    }),
    ARRAY({ 0x01, 0x00 })  // true false
);

TEST_VM(GreaterInt,
    ARRAY({ 
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_GREATER_INT,
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x01, 0x00, 0x00, 0x00,  // 1
        OP_GREATER_INT
    }),
    ARRAY({ 0x00, 0x01 })  // false true
);

TEST_VM(GreaterFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_GREATER_FLOAT,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,
        OP_GREATER_FLOAT
    }),
    ARRAY({ 0x00, 0x01 })  // false true
);

TEST_VM(LessInt,
    ARRAY({ 
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x03, 0x00, 0x00, 0x00,  // 3
        OP_LESS_INT,
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_PUSH_INT, 0x02, 0x00, 0x00, 0x00,  // 2
        OP_LESS_INT
    }),
    ARRAY({ 0x01, 0x00 })  // true false
);

TEST_VM(LessFloat,
    ARRAY({
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_LESS_FLOAT,
        OP_PUSH_FLOAT, BINARY_FLOAT_2,
        OP_PUSH_FLOAT, BINARY_FLOAT_4,
        OP_LESS_FLOAT
    }),
    ARRAY({ 0x00, 0x01 })  // false true
);

TEST_VM(Variables,
    ARRAY({
        OP_PUSH_TRUE,                           // var b1: bool  = true
        OP_PUSH_FALSE,                          // var b2: bool  = false
        OP_PUSH_INT,   0x04, 0x00, 0x00, 0x00,  // var i1: int   = 4
        OP_PUSH_INT,   0x08, 0x00, 0x00, 0x00,  // var i2: int   = 8
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,        // var f1: float = 0.5
        OP_PUSH_FLOAT, BINARY_FLOAT_2,          // var f2: float = 2
        // b1 = b2
        OP_GET_GLOBAL_BYTE,  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1 (b2)
        OP_SET_GLOBAL_BYTE,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0 (b1)
        // i1 = i2
        OP_GET_GLOBAL_INT,   0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 6 (i2)
        OP_SET_GLOBAL_INT,   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 2 (i1)
        // f1 = f2
        OP_GET_GLOBAL_FLOAT, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 18 (f2)
        OP_SET_GLOBAL_FLOAT, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 10 (f1)
    }),
    ARRAY({
        0x00,                    // false (b1)
        0x00,                    // false (b2)
        0x08, 0x00, 0x00, 0x00,  // 8 (i1)
        0x08, 0x00, 0x00, 0x00,  // 8 (i2)
        BINARY_FLOAT_2,          // 2 (f1)
        BINARY_FLOAT_2           // 2 (f2)
    })
);

TEST_VM(Temp,
    ARRAY({
        OP_PUSH_INT,       0x0A, 0x00, 0x00, 0x00,
        OP_GET_GLOBAL_INT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }),
    ARRAY({
        0x0A, 0x00, 0x00, 0x00,
        0x0A, 0x00, 0x00, 0x00
    })
);


#undef TEST_VM
#undef EXPECT_STACK_STATE

#undef ARRAY

