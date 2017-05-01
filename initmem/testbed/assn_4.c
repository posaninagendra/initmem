#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<errno.h>
#include "data_structures.h"
#define DEF 100
#define CMD_LEN 1024

int btree_order = 0;
long root_offset = 0;
bool debug=false;
FILE *fin = NULL;
int size_of_tree=0;
long leaf_offset = 0;


int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Insufficient number of arguments supplied\n");
		exit(-1);
	}

	if(argc > 3)
	{
		printf("Too many arguments supplied\n");
		exit(-1);
	}

	char index_file[DEF];
	memset(index_file,0,sizeof(char)*DEF);
	if(argv[1]==NULL)
	{
		printf("Received a NULL index filename");
		exit(-1);
	}
	//Index file
	strcpy(index_file,argv[1]);

	if(argv[2]==NULL)
	{
		printf("Received a NULL B Tree order");
		exit(-1);
	}
	//B tree order
	btree_order=atoi(argv[2]);

	//debug bool
	//bool debug=true;
	if(debug)	printf("Index filename received : %s\n",index_file);
	if(debug)	printf("B Tree order : %d\n",btree_order);
	bool cmdIsValid=false;
	//File needs to be opened here
	//FILE *fin;
	fin = fopen (index_file, "r+b");
	if(fin == NULL)
	{
		root_offset = -1;
		//Write -1 to the file
		fin = fopen (index_file, "w+b");
		if(fin == NULL)
		{
			perror("Index file open error: ");
			exit(-1);
		}
		//else
		//	fwrite(&root_offset, sizeof(long), 1, fin);
	}
	else
		fread(&root_offset, sizeof(long), 1, fin);

	char *cmd=(char *)malloc(sizeof(char)*CMD_LEN);
	char *temp_cmd=(char *)malloc(sizeof(char)*CMD_LEN);
	char *temp2_cmd=(char *)malloc(sizeof(char)*CMD_LEN);
	memset(cmd,0,sizeof(char)*CMD_LEN);
	memset(temp_cmd,0,sizeof(char)*CMD_LEN);
	memset(temp2_cmd,0,sizeof(char)*CMD_LEN);

	while(1)
	{
		cmdIsValid=false;
		int ret;
		size_t len = sizeof(char)*CMD_LEN;
		memset(cmd,0,sizeof(char)*CMD_LEN);
		memset(temp_cmd,0,sizeof(char)*CMD_LEN);
		memset(temp2_cmd,0,sizeof(char)*CMD_LEN);
		ret=getline(&cmd,&len,stdin);
		if(ret==-1)
		{
			perror("Command Read Error :");
			exit(-1);
		}
		
		//Strip \n
		cmd[strlen(cmd)-1]='\0';
		if(strlen(cmd)==0)	continue;
	
		if(debug)	printf("getline returns : %s\n",cmd);
		strcpy(temp_cmd,cmd);
		strcpy(temp2_cmd,cmd);
		char *tok=strtok(temp_cmd," ");
		if(strcmp(tok,"add")==0 || strcmp(tok,"find")==0 || strcmp(tok,"print")==0 || strcmp(tok,"end")==0)
			cmdIsValid=true;
		if(cmdIsValid)
		{
		if(strcmp(tok,"add")==0)
		{
			tok=strtok(NULL," ");
			if(tok==NULL)
			{
				printf("Key value not supplied to 'add' utility\n");
				continue;
			}
			int key=atoi(tok);
			int found=0;
			found=find_key_in_btree(index_file, key);
			if(found)
				printf("Entry with key=%d already exists\n",key);
			else
			{
				//Add logic begins here
				size_of_tree++;
				if(root_offset==-1)
				{
					//The tree is empty. Construct the root;
					btree_node_ptr root = new_element_node_init();
					(root)->keys[0] = key;
					(root)->no_of_keys++;
					if(debug)	printf("Root data : %d\t%d\n",(root)->keys[0],(root)->no_of_keys);
					//Write this node to the file
					//btree_node_ptr node= *root;
					//memcpy(node, *root, sizeof(btree_node));
					write_node(root);
					root_offset = sizeof(long);
				}
				
				//Tree is not empty
				else
				{
					//Check if the root is not full
					//fseek(fin, sizeof(long), SEEK_SET);
					fseek(fin, root_offset, SEEK_SET);
					int keys_in_root=0;
					fread(&keys_in_root, sizeof(int), 1, fin);
					if(debug)	printf("keys_in_root : %d\n",keys_in_root);
					{
						//printf("Root is Full \n");
						//The recursive procedure goes here to split upto the root
						//Step 1 : Populate the parent pointers upto the leaf responsible for this insert
						//long parent_pointers[size_of_tree];
						int i=0;
						/*for(i=0;i<size_of_tree;i++)
						{
							parent_pointers[i] = -1;
							if(debug)	printf("parent pointer : %d\t%ld",i,parent_pointers[i]);
						}*/
						
						//Essentially a find operation that pushes in the parent offsets in a stack
						cleanup_stack();
						populate_parents( index_file, key);
						//print_stack();

						//Step 2 : Get the leaf offset
						if(debug)	printf("Leaf offset : %ld\n",leaf_offset);
						//Get the number of nodes in the leaf
						fseek(fin, 0, SEEK_SET);
						fseek(fin, leaf_offset, SEEK_CUR);
						int node_keys = 0;
						fread(&node_keys, sizeof(int), 1, fin);
						if(debug)	printf("Keys in node : %d\n",node_keys);
						if(node_keys == btree_order - 1)
							add_key_to_tree(node_keys, leaf_offset, key);
						else
							insert_in_node(leaf_offset, node_keys, key);
						
					}
				}
			}
			
		}

		else if(strcmp(tok,"find")==0)
		{
			
			tok=strtok(NULL," ");
			if(tok==NULL)
			{
				printf("Key value not supplied to 'find' utility\n");
				continue;
			}
			int key=atoi(tok);
			bool found=find_key_in_btree(index_file, key);
			if(found)
				printf("Entry with key=%d exists\n",key);
			else
				printf("Entry with key=%d does not exist\n",key);
		}

		else if(strcmp(tok,"print")==0)
		{
			print_tree(index_file);
		}

		else if(strcmp(tok,"end")==0)
		{
			//Write the updates root offset
			fseek(fin, 0, SEEK_SET);
			fwrite(&root_offset, sizeof(long), 1, fin);
			fclose(fin);
			break;
		}
	
		else
			printf("Invalid command received\n");
		
		}
		else
			printf("Invalid command received\n");
	}
	return 0;
}
