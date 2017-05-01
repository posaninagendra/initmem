#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<math.h>
#include "data_structures.h"

int btree_order;
long root_offset;
bool debug;
FILE *fin;
int size_of_tree;
long leaf_offset;
stack *top=NULL;
queue *head=NULL;
queue *tail=NULL;
long queue_position = 0;


//Stack API
void push(long offset)
{
	stack *new_element = (stack *)malloc(sizeof(stack));
	new_element->offset = offset;
	new_element->next = NULL;
	if(top == NULL)
		top = new_element;
	else
	{
		new_element->next = top;
		top = new_element; 
	}
}

long pop()
{
	if(top==NULL)
		return -1;
	else
	{
		long pop_val = top->offset;
		top = top->next;
		return pop_val;
	}
}

bool stack_is_empty()
{
	if(top==NULL)
		return true;
	else
		return false;
}

long peek()
{
	if(top==NULL)
		return -1;
	else
		return top->offset;
}

void print_stack()
{
	if(top==NULL)
		printf("Stack is empty\n");
	else
	{
		printf("This prints the parents of the nodes visited in a bottom up fashion\n");
		stack *temp = top;
		for ( temp = top ; temp !=NULL ; temp = temp->next )
			printf("%ld --> ", temp->offset);
		printf("NULL\n");
	}
}

void cleanup_stack()
{
	top=NULL;
}

//-----------------------------------Queue API-------------------------------------------
void enqueue(long node_offset )
{
	queue *new_element = (queue *) malloc(sizeof(queue));
	//new_element->node_level = node_level;
	new_element->node_offset = node_offset;
	new_element->queue_position = queue_position;
	queue_position++;
	new_element->next = NULL;
	//Need to enqueue at tail
	if(head==NULL)
	{
		//check for a redundant check
		if(tail == NULL){}
			//if(debug)	printf("Yes, it is truly the 1st element\n");
		tail = new_element;
		head = new_element;
	}
	//Not the 1st element
	else
	{
		tail->next = new_element;
		tail = new_element;
	}
}


q_ret_ptr dequeue()
{
	if(head==NULL)
	{
		q_ret_ptr ret = (q_ret_ptr) malloc(sizeof(q_ret));
		ret->level = -1;
		ret->offset = -1;
		ret->queue_position = -1;
		return ret;
	}
	else
	{
		q_ret_ptr ret = (q_ret_ptr) malloc(sizeof(q_ret));
		//ret->level = head->node_level;
		ret->offset = head->node_offset;
		ret->queue_position = head->queue_position;
		//Queue only has 1 element
		if(head==tail)
		{
			head=NULL;
			tail=NULL;
		}
		else
			head = head -> next;
		return ret;
	}
}

void print_queue()
{
	if(head==NULL)
		printf("Queue Empty\n");
	else
	{
		queue *temp = head;
		while(temp!=NULL)
		{
			printf("(%ld, %ld) --> ",temp->queue_position, temp->node_offset);
			temp=temp->next;
		}
		printf("NULL\n");
	}
}

bool queue_is_empty()
{
        if(head==NULL && tail==NULL)
                return true;
        else
                return false;
}

//-----------------------------------Queue API ends here-------------------------------

int compare(const void *a, const void *b)
{
        int *a1=(int *) a;
        int *b1=(int *) b;

        if(*a1 < *b1)
                return -1;
        else if ( *a1 == *b1)
                return 0;
        else
                return 1;
}


btree_node_ptr new_element_node_init()
{
	btree_node_ptr new_element= (btree_node_ptr)malloc(sizeof(btree_node));
	new_element->no_of_keys = 0;
	new_element-> keys = ( int *) malloc(sizeof(int)*(btree_order - 1));
	//Initialize all of them to -1
	int i=0;
	for(i=0;i<btree_order-1;i++)
		new_element->keys[i] = -1;
	
	new_element->children_offsets = (long *) malloc(sizeof(long)*btree_order);
	//Initialize all of them to -1
	for(i=0;i<btree_order;i++)
		new_element->children_offsets[i] = -1;

	return new_element;
}

btree_node_ptr new_element_aux_node()
{
        btree_node_ptr new_element= (btree_node_ptr)malloc(sizeof(btree_node));
        new_element->no_of_keys = 0;
        new_element-> keys = ( int *) malloc(sizeof(int)*(btree_order));
        //Initialize all of them to -1
        int i=0;
        for(i=0;i<btree_order;i++)
                new_element->keys[i] = -1;

        new_element->children_offsets = (long *) malloc(sizeof(long)*(btree_order + 1));
        //Initialize all of them to -1
        for(i=0;i<btree_order + 1;i++)
                new_element->children_offsets[i] = -1;

        return new_element;
}

btree_node_ptr get_node_at_offset(long offset)
{
	btree_node_ptr new_element = new_element_node_init();
	fseek(fin,0,SEEK_SET);
	fseek(fin, offset, SEEK_CUR);
	int no_of_keys = 0;
	fread(&no_of_keys, sizeof(int), 1, fin);
	new_element->no_of_keys = no_of_keys;
	int i=0;
	int test;
	for(i=0;i<btree_order - 1;i++)
	{
		fread(&test, sizeof(int), 1, fin);
		new_element->keys[i] = test;
	}

	//Reached at the offset's begining
	long test_off;
	for(i=0;i<btree_order;i++)
	{
		fread(&test_off, sizeof(long), 1, fin);
		new_element->children_offsets[i] = test_off;
	}

	return new_element;
}

