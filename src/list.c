#include "list.h"

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

result_node* list_insert_sorted(result_node *list, char *result) {
  result_node *p;

  if((p = (result_node*)malloc(sizeof(result_node))) == NULL) {
    perror("Unable to allocate new result node");
    exit(1);
  }

  p->result = strdup(result);

  if(list == NULL || strcmp(list->result, result) > 0) {
    p->next = list;
    return p;
  }
  else {
    result_node *l = list;
    while(l->next != NULL && strcmp(l->next->result, result) < 0)
      l = l->next;

    p->next = l->next;
    l->next = p;
    return list;
  }
  
}

void list_print(result_node *list) {
  result_node *p;

  for(p = list; p != NULL; p = p->next)
    printf("%s", p->result);
}

void list_free(result_node *list) {
  result_node *p;
  
  while(list != NULL) {
    p = list->next;
    free(list->result);
    free(list);
    list = p;
  }
}

result_node* list_find(result_node *list, const char *match) {
  result_node *p;
  int match_length;

  match_length = strlen(match);
  
  for(p = list; p != NULL; p = p->next) {
    if(strncmp(match, p->result, match_length) == 0)
      return p;
  }
  return NULL;
}

int list_count(result_node *list) {
  int count = 0;
  result_node *p;

  for(p = list; p != NULL; p = p->next)
    count++;

  return count;
}
