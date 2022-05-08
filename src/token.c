#include "token.h"


const char* tokenTypeName(TokenType type) {
    switch (type) {

        // Utility
        case TOKEN_END:                return "END";
        case TOKEN_ERROR:              return "ERROR";

        // Symbols
        case TOKEN_COLON:              return "COLON";
        case TOKEN_COMMA:              return "COMMA";
        case TOKEN_DOT:                return "DOT";
        case TOKEN_EQUAL:              return "EQUAL";
        case TOKEN_EQUAL_EQUAL:        return "EQUAL_EQUAL";
        case TOKEN_EXCLAMATION:        return "EXCLAMATION";
        case TOKEN_EXCLAMATION_EQUAL:  return "EXCLAMATION_EQUAL";
        case TOKEN_GREATER:            return "GREATER";
        case TOKEN_GREATER_EQUAL:      return "GREATER_EQUAL";
        case TOKEN_LBRACE:             return "LBRACE";
        case TOKEN_LBRACKET:           return "LBRACKET";
        case TOKEN_LESS:               return "LESS";
        case TOKEN_LESS_EQUAL:         return "LESS_EQUAL";
        case TOKEN_LPAREN:             return "LPAREN";
        case TOKEN_MINUS:              return "MINUS";
        case TOKEN_MINUS_EQUAL:        return "MINUS_EQUAL";
        case TOKEN_PERCENT:            return "PERCENT";
        case TOKEN_PERCENT_EQUAL:      return "PERCENT_EQUAL";
        case TOKEN_PLUS:               return "PLUS";
        case TOKEN_PLUS_EQUAL:         return "PLUS_EQUAL";
        case TOKEN_RBRACE:             return "RBRACE";
        case TOKEN_RBRACKET:           return "RBRACKET";
        case TOKEN_RPAREN:             return "RPAREN";
        case TOKEN_SEMICOLON:          return "SEMICOLON";
        case TOKEN_SLASH:              return "SLASH";
        case TOKEN_SLASH_EQUAL:        return "SLASH_EQUAL";
        case TOKEN_STAR:               return "STAR";
        case TOKEN_STAR_EQUAL:         return "STAR_EQUAL";

        // Keyords
        case TOKEN_AND:                return "AND";
        case TOKEN_ASSERT:             return "ASSERT";
        case TOKEN_BOOL:               return "BOOL";
        case TOKEN_BREAK:              return "BREAK";
        case TOKEN_CONTINUE:           return "CONTINUE";
        case TOKEN_DO:                 return "DO";
        case TOKEN_ELSE:               return "ELSE";
        case TOKEN_ENUM:               return "ENUM";
        case TOKEN_FALSE:              return "FALSE";
        case TOKEN_FLOAT:              return "FLOAT";
        case TOKEN_FOR:                return "FOR";
        case TOKEN_FUNCTION:           return "FUNCTION";
        case TOKEN_IF:                 return "IF";
        case TOKEN_IN:                 return "IN";
        case TOKEN_INCLUDE:            return "INCLUDE";
        case TOKEN_INT:                return "INT";
        case TOKEN_MUTABLE:            return "MUTABLE";
        case TOKEN_OR:                 return "OR";
        case TOKEN_PREDICATE:          return "PREDICATE";
        case TOKEN_PRINT:              return "PRINT";
        case TOKEN_RETURN:             return "RETURN";
        case TOKEN_STRING:             return "STRING";
        case TOKEN_STRUCTURE:          return "STRUCTURE";
        case TOKEN_TRUE:               return "TRUE";
        case TOKEN_VAR:                return "VAR";
        case TOKEN_VOID:               return "VOID";
        case TOKEN_WHILE:              return "WHILE";

        // Complex
        case TOKEN_INTEGER_VALUE:      return "INTEGER_VALUE";
        case TOKEN_FLOAT_VALUE:        return "FLOAT_VALUE";
        case TOKEN_STRING_VALUE:       return "STRING_VALUE";
        case TOKEN_IDENTIFIER:         return "IDENTIFIER";

    }

    return "INVALID_TOKEN";
}

