#include <stdio.h>
#include <stdlib.h>

void removeComments(FILE *in, FILE *out) {
    char ch, next;

    while ((ch = fgetc(in)) != EOF) {

        if (ch == '/') {
            next = fgetc(in);

            // Single-line comment
            if (next == '/') {
                while ((ch = fgetc(in)) != '\n' && ch != EOF);
                fputc('\n', out);
            }

            // Multi-line comment
            else if (next == '*') {
                char prev = 0;
                while ((ch = fgetc(in)) != EOF) {
                    if (prev == '*' && ch == '/')
                        break;
                    prev = ch;
                }
            }

            // Not a comment
            else {
                fputc(ch, out);
                ungetc(next, in);
            }
        }
        else {
            fputc(ch, out);
        }
    }
}

int main() {
    FILE *in = fopen("input_scanner.c", "r");
    FILE *out = fopen("output_no_comments.c", "w");

    if (in == NULL || out == NULL) {
        printf("File error\n");
        return 1;
    }

    removeComments(in, out);

    fclose(in);
    fclose(out);

    printf("Comments removed successfully.\n");
    return 0;
}