//Returns 0 if not found, else returns 1
int find_key_in_btree(char *input_filename, int key)
{
	bool debug = false;
	if(root_offset==-1)	//Tree is empty
		return 0;
	else
	{
		//FILE *fin;
		//fin = fopen (input_filename,"r+b");
		if(fin==NULL)
		{
			perror("Index file open error:");
			return 0;
		}
		//Seek to 0 + sizeof(long) to point to the 1st element
		//fseek(fin,sizeof(long),SEEK_SET);
		//Go to the root
		fseek(fin,root_offset,SEEK_SET);
		//Verify if the root is reached
		if(debug)	printf("In find_key()\n");
		long curr_pos=ftell(fin);
		if(debug)
		{
			printf("Curr position : %ld\n",curr_pos);
			printf("Root offset + sizeof(long) = %ld\n",root_offset);
		}

		int keys_in_node=0;
		int key_to_test=0;
		long child_offset_to_go=0;
		/*fread(&keys_in_node, 1, sizeof(int), fin);
		if(keys_in_node==0)
		{
			//Root has no element
			if(debug)	printf("Root has no element\n");
			return 0;
		}
		*/
		//Iterate now until you hit a leaf
		//Have a pointer point to the offset where children offsets begin
		FILE *f_child_offset;
		f_child_offset = fopen ( input_filename, "rb");
		if(f_child_offset == NULL)
		{
			perror("Index file read error in find_key():");
			exit(-1);
		}
		
		while(1)
		{
			//Compare key to all the node's elements
			//btree_order holds the tree order
			fread(&keys_in_node, sizeof(int), 1, fin);
			if(debug)	printf("find_key() : Keys in node : %d\n",keys_in_node);
			if(keys_in_node==0)
			{
				//Node has no key
				if(debug)	printf("Root has no element\n");
				return 0;
			}

			fseek(f_child_offset, ftell(fin), SEEK_SET);
			//Point child offset to where it should be
			fseek(f_child_offset, (btree_order -1)*sizeof(int) , SEEK_CUR);
			long offset=ftell(f_child_offset);
			if(debug)	printf("f_child_offset : %ld\n",offset);
			if(debug)	printf("f_in : %ld\n",ftell(fin));


			int keys_read=0;
			bool greatest = false;
			bool node_full = false;
			bool smallest = false;
			key_to_test = -10;
			child_offset_to_go=0;
			while(key > key_to_test)
			//for(i=0 ; i < btree_order - 1;i++)
			{
				//Investigate each key and determine where to go
				//Check here if all the keys have been read
				if(keys_read == keys_in_node)
				{
					greatest=true;
					if(keys_read == btree_order - 1)
						node_full = true;
					break;
				}
				if(debug)	printf("find_key() : Reading from offset : %ld\n",ftell(fin));
				fread(&key_to_test, sizeof(int), 1, fin);
				if(debug)	printf("find_key() : Key_to_test : %d\n",key_to_test);
				keys_read++;
				if(key==key_to_test)
					return 1; 
			}
			if(key <= key_to_test && keys_read==1)
				smallest = true;

			//Check if the key is the greatest
			if(node_full)
			{
				//Take the child offset pointer to the last offset field
				fseek( f_child_offset, (btree_order - 1)*sizeof(long), SEEK_CUR);
				long test_off;
				test_off = ftell(f_child_offset);
				if(debug)	printf("Last child offset : %ld\n",test_off);
			}
			
			else
			{
				//Loop broken where key < key_to_test
				//Should be always the case
				if(!smallest)
				{
					if(greatest)
						fseek( f_child_offset, (keys_read)*sizeof(long), SEEK_CUR);
					else
						fseek( f_child_offset, (keys_read - 1)*sizeof(long), SEEK_CUR);
				}
				long test_off;
				test_off = ftell(f_child_offset);
				if(debug)	printf("child offset to go to: %ld\n",test_off);
			}
		
			//Checking if there is a child node at this offset
			fread( &child_offset_to_go, sizeof(long), 1, f_child_offset);
			if(child_offset_to_go==-1)
			{
				if(debug)	printf("find_key() : Returning 0 from the function as the child offset was found to be -1\n");
				return 0;
			}
			//Continue, Set the pointers
			else
			{
				//Set child_offset to the start of file
				fseek(f_child_offset, 0, SEEK_SET);

				//Set fin
				fseek(fin, 0, SEEK_SET);
				//fseek(fin, sizeof(long), SEEK_CUR);
				fseek(fin, child_offset_to_go, SEEK_CUR);
			}
		}
		
	}
}

