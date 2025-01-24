#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <stdexcept>

using namespace std;

class ParseTree {
public:
    string value;
    vector<ParseTree*> children;

    ParseTree(string val) : value(val) {}

    void addChild(ParseTree* child) {
        children.push_back(child);
    }

    void printTree(int depth = 0) {
        for (int i = 0; i < depth; ++i) cout << "  ";
        cout << value << endl;
        for (auto* child : children) {
            child->printTree(depth + 1);
        }
    }
};

class Parser {
    vector<string> tokens;
    int currentToken;

    string getToken() {
        return currentToken < tokens.size() ? tokens[currentToken] : "";
    }

    void consumeToken() {
        if (currentToken < tokens.size()) currentToken++;
    }

    ParseTree* match(string expected) {
        if (getToken() == expected) {
            ParseTree* node = new ParseTree(expected);
            consumeToken();
            return node;
        }
        throw runtime_error("Syntax Error: Expected " + expected + " but found " + getToken());
    }

    ParseTree* parseType() {
        string token = getToken();
        if (token == "int" || token == "float") {
            consumeToken();
            return new ParseTree("Type Specifier: " + token);
        }
        throw runtime_error("Syntax Error: Expected type, found " + token);
    }

    ParseTree* parseIdentifier(const string& prefix = "") {
    string token = getToken();
    if (!token.empty() && (isalpha(token[0]) || token[0] == '_')) {
        bool valid = true;
        for (char c : token) {
            if (!isalnum(c) && c != '_') {
                valid = false;
                break;
            }
        }
        if (valid) {
            consumeToken();
            // Add the prefix to the token before returning
            return new ParseTree(prefix + token);
        }
    }
    throw runtime_error("Syntax Error: Expected identifier, found " + token);
}

    ParseTree* parseLiteral() {
        string token = getToken();
        bool isLiteral = true;
        bool hasDot = false;
        for (char c : token) {
            if (c == '.') {
                if (hasDot) { isLiteral = false; break; }
                hasDot = true;
            }
            else if (!isdigit(c)) {
                isLiteral = false;
                break;
            }
        }
        if (isLiteral) {
            consumeToken();
            return new ParseTree("Literal: " + token);
        }
        throw runtime_error("Syntax Error: Expected literal, found " + token);
    }

    string parseExpressionStr() {
        string expr = parseTermStr();
        while (getToken() == "+" || getToken() == "-" || getToken() == "*" || getToken() == "/") {
            string op = getToken();
            consumeToken();
            string right = parseExpressionStr();
            expr += " " + op + " " + right;
            break; 
        }
        return expr;
    }
    

    ParseTree* parseExpression() {
        string exprStr = parseExpressionStr();
        return new ParseTree("Expression: " + exprStr + "\n           Variable: " + exprStr);
    }

    string parseTermStr() {
        if (getToken() == "(") {
            consumeToken(); 
            if (getToken() == "int" || getToken() == "float") {
                string type = getToken();
                consumeToken();
                match(")");
                string identifier = getToken();
                consumeToken();
                return "(" + type + ") " + identifier;
            } else {
                string subExpr = parseExpressionStr();
                match(")");
                return "(" + subExpr + ")";
            }
        }

        if (isalpha(getToken()[0]) || getToken()[0] == '_') {
            string identifier = getToken();
            consumeToken();
            return identifier;
        }

        return parseLiteral()->value.substr(9); 
    }

    ParseTree* parseArithmeticExpression() {
        string exprStr = parseExpressionStr();
        return new ParseTree("Arithmetic Expression: " + exprStr);
    }

    ParseTree* parseReturnStatement() {
        ParseTree* node = new ParseTree("Return Statement:");
        node->addChild(match("return"));
        ParseTree* expression = parseExpression();
        node->addChild(expression);
        return node;
    }

    ParseTree* parseStatement() {        
        if (getToken() == "return") {
            ParseTree* node = parseReturnStatement();
            match(";");
            return node;
        } else {
            string variable = getToken();
            ParseTree* identifierNode = parseIdentifier("Variable: ");
            if (getToken() != "=") {
                throw runtime_error("Syntax Error: Expected '=', found " + getToken());
            }
            consumeToken(); 

            string exprStr = parseExpressionStr();
            match(";");

            string assignment = "Assignment Statement: " + variable + " = " + exprStr;
            ParseTree* node = new ParseTree(assignment);
            node->addChild(new ParseTree("Left Variable: " + variable));

            size_t pos = exprStr.find('(');
            if (pos != string::npos) {
                size_t endPos = exprStr.find(')', pos);
                if (endPos != string::npos) {
                    string type = exprStr.substr(pos + 1, endPos - pos - 1);
                    string identifier = exprStr.substr(endPos + 2); // Skip ") "
                    ParseTree* typeCastNode = parseTypeCastExpression(type, identifier);
                    node->addChild(typeCastNode);
                }
            } else {
                ParseTree* arithmeticExprNode = parseArithmeticExpression(exprStr);
                node->addChild(arithmeticExprNode);
            }
            return node;
        }
    }
    
