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

//Lloyd - Tuesday 1/14/2014

struct command_stream
{
	
	int (*getbyte) (void *); 
	void *arg;
	token* tokenArray;
	int tokenCount; 
 // A char that will be ignored
	int ignore_char;
 // Line count
	int line_number;
	
 // For storing the token strings
	char *next_token_string;
	enum token_type next_token;
	char *current_token_string;
	enum token_type current_token;
 
};




struct token //Need a token because words can have a value
{
	token_type type;
	char* wordString;
}

token_type get_current_token_type(command_stream_t)
{
	return (tokenArray[tokenCount-1]).token_type;
}

int test_word_char_valid(int curChar)
{
if ((curChar >= '0' && curChar <= '9') || (curChar >= 'a' && curChar <= 'z')
       || (curChar >= 'A' && curChar <= 'Z')
       || curChar == '.' || curChar == ',' || curChar == '+' || curChar == '-'
       || curChar == '-' || curChar == '_' || curChar == '!' || curChar == '@'
       || curChar == '/' || curChar == ':' || curChar == '^' || curChar == '%')
               return 1;
else
       return 0;

}

char get_next_char(command_stream_t cStream) 
{
	return cStream->getbyte(cStream->arg);
}

// Store the character to mark it to be ignored in the function get_next_char
void ignoreChar(int targetChar, command_stream_t cStream)
{
 cStream->ignore_char = targetChar;
}

void read_next_token(command_stream_t cStream);
{
 // A while loop that determines the token type the next time produce_token is called
	int curChar = get_next_char(cStream);
	int stringIndex = 0;
	while(curChar != EOF && curChar >= 0) // Negative return from get_next_char means no more input
	{
  
	// Ignore any leading whitespace
		else if(curChar == ' ' || curChar == '\t')
			continue;

	// Ignore comment text until curChar is a newline
		else if(curChar == '#')
		{
			while((curChar != '\n')
				curChar = get_next_char; //Ignore until a newline
			continue;
		}
   
   // Form a WORD token from valid WORD characters
		else if(test_word_char_valid(curChar))
		{
			next_token_string[index] = curChar;
			index++;
     
	// Consume all word characters to form the word until a seperator
			while((curChar = get_next_char(cStream)))
			{
       
       // If the character is no longer one of the valid WORD characters,
       // return that character, and stop
				if(!test_word_char_valid(curChar))
				{
					ignoreChar(curChar, cStream);
					break;
				}
       
       // Grow allocated memory when necessary. Size of next_token_string 
       // and current_token_string are equal because all data in 
       // next_token_string passes into current_token_string        
				if(index >= cStream->max_token_length)
				{
					command_stream_reallocate(cStream);
					next_token_string = cStream->next_token_string;
				}
       
       // Add the array forming the next token
				next_token_string[index] = curChar;
				index++;
       
			}

     // Allocate memory if needed, then add the zero byte
			if(index >= cStream->max_token_length)
			{
				command_stream_reallocate(cStream);
				next_token_string = cStream->next_token_string;
			}
				next_token_string[index] = 0;     
				cStream->next_token = WORD;
				break;
		}
   
   // Check for &&
		else if(curChar == '&')
		{
			next_token_string[index++] = curChar;  
			if((curChar = get_next_char(cStream)) != '&')
			{
				error(1, 0, "%d: Found lone &, invalid character", cStream->line_number);
				break;
			}
     
			next_token_string[index++] = curChar;     

     //Add the end zero byte
			next_token_string[index] = 0;     
			cStream->next_token = D_AND;
			break;
   		}
   
   // Differentiate between a PIPE (|) and a D_OR (||)
   		else if(curChar == '|')
   		{
			next_token_string[index++] = curChar;  
			if((curChar = get_next_char(cStream)) == '|')
     			{
       				next_token_string[index++] = curChar;     
       				next_token_string[index] = 0;
       				cStream->next_token = D_OR;
       				break;
     			}
     			ignoreChar(curChar, cStream);     
     			next_token_string[index] = 0;     
     			cStream->next_token = PIPE;
     			break;
   		}
   
   		else if(curChar == '(')
   		{
   			next_token_string[index++] = curChar;
     			next_token_string[index] = 0;     
     			cStream->next_token = LEFT_PARAN;
     			break;
   		}
   
   		else if(curChar == ')')
   		{
    			next_token_string[index++] = curChar;
     			next_token_string[index] = 0;     
     			cStream->next_token = RIGHT_PARAN;
     			break;
   		}
   
   		else if(curChar == '<')
   		{
    			next_token_string[index++] = curChar;
     			next_token_string[index] = 0;     
     			cStream->next_token = LESS;
     			break;
   		}
   
   		else if(curChar == '>')
   		{
     			next_token_string[index++] = curChar;
     			next_token_string[index] = 0;     
     			cStream->next_token = GREATER;
     			break;
   		}
   
   		else if(curChar == '\n')
   		{
    			cStream->line_number++;
     
     // Ignore any subsequent newlines, but keep line count
     			while((curChar = get_next_char(cStream)) == '\n')
     			{
       				cStream->line_number++;
     			}
       
     			if(curChar == EOF)
     			{
       				cStream->next_token = END;
       				break;
     			}
     			ignoreChar(curChar, cStream);
     
     			next_token_string[index++] = ' ';
     			next_token_string[index] = 0;     
     			cStream->next_token = NEWLINE;
     			break;
   		}
   
  	 	else if(curChar == ';')
   		{
    			next_token_string[index++] = curChar;
     			next_token_string[index] = 0;     
     			cStream->next_token = SEMICOLON;
     			break;
   		}
   
   		else
   		{
     			error(1, 0, "%d: Unrecognized character", 
           		cStream->line_number);
     			break;
   		}
	}
}

enum token_type produce_token(command_stream_t cStream)
{
char* next_token_string = cStream->next_token_string;
 
// Move the next token up to current, then read a new one
strcpy(cStream->current_token_string, next_token_string);
cStream->current_token = cStream->next_token;
next_token_string[0] = 0;
//read next token before returning current token
void read_next_token(command_stream_t cStream);
return cStream->current_token;

}

// Peek at next token without fetching
enum token_type check_next_token(command_stream_t s)
{
 return s->next_token;
}

command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	

	command_stream_t cStream = checked_malloc(sizeof(struct command_stream));
	
	cStream->tokenCount = 10;
	cStream->tokenArray = checked_malloc(sizeof(token)*(cStream->tokenCount);

	cStream->streamHead = NULL;
	cStream->getbyte = get_next_byte;
	cStream->arg = get_next_byte_argument;
	
 // Allocate inital size for token string arrays
 cStream->next_token_string = checked_malloc ( 50 * sizeof(char) );
 cStream->current_token_string = checked_malloc ( 50 * sizeof(char) );
 
 // -2 if no char exists, initialized as such 
	cStream->ignore_char = -2;
	cStream->line_number = 1;

	do read_next_token(cStream)
	while(get_current_token_type(cStream) != END); //insert all tokens into array

	return cStream;
}

command_t read_command_stream (command_stream_t s)
{
	
 if(check_next_token(s) == END)
   return NULL;
 read_next_token(s);
//parser
//return parsed result
}
