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
	
	cStream->lineNumber = 1;

	do {
	read_next_token(cStream, cStream->curCh);
	}
	while(get_current_token_type(cStream) != END); //insert all tokens into array

	//make sure that all syntax is correct. Return error if it isn't.
	error_check_syntax(cStream);
	
	int i;
	for(i = 0; i != cStream->tokenCount; i++)
		cStream->tokenArray[i].isParenRead = 0;

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

	
	cStream->newHomeIterator = 0;
	cStream->newHome = checked_malloc(sizeof(struct command)*(cStream->tokenCount)*2);
	
	//puts the tokens into actual command structs, and also parses redirections as well
	create_command_array(cStream, 0, cStream->tokenCount);

		
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
	curToken->isParenRead = 0;
	
	//add the BEGIN token at the start
	if(cStream->tokenCount == 0)
	{
		curToken->tType = BEGIN;
		cStream->tokenArray[cStream->tokenCount] = *curToken;
		cStream->tokenCount++;
		return;
	}

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
				error(1, 0, "line %d: Found lone &, invalid character", cStream->lineNumber);
				break;
			}
			
     			curToken->wordString[index] = curChar;
			index++;     
			
     //Add the end zero byte
			curToken->wordString[index] = '\0';     
			curToken->tType = AND;
			curToken->lineFound = cStream->lineNumber;
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
			curToken->lineFound = cStream->lineNumber;
     			break;
   		}
	
		
   		else if(curChar == '(')
   		{
   			curToken->wordString[index] = curChar;
			index++;
     			curToken->wordString[index] = '\0';     
     			curToken->tType = LEFT_PAREN;
			curToken->lineFound = cStream->lineNumber;
     			break;
   		}
		
		
   		else if(curChar == ')')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = RIGHT_PAREN;
			curToken->lineFound = cStream->lineNumber;
     			break;
   		}

   		else if(curChar == '<')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = LESS_THAN;
			curToken->lineFound = cStream->lineNumber;
     			break;
   		}

   		else if(curChar == '>')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = GREATER_THAN;
			curToken->lineFound = cStream->lineNumber;
     			break;
   		}
    
   		else if(curChar == ';')
   		{
    			curToken->wordString[index] = curChar;
     			index++; 
			curToken->wordString[index] = '\0';
     			curToken->tType = SEMICOLON;
			curToken->lineFound = cStream->lineNumber;
     			break;
   		}

   		else if(curChar == '\n')
   		{
    			cStream->lineNumber++;
     			curChar = get_next_char(cStream);
     // Ignore any subsequent whitespace, but keep line count
     			while(curChar == '\n' || curChar == '\t' || curChar == ' ')
     			{
				if(curChar == '\n')
       					cStream->lineNumber++;
				curChar = get_next_char(cStream);
     			}
       
     			if(curChar == EOF)
     			{
       				curToken->tType = END;
				break;
     			}
     			cStream->dontGet = 1;
     			if(cStream->tokenArray[cStream->tokenCount - 1].tType != AND && cStream->tokenArray[cStream->tokenCount - 1].tType != OR &&
			   cStream->tokenArray[cStream->tokenCount - 1].tType != PIPE && cStream->tokenArray[cStream->tokenCount - 1].tType != SEMICOLON)
			{	
     				curToken->wordString[index] = '\n';
				index++;
     				curToken->wordString[index] = '\0';     
     				curToken->tType = NEWLINE;
				curToken->lineFound = cStream->lineNumber;
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
					maxTokenStringLength += 30;
					curToken->wordString = checked_realloc(curToken->wordString, sizeof(char)*maxTokenStringLength);
				}
			}
     // Allocate memory if needed, then add the zero byte
				cStream->dontGet = 1;
				
				curToken->wordString[index] = '\0';     
				curToken->tType = WORD;
				curToken->lineFound = cStream->lineNumber;
				break;
		}
   
   		else
   		{
     			error(1, 0, "line %d: Invalid character", 
           		cStream->lineNumber);
     			break;
   		}
	}
	
	cStream->tokenArray[cStream->tokenCount] = *curToken;
	cStream->tokenCount++;
	if(cStream->tokenCount >= cStream->maxTokens)
	{
		cStream->maxTokens += 40;
		cStream->tokenArray = checked_realloc(cStream->tokenArray, sizeof(struct token)*(cStream->maxTokens));
	}

	cStream->curCh = curChar;

}

