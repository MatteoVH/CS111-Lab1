// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <errno.h>

#include <fcntl.h> 
#include <sys/stat.h> 

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}


int normal_exec_cmd (command_t c){
	if(c->type == AND_COMMAND){
		return (normal_exec_cmd(c->u.command[0]) && normal_exec_cmd(c->u.command[1]));	
	}
	else if(c->type == OR_COMMAND){
		int c0 = normal_exec_cmd(c->u.command[0]);
		int c1 = normal_exec_cmd(c->u.command[1]);
		return c0||c1 ;
	}
	else if(c->type == SEQUENCE_COMMAND){
		normal_exec_cmd(c->u.command[0]);
		return normal_exec_cmd(c->u.command[1]);
	}
	else if(c->type == PIPE_COMMAND){
		int buf[2];
		pipe(buf);
		pid_t p;
		p=fork();
		int result;
		if(p==0){ //child
			dup2(buf[0],0);
			close(buf[1]);
			result = normal_exec_cmd(c->u.command[1]);
			close(buf[0]);
			close(0);
			exit(1);
		}
		else if(p>0){	//parent 
			pid_t p2;
			p2 = fork();
			if(p2==0)
			{
				dup2(buf[1],1);
				close(buf[0]);
				normal_exec_cmd(c->u.command[0]);
				close(buf[1]); //finish pipeing
				exit(1);
			}
			else{
				close(buf[0]);
				close(buf[1]);
				int status;
				if(waitpid(p2,&status,0)>0){
					if(WIFEXITED(status)){
						return WEXITSTATUS(status);
					}
					else if(WIFSIGNALED(status)){
						return WTERMSIG(status);
					}
					else{
						perror("Execution interrupted\n");
						return 0;
					}
				}
				else{
					perror("Pipeline return error\n");
					return 0;
				}
			}
		}
		else{	
			perror("Cannot create child process");
			return 0;
		}
	}
	else if(c->type == SUBSHELL_COMMAND){
		return normal_exec_cmd(c->u.subshell_command);
	}
	else if(c->type == SIMPLE_COMMAND){
		pid_t p;
		p=fork();	
		if(p==0){
			if(c->input!=0){ //input is not NULL and therfore exists
				int inputFD = open(c->input,O_RDONLY);
				if(inputFD==-1){
					perror("Unable to open input file");
					return 0; 
				}	
				dup2(inputFD,0);
				close(inputFD);
			}
			if(c->output!=0){ // output exists
				int outputFD = open(c->output,O_WRONLY | O_TRUNC | O_CREAT);
				if(outputFD==-1){
					perror("Unable to open output file");
					return 0;
				}
				dup2(outputFD,1);
				close(outputFD);
			}
			execvp(c->u.word[0],c->u.word);
			perror(c->u.word[0]); //Error
			return 0;
		}
		else if(p>0){	//parent
			int status;
			int pid = wait(&status);
			if(pid>0){
				if(WIFEXITED(status)){
					return !WEXITSTATUS(status);
				}
				else if(WIFSIGNALED(status)){
					return !WTERMSIG(status);
				}
				else{
					perror("Execution interrupted");
					return 0;
				}
			}
			else{
				perror("Waiting");
				return 0;
			}
		}
		else{	
			perror("Cannot create child");
			return 0;
		}
	}
	else{
		error(1,0,"Invalid command type");
	}
	return 0;
}


//Time travel functions
////////////////////////////////////////////////////////////////////////////////////


void generate_word_dependencies(word_node_t word_list, char* word)
{
if(strcmp(word_list->word, word) == 0)
	return;
else if(word_list->next == NULL)
{
	word_list->next = checked_malloc(sizeof(word_node_t));
	word_list->next->word = word;
	word_list->next->next = NULL;
}
else
	generate_word_dependencies(word_list->next, word);
}


word_node_t generate_word_list(char* word)
{
	word_node_t head = checked_malloc(sizeof(word_node_t));
	head->word = word;
	head->next = NULL;
	return head;
}

