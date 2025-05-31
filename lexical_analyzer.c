#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN 100
#define MAX_STRING_LEN 1024
#define MAX_VAR_LEN 20

// Token types enumeration
typedef enum {
    TOKEN_KEYWORD,          // Represents a keyword
    TOKEN_IDENTIFIER,       // Represents an identifier
    TOKEN_END_OF_LINE,      // Represents end of line
    TOKEN_OPERATOR,         // Represents an operator
    TOKEN_INT_CONSTANT,     // Represents an integer constant
    TOKEN_STRING_CONSTANT,  // Represents a string constant
    TOKEN_OPEN_BLOCK,       // Represents an open block '{'
    TOKEN_CLOSE_BLOCK,      // Represents a close block '}'
    TOKEN_ERROR,            // Represents an error
    TOKEN_EOF               // Represents end of file
} TokenType;

// List of keywords
const char *keywords[] = {
    "number", "write", "and", "newline", "repeat", "times", NULL
};

// List of operators
const char *operators[] = {
    ":=", "+=", "-=", NULL
};

// Token structure to store token details
typedef struct {
    TokenType type;         // Type of the token
    char value[MAX_TOKEN_LEN + 1]; // Token value
    int line;               // Line number of the token
    int col;                // Column number of the token
} Token;

// Global variables for file handling and character tracking
FILE *input_file;
FILE *output_file;
int current_line = 1;       // Current line number
int current_col = 0;        // Current column number
int last_char_col = 0;      // Column of the last character
int last_char_line = 1;     // Line of the last character
int current_char = 0;       // Current character being processed

// Function prototypes
void next_char();           // Reads the next character
Token get_next_token();     // Retrieves the next token
bool is_keyword(const char *str); // Checks if a string is a keyword
bool is_operator(const char *str); // Checks if a string is an operator
void write_token_to_file(Token token); // Writes a token to the output file

// Reads the next character and updates line/column counters
void next_char() {
    last_char_line = current_line;
    last_char_col = current_col;

    current_char = fgetc(input_file);

    
    if (current_char == '\n') {
        current_line++;
        current_col = 0;
    } else {
        current_col++;
    }
}

