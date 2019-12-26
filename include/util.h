#pragma once

#include <stdio.h>

extern FILE *LOG_FILE;

// LINKED LIST 
struct ll_st {
  void *val;
  struct ll_st *next;
};

typedef struct ll_st* ll_t;

/* New Linked List
 *
 * Create a new linked list
 */
ll_t ll_new();

/* Append to Linked List
 * l - Linked list to append data to
 * data - Data to append
 *
 * Append data to the end of a linked list
 */
void ll_append(ll_t l, void *data);

int ll_length(ll_t ll);

// ERROR

/* Print Error Message and Exit
 * message - Message to print
 * 
 * Prints error message and exits program with exit code 1 
 */
void error_and_exit(char* message, int line_no);

// Writes to the log file
// (right now this is just pointed at stdout
void write_log(char *msg, ...);

// Append an error to the error queue
// Errors are FIFO
void append_error(char *file, int line, int col, char *title, char *msg, ...);

// Print all errors in order that they were appended
void print_errors();

int had_errors();

char* concat(char* s1, char* s2);

char* itoa(unsigned int x);

char* remove_empty_lines(char* s);

unsigned long f_to_int(char *d);
