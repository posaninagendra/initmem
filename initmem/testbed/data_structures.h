#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

typedef struct
{
	int no_of_keys;	//The number of keys this node holds
	int *keys;	//Pointer to the Node's keys
	long *children_offsets;	//Pointer to the childrens' offsets in the file
}btree_node, *btree_node_ptr, **btree_node_dptr;

typedef struct node
{
	long offset;
	struct node *next;
}stack;

typedef struct node_queue
{
	//int node_level;
	long node_offset;
	long queue_position;
	struct node_queue *next;
}queue, *queue_ptr;

typedef struct queue_ret
{
	int level;
	long offset;
	long queue_position;
}q_ret, *q_ret_ptr;

//-----------------------Stack API
void push(long offset);

//Returns -1 if stack is empty
long pop();

//Returns true if empty
bool stack_is_empty();

//Returns the top of the stack, -1 if stack is empty
long peek();

void print_stack();

void cleanup_stack();

//-----------------------------------------Queue API for printing to do a BFS---------------------------
//Enqueue at rear, dequeue at head
//This is to enqueue a node's metadata into the queue. node_level is its depth in the B tree, child_offset is its position in the file
void enqueue( long child_offset );

//Dequeue function. Will return the offset of the node that is pointed to by the head of the queue
q_ret_ptr dequeue();

//Print queue
void print_queue();

bool queue_is_empty();
//-------------------------------------------End of queue API
//Returns a new initialized node
//All keys set to -1, All offsets set to -1
btree_node_ptr new_node_init();

btree_node_ptr new_aux_node();

//find a given key in the btree
int find_key_in_btree(char *input_filename,int key);

//Write a node's contents to the index file
void write_node(btree_node_ptr node);

//Populate parent pointers during a tree walk
void populate_parents(char *input_filename, int key);

//A helper function for qsort
int compare(const void *a, const void *b);

void insert_in_node( long leaf_offset, int node_keys, int key);

//Creates a new node for the right child and returns its offset
long get_child_after_split(int arr[], int size, int split_value);

//Prints a node's contents at offset offset
void print_node_at_offset(long offset);

btree_node_ptr get_node_at_offset(long offset);

//Updates the left child of the btree_node in case of a split
void update_left_child(int temp_arr[], int size, long leaf_offset, int split_value);

//Change root node. *
void check_parent_and_update(int split_value, long left_child_offset, long right_child_offset);

//Subroutine that will be called recursively. This function adds KL and KR and sends the median value up
void add_with_split(int node_keys, long leaf_offset, int key, btree_node_ptr aux_node, long parent_offset);

//This is the routine that will be called once
void add_key_to_tree(int node_keys, long leaf_offset, int key);

//---------------------Print utility---------------------------
void print_tree(char *input_filename);

void write_node_at_offset(btree_node_ptr node, long offset);