    ParseTree* parseTypeCastExpression(const string& type, const string& identifier) {
        string typeCastStr = "(" + type + ") " + identifier;
        ParseTree* typeCastNode = new ParseTree("Type Cast Expression: " + typeCastStr);
        typeCastNode->addChild(new ParseTree("Type: " + type));
        typeCastNode->addChild(new ParseTree("Identifier: " + identifier));
        return typeCastNode;
    }

    ParseTree* parseArithmeticExpression(const string& exprStr) {
        ParseTree* arithmeticExprNode = new ParseTree("Arithmetic Expression: " + exprStr);

        size_t opPos = exprStr.find_first_of("+-*/");
        if (opPos != string::npos) {
            string left = exprStr.substr(0, opPos - 1);
            string op = exprStr.substr(opPos, 1);
            string right = exprStr.substr(opPos + 2);

            arithmeticExprNode->addChild(new ParseTree("Variable: " + left));
            arithmeticExprNode->addChild(new ParseTree("Operator: " + op));
            arithmeticExprNode->addChild(new ParseTree("Literal: " + right));
        }
        return arithmeticExprNode;
    }

    ParseTree* parseStatementList() {
        ParseTree* node = new ParseTree("Compound Statement:");
        while (getToken() != "}") {
            node->addChild(parseStatement());
        }
        return node;
    }

    ParseTree* parseFunction() {
        string type = getToken();
        ParseTree* typeNode = parseType();

        string functionName = getToken();
        ParseTree* identifierNode = parseIdentifier("Function Name: ");

        match("(");
        ParseTree* parameters = parseParameterList();
        match(")");

        ParseTree* compoundStmt = parseCompoundStatement();

        string functionSignature = "Function Definition: " + typeNode->value.substr(6) + " " + 
                                   identifierNode->value.substr(11) + "(";

        vector<string> paramStrs;
        for (auto* param : parameters->children) {
            paramStrs.push_back(param->value.substr(22)); 
        }
        for (size_t i = 0; i < paramStrs.size(); ++i) {
            functionSignature += paramStrs[i];
            if (i < paramStrs.size() - 1) {
                functionSignature += ", ";
            }
        }
        functionSignature += ")";

        ParseTree* node = new ParseTree(functionSignature);
        node->addChild(typeNode);
        node->addChild(identifierNode);
        node->addChild(parameters);
        node->addChild(compoundStmt);
        return node;
    }

    ParseTree* parseParameterList() {
        ParseTree* node = new ParseTree("Parameters:");
        if (getToken() != ")") { 
            node->addChild(parseParameter());
            while (getToken() == ",") {
                consumeToken();
                node->addChild(parseParameter());
            }
        }
        return node;
    }

    ParseTree* parseParameter() {
        string type = getToken();
        ParseTree* typeNode = parseType();
        string identifier = getToken();
        ParseTree* identifierNode = parseIdentifier("Variable: ");

        string declaration = "Parameter Declaration: " + type + " " + identifier;
        ParseTree* node = new ParseTree(declaration);
        node->addChild(typeNode);
        node->addChild(identifierNode);
        return node;
    }

    ParseTree* parseCompoundStatement() {
        ParseTree* node = new ParseTree("Compound Statement:");
        node->addChild(match("{"));
        node->addChild(parseStatementList());
        node->addChild(match("}"));
        return node;
    }

public:
    Parser(vector<string> tokenStream) : tokens(tokenStream), currentToken(0) {}

    ParseTree* parse() {
        return parseFunction();
    }
};

vector<string> tokenize(const string& code) {
    istringstream iss(code);
    vector<string> tokens;
    string token;
    while (iss >> token) {
        string current;
        for (char c : token) {
            if (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ';' || c == '=') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                string symbol(1, c);
                tokens.push_back(symbol);
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) {
            tokens.push_back(current);
        }
    }
    return tokens;
}

int main() {
    string code = "int foo ( float x , int y ) { y = x + 10 ; x = y / 2.0 ; y = ( int ) x ; return x ; }";
    vector<string> tokens = tokenize(code);
    Parser parser(tokens);

    try {
        ParseTree* parseTree = parser.parse();
        parseTree->printTree();
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}