void write_node(btree_node_ptr node)
{
	bool debug = false;
	if(fin==NULL)
	{
		if(debug)	printf("In write_node() : Received a NULL file pointer\n");
		return;
	}

	//Place the file pointer in the END
	if(root_offset==-1)
		fseek(fin, sizeof(long), SEEK_SET);
	else
		fseek(fin, 0, SEEK_END);

	//btree_node_ptr node = *addr_node;

	//Write the node's #ofkeys
	if(debug)	printf("Node's keys : %d\n",node->no_of_keys);
	fwrite(&(node->no_of_keys), sizeof(int), 1, fin);

	//Write the node's keys
	int i=0;
	if(debug)	printf("Tree order : %d\n",btree_order);
	for(i=0;i < btree_order - 1;i++)
	{
		if(debug)	printf("Writing node value : %d\n",node->keys[i]);
		fwrite(&(node->keys[i]), sizeof(int), 1, fin);
	}

	//Write the children offsets now
	i=0;
	for(i=0;i<btree_order;i++)
	{
		if(debug)	printf("Writing child offset value : %ld\n",node->children_offsets[i]);
		fwrite(&(node->children_offsets[i]), sizeof(long), 1, fin);
	}

	//Place the file pointer at logical 0
	fseek(fin, sizeof(long), SEEK_SET);
}

void write_node_at_offset(btree_node_ptr node, long offset)
{
	bool debug = false;
        if(fin==NULL)
        {
                if(debug)       printf("In write_node() : Received a NULL file pointer\n");
                return;
        }

        //Place the file pointer at 'offset'
	fseek(fin, offset, SEEK_SET);
        

        //btree_node_ptr node = *addr_node;

        //Write the node's #ofkeys
        if(debug)       printf("Node's keys : %d\n",node->no_of_keys);
        fwrite(&(node->no_of_keys), sizeof(int), 1, fin);

        //Write the node's keys
        int i=0;
        if(debug)       printf("Tree order : %d\n",btree_order);
        for(i=0;i < btree_order - 1;i++)
        {
                if(debug)       printf("Writing node value : %d\n",node->keys[i]);
                fwrite(&(node->keys[i]), sizeof(int), 1, fin);
        }

        //Write the children offsets now
        i=0;
        for(i=0;i<btree_order;i++)
        {
                if(debug)       printf("Writing child offset value : %ld\n",node->children_offsets[i]);
                fwrite(&(node->children_offsets[i]), sizeof(long), 1, fin);
        }

        //Place the file pointer at logical 0
        fseek(fin, sizeof(long), SEEK_SET);
}

void populate_parents(char *input_filename, int key)
{
	bool debug = false;
	if(root_offset==-1)     //Tree is empty
                return;
        else
        {
                if(fin==NULL)
                {
                        perror("Index file open error:");
                        return;
                }
                //Go to the root
                fseek(fin,root_offset,SEEK_SET);
                //Verify if the root is reached
                if(debug)       printf("In populate_parents()\n");
                long curr_pos=ftell(fin);
                if(debug)
                {
                        printf("Curr position : %ld\n",curr_pos);
                        printf("Root offset + sizeof(long) = %ld\n",root_offset);
                }

                int keys_in_node=0;
                int key_to_test=0;
                long child_offset_to_go=0;

                FILE *f_child_offset;
                f_child_offset = fopen ( input_filename, "rb");
                if(f_child_offset == NULL)
                {
                        perror("Index file read error in populate_parents():");
                        exit(-1);
                }

		long curr_offset = root_offset;
		long prev_offset = root_offset;
		int curr_node = (root_offset - sizeof(long))/sizeof(btree_node);
		int prev_node = (root_offset - sizeof(long))/sizeof(btree_node); 

                while(1)
                {
                        //Compare key to all the node's elements
                        //btree_order holds the tree order
                        fread(&keys_in_node, sizeof(int), 1, fin);
                        if(debug)       printf("populate_parents() : Keys in node : %d\n",keys_in_node);
                        if(keys_in_node==0)
                        {
                                //Node has no key
                                if(debug)       printf("Root has no element\n");
                                return;
                        }

                        fseek(f_child_offset, ftell(fin), SEEK_SET);
                        //Point child offset to where it should be
                        fseek(f_child_offset, (btree_order -1)*sizeof(int) , SEEK_CUR);
                        long offset=ftell(f_child_offset);
                        if(debug)       printf("f_child_offset : %ld\n",offset);
                        if(debug)       printf("f_in : %ld\n",ftell(fin));


                        int keys_read=0;
                        bool greatest = false;
                        bool node_full = false;
                        bool smallest = false;
                        key_to_test = -10;
                        child_offset_to_go=0;

                        while(key > key_to_test)
                        //for(i=0 ; i < btree_order - 1;i++)
                        {
                                //Investigate each key and determine where to go
                                //Check here if all the keys have been read
                                if(keys_read == keys_in_node)
                                {
                                        greatest=true;
                                        if(keys_read == btree_order - 1)
                                                node_full = true;
                                        break;
                                }
                                if(debug)       printf("populate_parents() : Reading from offset : %ld\n",ftell(fin));
                                fread(&key_to_test, sizeof(int), 1, fin);
                                if(debug)       printf("populate_parents() : Key_to_test : %d\n",key_to_test);
                                keys_read++;
                        }
                        if(key <= key_to_test && keys_read==1)
                                smallest = true;

                        //Check if the key is the greatest
                        if(node_full)
                        {
                                //Take the child offset pointer to the last offset field
                                fseek( f_child_offset, (btree_order - 1)*sizeof(long), SEEK_CUR);
                                long test_off;
                                test_off = ftell(f_child_offset);
                                if(debug)       printf("Last child offset : %ld\n",test_off);

				//Root node
				if(curr_node == (root_offset - sizeof(long))/sizeof(btree_node))
				{
					//res[curr_node] = -10;	//Parent of root doesnt exist
					push(-10);
				}
				else
				{
					//res[curr_node] = prev_offset;
					//Pushing the parent onto the stack
					push(prev_offset);
				}
                        }

                        else
                        {
                                //Loop broken where key < key_to_test
                                //Should be always the case
                                if(!smallest)
                                {
                                        if(greatest)
                                                fseek( f_child_offset, (keys_read)*sizeof(long), SEEK_CUR);
                                        else
                                                fseek( f_child_offset, (keys_read - 1)*sizeof(long), SEEK_CUR);
                                }
                                long test_off;
                                test_off = ftell(f_child_offset);
                                if(debug)       printf("child offset to go to: %ld\n",test_off);

				//Root node
				if(curr_node == (root_offset - sizeof(long))/sizeof(btree_node))
				{
					//res[curr_node] = -10;	//Parent of root doesnt exist
					push(-10);
				}
				else
				{
					//res[curr_node] = prev_offset;
					push(prev_offset);
				}
                        }

                        //Checking if there is a child node at this offset
                        fread( &child_offset_to_go, sizeof(long), 1, f_child_offset);
                        if(child_offset_to_go==-1)
                        {
                                if(debug)       printf("populate_parents() : Returning 0 from the function as the child offset was found to be -1\n");
				leaf_offset = curr_offset;
                                break;
                        }
                        //Continue, Set the pointers
                        else
                        {
                                //Set child_offset to the start of file
                                fseek(f_child_offset, 0, SEEK_SET);

                                //Set fin
                                fseek(fin, 0, SEEK_SET);
                                //fseek(fin, sizeof(long), SEEK_CUR);
                                fseek(fin, child_offset_to_go, SEEK_CUR);
                        }

			//Update the offset variables
			prev_node = curr_node;
			prev_offset = curr_offset;
			curr_node = (ftell(fin) - sizeof(long))/sizeof(btree_node);
			curr_offset = ftell(fin);
                }

        }

	//return res;
}


