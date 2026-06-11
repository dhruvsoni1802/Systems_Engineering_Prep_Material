#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Reverses a string in-place
void reverse_string(char *s) {
    int left  = 0;
    int right = strlen(s) - 1;
    while (left < right) {
        char tmp  = s[left];
        s[left]   = s[right];
        s[right]  = tmp;
        left++;
        right--;
    }
}

// Returns count of a character in a string
int count_char(const char *s, char c) {
    int count = 0;
    while (*s) {
        if (*s == c) count++;
        s++;
    }
    return count;
}

// Concatenates two strings into a newly malloc'd buffer
// Caller is responsible for freeing
char *concat(const char *a, const char *b) {
    char *result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

int main()
{
  char s[10] = "Hello!";
  reverse_string(s);

  //Checking function 1
  assert(strcmp(s,"!olleH") == 0);

  //Checking function 2
  assert(count_char(s,'l') == 2);

  //Checking function 3
  char a[10] = "Hello";
  char b[10] = "World";
  assert(strcmp(concat(a,b),"HelloWorld") == 0);


  return 0;
}