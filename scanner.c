#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

Scanner scanner;

void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}
//> Scanner Helpers
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

static bool isAtEnd() {
  return *scanner.current == '\0';
}

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static char peek() {
  return *scanner.current;
}

static char peekNext() {
  if (isAtEnd()) return '\0';
  return scanner.current[1];
}

static bool match(char expected) {
  if (isAtEnd()) return false;
  if (*scanner.current != expected) return false;
  scanner.current++; // TODO would implement something similar for meaningful new lines
  return true;
}
//^ Scanner Helpers

//> Token Helpers
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

static void skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
      case ' ': 
      case '\r':
      case '\t':
        advance();
        break;
      case '\n': // newline
        scanner.line++;
        advance();
        break;
      case '/':
        if (peekNext() == '/') { // comment goes until the end of the line.
          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          return;
        }
        break;
      default: //encountering anything not defined as whitespace
        return;
    }
  }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }
  return L_IDENTIFIER; // TODO create constant, mutable path
}

static TokenType identifierType() { // tests for keywords
  switch (scanner.start[0]) {
    case 'a': //branch our to "and", "as"
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'n' : return checkKeyword(2, 1, "d", K_AND); 
          case 's' : return checkKeyword(2, 0, "", K_AS);
        }
      }
      break;
    case 'b': return checkKeyword(1, 4, "uild", K_BUILD);
    case 'd': 
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'e' : return checkKeyword(2, 4, "fine", K_DEFINE);
          case 'o' : return checkKeyword(2, 0, "", K_DO);
        }
      }
      break;
    case 'e': // branch out to "else", "end"
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a' : return checkKeyword(2, 2, "ch", K_EACH);
          case 'l' : return checkKeyword(2, 2, "se", K_ELSE);
        }
      }
      break;
    case 'f': return checkKeyword(1, 4, "alse", K_FALSE);
    case 'i': // branch out to 'if', 'is'
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'f': return checkKeyword(2, 0, "", K_IF);
          case 's': return checkKeyword(2, 0, "", K_IS);
        }
      }
      break;
    case 'l': return checkKeyword(1, 2, "et", K_LET);
    case 'n': return checkKeyword(1, 3, "ull", K_NULL); // because JSON uses 'null'
    case 'o': return checkKeyword(1, 1, "r", K_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT); // TODO replace with native function
    case 'r': return checkKeyword(1, 5, "eturn", K_RETURN);
    case 's': return checkKeyword(1, 3, "elf", K_SELF);
    case 't': // branch out to "to", "true"
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'o': return checkKeyword(2, 0, "", K_TO);
          case 'r': return checkKeyword(2, 2, "ue", K_TRUE);
        }
      }
      break;
    case 'u': // if 'n' then branch out to 'unless', 'until'
      if (scanner.current - scanner.start > 1 && scanner.start[1] == 'n') {
        switch (scanner.start[2]) {
          case 'l': return checkKeyword(3, 3, "ess", K_UNLESS);
          case 't': return checkKeyword(3, 2, "il", K_UNTIL);
        }
      }
      break;
    case 'w': // if 'h' then branch out to 'when', 'while'
      if (scanner.current - scanner.start > 1 && scanner.start[1] == 'h') {
        switch (scanner.start[2]) {
          case 'e': return checkKeyword(3, 1, "n", K_WHEN);
          case 'i': return checkKeyword(3, 2, "le", K_WHILE);
        }
      }
      break;
    case 'q': return checkKeyword(1, 3, "uit", K_QUIT);
    case '#': return L_VARIABLE;
  }
  return L_IDENTIFIER;
}

static Token identifier() {
  if (peek() == '#') {
    advance();
  }
  while (isAlpha(peek()) || isDigit(peek())) advance();
  return makeToken(identifierType());
}

static Token number() {
  while (isDigit(peek())) advance();

  // Look for a fractional part.
  if (peek() == '.' && isDigit(peekNext())) {
    advance(); // Consume the "."

    while (isDigit(peek())) advance();
  }
  return makeToken(L_NUMBER);
}

static Token string() {
  while (peek() != '"' && !isAtEnd()) { // TODO use similar logic if implementing meaningufl new lines
    if (peek() == '\n') scanner.line++;
    advance();
  }
  if (isAtEnd()) return errorToken("Unterminated string.");

  // The closing quote.
  advance();
  return makeToken(L_STRING);
}
//^ Token Helpers

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;
  if (isAtEnd()) return makeToken(END_OF_FILE);

  // check for constants
  char rune = advance();
  if (isAlpha(rune)) return identifier(); 
  if (isDigit(rune)) return number(); 

  switch (rune) {
    // single character
    case '#': return identifier();
    case '(': return makeToken(S_LEFT_PARENTHESES); // TODO would be new line aware
    case ')': return makeToken(S_RIGHT_PARENTHESES);
    case '{': return makeToken(S_LEFT_CURLY); // TODO would be new line aware
    case '}': return makeToken(S_RIGHT_CURLY);
    case '?': return makeToken(S_QUESTION); // TODO would be new line aware
    case ';': return makeToken(S_SEMICOLON);
    case '=': return makeToken(S_EQUAL); //TOKEN_EQUAL_EQUAL
    case '-': return makeToken(S_MINUS);
    // two characters
    case '.': 
      return makeToken(match('=') ? D_DOT_EQUAL : S_DOT);
    case '+': 
      return makeToken(match('=') ? D_PLUS_EQUAL : S_PLUS);
    case '/': 
      return makeToken(match('=') ? D_SLASH_EQUAL : S_SLASH);
    case '*': 
      return makeToken(match('=') ? D_STAR_EQUAL : S_STAR);
    case '%': 
      return makeToken(match('=') ? D_MODULO_EQUAL : S_MODULO);
    case ',': 
      return makeToken(match(',') ? D_COMMA : S_COMMA);
    case ':': 
      return makeToken(match('=') ? D_COLON_EQUAL: S_COLON); // TODO would be new line aware
    case '!':
      return makeToken(match('~') ? D_BANG_TILDE : S_BANG);
    case '<':
      return makeToken(match('=') ? D_LESS_EQUAL : S_LESS);
    case '>':
      return makeToken(match('=') ? D_GREATER_EQUAL : S_GREATER);
    case '"': return string();
  }
  return errorToken("Unexpected character.");
}