void insert_in_node( long leaf_offset, int node_keys, int key)
{}
/*
	//Place the fp at the 1st integer read
	fseek(fin, leaf_offset + sizeof(int) , SEEK_SET);
        int temp_arr[node_keys + 1];
        int i=0;
        for(i=0;i<node_keys + 1;i++)
        	temp_arr[i]=-1;
        int key_read = 0;
       	//fin points to the 1st integer that we can read
        for(i=0 ; i < node_keys; i++ )
        {
        	fread(&key_read, sizeof(int), 1, fin);
                if(debug)       printf("Add at root : %d\n", key_read);
                temp_arr[i] = key_read;
        }
        
	temp_arr[i] = key;

        //Sort temp_arr
        qsort(temp_arr, node_keys + 1,sizeof(int), compare);

        //Get the fp back
        fseek(fin, (-1 * node_keys )*sizeof(int), SEEK_CUR);
        fseek(fin, -1*sizeof(int), SEEK_CUR);
        int new_element_keys = node_keys + 1;
        fwrite(&new_element_keys, sizeof(int), 1, fin);

        i=0;
        //Start writing back
        for(i=0;i < node_keys + 1;i++)
        {
        	if(debug)       printf("Add at root : Key %d at offset %ld\n",temp_arr[i],ftell(fin));
             	fwrite(&temp_arr[i] , sizeof(int), 1, fin);
        }
        //Reset fp
                                                
	fseek(fin, 0, SEEK_SET);
}
*/

long get_child_after_split(int arr[], int size, int split_value)
{
	if(fin==NULL)	
	{
		printf("get_child_after_split() : Received a NULL file pointer\n");
		return -1;
	}

	int iter=0;
	for (iter =0; arr[iter] <= split_value ;iter++)
	{}

	//Start writing from index iter
	btree_node_ptr new_element = new_element_node_init();
	//Update the keys of this new_element node
	//int size = sizeof(arr) / sizeof(int);
	if(debug)	printf("Size : %d\n",size);
	int k=0;
	for(;iter < size ; iter++)
	{
		(new_element)->keys[k] = arr[iter];
		if(debug)	printf("Iter : %d\tWrote this to the new_element node : %d\n",iter,(new_element)->keys[k]);
		k++;
	}

	//No need to change the node's child offsets;
	//Change the no. of keys value of the node
	(new_element)->no_of_keys = k;

	//Write this node to the EOF
	fseek(fin, 0, SEEK_END);
	long right_offset = ftell(fin);
	//btree_node_ptr node = *new_element;
	if(debug)	printf("%d\n",(new_element)->no_of_keys);
	if(debug)	printf("Right_offset : %ld\n",right_offset);
	write_node(new_element);
	if(debug)	printf("Returning right_offset : %ld\n",right_offset);
	return right_offset;
}
//Prints a node's contents at offset offset
void print_node_at_offset(long offset)
{
	fseek(fin, offset, SEEK_SET);
	int int_val;
	long long_val;
	printf("At offset : %ld\n",offset);	
	fread(&int_val, sizeof(int), 1, fin);
	printf("Number of keys in the nodes : %d\n",int_val);
	int i=0;
	for(i=0;i < btree_order - 1;i++)
	{
		fread(&int_val, sizeof(int), 1, fin);
		printf("Node's keys : %d\n",int_val);
	}

	for(i=0; i<btree_order; i++)
	{
		fread(&long_val, sizeof(long), 1, fin);
		printf("Node's child offsets : %ld\n",long_val);
	}

	fseek(fin,0,SEEK_SET);
}