void error_check_syntax(command_stream_t cStream)
{
	//run through all tokens
	int tokenIterator = 0;
	int temp;
	while(cStream->tokenArray[tokenIterator].tType != END)
	{
		switch(cStream->tokenArray[tokenIterator].tType)
		{
			case RIGHT_PAREN:
				if(cStream->tokenArray[tokenIterator - 1].tType != WORD)
					error(1, 0, "line %d: Expected a command before ')'.", cStream->tokenArray[tokenIterator].lineFound);
				temp = tokenIterator;
				while(cStream->tokenArray[temp].tType != BEGIN)
				{
					if(cStream->tokenArray[temp].tType == LEFT_PAREN && cStream->tokenArray[temp].isParenRead == 0)
					{	
						cStream->tokenArray[temp].isParenRead = 1;
						break;
					}
					temp--;
				}	
					if(cStream->tokenArray[temp].tType == BEGIN)
						error(1, 0, "line %d: Did not find matching left parenthesis.", cStream->tokenArray[tokenIterator].lineFound);
				break;
			case LEFT_PAREN:
				if(cStream->tokenArray[tokenIterator + 1].tType != WORD)
					error(1, 0, "line %d: Expected a command after '('.", cStream->tokenArray[tokenIterator].lineFound);
				temp = tokenIterator;
				while(cStream->tokenArray[temp].tType != END)
				{
					if(cStream->tokenArray[temp].tType == RIGHT_PAREN && cStream->tokenArray[temp].isParenRead == 0)
					{	
						cStream->tokenArray[temp].isParenRead = 1;
						break;
					}
					temp++;
				}	
					if(cStream->tokenArray[temp].tType == END)
						error(1, 0, "line %d: Did not find matching right parenthesis.", cStream->tokenArray[tokenIterator].lineFound);		
				break;	
			case PIPE:
			case AND:
			case OR:
				if(cStream->tokenArray[tokenIterator + 1].tType != WORD && cStream->tokenArray[tokenIterator + 1].tType != LEFT_PAREN)
					error(1, 0, "line %d: Binary operator not followed by a valid command.", cStream->tokenArray[tokenIterator].lineFound);
			case SEMICOLON:
		
				if(cStream->tokenArray[tokenIterator - 1].tType != WORD && cStream->tokenArray[tokenIterator - 1].tType != RIGHT_PAREN)
					error(1, 0, "line %d: Binary operator not preceded by a valid command.", cStream->tokenArray[tokenIterator].lineFound);
				break;					
			case GREATER_THAN:
				if(cStream->tokenArray[tokenIterator + 2].tType == LESS_THAN)
					error(1, 0, "line %d: Cannot have input redirection directly following output redirection.", cStream->tokenArray[tokenIterator].lineFound);
					
			case LESS_THAN:
				if(cStream->tokenArray[tokenIterator - 1].tType != WORD && cStream->tokenArray[tokenIterator - 1].tType != RIGHT_PAREN)
					error(1, 0, "line %d: Redirection not preceded by a valid command.", cStream->tokenArray[tokenIterator].lineFound);
				if(cStream->tokenArray[tokenIterator + 1].tType != WORD)
					error(1, 0, "line %d: Redirection not followed by a valid command.", cStream->tokenArray[tokenIterator].lineFound);
				break;
			default: 
				break;
		}
		tokenIterator++;
	}
	 
}

