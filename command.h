// UCLA CS 111 Lab 1 command interface

typedef struct command_stream *command_stream_t;
typedef struct command *command_t;
typedef struct token *token_t;

void create_command_array(command_stream_t);

void parse_andor(command_stream_t);

void parse_pipe(command_stream_t);

void parse_redirection(command_stream_t);

void parse_token(command_stream_t);

enum token_type get_current_token_type(command_stream_t);

void read_next_token(command_stream_t, char);

void parse_into_linear_array(command_stream_t, command_t);
 
void parse_serialcommand(command_stream_t);

void parse_spacers(command_stream_t);

command_t buildTree(command_stream_t, token_t, int);

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t, int);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);
