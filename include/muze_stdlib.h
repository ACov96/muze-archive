#ifndef _MUZE_STDLIB_H
#define _MUZE_STDLIB_H

typedef unsigned long type_descriptor_t;
typedef struct data_st *data_t;
typedef void *member_t;
typedef data_t (*morph_f)(data_t d);

/* Here's the skinny on the self-descriptive data structure:
 * - self is just a pointer to itself. This may seem redundant, 
 *   but it's important for getting the type information.
 * - type_overflow is the overflow space in case the program has more than
 *   2047 types.
 * - length describes how many 64-bit words are allocated in the data 
 *   section of the structure (which part of the union to use). This doesn't
 *   give you all of the information in order to use the union, you still
 *   need to extract the type information.
 * - members are the data values for the data structure. length describes the
 *   length of this array.
 */
struct data_st {
  // DATA HEADER
  data_t self;
  unsigned long type_overflow;
  unsigned long length;

  // DATA MEMBERS
  member_t members[];
};

/* Create a new data structure.
 *
 * size - Number of children to allocate for the structure. If you are
 *        allocating a primitive type (integer, real, boolean, string),
 *        just pass 1.
 *
 * Returns a data_t that is partially complete. Caller of this method is
 * responsible for doing the following after allocating the data_t:
 * - Assign the various members
 * - Call the helper function to set the appropriate type header
 */
data_t __create_new_data(unsigned long size);

/* Set the type header of a data structure.
 * Still WIP.
 */
void __set_data_type_header(data_t *d, type_descriptor_t td);

type_descriptor_t __get_data_type_header(data_t d);

/* Get the child at index idx of data structure d.
 * Still WIP.
 */
member_t __get_data_member(data_t d, int idx);

/* Set the child data structure c to the index idx of parent data structure d.
 * Still WIP.
 */
void __set_data_member(data_t d, member_t c, int idx);

void panic(char *msg);
data_t __morph(data_t d, char *target);
void __assign_simple(data_t src, data_t dest);

#endif
