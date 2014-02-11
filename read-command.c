// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
	
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>


/* FIXME: You may need to add #include directives, macro definitions,
  static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
  complete the incomplete type declaration in command.h.  */



command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	
	
	command_stream_t cStream = checked_malloc(sizeof(struct command_stream));
	
	cStream->curCh = '\0';
	cStream->tokenCount = 0;
	cStream->dontGet = 0;
	cStream->maxTokens = 20;
	cStream->tokenArray = checked_malloc(sizeof(struct token)*(cStream->maxTokens));

	cStream->getbyte = get_next_byte;
	cStream->arg = get_next_byte_argument;
	cStream->iterator = 0;
 
	cStream->tokenIndex = 0;
	


 // -2 if no char exists, initialized as such 
	cStream->line_number = 1;

	do {
	read_next_token(cStream, cStream->curCh);
	}
	while(get_current_token_type(cStream) != END); //insert all tokens into array
	
	cStream->arrayCommands = checked_malloc(sizeof(struct command)*(cStream->tokenCount));
	cStream->arrayCommandsIndex = 0;
	
	int temp;
	for(temp = 0; temp != cStream->tokenCount; temp++)
	{
		cStream->arrayCommands[temp].status = -1;
		cStream->arrayCommands[temp].input = NULL;
		cStream->arrayCommands[temp].output = NULL;
		cStream->arrayCommands[temp].u.command[0] = NULL;
		cStream->arrayCommands[temp].u.command[1] = NULL;
	}	
	//puts the tokens into actual command structs, and also parses redirections as well
	create_command_array(cStream);

	cStream->newHomeIterator = 0;
	cStream->newHome = checked_malloc(sizeof(struct command)*(cStream->arrayCommandsIndex)*2);

	//parses all pipe commands
	parse_pipe(cStream);

	//parses all AND or OR commands
	parse_andor(cStream);

	//parses all SEMICOLON commands
	parse_serialcommand(cStream);

	//parse NEWLINE spacers
	parse_spacers(cStream);
		
	//Initialize the linear command array with memory and also set the counter to 0
	cStream->linearCommandArray = checked_malloc(sizeof(struct command)*(cStream->arrayCommandsIndex));
	cStream->linearCounter = 0;

	//Parse the tree using a recursive in-order traversal to generate a linear array of the commands so we can return them in read_command_stream
	parse_into_linear_array(cStream, &(cStream->arrayCommands[0]));	

	//initialize the output counter so that the read_command_stream function can use it
	cStream->outputCounter = 0;
	
	return cStream;
}



enum token_type get_current_token_type(command_stream_t cStream)
{
	return (cStream->tokenArray[cStream->tokenCount-1]).tType;
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


void read_next_token(command_stream_t cStream, char curChar)
{
 // A while loop that determines the token type the next time produce_token is called
	token_t curToken = checked_malloc(sizeof(struct token)); //initialize a temporary token
	int maxTokenStringLength = 15;
	curToken->wordString = checked_malloc(sizeof(char)*maxTokenStringLength); //initialize c-string
	
	curToken->tType = END;
	if(cStream->dontGet == 0)
		curChar = get_next_char(cStream);
	cStream->dontGet = 0; //reset the dontGet boolean
	int index = 0;
	while(curChar != EOF && curChar >= 0) // Negative return from get_next_char means no more input
	{
  
	// Ignore any leading whitespace
		if(curChar == ' ' || curChar == '\t')
		{	
			curChar = get_next_char(cStream);
			continue;
		}

	// Ignore comment text until curChar is a newline
		else if(curChar == '#')
		{
			while(curChar != '\n')
				curChar = get_next_char(cStream); //Ignore until a newline
			continue;
		}
   		
		else if(curChar == '&')
		{
			curToken->wordString[index] = curChar;
			index++;
			curChar = get_next_char(cStream);
			if(curChar != '&')
			{
				error(1, 0, "%d: Found lone &, invalid character", cStream->line_number);
				break;
			}
			
     			curToken->wordString[index] = curChar;
			index++;     
			
     //Add the end zero byte
			curToken->wordString[index] = '\0';     
			curToken->tType = AND;
			break;
   		}

		
   		else if(curChar == '|')
   		{
			curToken->wordString[index] = curChar;
			index++;
			curChar = get_next_char(cStream);
			  
			if(curChar == '|')
     			{
				
				curToken->wordString[index] = curChar;
				index++;
				
       				     
       				curToken->wordString[index] = '\0';
       				curToken->tType = OR;
       				break;
     			}
     			cStream->dontGet = 1;     
     			curToken->wordString[index] = '\0';     
     			curToken->tType = PIPE;
     			break;
   		}
	
		
   		else if(curChar == '(')
   		{
   			curToken->wordString[index] = curChar;
			index++;
     			curToken->wordString[index] = '\0';     
     			curToken->tType = LEFT_PAREN;
     			break;
   		}
		
		
   		else if(curChar == ')')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = RIGHT_PAREN;
     			break;
   		}

   		else if(curChar == '<')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = LESS_THAN;
     			break;
   		}

