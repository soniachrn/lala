#include "token.h"


const char* tokenTypeName(TokenType type) {
    switch (type) {

        // Utility
        case TOKEN_END:                return "TOKEN_END";
        case TOKEN_ERROR:              return "TOKEN_ERROR";

        // Symbols
        case TOKEN_COLON:              return "TOKEN_COLON";
        case TOKEN_COMMA:              return "TOKEN_COMMA";
        case TOKEN_DOT:                return "TOKEN_DOT";
        case TOKEN_EQUAL:              return "TOKEN_EQUAL";
        case TOKEN_EQUAL_EQUAL:        return "TOKEN_EQUAL_EQUAL";
        case TOKEN_EXCLAMATION:        return "TOKEN_EXCLAMATION";
        case TOKEN_EXCLAMATION_EQUAL:  return "TOKEN_EXCLAMATION_EQUAL";
        case TOKEN_GREATER:            return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL:      return "TOKEN_GREATER_EQUAL";
        case TOKEN_LBRACE:             return "TOKEN_LBRACE";
        case TOKEN_LBRACKET:           return "TOKEN_LBRACKET";
        case TOKEN_LESS:               return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL:         return "TOKEN_LESS_EQUAL";
        case TOKEN_LPAREN:             return "TOKEN_LPAREN";
        case TOKEN_MINUS:              return "TOKEN_MINUS";
        case TOKEN_MINUS_EQUAL:        return "TOKEN_MINUS_EQUAL";
        case TOKEN_PERCENT:            return "TOKEN_PERCENT";
        case TOKEN_PERCENT_EQUAL:      return "TOKEN_PERCENT_EQUAL";
        case TOKEN_PLUS:               return "TOKEN_PLUS";
        case TOKEN_PLUS_EQUAL:         return "TOKEN_PLUS_EQUAL";
        case TOKEN_RBRACE:             return "TOKEN_RBRACE";
        case TOKEN_RBRACKET:           return "TOKEN_RBRACKET";
        case TOKEN_RPAREN:             return "TOKEN_RPAREN";
        case TOKEN_SEMICOLON:          return "TOKEN_SEMICOLON";
        case TOKEN_SLASH:              return "TOKEN_SLASH";
        case TOKEN_SLASH_EQUAL:        return "TOKEN_SLASH_EQUAL";
        case TOKEN_STAR:               return "TOKEN_STAR";
        case TOKEN_STAR_EQUAL:         return "TOKEN_STAR_EQUAL";

        // Keyords
        case TOKEN_AND:                return "TOKEN_AND";
        case TOKEN_ASSERT:             return "TOKEN_ASSERT";
        case TOKEN_BOOL:               return "TOKEN_BOOL";
        case TOKEN_BREAK:              return "TOKEN_BREAK";
        case TOKEN_CONTINUE:           return "TOKEN_CONTINUE";
        case TOKEN_DO:                 return "TOKEN_DO";
        case TOKEN_ELSE:               return "TOKEN_ELSE";
        case TOKEN_ENUM:               return "TOKEN_ENUM";
        case TOKEN_FALSE:              return "TOKEN_FALSE";
        case TOKEN_FLOAT:              return "TOKEN_FLOAT";
        case TOKEN_FOR:                return "TOKEN_FOR";
        case TOKEN_FUNCTION:           return "TOKEN_FUNCTION";
        case TOKEN_IF:                 return "TOKEN_IF";
        case TOKEN_IN:                 return "TOKEN_IN";
        case TOKEN_INT:                return "TOKEN_INT";
        case TOKEN_MUTABLE:            return "TOKEN_MUTABLE";
        case TOKEN_OR:                 return "TOKEN_OR";
        case TOKEN_PREDICATE:          return "TOKEN_PREDICATE";
        case TOKEN_PRINT:              return "TOKEN_PRINT";
        case TOKEN_RETURN:             return "TOKEN_RETURN";
        case TOKEN_STRING:             return "TOKEN_STRING";
        case TOKEN_STRUCTURE:          return "TOKEN_STRUCTURE";
        case TOKEN_TRUE:               return "TOKEN_TRUE";
        case TOKEN_VAR:                return "TOKEN_VAR";
        case TOKEN_WHILE:              return "TOKEN_WHILE";

        // Complex
        case TOKEN_INTEGER_VALUE:      return "TOKEN_INTEGER_VALUE";
        case TOKEN_FLOAT_VALUE:        return "TOKEN_FLOAT_VALUE";
        case TOKEN_STRING_VALUE:       return "TOKEN_STRING_VALUE";
        case TOKEN_IDENTIFIER:         return "TOKEN_IDENTIFIER";

    }

    return "INVALID_TOKEN";
}

