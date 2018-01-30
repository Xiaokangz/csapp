/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    /* Remember to handle the case if malloc returned NULL */
    queue_t *q = (queue_t *)malloc(sizeof(queue_t));
    if (q == NULL) {
      return NULL;
    }
    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* Remember to free the queue structue and list elements */
    if (q == NULL) {
        return;
    }
    list_ele_t *head = q->head;
    list_ele_t *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, int v)
{
    /* What should you do if the q is NULL? */
    /* What if malloc returned NULL? */
    if (q == NULL) {
      return false;
    }
    list_ele_t *head = (list_ele_t *)malloc(sizeof(list_ele_t));
    if (head == NULL) {
        return false;
    }
    head->value = v;
    head->next = q->head;
    if (q->head == NULL) {
        q->tail = head;
    } 
    q->head = head;
    q->size = q->size + 1;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, int v)
{
    /* Remember: It should operate in O(1) time */
    if (q == NULL) {
      return false;
    }
    list_ele_t *tail = (list_ele_t *)malloc(sizeof(list_ele_t));
    if (tail == NULL) {
        return false;
    }
    tail->value = v;
    tail->next = NULL;
    if (q->head == NULL) {
        q->tail = q->head = tail;
    } else {
        q->tail->next = tail;
        q->tail = tail;
    }
    q->size = q->size + 1;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, int *vp)
{
    if (q == NULL || q->head == NULL) {
      return false;
    }
    int temp;
    list_ele_t *head = q->head;
    temp = q->head->value;
    if (q->size == 1) {
        q->head = q->tail = NULL;
    } else {
        q->head = head->next;
    }
    q->size = q->size - 1;
    free(head);
    if (vp != NULL) {
      *vp = temp;
    }
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* Remember: It should operate in O(1) time */
    if (q == NULL) {
      return 0;
    } else {
      return q->size;
    }
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
    if (q == NULL || q->head == NULL) {
      return;
    }
    list_ele_t *new_head = NULL;
    list_ele_t *head = q->head;
    list_ele_t *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        temp -> next = new_head;
        new_head = temp;
    }
    q->tail = q->head;
    q->head = new_head;
}

