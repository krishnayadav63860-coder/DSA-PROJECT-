#include "common.h"

void trim_whitespace(char *str) {
    if (!str) return;

    /* Leading whitespace */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    /* All spaces */
    if (*start == '\0') {
        *str = '\0';
        return;
    }

    /* Trailing whitespace */
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int str_case_cmp(const char *s1, const char *s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

void safe_read_line(char *buffer, size_t size) {
    if (!buffer || size == 0) return;

    if (fgets(buffer, (int)size, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        trim_whitespace(buffer);
    } else {
        buffer[0] = '\0';
        clearerr(stdin);
    }
}

int safe_read_int(int min_val, int max_val) {
    char buf[64];
    long val;
    char *endptr;

    while (1) {
        printf("Choice (%d - %d): ", min_val, max_val);
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            clearerr(stdin);
            return min_val;
        }

        buf[strcspn(buf, "\r\n")] = '\0';
        trim_whitespace(buf);

        if (buf[0] == '\0') {
            printf("[!] Input cannot be empty. Please enter a valid number.\n");
            continue;
        }

        val = strtol(buf, &endptr, 10);
        if (*endptr != '\0') {
            printf("[!] Invalid input. Please enter numeric digits only.\n");
            continue;
        }

        if (val < min_val || val > max_val) {
            printf("[!] Value out of bounds. Please enter a number between %d and %d.\n", min_val, max_val);
            continue;
        }

        return (int)val;
    }
}
