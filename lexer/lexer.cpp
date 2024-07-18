    #include <iostream>
    #include <vector>
    #include <string>
    #include <cctype>
    #include <unordered_map>
    #include "token.hpp"

    class Lexer {
    public:
        Lexer(const std::string& source)
            : source(source), position(0), line(1), column(1) {}

        std::vector<Token> tokenize() {
            std::vector<Token> tokens;
            while (position < source.length()) {
                char currentChar = source[position];

                if (isspace(currentChar)) {
                    handleWhitespace();
                } else if (isalpha(currentChar) || currentChar == '_') {
                    tokens.push_back(handleIdentifierOrKeyword());
                } else if (isdigit(currentChar)) {
                    tokens.push_back(handleNumber());
                } else if (currentChar == '\"') {
                    tokens.push_back(handleStringLiteral());
                } else {
                    tokens.push_back(handleOperatorOrDelimiter());
                }
            }
            tokens.emplace_back(TOK_EOF, "", line, column);
            return tokens;
        }

    private:
        std::string source;
        size_t position;
        int line;
        int column;

        void handleWhitespace() {
            while (isspace(source[position])) {
                if (source[position] == '\n') {
                    line++;
                    column = 0;
                }
                position++;
                column++;
            }
        }

        Token handleIdentifierOrKeyword() {
            int startColumn = column;
            std::string value;
            while (isalnum(source[position]) || source[position] == '_') {
                value += source[position];
                position++;
                column++;
            }

            static const std::unordered_map<std::string, TokenType> keywords = {
                {"let", TOK_LET}, {"fn", TOK_FN}, {"struct", TOK_STRUCT}, {"return", TOK_RETURN},
                {"for", TOK_FOR}, {"while", TOK_WHILE}, {"if", TOK_IF}, {"else", TOK_ELSE},
                {"switch", TOK_SWITCH}, {"case", TOK_CASE}, {"default", TOK_DEFAULT}, {"new", TOK_NEW}
            };

            auto it = keywords.find(value);
            if (it != keywords.end()) {
                return Token(it->second, value, line, startColumn);
            }

            static const std::unordered_map<std::string, TokenType> types = {
                {"i8", TOK_TYPE}, {"i16", TOK_TYPE}, {"i32", TOK_TYPE}, {"i64", TOK_TYPE},
                {"u8", TOK_TYPE}, {"u16", TOK_TYPE}, {"u32", TOK_TYPE}, {"u64", TOK_TYPE},
                {"f8", TOK_TYPE}, {"f16", TOK_TYPE}, {"f32", TOK_TYPE}, {"f64", TOK_TYPE},
                {"bool", TOK_TYPE}, {"string", TOK_TYPE}
            };

            it = types.find(value);
            if (it != types.end()) {
                return Token(it->second, value, line, startColumn);
            }

            return Token(TOK_IDENTIFIER, value, line, startColumn);
        }

        Token handleNumber() {
            int startColumn = column;
            std::string value;
            bool isFloat = false;
            while (isdigit(source[position]) || source[position] == '.') {
                if (source[position] == '.') {
                    if (isFloat) break; 
                    isFloat = true;
                }
                value += source[position];
                position++;
                column++;
            }
            return Token(isFloat ? TOK_FLOAT_LITERAL : TOK_INTEGER_LITERAL, value, line, startColumn);
        }

        Token handleStringLiteral() {
            int startColumn = column;
            std::string value;
            position++; 
            column++;
            while (position < source.length() && source[position] != '\"') {
                value += source[position];
                position++;
                column++;
            }
            position++; 
            column++;
            return Token(TOK_STRING_LITERAL, value, line, startColumn);
        }

        Token handleOperatorOrDelimiter() {
            int startColumn = column;
            char currentChar = source[position];
            position++;
            column++;
            switch (currentChar) {
                case '=':
                    if (source[position] == '=') {
                        position++;
                        column++;
                        return Token(TOK_EQUAL, "==", line, startColumn);
                    }
                    return Token(TOK_ASSIGN, "=", line, startColumn);
                case ';': return Token(TOK_SEMICOLON, ";", line, startColumn);
                case ',': return Token(TOK_COMMA, ",", line, startColumn);
                case '(': return Token(TOK_LPAREN, "(", line, startColumn);
                case ')': return Token(TOK_RPAREN, ")", line, startColumn);
                case '{': return Token(TOK_LBRACE, "{", line, startColumn);
                case '}': return Token(TOK_RBRACE, "}", line, startColumn);
                case '[': return Token(TOK_LBRACKET, "[", line, startColumn);
                case ']': return Token(TOK_RBRACKET, "]", line, startColumn);
                case '+': return Token(TOK_PLUS, "+", line, startColumn);
                case '-': return Token(TOK_MINUS, "-", line, startColumn);
                case '*': return Token(TOK_STAR, "*", line, startColumn);
                case '/': return Token(TOK_SLASH, "/", line, startColumn);
                case '!':
                    if (source[position] == '=') {
                        position++;
                        column++;
                        return Token(TOK_NOT_EQUAL, "!=", line, startColumn);
                    }
                    break;
                case '<':
                    if (source[position] == '=') {
                        position++;
                        column++;
                        return Token(TOK_LE, "<=", line, startColumn);
                    }
                    return Token(TOK_LT, "<", line, startColumn);
                case '>':
                    if (source[position] == '=') {
                        position++;
                        column++;
                        return Token(TOK_GE, ">=", line, startColumn);
                    }
                    return Token(TOK_GT, ">", line, startColumn);
                case ':': return Token(TOK_COLON, ":", line, startColumn);
                case '.': return Token(TOK_DOT, ".", line, startColumn);
                default: break;
            }
            return Token(TOK_ERROR, std::string(1, currentChar), line, startColumn);
        }
    };