void update_left_child(int temp_arr[], int size, long leaf_offset, int split_value)
{
	if(fin==NULL)
	{
		printf("update_left_child(): Received a NULL file pointer \n");
		return;
	}

	fseek(fin,leaf_offset,SEEK_SET);
	//int size = sizeof(temp_arr) / sizeof(int);
	int iter =0;
	for(iter = 0; temp_arr[iter] < split_value ; iter++)
	{}

	//Make all the values now as -1
	int test_key = iter;
	if(debug)	printf("Updated number of keys in the left child : %d\n", test_key);
	fseek(fin, leaf_offset , SEEK_SET);
	
	//Update the 'number of keys in the node'
	fwrite( &test_key, sizeof(int), 1, fin);

	//Take it to split node
	fseek(fin, (iter)*sizeof(int), SEEK_CUR);
	test_key = -1;

	for(; iter < btree_order - 1 ;iter ++)
	{
		if(debug)	printf("%d Written at offset : %ld\n",test_key, ftell(fin));
		fwrite(&test_key, sizeof(int), 1, fin);
	}
	
	//Update the child offsets now
	//Set them to -1
	long long_read = 0;
	long long_write = -1;
	int i=0;
	for(i=0;i<btree_order;i++)
	{
		fread(&long_read, sizeof(long), 1, fin);
		if(long_read != -1)
		{
			fseek(fin, -1*sizeof(long), SEEK_CUR);
			fwrite(&long_write, sizeof(long), 1, fin);
		}
	}
	fseek(fin, 0, SEEK_SET);
}

void check_parent_and_update(int split_value, long left_child_offset, long right_child_offset)
{
	if(fin==NULL)
	{
		printf("check_parent_and_update() : Received a NULL file pointer\n");
		return;
	}
	
	while(!stack_is_empty())
	{
		long off = pop();
		if(debug)	printf("Offset received : %ld\n",off);

		//Currently we only have root
		if(off == -10)
		{
			//Need not recurse further
			btree_node_ptr new_element = new_element_node_init();
			new_element->keys[0] = split_value;
			new_element->children_offsets[0] = left_child_offset;
			new_element->children_offsets[1] = right_child_offset;
			new_element->no_of_keys++;
			
			//Place the fp to the EOF
			fseek(fin, 0, SEEK_END);
			root_offset = ftell(fin);

			//Write node to file
			write_node(new_element);

			//print node
			print_node_at_offset(root_offset);
		}
		
	}
}