// Recursively generate all dependencies of the command to the node, and furthermore recursively
// add all dependencies of the command's subcommands to the node
void generate_cmd_dependencies(parent_node_t node, command_t cmd)
{
	if(cmd->input != 0)
	{
		if(node->inputs == NULL)
			node->inputs = generate_word_list(cmd->input);
		else
			generate_word_dependencies(node->inputs, cmd->input);
	}

	if(cmd->output != 0)
	{
		if(node->outputs == NULL)
			node->outputs = generate_word_list(cmd->output);
		else
			generate_word_dependencies(node->outputs, cmd->output);
	}

	int x;
switch(cmd->type)
	{

	// Case single subcommand
	case SUBSHELL_COMMAND:
		generate_cmd_dependencies(node, cmd->u.subshell_command);
		break;

	//Case two subcommands	
	case AND_COMMAND:
	case OR_COMMAND:
	case SEQUENCE_COMMAND:
	case PIPE_COMMAND:
		generate_cmd_dependencies(node, cmd->u.command[0]);
		generate_cmd_dependencies(node, cmd->u.command[1]);
		break;

	// Case no subcommands
	case SIMPLE_COMMAND:
		x = 1;
		while(cmd->u.word[x] != NULL)
		{
			if(node->inputs == NULL)
				node->inputs = generate_word_list(cmd->u.word[x]);
			else
				generate_word_dependencies(node->inputs, cmd->u.word[x]);
				x++;
		}
		break;
	default:
		break;
	}
}

void add_to_list(parent_node_t arg_dependent, parent_node_t new_parent)
{
	dep_node_t dep_list = arg_dependent->dependencies;
  	dep_node_t last_node = dep_list;
  	while(dep_list != NULL)
  	{
    		last_node = dep_list;
    		dep_list = dep_list->next;
  	}
  	dep_node_t new_node = checked_malloc(sizeof(struct dep_node));
 	 new_node->dependent = new_parent;
 	 new_node->next = NULL;
	if(last_node == NULL)
		arg_dependent->dependencies = new_node;
	else
		last_node->next = new_node;
}


command_t exec_time_travel (command_stream_t s)
{
  parent_node_t list_head = NULL;

  command_t final_command = NULL;
  command_t command;
  while ((command = read_command_stream (s)))
  {
    parent_node_t new_node = checked_malloc(sizeof(struct parent_node));
    new_node->command = command;
    new_node->inputs = NULL;
    new_node->outputs = NULL;
    new_node->dep_counter = 0;
    new_node->dependencies = NULL;
    new_node->pid = -1;

    generate_cmd_dependencies(new_node, command); 

    
    parent_node_t last_node = list_head;
    parent_node_t curr_node = list_head;
while(curr_node != NULL)
{
//compare the word list of new node's outputs to current node's inputs and vice versa     
  word_node_t curr_output1 = new_node->outputs;
  
  while(curr_output1 != NULL)
  {
    int finish = 0;
    word_node_t curr_input1 = curr_node->inputs;
    while(curr_input1 != NULL)
    {
      if(strcmp(curr_input1->word, curr_output1->word) == 0)
      {
          //Dependency exists
          new_node->dep_counter += 1;
          add_to_list( curr_node, new_node);
	  finish++;
	  break;
      }
      curr_input1 = curr_input1->next;
    }
   	 if(finish != 0)
		break;

	curr_output1 = curr_output1->next;
  }
//Vice versa
  word_node_t curr_output2 = curr_node->outputs;
  
  while(curr_output2 != NULL)
  {
    int end = 0;
    word_node_t curr_input2 = new_node->inputs;
    while(curr_input2 != NULL)
    {
      if(strcmp(curr_input2->word, curr_output2->word) == 0)
      {
          //dependent
	  new_node->dep_counter += 1;
          add_to_list(curr_node, new_node);
	  end++;
          break;
      }
      curr_input2 = curr_input2->next;
    }
	if(end != 0)
		break;

    curr_output2 = curr_output2->next;
  }
 
      last_node = curr_node;
      curr_node = curr_node->next;
}

    // Add node to waiting list
    if( last_node == NULL)
      list_head = new_node;
    else
      last_node->next = new_node;

    final_command = command;
  }


  return final_command;
}


void
execute_command (command_t c)
{
  /* FIXME: Replace this with your implementation. You may need to
add auxiliary functions and otherwise modify the source code.
You can also use external functions defined in the GNU C Library. */

 //normal mode
normal_exec_cmd(c);	
}
