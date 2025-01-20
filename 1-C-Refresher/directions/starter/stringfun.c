#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>   // for boolean usage (optional)
#include <ctype.h>     // for isspace() if you wish to use it, but not required
#include <string.h>    // included for memcpy, memset if needed

#define BUFFER_SZ 50

// Prototypes
void usage(char *exename);
void print_buff(char *buff, int len);
int  setup_buff(char *buff, char *user_str, int len);
int  count_words(char *buff, int buff_len, int str_len);
int  reverse_string(char *buff, int str_len);
int  print_words(char *buff, int buff_len, int str_len);

int main(int argc, char *argv[])
{
    char *buff;             // internal buffer pointer
    char *input_string;     // holds the string provided by the user on the cmd line
    char opt;               // used to capture user option from cmd line
    int  rc;                // used for return codes
    int  user_str_len;      // length of user-supplied string

    //------------------------------------------------------------------------------
    // TODO #1: WHY IS THIS SAFE?
    //
    // Because we first check if (argc < 2). If so, we exit, so we never call argv[1]
    // if it doesn’t exist. We also check (*argv[1] != '-'). Thus, we only proceed
    // to use argv[1] if it definitely exists and starts with '-'.
    //------------------------------------------------------------------------------
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);  // 1 = command line problem
    }

    opt = (char)*(argv[1] + 1);  // extract the single character after '-'

    // handle the help flag and then exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);  // 0 = success
    }

    //------------------------------------------------------------------------------
    // TODO #2: Document the purpose of the if statement below:
    //
    // This checks if the user provided at least 3 arguments: 
    //    1) the executable name (argv[0])
    //    2) the option flag   (argv[1])
    //    3) the "sample string" (argv[2]) 
    // If not, we print usage and exit with an error code.
    //------------------------------------------------------------------------------
    if (argc < 3) {
        usage(argv[0]);
        exit(1);  // 1 = command line problem
    }

    // Capture the input string
    input_string = argv[2];

    //------------------------------------------------------------------------------
    // TODO #3: Allocate space for the buffer using malloc() and
    //          handle error if malloc fails by exiting with code 2 
    //          (memory allocation failure).
    //------------------------------------------------------------------------------
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(2); // 2 = memory allocation failure
    }

    //------------------------------------------------------------------------------
    // Copy user_str into buff (collapsing whitespace, etc.)
    //------------------------------------------------------------------------------
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);

    // If setup_buff returned a negative number, it indicates an error
    if (user_str_len < 0) {
        printf("Error setting up buffer. Error code = %d\n", user_str_len);
        free(buff);
        exit(3);  // 3 = error with one of your services (e.g., too long, etc.)
    }

    //------------------------------------------------------------------------------
    // Switch on the user’s option to decide what operation to perform
    //------------------------------------------------------------------------------
    switch (opt) {
        case 'c':  // count words
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(3); 
            }
            printf("Word Count: %d\n", rc);
            break;

        case 'r':  // reverse string
            rc = reverse_string(buff, user_str_len);
            if (rc < 0) {
                printf("Error reversing string, rc = %d\n", rc);
                free(buff);
                exit(3);
            }
            printf("Reversed String: ");
            // Print only the user input portion (no trailing dots)
            for (int i = 0; i < user_str_len; i++) {
                putchar(*(buff + i));
            }
            putchar('\n');
            break;

        case 'w':  // print words and their lengths
            rc = print_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error printing words, rc = %d\n", rc);
                free(buff);
                exit(3);
            }
            // If you also want to show the total word count:
            // printf("Total words: %d\n", rc);
            break;

        case 'x':
            // For basic assignment: Just ensure we have enough arguments
            // (argc should be 5 if we do: -x "some string" from to)
            // Then print "Not Implemented!"
            if (argc < 5) {
                printf("Error: Missing arguments for -x option.\n");
                free(buff);
                exit(1);
            }
            // If you do the extra credit, implement the replace logic here.
            printf("Not Implemented!\n");
            // or exit(0) with your own logic...
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //------------------------------------------------------------------------------
    // TODO #6: Free your buffer before exiting
    //------------------------------------------------------------------------------
    print_buff(buff, BUFFER_SZ); // final diagnostic print
    free(buff);

    // exit success
    exit(0);
}


/** 
 *  usage()
 *  Prints out the usage for the program.
 */
void usage(char *exename)
{
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
    printf("  -h   prints this help message\n");
    printf("  -c   counts the number of words in the \"sample string\"\n");
    printf("  -r   reverses the characters in the \"sample string\"\n");
    printf("  -w   prints the individual words and their length\n");
    printf("  -x   (extra) replace the first occurrence of [arg3] with [arg4]\n");
}


/**
 *  print_buff()
 *  Debug/diagnostic function to show the entire internal buffer 
 *  (including trailing dot characters).
 */
void print_buff(char *buff, int len)
{
    printf("Buffer:  ");
    for (int i = 0; i < len; i++) {
        putchar(*(buff + i));
    }
    putchar('\n');
}


/**
 *  setup_buff()
 *
 *  Copies user_str into buff, ignoring all leading/trailing whitespace, 
 *  collapsing consecutive whitespace into a single `' '`, and replacing 
 *  tabs or other whitespace with `' '`.
 *
 *  Then, after the user_str content is copied, fills the remainder of buff 
 *  with '.' characters (no null terminator).
 *
 *  Returns:
 *    > 0 = number of characters actually copied (length of processed string)
 *    -1  = user_str is too large for the 50-byte buff
 *    -2  = any other error you want to define
 */