void add_with_split(int node_keys, long leaf_offset, int key, btree_node_ptr aux_node, long node_offset)
{
	int split_index = (int) ceil((btree_order)/2);
	int split_value = aux_node -> keys[split_index];

	//We have come here as the node is overfull. So prepare the left and right children
	//Prepare the left child
	btree_node_ptr left_aux = new_element_aux_node();
	
	//Copy the keys upto split_index in the LC
	int i=0;
	for(i=0;i < split_index; i++)
	{
		left_aux->no_of_keys++;
		left_aux->keys[i] = aux_node->keys[i];
		left_aux->children_offsets[i] = aux_node->children_offsets[i];
	}
	//Need to copy the last offset to LC
	left_aux->children_offsets[i] = aux_node->children_offsets[i];
	//Prepare the right child
	btree_node_ptr right_aux = new_element_aux_node();
	int iter=0;
	for(i=split_index + 1; i< btree_order; i++)
	{
		right_aux->no_of_keys++;
		right_aux->keys[iter] = aux_node->keys[i];
		right_aux->children_offsets[iter] = aux_node->children_offsets[i];
		iter++;
	}
	//Need to copy the last offset to RC
	//iter++;
	right_aux->children_offsets[iter] = aux_node->children_offsets[i];

	//Check both the nodes
	if(debug)
	{
		printf("LC IS : \n");
		printf("LC : No. of keys : %d\n", left_aux->no_of_keys);
		printf("LC keys : \n");
		for(i=0;i <  btree_order ; i++)
			printf("%d\t", left_aux->keys[i]);
		printf("\n");
		printf("LC children offsets : \n");
		for(i=0;i < btree_order + 1 ; i++)
			printf("%ld\t",left_aux->children_offsets[i]);
		printf("\n");
		printf("RC IS : \n");
		printf("RC : No. of keys : %d\n", right_aux->no_of_keys);
		printf("RC keys : \n");
		for(i=0;i <  btree_order ; i++)
			printf("%d\t", right_aux->keys[i]);
		printf("\n");
		printf("RC children offsets : \n");
		for(i=0;i < btree_order + 1 ; i++)
			printf("%ld\t",right_aux->children_offsets[i]);
		printf("\n");
	}

	//Check for the parent offset. If root, different things need to be done
	//Look at the parent offfset
	long next_parent = pop();
	long right_child_offset;
	if(next_parent !=-10)
	{
		//-10 will be root
		//Write the children to the files
		if(debug)	printf("LC to be written at : %ld\n",node_offset);
		write_node_at_offset(left_aux, node_offset);

		//Write the RC
		fseek(fin, 0, SEEK_END);
		right_child_offset = ftell(fin);
		if(debug)	printf("RC to be written at : %ld\n",right_child_offset);
		fseek(fin, 0, SEEK_SET);
		write_node(right_aux);
		//print_node_at_offset(right_child_offset);


		//Get the parent's details in an aux node
		btree_node_ptr aux_parent = new_element_aux_node();
		
		//Prepare the parent node
		fseek(fin,0,SEEK_SET);
		if(debug)	printf("Next parent : %ld\n",next_parent);
		fseek(fin,next_parent, SEEK_CUR);

		fread( &(aux_parent->no_of_keys) , sizeof(int), 1, fin);
		for(i=0;i<btree_order - 1;i++)
			fread( &(aux_parent->keys[i]), sizeof(int), 1, fin);
		//Copy the children pointers
		for(i=0 ; i < btree_order ; i++)
			fread( &(aux_parent->children_offsets[i]), sizeof(long), 1, fin);

		if(debug)
		{
			printf("The new_elemently formed parent : \n");
			printf("Parent's keys : \n");
			for(i=0;i<btree_order - 1;i++)
				printf("%d\t", aux_parent->keys[i]);
			printf("\n");
			printf("Children offsets : \n");
			for(i=0 ; i < btree_order ; i++)
				printf("%ld\t",aux_parent->children_offsets[i]);
		}		

		//Now need to insert split_value into its correct rank in the parent and shift the children offsets
		//Taking the loop from find()
		int key_to_test = -10;
		int rank = 0;
		int keys_in_node = aux_parent->no_of_keys;
		bool greatest = false;
		bool node_full = false;
		bool smallest = false;
		int keys_read = 0;
		if(debug)	printf("Keys in node in the parent: %d\n",keys_in_node);
		//Re navigate the fp
		fseek(fin,next_parent + sizeof(int), SEEK_SET);
		 while(split_value > key_to_test)
                        //for(i=0 ; i < btree_order - 1;i++)
                        {
                                //Investigate each key and determine where to go
                                //Check here if all the keys have been read
                                if(keys_read == keys_in_node)
                                {
                                        greatest=true;
                                        if(keys_read == btree_order - 1)
                                                node_full = true;
                                        break;
                                }
                                if(debug)       printf("add_split() : Reading from offset : %ld\n",ftell(fin));
                                fread(&key_to_test, sizeof(int), 1, fin);
                                if(debug)       printf("add_split() : Key_to_test : %d\n",key_to_test);
                                keys_read++;
                        }
                        if(key <= key_to_test && keys_read==1)
                                smallest = true;

			if(smallest)	rank=0;
			if(greatest)
			{
				if(node_full)	rank = btree_order-1;
				else		rank = keys_in_node;
			}
                        else
                                rank = keys_read - 1;

			//Next item : place split value correctly at 'rank' index 
                        if(debug)
                        {
                                printf("Parent before updates : \n");
                                printf("No. of keys in parent : %d\n",aux_parent->no_of_keys);
                                printf("Parent's keys : \n");
                                for(i=0;i<btree_order;i++)
                                        printf("%d\t", aux_parent->keys[i]);
                                printf("\n");
                                printf("Children offsets : \n");
                                for(i=0 ; i < btree_order + 1 ; i++)
                                        printf("%ld\t",aux_parent->children_offsets[i]);
                        }

			if(debug)	printf("Rank of the split_value : %d\n",rank);

			int iter2=btree_order - 1;
			for( iter2=btree_order - 1 ; iter2 > rank ; iter2-- )
			{
				//make place for the split_value
				aux_parent->keys[iter2] = aux_parent->keys[iter2-1];
				aux_parent->children_offsets[iter2 + 1] = aux_parent->children_offsets[iter2];
			}
			//Insert the split value at rank
			aux_parent->no_of_keys++;
			aux_parent->keys[rank] = split_value;
			aux_parent->children_offsets[rank + 1] = right_child_offset;
			//Verify parent
			if(debug)
			{
				printf("Parent after updates : \n");
				printf("No. of keys in parent : %d\n",aux_parent->no_of_keys);
                                printf("Parent's keys : \n");
                                for(i=0;i<btree_order;i++)
                                        printf("%d\t", aux_parent->keys[i]);
                                printf("\n");
                                printf("Children offsets : \n");
                                for(i=0 ; i < btree_order + 1 ; i++)
                                        printf("%ld\t",aux_parent->children_offsets[i]);
			}

			//Check the cardinality of aux_parent 
			if(aux_parent->no_of_keys <= btree_order - 1 )
			//We are good and we require no split further
			{
				write_node_at_offset(aux_parent, next_parent);
				//print_node_at_offset(next_parent);
			}

			else if(aux_parent->no_of_keys == btree_order)
			//Needs a split
				add_with_split(node_keys,leaf_offset,key, aux_parent, next_parent);
			else
			{
				printf("An exception in add_with_split() \n");
			}
	
	}	
	else
	{
		//Root here
		//Write KL at the current root_offset
                write_node_at_offset(left_aux, root_offset);

                //Write the RC
                fseek(fin, 0, SEEK_END);
                long right_child_offset = ftell(fin);
                fseek(fin, 0, SEEK_SET);
		//Write right node to file
                write_node(right_aux);

		//Write the new_element root
		btree_node_ptr root = new_element_node_init();
		//set keys to 1;
		root->no_of_keys=1;
		//Set the key value
		root->keys[0]= split_value;
		//Set the children offsets
		root->children_offsets[0] = root_offset;
		root->children_offsets[1] = right_child_offset;
		fseek(fin, 0, SEEK_END);
		root_offset = ftell(fin);
		write_node_at_offset(root, root_offset);
		//print_node_at_offset(root_offset);
		
	}	
	//check parent                                                
	//check_parent_and_update(split_value, left_child_offset, right_child_offset);

}

