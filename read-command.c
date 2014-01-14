// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <string.h>
#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream
{
	command_t streamHead; //link to the uppermost command
	int (*getbyte) (void *); 
  	void *arg;
	token_type_t tokenHead; 
	
}

add_node_to_stream(command_stream_t cStream, char* wordBuf )
{
	if(streamHead == NULL)
	// implement the rest
	
}

void add_token(token_type token, token_value, )
{

}

token

command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	command_stream_t cStream = checked_malloc(sizeof(struct command_stream));
	cStream->streamHead = NULL;
	cStream->tokenHead = checked_malloc(sizeof(token)*50)
	cStream->getbyte = get_next_byte;
  	cStream->arg = get_next_byte_argument;

	return cStream;
}

command_t
read_command_stream (command_stream_t s)
{
	char* wordBuf = checkedmalloc(sizeof((char)*50));
	char curChar;
	char nextChar;
	int lineCount = 1;
	curChar = getbyte(arg);
	nextChar = getbyte(arg);
	tokenizer(s);
	while(nextChar != EOF)
	{
		int index = 0;
		if(curChar != ' ')
			wordBuf[x] = curChar;
		else
		{
			add_token(;
			x = 0;
		}
		
	}
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