int setup_buff(char *buff, char *user_str, int buff_size)
{
    char *read_ptr  = user_str;
    char *write_ptr = buff;
    int   count     = 0;        // how many characters we've actually placed
    bool  in_space  = false;    // track if the last character we wrote was space

    // While we haven't hit the user_str's null terminator
    while (*read_ptr != '\0') {

        // Decide if the current character is "whitespace"
        // We treat space, tab, etc. all as spaces in final output
        // (You could also do manual checks if you don't want to use ctype.h)
        if (*read_ptr == ' ' || *read_ptr == '\t') {
            // We only want to write a single space if we're not already
            // in a "space block"
            if (!in_space && count < buff_size) {
                *write_ptr = ' ';
                write_ptr++;
                count++;
                in_space = true;
                // If count == buff_size after writing, we must stop
                if (count == buff_size) {
                    break;
                }
            }
            // else if already in_space, ignore
        } else {
            // It's a non-whitespace character
            in_space = false;
            // Check if we have room
            if (count < buff_size) {
                *write_ptr = *read_ptr;
                write_ptr++;
                count++;
            } else {
                // We ran out of space
                return -1; // input too large
            }
        }
        read_ptr++;
    }

    // After copying user_str, fill the remainder with '.'
    // only if we haven't already used up the entire buffer
    while (count < buff_size) {
        *write_ptr = '.';
        write_ptr++;
        count++;
    }

    // The number of actual "meaningful" characters in the user string portion
    // is everything up to the first dot, i.e., how many we wrote before dot-filling.
    // But that number is the `count` minus the number of trailing dots.  However,
    // we used `count` as total written. The actual user string length = 
    // total characters minus trailing dots.  We can do that by scanning from 
    // the left to see how many real (non-dot) chars we wrote, or keep a separate 
    // variable. For simplicity, let's keep a separate variable while we’re copying.
    //
    // In the code above, we used `count` to track *all writes*, so the "meaningful"
    // user string length is how many times we wrote either a space or a real char
    // until read_ptr ended. Let's track that in a second variable or just do 
    // a quick pass:
    int user_len = 0;
    for (int i = 0; i < buff_size; i++) {
        if (*(buff + i) == '.') {
            break;
        }
        user_len++;
    }

    return user_len;
}


/**
 *  count_words()
 *
 *  Returns the number of words found in buff. 
 *  We assume that buff has no leading/trailing or consecutive spaces 
 *  besides single `' '` (because of how setup_buff() preprocessed).
 *
 *  Return:
 *    >= 0 = number of words
 *    <  0 = error
 */
int count_words(char *buff, int buff_len, int str_len)
{
    if (str_len > buff_len) {
        return -1; // error: string length bigger than buffer?
    }
    if (str_len == 0) {
        return 0;  // no words in an empty string
    }

    int word_count = 0;
    bool in_space  = true;  // we'll say we start "in space"

    for (int i = 0; i < str_len; i++) {
        char c = *(buff + i);
        if (c == ' ') {
            in_space = true;
        } else {
            // c is not space
            // if the last char was space, that means we've hit the start of a word
            if (in_space) {
                word_count++;
                in_space = false;
            }
        }
    }

    return word_count;
}


/**
 *  reverse_string()
 *
 *  Reverse the first str_len characters in buff (in place).
 *  Return 0 on success, < 0 on error.
 */
int reverse_string(char *buff, int str_len)
{
    if (str_len <= 0) {
        return 0; // nothing to reverse
    }

    char *start = buff;
    char *end   = buff + (str_len - 1);

    while (start < end) {
        // swap *start, *end
        char temp = *start;
        *start    = *end;
        *end      = temp;
        start++;
        end--;
    }
    return 0;
}


/**
 *  print_words()
 *
 *  Prints each word on its own line, with a running index and the word's length.
 *  Example:
 *      Word Print
 *      ----------
 *      1. Systems (7)
 *      2. Programming (11)
 *      3. is (2)
 *      4. Great! (6)
 *
 *  Returns the total number of words, or < 0 on error.
 */
int print_words(char *buff, int buff_len, int str_len)
{
    if (str_len > buff_len) {
        return -1;
    }
    if (str_len == 0) {
        printf("No words to print.\n");
        return 0;
    }

    printf("Word Print\n");
    printf("----------\n");

    int  word_count = 0;
    bool in_space   = true;   // to detect new word boundaries
    int  char_count = 0;      // how many chars in the current word

    for (int i = 0; i < str_len; i++) {
        char c = *(buff + i);

        if (c == ' ') {
            // We reached the end of a word if we are currently in a word
            if (!in_space) {
                // print the length and close the line
                printf(" (%d)\n", char_count);
                char_count = 0;
                in_space   = true;
            }
        } else {
            // c is not space
            if (in_space) {
                // we are starting a new word
                word_count++;
                printf("%d. ", word_count);
                in_space = false;
            }
            // print the character
            putchar(c);
            char_count++;
        }
    }

    // If the string didn't end with a space, we need to close off the last word
    if (!in_space) {
        printf(" (%d)\n", char_count);
    }

    return word_count;
}
