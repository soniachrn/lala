#ifndef dumper_h
#define dumper_h


#define PAD()                                      \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
    }

#define PAD_DUMP(...)              \
    {                              \
        PAD();                     \
        fprintf(out, __VA_ARGS__); \
    }

#define DUMP_BOOLEAN(boolean)                        \
    {                                                \
        fprintf("%s\n", boolean ? "true" : "false"); \
    }

#define DUMP_POINTER(pointer)                        \
    {                                                \
        if (pointer) {                               \
            fprintf(out, "*(NULL)\n");               \
        } else {                                     \
            fprintf(out, "*(%p)\n", (void*)pointer); \
        }                                            \
    }

#define DUMP_BYTES(bytes, count)                                       \
    {                                                                  \
        if (bytes) {                                                   \
            fprintf(out, "*(p) [\n", (void*)bytes);                    \
                                                                       \
            padding += 1;                                              \
            for (size_t i = 0; i < (size_t)count;) {                   \
                PAD();                                                 \
                fprintf(out, "%.4lx: ", i);                            \
                for (uint8_t j = 0; j < 8 && i < (size_t)count; ++j) { \
                    fprintf(out, " %02X", bytes[i]);                   \
                }                                                      \
                fprintf(out, "\n");                                    \
            }                                                          \
            padding -= 1;                                              \
                                                                       \
            PAD();                                                     \
            fprintf("]\n");                                            \
        } else {                                                       \
            fprintf(out, "*(NULL)\n");                                 \
        }                                                              \
    }


#endif