//This is the routine that will be called once
void add_key_to_tree(int node_keys, long leaf_offset, int key)
{

	//This will be called only when the leaf is overfull upon addition of the element
	btree_node_ptr aux_node = new_element_aux_node();
	//Leaf is at leaf offset
	//Load the node's contents into aux_node
	fseek(fin,0,SEEK_SET);
	fseek(fin,leaf_offset, SEEK_CUR);
	fread(&(aux_node->no_of_keys), sizeof(int), 1, fin);
	//read the keys
	int i=0;
	for(i=0;i<btree_order - 1 ;i++)
		fread(&(aux_node->keys[i]), sizeof(int), 1, fin);

	//Load the offsets
	for(i=0;i<btree_order;i++)
		fread(&(aux_node->children_offsets[i]), sizeof(long), 1, fin);
	//The node is ready
	//print node
	if(debug)
	{                       
		printf("Full leaf before updates : \n");
                printf("No. of keys in leaf : %d\n",aux_node->no_of_keys);
                printf("Aux's keys : \n");
                for(i=0;i<btree_order;i++)
                	printf("%d\t", aux_node->keys[i]);
                printf("\n");
                printf("Children offsets : \n");
                for(i=0 ; i < btree_order + 1 ; i++)
                	printf("%ld\t",aux_node->children_offsets[i]);
	}

	int rank =0;
                int key_to_test = -10;
                int keys_in_node = aux_node->no_of_keys;
                bool greatest = false;
                bool node_full = false;
                bool smallest = false;
                int keys_read = 0;
                if(debug)       printf("Keys in node in the parent: %d\n",keys_in_node);
		fseek(fin, leaf_offset + sizeof(int), SEEK_SET);
                 while(key > key_to_test)
                        //for(i=0 ; i < btree_order - 1;i++)
                        {
                                //Investigate each key and determine where to go
                                //Check here if all the keys have been read
                                if(keys_read == keys_in_node)
                                {
                                        greatest=true;
                                        if(keys_read == btree_order - 1)
                                                node_full = true;
                                        break;
                                }
                                if(debug)       printf("add_split() : Reading from offset : %ld\n",ftell(fin));
                                fread(&key_to_test, sizeof(int), 1, fin);
                                if(debug)       printf("add_split() : Key_to_test : %d\n",key_to_test);
                                keys_read++;
                        }
                        if(key <= key_to_test && keys_read==1)
                                smallest = true;

                        if(smallest)    rank=0;
                        if(greatest)
                        {
                                if(node_full)   rank = btree_order-1;
                                else            rank = keys_in_node;
                        }
			else
				rank = keys_read - 1; 
                        int iter2=btree_order - 1;
                        for( iter2=btree_order - 1 ; iter2 > rank ; iter2-- )
                        {
                                //make place for the split_value
                                aux_node->keys[iter2] = aux_node->keys[iter2-1];
                                aux_node->children_offsets[iter2 + 1] = aux_node->children_offsets[iter2];
                        }
                        //Insert the key  at rank
			if(debug)	printf("Rank : %d\tKey : %d\n",rank,key);
                        aux_node->no_of_keys++;
                        aux_node->keys[rank] = key;
                        //aux_node->keys[rank + 1] = right_child_offset;
        if(debug)
        {
                printf("Full leaf after updates : \n");
                printf("No. of keys in leaf : %d\n",aux_node->no_of_keys);
                printf("Aux's keys : \n");
                for(i=0;i<btree_order;i++)
                        printf("%d\t", aux_node->keys[i]);
                printf("\n");
                printf("Children offsets : \n");
                for(i=0 ; i < btree_order + 1 ; i++)
                        printf("%ld\t",aux_node->children_offsets[i]);
        }

	//Call recursive split
	add_with_split(node_keys, leaf_offset, key, aux_node, leaf_offset);
	

}

