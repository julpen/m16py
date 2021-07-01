#include <../interpreter/scanner.h>
#include <../interpreter/interpreter.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

int main(int argc, char const *argv[])
{
    NodeType nodes[1000];
    scannerInit(&consumeToken);

    interpreterInit(&putchar, nodes);

    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);

    read_history(".history");

    while(true) {
        char *l = readline(">");
        if (l == NULL) { // EOF
            write_history(".history");
            putchar('\n');
            return 0;
        }

        if (strlen(trim(l)) > 0) {
            add_history(l);
        }

        if (strncmp("load ", l, 5) == 0) { // Load command
            FILE *f;
            char *file = trim(&l[5]);
            f = fopen(file, "r");

            if (f == NULL) {
                printf("Could not open file '%s'\n", file);
            }
            else {
                char input[11];

                int readBytes = fread(input, 1, 10, f);
                while (readBytes != 0) {
                    input[readBytes] = '\00';
                    printf("%s", input);
                    processStr(input, readBytes);
                    readBytes = fread(input, 1, 10, f);
                }
            }
        }
        else {
            processStr(l, strlen(l));
            processStr("\n", 1);
            free(l);
        }
    }

    write_history(".history");
    return 0;
}
