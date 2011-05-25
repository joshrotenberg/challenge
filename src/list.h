/**
 * @file list.h
 * @brief Result Linked List Declarations
 */

#ifndef __LIST_H__
#define __LIST_H__

/**
 *
 * a node for holding a child result string
 */
typedef struct result_node {
  struct result_node *next;
  char *result;
} result_node;

/**
 *
 * insert a node into the list in sorted order (using strcmp)
 *
 * @param list the current result_node
 * @param result the child result string
 *
 * @return the next node in the list
 */
result_node* list_insert_sorted(result_node *list, char *result);

/**
 * count the number of elements in a list
 *
 * @param list the list
 * 
 * @return the number of elements found
 */
int list_count(result_node *list);

/**
 * print the list contents
 *
 * @param list the list 
 */
void list_print(result_node *list);

/**
 * free the list's memory
 *
 * @param list the list
 */
void list_free(result_node *list);

/**
 * find (the first) item in the list that strncmps the match param
 *
 * @param list the list
 * @param match the string to look for (in the result item)
 *
 * @return the found node, or NULL if not found
 */
result_node* list_find(result_node *list, const char* match);

#endif