//Prints the tree
void print_tree(char *input_filename)
{
	bool debug = false;
	if(fin==NULL)
	{
		printf("print_tree() : Received a NULL file pointer, fin\n");
		return;
	}

	fseek(fin, 0, SEEK_SET);
	//Take it to the root
	if(root_offset == -1)
	{
		printf("Tree empty\n");
		return;
	}
	fseek(fin, root_offset, SEEK_CUR);
	int curr_level = 1;
	FILE *f_child_offset = fopen(input_filename, "rb");
	if(f_child_offset == NULL)
	{
		printf("print_tree() : Could not open the index file\n");
		return;
	}
	//Push it into the queue
	enqueue(root_offset );
	q_ret_ptr queue_ret;
	int test;
	int queue_pos;
	int keys_in_node = 0;
	int level_till_pos = 0;
	int children=0;
	if(!queue_is_empty())
		printf(" %d: ", curr_level);
	while(!queue_is_empty())
	{
		queue_ret = dequeue();
		//Increment to note that a node of this level has been read
		queue_pos = queue_ret->queue_position;
		if(debug)	printf("queue_pos : %ld\n",queue_pos);
		//Take the fp to there
		fseek(fin, queue_ret->offset, SEEK_SET);
		//Place the child offset ptr at its proper location
		//if(debug)	printf("fin's offset : %ld\n", ftell(fin));
		fread(&keys_in_node, sizeof(int), 1, fin);
		fseek(f_child_offset, ftell(fin), SEEK_SET);
		fseek(f_child_offset, (btree_order -1)*sizeof(int) , SEEK_CUR);
		//if(debug)	printf("f_child's offset : %ld\n",ftell(f_child_offset));
		if(debug)	printf("print_tree() : Keys_in_node : %d\n",keys_in_node);
		int index =0;
		while(index < keys_in_node)
		{
			fread(&test, sizeof(int), 1, fin);
			if(index == keys_in_node - 1)
				printf("%d ", test);
			else
				printf("%d,",test);
			index++;
		}
		if(queue_ret->offset == root_offset)
		{
			curr_level++;
			{
				//printf("\n");
				//printf(" %d: ", curr_level);
			}
			children = 0;
			FILE *ftemp;
			long temp_off=0;
			ftemp = fopen (input_filename, "rb");
			if(ftemp==NULL)
			{
				perror("print_tree() : ftemp open error : ");
				return;
			}
			fseek(ftemp, ftell(f_child_offset), SEEK_SET);
			int iter2=0;
			while(1)
			{
				fread(&temp_off, sizeof(long), 1, ftemp);
				if(temp_off==-1)
					break;
				if(debug)	printf("print_tree() : Read offset %ld\n",temp_off);
				iter2++;children++;
				if(iter2==btree_order)
					break;
			}

			//Set the variable : level_till_pos
			level_till_pos = queue_pos + children;
			if(children > 0)
                        {
                                printf("\n");
                                printf(" %d: ", curr_level);
                        }
			children=0;
			fclose(ftemp);
		}
		//Internal node
		else
		{
			//Loop to increment children count at this level
			FILE *ftemp;
                        long temp_off=0;
                        ftemp = fopen (input_filename, "rb");
                        if(ftemp==NULL)
                        {
                                perror("print_tree() : ftemp open error : ");
                                return;
                        }
                        fseek(ftemp, ftell(f_child_offset), SEEK_SET);
                        int iter2=0;
                        while(1)
                        {
                                fread(&temp_off, sizeof(long), 1, ftemp);
                                if(temp_off==-1)
                                        break;
                                if(debug)       printf("print_tree() : Read offset %ld\n",temp_off);
                                iter2++;children++;
                                if(iter2==btree_order)
                                        break;
                        }

			if(debug)	printf("Level_till_pos : %d\tqueue_pos: %d\tChildren in this level : %d\n",level_till_pos, queue_pos, children);

			if(queue_pos == level_till_pos)
			{
				level_till_pos = queue_pos + children;
				children=0;
				if(debug)	printf("-------------------------------------Level change---------------------------------\n");
				curr_level++;
				if((!queue_is_empty()))
				{
					printf("\n");
					printf(" %d: ", curr_level);
				}
			}

			fclose(ftemp);

		}
		long test_offset = 0;
		int iter = 0;
		//Enqueue activity
		while(1)
		{
			fread(&test_offset, sizeof(long), 1, f_child_offset);
			iter++;
			if(test_offset == -1)
				break;
			//Enqueue this child offset into the queue
			if(debug)	printf("Enqueueing %ld to the queue\n",test_offset);
			enqueue(test_offset);
 
			if(iter==btree_order)
				break; 
		}
		//Debug
		//print_queue();
	}	
	printf("\n");
}

int main() {
	return 0;
}
