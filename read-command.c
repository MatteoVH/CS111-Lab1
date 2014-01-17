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
	int maxTokens;
 
	int dontGet; //if set to 1 causes get_next_char to not read the next character immediately
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


void read_next_token(command_stream_t cStream);
{
 // A while loop that determines the token type the next time produce_token is called
	token curToken; //initialize a temporary token
	int maxTokenStringLength = 15;
	curToken.wordString = checkedmalloc(sizeof(char)*maxTokenStringLength) //initialize c-string
	
	curToken.type = END;
	
	if(cStream->dontGet = 0)
		int curChar = get_nextChar(cStream);
	cStream->dontGet = 0; //reset the dontGet boolean
	int index = 0;
	while(curChar != EOF && curChar >= 0) // Negative return from get_next_char means no more input
	{
  
	// Ignore any leading whitespace
		if(curChar == ' ' || curChar == '\t')
			continue;

	// Ignore comment text until curChar is a newline
		else if(curChar == '#')
		{
			while((curChar != '\n')
				curChar = get_next_char(cStream); //Ignore until a newline
			continue;
		}
   		
		else if(curChar == '&')
		{
			curToken.wordString[index] = curChar;
			index++;
			curChar = get_next_char(cStream);
			if(curChar != '&')
			{
				error(1, 0, "%d: Found lone &, invalid character", cStream->line_number);
				break;
			}
			
     			curToken.wordString[index] = curChar;
			index++;     
			
     //Add the end zero byte
			curToken.wordString[index] = '\0';     
			curToken.type = AND;
			break;
   		}

		
   		else if(curChar == '|')
   		{
			curToken.wordString[index] = curChar;
			index++;
			curChar = get_next_char;
			  
			if((curChar == '|')
     			{
				
				curToken.wordString[index] = curChar;
				index++;
				
       				     
       				curToken.wordString[index] = '\0';
       				curToken.type = OR;
       				break;
     			}
     			cStream->dontGet = 1;     
     			curToken.wordString[index] = '\0';     
     			curToken.type = PIPE;
     			break;
   		}
	
		
   		else if(curChar == '(')
   		{
   			curToken.wordString[index] = curChar;
			index++;
     			curToken.wordString[index] = '\0';     
     			curToken.type = LEFT_PAREN;
     			break;
   		}
		
		
   		else if(curChar == ')')
   		{
    			curToken.wordString[index] = curChar;
     			index++; 
			curToken.wordString[index] = '\0';
     			curToken.type = RIGHT_PAREN;
     			break;
   		}

   		else if(curChar == '<')
   		{
    			curToken.wordString[index] = curChar;
     			index++; 
			curToken.wordString[index] = '\0';
     			curToken.type = LESS_THAN;
     			break;
   		}

   		else if(curChar == '>')
   		{
    			curToken.wordString[index] = curChar;
     			index++; 
			curToken.wordString[index] = '\0';
     			curToken.type = GREATER_THAN;
     			break;
   		}
    
   		else if(curChar == ';')
   		{
    			curToken.wordString[index] = curChar;
     			index++; 
			curToken.wordString[index] = '\0';
     			curToken.type = SEMICOLON;
     			break;
   		}

   		else if(curChar == '\n')
   		{
    			cStream->line_number++;
     			curChar = get_next_char(cStream);
     // Ignore any subsequent newlines, but keep line count
     			while((curChar == '\n')
     			{
       				cStream->line_number++;
				curChar = get_next_char(cStream);
     			}
       
     			if(curChar == EOF)
     			{
       				cStream->next_token = END;
       				break;
     			}
     			cStream->dontGet = 1;
     
     			curToken.wordString[index] = ';';
			index++;
     			curToken.wordString[index] = '\0';     
     			curToken.type = SEMICOLON;
     			break;
   		}

		else if(test_word_char_valid(curChar))
		{
			curToken.wordString[index] = curChar;
			index++;
     			curChar = get_next_char(cStream);
	// Consume all word characters to form the word until a seperator
			while(test_word_char_valid(curChar))
			{
       				
       // If the character is no longer one of the valid WORD characters,
       
       
       // Grow allocated memory when necessary. Size of next_token_string 
       // and current_token_string are equal because all data in 
       // next_token_string passes into current_token_string        
       
       // Add the array forming the next token
				curToken.wordString[index] = curChar;
				index++;
       				curChar = get_next_char(cStream);
			
				if(index >= maxTokenStringLength)
				{
					checked_grow_alloc(curToken.wordString, 10);
				}
			}
     // Allocate memory if needed, then add the zero byte
				cStream->dontGet = 1;
				
				curToken.wordString[index] = '\0';     
				curToken.type = WORD;
				break;
		}
   
   		else
   		{
     			error(1, 0, "%d: Unrecognized character", 
           		cStream->line_number);
     			break;
   		}
	}
	
	cStream->tokenArray[tokenCount] = curToken;
	cStream->tokenCount++;
	if(cStream->tokenCount == cStream->maxTokens)
		checked_grow_alloc(tokenArray, sizeof(token)*20);
}


// Peek at next token without fetching
enum token_type check_next_token(command_stream_t s)
{
 return s->next_token;
}

command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	
	
	command_stream_t cStream = checked_malloc(sizeof(struct command_stream));
	
	cStream->dontGet = 0;
	
	
	cStream->tokenCount = 0;
	cStream->maxTokens = 20;
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