   		else if(curChar == '>')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = GREATER_THAN;
     			break;
   		}
    
   		else if(curChar == ';')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = SEMICOLON;
     			break;
   		}

   		else if(curChar == '\n')
   		{
    			cStream->line_number++;
     			curChar = get_next_char(cStream);
     // Ignore any subsequent newlines, but keep line count
     			while(curChar == '\n')
     			{
       				cStream->line_number++;
				curChar = get_next_char(cStream);
     			}
       
     			if(curChar == EOF)
     			{
       				curToken->tType = END;
				break;
     			}
     			cStream->dontGet = 1;
     			if(cStream->tokenArray[cStream->tokenCount - 1].tType != AND && cStream->tokenArray[cStream->tokenCount - 1].tType != OR &&
			   cStream->tokenArray[cStream->tokenCount - 1].tType != PIPE && cStream->tokenArray[cStream->tokenCount -1].tType != SEMICOLON)
			{	
     				curToken->wordString[index] = '\n';
				index++;
     				curToken->wordString[index] = '\0';     
     				curToken->tType = NEWLINE;
     			}	
			else continue;
			break;
			
   		}

		else if(test_word_char_valid(curChar))
		{
			curToken->wordString[index] = curChar;
			index++;
     			curChar = get_next_char(cStream);
	// Take in word characters to form the word until a seperator
			while(test_word_char_valid(curChar))
			{
       				
      
       // Add the array forming the next token
				curToken->wordString[index] = curChar;
				index++;
       				curChar = get_next_char(cStream);
			
				if(index >= maxTokenStringLength)
				{
					maxTokenStringLength += 10;
					checked_realloc(curToken->wordString, sizeof(char)*10);
				}
			}
     // Allocate memory if needed, then add the zero byte
				cStream->dontGet = 1;
				
				curToken->wordString[index] = '\0';     
				curToken->tType = WORD;
				break;
		}
   
   		else
   		{
     			error(1, 0, "%d: Invalid character", 
           		cStream->line_number);
     			break;
   		}
	}
	
	cStream->tokenArray[cStream->tokenCount] = *curToken;
	cStream->tokenCount++;
	if(cStream->tokenCount == cStream->maxTokens)
	{
		cStream->maxTokens += 20;
		checked_realloc(cStream->tokenArray, sizeof(struct token)*(cStream->maxTokens));
	}

	cStream->curCh = curChar;

}

void create_command_array(command_stream_t cStream)
{
	int tokenIterator = 0;
	while(cStream->tokenArray[tokenIterator].tType != END)
	{
		
		//declared in case of redirection
		int tempTokenIterator;
		int temp;

		switch(cStream->tokenArray[tokenIterator].tType)
		{		
			case AND:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = AND_COMMAND;
				cStream->arrayCommandsIndex++;
				break;
			case OR:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = OR_COMMAND;
				cStream->arrayCommandsIndex++;
				break;
			case WORD:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = SIMPLE_COMMAND;
				cStream->arrayCommands[cStream->arrayCommandsIndex].u.word = checked_malloc(sizeof(char*)*15);
				int optionIterator = 0;
				
				while(cStream->tokenArray[tokenIterator].tType == WORD)	
				{
					cStream->arrayCommands[cStream->arrayCommandsIndex].u.word[optionIterator] = cStream->tokenArray[tokenIterator].wordString;
					optionIterator++;
					tokenIterator++;
				}
				tokenIterator--;
				cStream->arrayCommandsIndex++;
				break;
			case PIPE:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = PIPE_COMMAND;
				cStream->arrayCommandsIndex++;
				break;
			case LESS_THAN:
				//set the input of the previous command
				cStream->arrayCommands[cStream->arrayCommandsIndex - 1].input = cStream->tokenArray[tokenIterator + 1].wordString;
				
				//move the array elements to the left 2 to remove the less_than sign and its argument
				for(temp = 0; temp < 2; temp++)
				{
					for(tempTokenIterator = tokenIterator; tempTokenIterator < cStream->tokenCount-1; tempTokenIterator++)
					{
						cStream->tokenArray[tempTokenIterator] = cStream->tokenArray[tempTokenIterator+1];
						
					}
				}
				tokenIterator--;
				break;
			case GREATER_THAN:
				//set the output of the previous command
				cStream->arrayCommands[cStream->arrayCommandsIndex - 1].output = cStream->tokenArray[tokenIterator + 1].wordString;
				
				//move the array elements to the left 2 to remove the greater_than sign and its argument
				for(temp = 0; temp < 2; temp++)
				{
					for(tempTokenIterator = tokenIterator; tempTokenIterator < cStream->tokenCount-1; tempTokenIterator++)
					{
						cStream->tokenArray[tempTokenIterator] = cStream->tokenArray[tempTokenIterator+1];
						
					}
				}
				tokenIterator--;
				break;
			case SEMICOLON:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = SEQUENCE_COMMAND;
				cStream->arrayCommandsIndex++;
				break;
			case NEWLINE:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = SPACER;
				cStream->arrayCommandsIndex++;
			case LEFT_PAREN:
				break;
			default: 
				break;
		}	
		tokenIterator++;	
	}
}

