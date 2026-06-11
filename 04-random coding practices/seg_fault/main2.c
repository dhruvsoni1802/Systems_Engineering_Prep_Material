#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Use GDB to figure out the error here
typedef struct {
    char *name;
    int  *scores;
    int   count;
} Student;

Student *create_student(const char *name, int count) {
    Student *s  = malloc(sizeof(Student));
    s->name     = strdup(name);
    s->scores   = malloc(sizeof(int) * count);
    s->count    = count;
    return s;
}

int average_score(Student *s) {
    int sum = 0;
    for (int i = 0; i <= s->count; i++) {
        sum += s->scores[i];
    }
    return sum / s->count;
}

void print_student(Student *s) {
    printf("Name: %s | Average: %d\n", s->name, average_score(s));
}

void free_student(Student *s) {
    free(s->scores);
    free(s->name);
    free(s);
}

int main(void) {
    const char *names[] = { "Alice", "Bob", NULL };
    int counts[]        = { 3, 4 };
    int scores[][4]     = {
        { 80, 90, 85, 0 },
        { 70, 60, 75, 80 }
    };

    Student *students[2];

    for (int i = 0; i < 2; i++) {
        students[i] = create_student(names[i], counts[i]);
        memcpy(students[i]->scores, scores[i], sizeof(int) * counts[i]);
    }

    for (int i = 0; i <= 2; i++) {
        print_student(students[i]);
    }

    for (int i = 0; i < 2; i++) {
        free_student(students[i]);
    }

    return 0;
}