// Checks if a string matches any keyword
bool is_keyword(const char *str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Checks if a string matches any operator
bool is_operator(const char *str) {
    for (int i = 0; operators[i] != NULL; i++) {
        if (strcmp(str, operators[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Writes a token to the output file
void write_token_to_file(Token token) {
    if (token.type == TOKEN_EOF) return; // EOF yazma

    const char *type_str;
    switch (token.type) {
        case TOKEN_KEYWORD: type_str = "Keyword"; break;
        case TOKEN_IDENTIFIER: type_str = "Identifier"; break;
        case TOKEN_END_OF_LINE: type_str = "EndOfLine"; break;
        case TOKEN_OPERATOR: type_str = "Operator"; break;
        case TOKEN_INT_CONSTANT: type_str = "IntConstant"; break;
        case TOKEN_STRING_CONSTANT: type_str = "StringConstant"; break;
        case TOKEN_OPEN_BLOCK: type_str = "OpenBlock"; break;
        case TOKEN_CLOSE_BLOCK: type_str = "CloseBlock"; break;
        default: type_str = "Error"; break;
    }

    if (token.type == TOKEN_STRING_CONSTANT) {
        fprintf(output_file, "%s(\"%s\")\n", type_str, token.value);
    } else {
        fprintf(output_file, "%s(%s)\n", type_str, token.value);
    }
}

// Lexer function to retrieve the next token
Token get_next_token() {
    Token token;
    token.type = TOKEN_ERROR;
    token.value[0] = '\0';
    token.line = current_line;
    token.col = current_col;

    // Boşluk ve yorumları atla
    while (true) {
        // Boşluklar (newline hariç)
        while (current_char != EOF &&  isspace(current_char)) {
            next_char();
        }

        // Yorum kontrolü: * ... *
        if (current_char == '*') {
            next_char();
            bool comment_closed = false;

            while (current_char != EOF) {
                if (current_char == '*') {
                    next_char();
                    comment_closed = true;
                    break;
                }
                next_char();
            }

            if (!comment_closed) {
                // Yorum kapanmamış
                snprintf(token.value, MAX_TOKEN_LEN, "Unterminated comment");
                return token;
            }

            // Yorum kapandı, başa dön boşluk atla
            continue;
        }

        break;  // Yorum ve boşluk bitti
    }

    token.line = current_line;
    token.col = current_col;

    if (current_char == EOF) {
        token.type = TOKEN_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    /* Yeni satır tokenı
    if (current_char == '\n') {
        token.type = TOKEN_END_OF_LINE;
        strcpy(token.value, "\\n");
        next_char();
        return token;
    }*/


    // Identifier veya keyword
    if (isalpha(current_char) || current_char == '_') {
        int i = 0;
        while ((isalnum(current_char) || current_char == '_') && i < MAX_TOKEN_LEN) {
            token.value[i++] = (char)current_char;
            next_char();
        }
        token.value[i] = '\0';

        if (i > MAX_VAR_LEN) {
            snprintf(token.value, MAX_TOKEN_LEN, "Identifier too long (max %d chars)", MAX_VAR_LEN);
            return token;
        }

        token.type = is_keyword(token.value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
        return token;
    }

    // Sayı (tam sayı, negatif olabilir)
    if (current_char == '-' || isdigit(current_char)) {
        int i = 0;
        if (current_char == '-') {
            token.value[i++] = (char)current_char;
            next_char();
            if (!isdigit(current_char)) {
                snprintf(token.value, MAX_TOKEN_LEN, "Invalid number format");
                return token;
            }
        }
        while (isdigit(current_char) && i < MAX_TOKEN_LEN) {
            token.value[i++] = (char)current_char;
            next_char();
        }
        token.value[i] = '\0';

        // Nokta içeren sayılar yasak
        if (current_char == '.') {
            snprintf(token.value, MAX_TOKEN_LEN, "Floats are not allowed");
            return token;
        }

        token.type = TOKEN_INT_CONSTANT;
        return token;
    }

    // String constant: "..."
    if (current_char == '"') {
        next_char();
        int i = 0;
        while (current_char != '"' && current_char != EOF && current_char != '\n' && i < MAX_STRING_LEN) {
            token.value[i++] = (char)current_char;
            next_char();
        }

        if (current_char != '"') {
            snprintf(token.value, MAX_TOKEN_LEN, "Unterminated string");
            return token;
        }

        token.value[i] = '\0';
        next_char(); // " sonrasını oku
        token.type = TOKEN_STRING_CONSTANT;
        return token;
    }

    // Operatörler :=, +=, -=
    if (current_char == ':' || current_char == '+' || current_char == '-') {
        char op[4] = {0};
        op[0] = (char)current_char;
        next_char();

        if (current_char == '=') {
            op[1] = '=';
            op[2] = '\0';
            next_char();
        } else {
            op[1] = '\0';
        }

        if (is_operator(op)) {
            strcpy(token.value, op);
            token.type = TOKEN_OPERATOR;
            return token;
        } else {
            snprintf(token.value, MAX_TOKEN_LEN, "Invalid operator");
            return token;
        }
    }

    // Blok açma ve kapama
    if (current_char == '{') {
        token.type = TOKEN_OPEN_BLOCK;
        token.value[0] = (char)current_char;
        token.value[1] = '\0';
        next_char();
        return token;
    }

    if (current_char == '}') {
        token.type = TOKEN_CLOSE_BLOCK;
        token.value[0] = (char)current_char;
        token.value[1] = '\0';
        next_char();
        return token;
    }

    // Satır sonu (noktalı virgül)
    if (current_char == ';') {
        token.type = TOKEN_END_OF_LINE;
        token.value[0] = (char)current_char;
        token.value[1] = '\0';
        next_char();
        return token;
    }

    // Tanınmayan karakter
    snprintf(token.value, MAX_TOKEN_LEN, "Unrecognized character '%c'", current_char);
    next_char();
    return token;
}

// Main function to handle file input/output and lexical analysis
int main(int argc, char *argv[]) {
    if (argc != 2) {
        // Print usage instructions if the filename is not provided
        printf("Usage: %s filename_without_extension\n", argv[0]);
        printf("Note: The .plus extension will be automatically added\n");
        return 1;
    }

    // Construct input and output filenames
    char input_filename[256];
    snprintf(input_filename, sizeof(input_filename), "%s.plus", argv[1]);
    input_file = fopen(input_filename, "r");
    if (!input_file) {
        // Error if input file cannot be opened
        printf("Error: Cannot open input file %s\n", input_filename);
        return 1;
    }

    char output_filename[256];
    snprintf(output_filename, sizeof(output_filename), "%s.lx", argv[1]);
    output_file = fopen(output_filename, "w");
    if (!output_file) {
        // Error if output file cannot be created
        printf("Error: Cannot create output file %s\n", output_filename);
        fclose(input_file);
        return 1;
    }

    // Initialize lexer by reading the first character
    next_char();

    Token token;
    do {
        // Retrieve the next token
        token = get_next_token();

        if (token.type == TOKEN_ERROR) {
            // Handle lexical errors
            printf("Lexical error at line %d, column %d: %s\n", token.line, token.col, token.value);
            fclose(input_file);
            fclose(output_file);
            remove(output_filename); // Delete output file on error
            return 1;
        }

        // Write the token to the output file
        write_token_to_file(token);
    } while (token.type != TOKEN_EOF); // Continue until end of file

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    // Print success message
    printf("Lexical analysis completed successfully. Output written to %s\n", output_filename);
    return 0;
}
