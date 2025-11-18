#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD          "\x1b[1m"
#define ANSI_ITALIC        "\x1b[3m"
#define ANSI_FG            "\x1b[38;5;11m"

#define test(name)                                                       \
    do {                                                                 \
        printf("%s -> ", name);                                          \
    } while (0)

#define error(msg, ...)                                                  \
    do {                                                                 \
        printf(ANSI_COLOR_RED "ERROR: " ANSI_COLOR_RESET);               \
        printf("(line %d) ", __LINE__);                                  \
        printf(msg, ##__VA_ARGS__);                                      \
        printf("\n");                                                    \
        while (1) {}                                                     \
    } while (0)

#define pass(msg, ...)                                                   \
    do {                                                                 \
        printf(ANSI_COLOR_BLUE ANSI_ITALIC "passed " ANSI_COLOR_RESET);  \
        printf(msg);                                                     \
        printf("\n");                                                    \
    } while (0)