void parse_spacers(command_stream_t cStream)
{
	int spacerIterator = 1;
	int deletionIterator;
	while(spacerIterator != cStream->arrayCommandsIndex)
	{
		if(cStream->arrayCommands[spacerIterator].type == SPACER)
		{	
			//shift all left command
			for(deletionIterator = spacerIterator; deletionIterator != cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
	
			spacerIterator--;
			cStream->arrayCommandsIndex--;
		}
		spacerIterator++;
	}
}

void parse_serialcommand(command_stream_t cStream)
{
	int commandIterator;
	int deletionIterator;
	for(commandIterator = 0; commandIterator != cStream->arrayCommandsIndex; commandIterator++)
	{
		if(cStream->arrayCommands[commandIterator].type == SEQUENCE_COMMAND /*|| cStream->arrayCommands[commandIterator].type == OR_COMMAND*/)
		{
			//put them into a new home so they are not overwritten
			cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[commandIterator - 1];
			cStream->newHomeIterator++;
			cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[commandIterator + 1];
			cStream->newHomeIterator++;			

			//set the command array to the previous and next command; thus creating the node
			cStream->arrayCommands[commandIterator].u.command[0] = &cStream->newHome[cStream->newHomeIterator - 2];
			cStream->arrayCommands[commandIterator].u.command[1] = &cStream->newHome[cStream->newHomeIterator - 1];
			
			//delete the now useless nodes
			
			//first the node before the pipe
			for(deletionIterator = commandIterator - 1; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element
			cStream->arrayCommandsIndex--;
			//delete the now redundant command after pipe
			for(deletionIterator = commandIterator; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element again
			cStream->arrayCommandsIndex--;
		
			commandIterator--;	
		}
	}
}

void parse_andor(command_stream_t cStream)
{
	int commandIterator;
	int deletionIterator;
	for(commandIterator = 0; commandIterator != cStream->arrayCommandsIndex; commandIterator++)
	{
		if(cStream->arrayCommands[commandIterator].type == AND_COMMAND || cStream->arrayCommands[commandIterator].type == OR_COMMAND)
		{
			//put them into a new home so they are not overwritten
			cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[commandIterator - 1];
			cStream->newHomeIterator++;
			cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[commandIterator + 1];
			cStream->newHomeIterator++;			

			//set the command array to the previous and next command; thus creating the node
			cStream->arrayCommands[commandIterator].u.command[0] = &cStream->newHome[cStream->newHomeIterator - 2];
			cStream->arrayCommands[commandIterator].u.command[1] = &cStream->newHome[cStream->newHomeIterator - 1];
			
			//delete the now useless nodes
			
			//first the node before the pipe
			for(deletionIterator = commandIterator - 1; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element
			cStream->arrayCommandsIndex--;
			//delete the now redundant command after pipe
			for(deletionIterator = commandIterator; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element again
			cStream->arrayCommandsIndex--;
		
			commandIterator--;	
		}
	}
}


void parse_pipe(command_stream_t cStream)
{
	int commandIterator;
	int deletionIterator;
	for(commandIterator = 0; commandIterator != cStream->arrayCommandsIndex; commandIterator++)
	{
		if(cStream->arrayCommands[commandIterator].type == PIPE_COMMAND)
		{
			//put them into a new home so they are not overwritten
			cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[commandIterator - 1];
			cStream->newHomeIterator++;
			cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[commandIterator + 1];
			cStream->newHomeIterator++;			

			//set the command array to the previous and next command; thus creating the node
			cStream->arrayCommands[commandIterator].u.command[0] = &cStream->newHome[cStream->newHomeIterator - 2];
			cStream->arrayCommands[commandIterator].u.command[1] = &cStream->newHome[cStream->newHomeIterator - 1];
			
			//delete the now useless nodes
			
			//first the node before the pipe
			for(deletionIterator = commandIterator - 1; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element
			cStream->arrayCommandsIndex--;
			//delete the now redundant command after pipe
			for(deletionIterator = commandIterator; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element again
			cStream->arrayCommandsIndex--;
		
			commandIterator--;	
		}
	}
}


void parse_into_linear_array(command_stream_t cStream, command_t com)
{
	//traverse the left side of the current node
	if(com->u.command[0] != NULL && com->type != SUBSHELL_COMMAND && com->type != SIMPLE_COMMAND)
		parse_into_linear_array(cStream, &(*(com->u.command[0])));

	//insert current command and increment counter
	cStream->linearCommandArray[cStream->linearCounter] = *com;
	cStream->linearCounter++;
	
	//traverse the right side of the current node
	if(com->u.command[1] != NULL && com->type != SUBSHELL_COMMAND && com->type != SIMPLE_COMMAND)
		parse_into_linear_array(cStream, &(*(com->u.command[1])));
}

command_t read_command_stream (command_stream_t s)
{
	if(s->outputCounter != s->arrayCommandsIndex)
	{
		command_t readCommand = &(s->arrayCommands[s->outputCounter]);
		s->outputCounter++;
		return readCommand;	
	}
	else
		return NULL;
}
