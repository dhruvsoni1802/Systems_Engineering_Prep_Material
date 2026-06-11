#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Use compiler flags to figure out error here (if any)
char *repeat_word(const char *word, int times) {
    char *result = malloc(strlen(word) * times);
    result[0] = '\0';
    for (int i = 0; i < times; i++) {
        strcat(result, word);
    }
    return result;
}

int main(void) {
    char *words[] = { "hello ", "world ", "foo " };
    int reps[]    = { 3, 2, 4 };

    for (int i = 0; i <= 3; i++) {
        char *s = repeat_word(words[i], reps[i]);
        printf("%s\n", s);
        free(s);
    }

    return 0;
}