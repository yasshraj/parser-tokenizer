#include <iostream>
#include <cctype>
#include <string>
#include <fstream>

enum Token
{
    IDENT = 11,
    INT_LIT = 10,
    FLOAT_LIT = 12,
    ASSIGN_OP = 20,
    ADD_OP = 21,
    SUB_OP = 22,
    MULT_OP = 23,
    DIV_OP = 24,
    LEFT_PAREN = 25,
    RIGHT_PAREN = 26,
    COMMA = 27,
    LEFT_BRACE = 28,
    RIGHT_BRACE = 29,
    SEMICOLON = 30,
    EOF_TOKEN = -1,
    LETTER = 0,
    DIGIT = 1,
    UNKNOWN = 99,
    PERIOD = 2
};

int charClass;
char nextChar;
Token nextToken;
std::string lexeme;
std::ifstream inFile;

void addChar();
void getChar();
void getNonBlank();
Token lex();
Token lookup(char ch);

// #define LETTER 0
// #define DIGIT 1
// #define UNKNOWN 99
// #define PERIOD 2  

void addChar()
{
    if (lexeme.length() < 98)
    {
        lexeme += nextChar;
    }
    else
    {
        std::cerr << "Error: Lexeme is too long" << std::endl;
        // Handle the error, e.g., truncate the lexeme or report an error
        lexeme.resize(98);
    }
}

void getChar()
{
    if (inFile.get(nextChar))
    {
        if (isalpha(nextChar))
        {
            charClass = LETTER;
        }
        else if (isdigit(nextChar))
        {
            charClass = DIGIT;
        }
        else if (nextChar == '.')
        {
            charClass = PERIOD;
        }
        else if (isspace(nextChar))
        {
            charClass = UNKNOWN;
        }
        else
        {
            charClass = UNKNOWN;
        }
    }
    else
    {
        nextChar = EOF; 
        charClass = EOF_TOKEN;
    }
}

void getNonBlank()
{
    while (isspace(nextChar))
    {
        getChar();
    }
}

Token lookup(char ch)
{
    switch (ch)
    {
    case '(':
        addChar();
        return LEFT_PAREN;
    case ')':
        addChar();
        return RIGHT_PAREN;
    case '+':
        addChar();
        return ADD_OP;
    case '-':
        addChar();
        return SUB_OP;
    case '*':
        addChar();
        return MULT_OP;
    case '/':
        addChar();
        return DIV_OP;
    case '=':
        addChar();
        return ASSIGN_OP;
    case ',':
        addChar();
        return COMMA;
    case '{':
        addChar();
        return LEFT_BRACE;
    case '}':
        addChar();
        return RIGHT_BRACE;
    case ';':
        addChar();
        return SEMICOLON;

    case '\n':
        addChar();
        return EOF_TOKEN;

    default:
        return EOF_TOKEN;
    }
}

Token handleFloat()
{
    addChar();
    getChar();
    while (charClass == DIGIT)
    {
        addChar();
        getChar();
    }
    nextToken = FLOAT_LIT;
    return nextToken;
}

Token lex()
{
    lexeme.clear();
    getNonBlank();
    switch (charClass)
    {
    case LETTER:
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT)
        {
            addChar();
            getChar();
        }
        nextToken = IDENT;
        break;
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT)
        {
            addChar();
            getChar();
        }
        if (charClass == PERIOD)
        {
            handleFloat();
        }
        else
        {
            nextToken = INT_LIT;
        }
        break;
    case PERIOD:
        handleFloat();
        break;
    case UNKNOWN:
        nextToken = lookup(nextChar);
        getChar();
        break;
    case EOF_TOKEN:
        nextToken = EOF_TOKEN;
        lexeme = "EOF_TOKEN";
        exit(0);
    }
    std::cout << "Next token is: " << nextToken
              << ", Next lexeme is " << lexeme << std::endl;
    return nextToken;
}

int main()
{
    inFile.open("front.in");
    if (!inFile)
    {
        std::cerr << "Cannot open file" << std::endl;
        return 1;
    }
    getChar();
    while (nextToken != EOF_TOKEN)
    {
        lex();
    }
    inFile.close();
    return 0;
}