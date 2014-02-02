// UCLA CS 111 Lab 1 command internals

enum command_type
{
	AND_COMMAND,         // A && B - B only runs if A exists and returns zero (true)
	SEQUENCE_COMMAND,    // A ; B - Runs B serially after A
	OR_COMMAND,          // A || B - B only runs if A exists with nonzero (false)
	PIPE_COMMAND,        // A | B - pipes the output of A into the input of B
	SIMPLE_COMMAND,      // a simple command - just a regular command (i.e. ls)
	SUBSHELL_COMMAND,    // ( A ) - a child shell is spawned to handle these commands
};

enum token_type
{
	AND,
        OR,
        WORD,
        PIPE,
        LEFT_PAREN,
        RIGHT_PAREN,
        LESS_THAN,
        GREATER_THAN,
        SEMICOLON,
        END
};

struct command
{

  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
  char *input;
  char *output;

  union //this is a union because a command could only be one of these types. 
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
};

struct command_stream
{
	int tokenIndex; //so we can tell which token we are currently reading
        char curCh;
        int finalIndex;
        int (*getbyte) (void *);
        void *arg;
        token_t tokenArray;
        int tokenCount;
        int maxTokens;
        int maxCommands;
	int iterator;
        command_t finalCommandArray;

        int dontGet; //if set to 1 causes get_next_char to not read the next character immediately
 // Line count
        int line_number;

        token_t arrayOperators;
        command_t arrayOperands;
	int indexOperand;
}; 

struct token  //Need a token because words can have a value
{
        enum token_type tType;
// Data associated with a command.
        char* wordString;
};
