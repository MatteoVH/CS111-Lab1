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
		NEWLINE,
		END
	};
// Data associated with a command.
struct token
{
	token_type tType;
	char* string;
	
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