void create_command_array(command_stream_t cStream, int begin, int end)
{
	int nextBegin = cStream->arrayCommandsIndex;
	int tokenIterator = begin;
	while(tokenIterator < end)
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
				//count number of words to put in array
				int wordCount = 0;
				int wordIterator = tokenIterator;
				while(cStream->tokenArray[wordIterator].tType == WORD && wordIterator != end)
				{	
					wordCount++;		
					wordIterator++;
				}
			
				cStream->arrayCommands[cStream->arrayCommandsIndex].u.word = checked_malloc(sizeof(char*)*(wordCount+1));
				int optionIterator = 0;
				
				while(wordCount != 0)	
				{
					cStream->arrayCommands[cStream->arrayCommandsIndex].u.word[optionIterator] = cStream->tokenArray[tokenIterator].wordString;
					optionIterator++;
					tokenIterator++;
					wordCount--;
				}
				cStream->arrayCommands[cStream->arrayCommandsIndex].u.word[optionIterator] = NULL;
				tokenIterator--;
				cStream->arrayCommandsIndex++;
				break;
			case PIPE:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = PIPE_COMMAND;
				cStream->arrayCommandsIndex++;
				break;
			case LESS_THAN:
				//check for input overload
				if(cStream->arrayCommands[cStream->arrayCommandsIndex - 1].input != NULL)
					error(1, 0, "line %d: Input redirection overloaded.", cStream->tokenArray[tokenIterator].lineFound);

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
				end -= 2;
				break;
			case GREATER_THAN:
				//check for output overload
				if(cStream->arrayCommands[cStream->arrayCommandsIndex - 1].output != NULL)
					error(1, 0, "line %d: Output redirection overloaded.", cStream->tokenArray[tokenIterator].lineFound);
				
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
				end -= 2;
				tokenIterator--;
				break;
			case SEMICOLON:
				if(cStream->tokenArray[tokenIterator + 1].tType == NEWLINE || cStream->tokenArray[tokenIterator + 1].tType == END)
					break;
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = SEQUENCE_COMMAND;
				cStream->arrayCommandsIndex++;
				break;
			case NEWLINE:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = SPACER;
				cStream->arrayCommandsIndex++;
				break;
			case LEFT_PAREN:
				cStream->arrayCommands[cStream->arrayCommandsIndex].type = SUBSHELL_COMMAND;
				cStream->arrayCommandsIndex++;
				tokenIterator++;

				int indexNow = cStream->arrayCommandsIndex;

				//find position of last parenthesis
				int x;
				for(x = end; x != tokenIterator; x--)
				{
					if(cStream->tokenArray[x].tType == RIGHT_PAREN && cStream->tokenArray[x].isParenRead == 0)
					{
						cStream->tokenArray[x].isParenRead = 1;
						break;
					}
				}

				create_command_array(cStream, tokenIterator, x);
				int diff = cStream->arrayCommandsIndex - indexNow;
				
				cStream->arrayCommands[indexNow - 1].u.subshell_command = checked_malloc(sizeof(struct command)*diff);

				while(diff != 0)
				{
					cStream->newHome[cStream->newHomeIterator] = cStream->arrayCommands[cStream->arrayCommandsIndex - 1]; 	
					
					cStream->arrayCommands[indexNow - 1].u.subshell_command = &cStream->newHome[cStream->newHomeIterator];
					cStream->newHomeIterator++;
					cStream->arrayCommandsIndex--;
					diff--;
				}
				tokenIterator = x;	

				break;
			default: 
				break;
		}	
		tokenIterator++;	
	}
	//parses all pipe commands
	parse_pipe(cStream, nextBegin, cStream->arrayCommandsIndex);
}

void parse_spacers(command_stream_t cStream, int begin, int end)
{
	int spacerIterator = begin;
	int deletionIterator;
	while(spacerIterator != end)
	{
		if(cStream->arrayCommands[spacerIterator].type == SPACER)
		{	
			//shift all left command
			for(deletionIterator = spacerIterator; deletionIterator != cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
	
			spacerIterator--;
			cStream->arrayCommandsIndex--;
			end--;
		}
		spacerIterator++;
	}
}

void parse_serialcommand(command_stream_t cStream, int begin, int end)
{
	int commandIterator;
	int deletionIterator;
	for(commandIterator = begin; commandIterator != end; commandIterator++)
	{
		if(cStream->arrayCommands[commandIterator].type == SEQUENCE_COMMAND)
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
			end--;
			//delete the now redundant command after pipe
			for(deletionIterator = commandIterator; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element again
			cStream->arrayCommandsIndex--;
			end--;
			commandIterator--;	
		}
	}
	//parse NEWLINE spacers
	parse_spacers(cStream, begin, end);
}

void parse_andor(command_stream_t cStream, int begin, int end)
{
	int commandIterator;
	int deletionIterator;
	for(commandIterator = begin; commandIterator != end; commandIterator++)
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
			end--;
			//delete the now redundant command after pipe
			for(deletionIterator = commandIterator; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element again
			cStream->arrayCommandsIndex--;
			end--;
			commandIterator--;	
		}
	}
	//parses all SEMICOLON commands
	parse_serialcommand(cStream, begin, end);
}


void parse_pipe(command_stream_t cStream, int begin, int end)
{
	int commandIterator;
	int deletionIterator;
	for(commandIterator = begin; commandIterator != end; commandIterator++)
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
			end--;
			//delete the now redundant command after pipe
			for(deletionIterator = commandIterator; deletionIterator < cStream->arrayCommandsIndex - 1; deletionIterator++)
				cStream->arrayCommands[deletionIterator] = cStream->arrayCommands[deletionIterator + 1];
			//remove the last redundant element again
			cStream->arrayCommandsIndex--;
			end--;
			commandIterator--;	
		}
	}
	//parses all AND or OR commands
	parse_andor(cStream, begin, end);
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
