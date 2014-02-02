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
	cStream->tokenCount=0;
	cStream->dontGet = 0;
	cStream->maxTokens = 20;
	cStream->maxCommands = 20;
	cStream->tokenArray = checked_malloc(sizeof(struct token)*(cStream->tokenCount));

	cStream->getbyte = get_next_byte;
	cStream->arg = get_next_byte_argument;
	cStream->iterator = 0;
 
	cStream->tokenIndex = 0;
	
 	cStream->arrayOperators = checked_malloc(sizeof(struct token)*(cStream->tokenCount));
	cStream->arrayOperands = checked_malloc(sizeof(struct command)*(cStream->tokenCount));
 // -2 if no char exists, initialized as such 
	cStream->line_number = 1;

	do {
	read_next_token(cStream, cStream->curCh);
	}
	while(get_current_token_type(cStream) != END); //insert all tokens into array

	parse_token(cStream);

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
     
     			curToken->wordString[index] = ';';
			index++;
     			curToken->wordString[index] = '\0';     
     			curToken->tType = SEMICOLON;
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
		checked_realloc(cStream->tokenArray, sizeof(struct token)*20);
		cStream->maxTokens += 20;
	}

	cStream->curCh = curChar;

}

void parse_andor(command_stream_t cStream)
{
	command_t leftCommand = parse_pipe(cStream);

}

void parse_pipe(command_stream_t cStream)
{
	command_t leftCommand = parse_redirection(cStream);

}

void parse_redirection(command_stream_t cStream)
{
	while(	

}


//parse the token array with a dual operator/operand algorithm
void parse_token(command_stream_t cStream)
{

	parse_andor(cStream);

/*
	int indexOperator = 0;
	cStream->indexOperand = 0;
	int tokenIndex = 0;
	int wordIndex = 0;
	int wordSize = 15;
		
	//Distribute tokens into two arrays, one for operands and one for operators
	while (tokenIndex < cStream->tokenCount)
	{
		token_t curToken = &(cStream->tokenArray[tokenIndex]);
		if(curToken->tType == WORD)
		{
			command_t curComm = checked_malloc(sizeof(struct command));
			curComm->type = SIMPLE_COMMAND;
			curComm->status = -1;
			curComm->input = NULL;
			curComm->output = NULL;
			curComm->u.word = checked_malloc(sizeof(char*)*wordSize);
			curComm->u.word[wordIndex] = curToken->wordString;
			tokenIndex++;
			wordIndex++;
			while(cStream->tokenArray[tokenIndex].tType == WORD)
			{
				curComm->u.word[wordIndex] = cStream->tokenArray[tokenIndex].wordString;
				if(wordIndex >= wordSize)
				{
					checked_realloc(curComm->u.word, sizeof(char*)*15);
					wordSize += 15;
				}

				wordIndex++;
				tokenIndex++;
			}
			cStream->arrayOperands[cStream->indexOperand] = *curComm;
			cStream->indexOperand++;
		}
		else if(curToken->tType != END)
		{
			cStream->arrayOperators[indexOperator] = *curToken;
			indexOperator++;
			tokenIndex++;
		}
			tokenIndex++; //if END
		
	}

	int rank1; int rank2; int rank3;
	int x; int y;
//dual stack code to replace here
if (indexOperator > 0)
{
	for (rank1 = 0; rank1 < 4; rank1++)
	{
		token_t curToken1 = &(cStream->arrayOperators[rank1]);
		if(curToken1->tType == LESS_THAN || curToken1->tType == GREATER_THAN)
		{
			cStream->arrayOperands[rank1] = *(buildTree(cStream, curToken1, rank1));
			for(x = rank1+1; x < cStream->indexOperand-1; x++) // moves everything 2 elements above rank closer to rank by a space
			{
				cStream->arrayOperands[x] = cStream->arrayOperands[x+1];
			}
			for(y = rank1; y < indexOperator-1; y++) // moves everything 2 elements above rank closer to rank by a space
			{
				cStream->arrayOperators[y] = cStream->arrayOperators[y+1];
			}
		}
	}
	for (rank2 = 0; rank2 < 4; rank2++)
	{

		token_t curToken2 = &(cStream->arrayOperators[rank2]);
		if(curToken2->tType == PIPE)
		{
			cStream->arrayOperands[rank2] = *(buildTree(cStream, curToken2, rank2));
			
			for(x = rank2+1; x < cStream->indexOperand-1; x++) // moves everything 2 elements above rank closer to rank by a space
			{
				cStream->arrayOperands[x] = cStream->arrayOperands[x+1];
			}
			for(y = rank2; y < indexOperator-1; y++) // moves everything 2 elements above rank closer to rank by a space
			{
				cStream->arrayOperators[y] = cStream->arrayOperators[y+1];
			}
		}
	}
	for (rank3 = 0; rank3 < 4; rank3++)
	{

		token_t curToken3 = &(cStream->arrayOperators[rank3]);
		if(curToken3->tType == AND || curToken3->tType == OR)
		{
			cStream->arrayOperands[rank3] = *(buildTree(cStream, curToken3, rank3));
			for(x = rank3+1; x < cStream->indexOperand; x++) // moves everything 2 elements above rank closer to rank by a space
			{
				cStream->arrayOperands[x] = cStream->arrayOperands[x+1];
			}
			for(y = rank3; y < indexOperator-1; y++) // moves everything 2 elements above rank closer to rank by a space
			{
				cStream->arrayOperators[y] = cStream->arrayOperators[y+1];
			}
		}
	}
	
}	*/
}


command_t buildTree(command_stream_t cStream, token_t curToken, int rank)
{
//empty tree
 	command_t top = checked_malloc(sizeof(struct command));
//translate a token into a command
// doesn't work: top->type = curToken->tType;
	switch(curToken->tType)
	{
		case AND: top->type = AND_COMMAND; break;
		case OR: top->type = OR_COMMAND; break;
		case PIPE: top->type = PIPE_COMMAND; break;
		//case LESS_THAN: top->type = LESS_THAN; break;
		//case GREATER_THAN: top->type = GREATER_THAN; break;
		case SEMICOLON: top->type = SEQUENCE_COMMAND; break;
		//case END: top->type = ; break;
		case WORD: top->type = SIMPLE_COMMAND; break;
		default: error(1, 0, "unknown type");

	}
	top->status = -1;
	top->input = NULL;
	top->output = NULL;
	command_t a = NULL;
	command_t b = NULL;
	if (rank<(cStream->indexOperand))
	{	
		a = &(cStream->arrayOperands[rank+1]); 
	}
	if (rank<=(cStream->indexOperand))
		b = &(cStream->arrayOperands[rank]);
	top->u.command[0]=a;
	top->u.command[1]=b;	
	return top;
	
}	
	

command_t read_command_stream (command_stream_t s)
 {
 if (s->iterator == s->indexOperand)
  {
    s->iterator = 0;
    return NULL;
  }	
	return &(s->arrayOperands[s->iterator++]);
}
