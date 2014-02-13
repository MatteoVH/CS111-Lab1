enum command_type
{
	AND_COMMAND,
	SEQUENCE_COMMAND,
	OR_COMMAND,          // A || B - B only runs if A exists with nonzero (false)
	PIPE_COMMAND,        // A | B - pipes the output of A into the input of B
	SIMPLE_COMMAND,      // a simple command - just a regular command (i.e. ls)
	SUBSHELL_COMMAND,    // ( A ) - a child shell is spawned to handle these commands
	SPACER		     // for NEWLINES
};

enum token_type
{
	BEGIN,
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

struct command
{

  enum command_type type;

  // Exit status
  int status;

  // I/O redirections. If none, value is 0.
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

        int dontGet; //if set to 1 causes get_next_char to not read the next character immediately
 // Line count
        int lineNumber;

        command_t arrayCommands;
	int arrayCommandsIndex;

	command_t newHome;
	int newHomeIterator;

	//this int is to keep track of what command the read_command_stream function is at
	int outputCounter;
}; 

struct token  //Need a token because words can have a value
{
        enum token_type tType;
// Data associated with a command.
        char* wordString;
	//for error checking purposes
	int lineFound;
	int isParenRead;
};

struct word_node
{
  char* word;
  struct word_node* next;
};

struct dep_node
{
  struct parent_node* dependent;
  
  struct dep_node* next;

};

//Nodes describing parent commands and their dependencies
struct parent_node
{
  struct command* command;

  struct word_node* inputs;
  struct word_node* outputs;
  int dep_counter;
  struct dep_node* dependencies;
  
  int pid;
  
  struct parent_node* next;
};
