//
// Created by ivanbrekman on 22.09.2021.
//

#ifndef BASELIB_H
#define BASELIB_H

#ifndef VALIDATE_LEVEL
    #define VALIDATE_LEVEL 1
#endif

#ifndef LOG_PRINTF
    #define LOG_PRINTF 1
#endif

#ifndef LOG_GRAPH
    #define LOG_GRAPH 0
#endif

#define dbg(code) do{ printf("%s:%d\n", __FILE__, __LINE__); code }while(0)
#define LOCATION(var) { TYPE, #var, __FILE__, __FUNCTION__, __LINE__ }
#define VALID_PTR(ptr) !isbadreadptr((const void*)(ptr))

#define COLORED_OUTPUT(str, color, file) IS_TERMINAL(file) ? (color str NATURAL) : str
#define IS_TERMINAL(file) (file == stdin) || (file == stdout) || (file == stderr)

/*
Default define to ASSERT_OK. Use it to customize macros for each project.
!Note!  To use this macro check that open_file function is defined
        or use another function to open file.

#define ASSERT_OK(obj, type, reason, ret) {                                         \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE && type ## _error(obj)) {                   \
        type ## _dump(obj, reason);                                                 \
        if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                   \
            FILE* log = open_file("log.txt", "a");                                  \
            type ## _dump(obj, reason, "log.txt");                                  \
            fclose(log);                                                            \
        }                                                                           \
        ASSERT_IF(0, "verify failed", ret);                                         \
    } else if (type ## _error(obj)) {                                               \
        errno = type ## _error(obj);                                                \
        return ret;                                                                 \
    }                                                                               \
}

*/

#define LOG_DUMP(obj, reason, func) {                                               \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* log = open_file("log.txt", "a");                                      \
        func(obj, reason, log);                                                     \
        fclose(log);                                                                \
    }                                                                               \
}

#define LOG_DUMP_GRAPH(obj, reason, func) {                                         \
    FILE* gr_log = open_file("log.html", "a");                                      \
    if (LOG_GRAPH == 1) func(obj, reason, gr_log);                                  \
    fclose(gr_log);                                                                 \
}

#define PRINT_WARNING(text) {                                                       \
    printf(ORANGE text NATURAL);                                                    \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* wlog = open_file("log.txt", "a");                                     \
        fprintf(wlog, text);                                                        \
        fclose(wlog);                                                               \
    }                                                                               \
}

#define ASSERT_IF(cond, text, ret) {                                                \
    assert((cond) && text);                                                         \
    if (!(cond)) {                                                                  \
        PRINT_WARNING(text "\n");                                                   \
        errno = -1;                                                                 \
        return ret;                                                                 \
    }                                                                               \
}

#define FREE_PTR(ptr, type) {                   \
    free((ptr));                                \
    (ptr) = (type*)poisons::UNINITIALIZED_PTR;  \
}
#define LOG1(code) {        \
    if (LOG_PRINTF >= 1) {  \
        code                \
    }                       \
}
#define LOG2(code) {        \
    if (LOG_PRINTF >= 2) {  \
        code                \
    }                       \
}

// Colors----------------------------------------------------------------------
#define BLACK       "\033[1;30m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define ORANGE      "\033[1;33m"
#define BLUE        "\033[1;34m"
#define PURPLE      "\033[1;35m"
#define CYAN        "\033[1;36m"
#define GRAY        "\033[1;37m"

#define BLACK_UNL   "\033[4;30m"
#define RED_UNL     "\033[4;31m"
#define GREEN_UNL   "\033[4;32m"
#define ORANGE_UNL  "\033[4;33m"
#define BLUE_UNL    "\033[4;34m"
#define PURPLE_UNL  "\033[4;35m"
#define CYAN_UNL    "\033[4;36m"
#define GRAY_UNL    "\033[4;37m"

#define NATURAL     "\033[0m"
// ----------------------------------------------------------------------------

enum validate_level {
    NO_VALIDATE      = 0, // No checks in program
    WEAK_VALIDATE    = 1, // Checks only fields with  O(1) complexity
    MEDIUM_VALIDATE  = 2, // Checks filed, which need O(n) complexity
    STRONG_VALIDATE  = 3, // All checks (hash and others)
    HIGHEST_VALIDATE = 4  // Error will write in log file
};

enum poisons {
    UNINITIALIZED_PTR =  6,
    UNINITIALIZED_INT = -1 * (0xBAD666),

    FREED_ELEMENT     = -1 * (0xBAD667),
    FREED_PTR         = 12
};

#define PRINT_DATE(color) {                                     \
    char* date = (char*)calloc(40, sizeof(char));               \
    printf((color "Time: %s\n" NATURAL), datetime(date));       \
    free(date);                                                 \
    date = NULL;                                                \
};
#define FPRINT_DATE(file) {                                     \
    char* date = (char*)calloc(40, sizeof(char));               \
    fprintf(file, "Time: %s\n", datetime(date));                \
    free(date);                                                 \
    date = NULL;                                                \
};

int isbadreadptr(const void* ptr);
char* datetime(char* calendar_date);

int is_number(char* string);
int digits_number(int number, int radix=10);
int extract_bit(int number, int bit);

char* bin4(int number);
const char* to_string(int number);

int cmp_int(const void* num1, const void* num2);

#endif //BASELIB_H
