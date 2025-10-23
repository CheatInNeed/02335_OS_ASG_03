
/* You are not allowed to use <stdio.h> */
#include "io.h"
#include <stdlib.h>

/**
 * @name  main
 * @brief This function is the entry point to your program
 * @return 0 for success, anything else for failure
 *
 *
 * Then it has a place for you to implementation the command
 * interpreter as  specified in the handout.
 */

// A simple dynamic array based collection of ints
typedef struct {
    int *data;
    int size;
    int capacity;
} Collection;

// initialize the collection
static void col_init(Collection *c) {
    c->size = 0;
    c->capacity = 4;
    c->data = (int*)malloc(c->capacity * sizeof(int));
}

// adds an int to the collection, resizing if necessary
// returns 0 on success, EOF on failure
static int col_add(Collection *c, int value) {
    if (c->size == c->capacity) {
        int newCapacity = c->capacity * 2;
        int *p = (int*)realloc(c->data, newCapacity * sizeof(int));
        if (!p) return EOF;
        c->data = p;
        c->capacity = newCapacity;
    }
    c->data[c->size++] = value;
    return 0;
}
// removes the most recently added int from the collection
// does nothing if the collection is empty
static void col_pop(Collection *c) {
    if (c->size > 0) c->size--;
}
// frees the collection's resources
static void col_free(Collection *c) {
    free(c->data);
    c->data = NULL;
    c->size = c->capacity = 0;
}

// prints the collection in the specified format
// returns 0 on success, EOF on failure
static int col_print(const Collection *c) {
    if (c->size == 0) {
        if (write_char(';') == EOF) return EOF;
        if (write_char('\n') == EOF) return EOF;
        return 0;
    }
    for (int i = 0; i < c->size; i++) {
        if (write_int(c->data[i]) == EOF) return EOF;
        if (i < c->size - 1) {
            if (write_char(',') == EOF) return EOF;
        }
    }
    if (write_char(';') == EOF) return EOF;
    if (write_char('\n') == EOF) return EOF;
    return 0;
}

// main command interpreter loop
int main(void) {
    int counter = 0;
    Collection col;
    col_init(&col);

    while (1) {
        int c = read_char();
        if (c == EOF) {
            // EOF stop processing
            break;
        }

        if (c == 'a') {
            if (col_add(&col, counter) == EOF) {
                // error at adding, stop processing
                break;
            }
            counter++;
        } else if (c == 'b') {
            counter++;
        } else if (c == 'c') {
            col_pop(&col);
            counter++;
        } else {
            // any other char stops processing
            break;
        }
    }

    // print the collection and clean up
    col_print(&col);
    col_free(&col);
    return 0;
}

