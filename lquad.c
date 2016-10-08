/* lquad
Lazear's Linked List Library
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct node {
	void* data;

	struct node* prev;
	struct node* next;		/* Pointer to next item in the list */

	struct node* last;		/* Pointer to the last item in the list */
	struct node** head;		/* Pointer to the beginning of the list */

} __attribute__((packed));


struct node* new_node() {
	struct node* p = malloc(sizeof(struct node));
	p->data = NULL;
	p->next = NULL;
	p->prev = NULL;
	p->last = NULL;
	p->head = NULL;

	return p;
}

void push(struct node** head, struct node* n) {
	n->prev = (*head)->last;	/* old end-of-list is previous item */

	(*head)->last->next = n;	/* old end-of-list's next item is n */
	(*head)->last = n;			/* end-of-list is n */
	n->head = head;				/* beginning of list is head */
}


struct node* pop(struct node** head) {

	return (*head)->last;
}


void traverse_list(struct node** head) {
	struct node** h;
	for (h = head; *h; h = &(*h)->next) {
		printf("%x (prev %x next %x) (%x %x)", \
			(*h), (*h)->prev, (*h)->next, (*h)->last, *(*h)->head);
	}
}

int list_size(struct node** head) {
	struct node** h;
	int i = 0;
	for (h = head; *h; h = &(*h)->next)
		i++;
	return i;
}


int main(int argc, char* argv[]) {

	struct node* head = new_node();
	head->last = head;
	head->head = &head;

	printf("head @ %x\n", head);

	int i = 0;
	for (i = 0; i < 24; i++)
		push(&head, new_node());

	traverse_list(&head);
	printf("num nodes %d\n", list_size(&head));
	return 0;
}

