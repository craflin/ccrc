
#include <nstd/Pool.h>
#include <nstd/Debug.h>
#include <nstd/HashSet.h>

#include "Parser.h"
#include "ParserGenerated.h"

using namespace Parser;

namespace Parser
{
  struct State
  {
    const char_t* p;
    uint_t currentLine;
    String currentFile;
    Token token;
    String lastComment;
  };

  static const char_t* p;
  static uint_t currentLine;
  static String currentFile;
  Token token;
  static String lastComment; // todo: remove static?
  static String error;
  static TranslationUnit* root;
  static Pool<State> states;
  static HashSet<String> keywords;
};


bool_t Parser::parse(const String& data)
{
  p = data;
  currentLine = 1;

  static String keywords[] = {
    "private",
    "protected",
    "public",
    "asm",
    "false",
    "true",
    "class",
    "struct",
    "union",
    "const",
    "volatile",
    "friend",
    "typedef",
    "delete",
    "enum",
    "typename",
    "template",
    "throw",
    "inline",
    "virtual",
    "explicit",
    "try",
    "catch",
    "while",
    "do",
    "for",
    "break",
    "continue",
    "return",
    "goto",
    "case",
    "default",
    "extern",
    "namespace",
    "new",
    "operator",
    "dynamic_cast",
    "static_cast",
    "reinterpret_cast",
    "const_cast",
    "typeid",
    "this",
    "if",
    "else",
    "switch",
    "char",
    "wchar_t",
    "bool",
    "short",
    "int",
    "long",
    "signed",
    "unsigned",
    "float",
    "double",
    "void",
    "auto",
    "register",
    "static",
    "mutable",
    "export",
    "sizeof",
    "using",

    "__cdecl", // MSC
    "__declspec", // MSC

  };

  for(size_t i = 0; i < sizeof(keywords) / sizeof(*keywords); ++i)
    Parser::keywords.append(keywords[i]);

  try
  {
    readToken();
    root = parseTranslationUnit();
    int k = 32;
  }
  catch(const String& error)
  {
    Parser::error = error;
    return false;
  }
  return true;
}

void_t Parser::getError(String& file, uint_t& line, String& message)
{
  file = token.file;
  line = token.line;
  message = error;
}

void_t Parser::readToken()
{
  lastComment.clear();
  for(;;)
  {
    readTokenRaw();
    switch(token.type)
    {
    case commentType:
      lastComment = token.value;
      break;
    case preprocessorType:
      lastComment.clear();
      if(token.value.startsWith("#line "))
      {
        const char_t* p = token.value;
        p += 5;
        currentLine = readPreprocessorInteger(p) - 1;
        currentFile = readPreprocessorString(p);
      }
      break;
    default:
      if(token.value == "__cdecl")
      {
        int k = 42;
      }
       if(token.value == "(")
      {
        int k = 42;
      }
      return;
    }
  }
}

void_t Parser::readTokenRaw()
{
  for(;;)
    switch(*p)
    {
    case '\r':
      ++p;
      ++currentLine;
      if(*p == '\n')
        ++p;
      break;
    case '\n':
      ++p;
      ++currentLine;
      break;
    default:
      if(String::isSpace(*p))
        ++p;
      else
        goto endloop;
    }
endloop:;

  token.line = currentLine;
  token.file = currentFile;
  switch(*p)
  {
  case '+':
  case '-':
    if(String::isDigit(p[1]))
      return readNumberToken();
    token.type = operatorType;
    if(*p == '+')
    {
      if(p[1] == '+')
        return p += 2, token.value = "++", (void_t)0;
      if(p[1] == '=')
        return p += 2, token.value = "+=", (void_t)0;
    }
    else if(*p == '-')
    {
      if(p[1] == '-')
        return p += 2, token.value = "--", (void_t)0;
      if(p[1] == '=')
        return p += 2, token.value = "-=", (void_t)0;
      if(p[1] == '>')
      {
        if(p[2] == '*')
          return p += 3, token.value = "->*", (void_t)0;
        return p += 2, token.value = "->", (void_t)0;
      }
    }
    return token.value = String(p++, 1), (void_t)0;
  case '/':
    if(p[1] == '/' || p[1] == '*')
      return readCommentToken();
    if(p[1] == '=')
      return p += 2, token.type = operatorType, token.value = "/=", (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case ':':
    if(p[1] == ':')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '=':
  case '!':
  case '^':
  case '*':
  case '%':
    if(p[1] == '=')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '>':
    if(p[1] == '>' && p[2] == '=')
      return token.type = operatorType, token.value = String(p, 3), p += 3, (void_t)0;
    if(p[1] == '=' || p[1] == '>')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '<':
    if(p[1] == '<' && p[2] == '=')
      return token.type = operatorType, token.value = String(p, 3), p += 3, (void_t)0;
    if(p[1] == '=' || p[1] == '<')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '&':
    if(p[1] == '&' || p[1] == '=')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '|':
    if(p[1] == '|' || p[1] == '=')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '.':
    if(p[1] == '*')
      return token.type = operatorType, token.value = String(p, 2), p += 2, (void_t)0;
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  case '#':
    {
      const char_t* start = p;
      p = String::findOneOf(p, "\r\n");
      if(!p)
        p = start + String::length(start);
      return token.value = String(start, p - start), token.type = preprocessorType, (void_t)0;
    }
  case '\'':
    {
      token.type = characterType;
      const char_t* start = p++;
      for(;;)
        switch(*p)
        {
        case '\'':
          ++p;
          goto endloop3;
        case '\r':
        case '\n':
          throw String("New line in constant");
        case '\\':
          if(p[1])
          {
            p += 2;
            break;
          }
          // no break
        case '\0':
          p = start;
          throw String("Unmatched '");
        default:
          ++p;
        }
    endloop3:;
      return token.value = String(start, p - start), (void_t)0;
    }
  case '"':
    {
      token.type = stringType;
      const char_t* start = p++;
      for(;;)
        switch(*p)
        {
        case '\"':
          ++p;
          goto endloop4;
        case '\r':
        case '\n':
          throw String("New line in constant");
        case '\\':
          if(p[1])
          {
            p += 2;
            break;
          }
          // no break
        case '\0':
          p = start;
          throw String("Unmatched \"");
        default:
          ++p;
        }
    endloop4:;
      return token.value = String(start, p - start), (void_t)0;
    }
    break;
  case '\0':
    return token.type = eofType, token.value.clear(), (void_t)0;
  default:
    if(String::isDigit(*p))
      return readNumberToken();
    if(String::isAlpha(*p) || *p == '_')
      return readIdentifierOrKeywordToken();
    return token.type = operatorType, token.value = String(p++, 1), (void_t)0;
  }
}

void_t Parser::readNumberToken()
{
  ASSERT(*p == '+' || *p == '-' || String::isDigit(*p));
  const char_t* start = p;
  if(*p == '+' || *p == '-')
  {
    ++p;
    if(!String::isDigit(*p))
      throw String("Expected digit");
  }
  ++p;
  while(String::isDigit(*p))
    ++p;
  if(*p == '.')
  {
    ++p;
    while(String::isDigit(*p))
      ++p;
    if(String::toLowerCase(*p) == 'e')
    {
      ++p;
      if(*p == '+' || *p == '-')
        ++p;
      if(!String::isDigit(*p))
        throw String("Expected digit");
      ++p;
      while(String::isDigit(*p))
        ++p;
    }
    token.value = String(start, p - start);
    token.type = floatingType;
    return;
  }
  token.value = String(start, p - start);
  token.type = integerType;
  return;
}

void_t Parser::readIdentifierOrKeywordToken()
{
  ASSERT(String::isAlpha(*p) || *p == '_');
  const char_t* start = p;
  ++p;
  while(String::isAlpha(*p) || *p == '_' || String::isDigit(*p))
    ++p;
  token.value = String(start, p - start);
  token.type = keywords.contains(token.value) ? keywordType : identifierType;
  return;
}

void_t Parser::readCommentToken()
{
  ASSERT(*p == '/');
  if(p[1] == '/')
  {
    const char_t* start = p;
    p = String::findOneOf(p, "\r\n");
    if(!p)
      p = start + String::length(start);
    token.value = String(p, p - start);
    token.type = commentType;
    return;
  }
  else
  {
    ASSERT(p[1] == '*');
    const char_t* start = p;
    p += 2;
    for(;;)
    {
      p = String::findOneOf(p, "\r\n*");
      if(!p)
      {
        p = start + String::length(start);
        break;
      }
      switch(*p)
      {
      case '\r':
        ++currentLine;
        ++p;
        if(*p == '\n')
          ++p;
        break;
      case '\n':
        ++currentLine;
        ++p;
        break;
      default:
        if(p[1] == '/')
        {
          p += 2;
          goto endloop2;
        }
        else
          ++p;
      }
    }
  endloop2:;
    token.value = String(start, p - start);
    token.type = commentType;
    return;
  }
}

uint_t Parser::readPreprocessorInteger(const char_t*& p)
{
  while(String::isSpace(*p))
    ++p;
  if(!String::isDigit(*p))
    return 0;
  const char_t* start = p;
  uint_t result = String::toUInt(start);
  ++p;
  while(String::isDigit(*p))
    ++p;
  return result;
}

String Parser::readPreprocessorString(const char_t*& p)
{
  while(String::isSpace(*p))
    ++p;
  if(*p != '"')
    return String();
  String result(260);
  const char_t* start = ++p;
  for(;;)
    switch(*p)
    {
    case '\"':
      ++p;
      return result;
    case '\\':
      if(p[1])
      {
        char_t c = unescape(p);
        if(c)
          result.append(c);
        break;
      }
      // no break
    case '\0':
      return String();
    default:
      result.append(*p);
      ++p;
    }
}

char_t Parser::unescape(const char_t*& p)
{
  ++p;
  char_t c = *(p++);
  switch(c)
  {
  case 'a': return '\a';
  case 'b': return '\b';
  case 'f': return '\f';
  case 'n': return '\n';
  case 'r': return '\r';
  case 't': return '\t';
  case 'v': return '\v';
  case '\\': return '\\';
  case '\'': return '\'';
  case '"': return '"';
  case '?': return '\?';
  case 'x':
    {
      int_t val = 0;
      char_t c = *p;
      if(String::isXDigit(c))
      {
        ++p;
        val = String::isDigit(c) ? (c - '0') : ((c > 'F') ? (c - ('a' + 10)) : (c - ('A' + 10)));
        c = *p;
        if(String::isXDigit(c))
        {
          ++p;
          val <<= 4;
          val |= String::isDigit(c) ? (c - '0') : ((c > 'F') ? (c - ('a' + 10)) : (c - ('A' + 10)));
        }
      }
      return val;
    }
  default:
    if(c >= '0' && c <= '7')
    {
      int_t val = c - '0';
      c = *p;
      if(c >= '0' && c <= '7')
      {
        ++p;
        val <<= 3;
        val |=  c - '0';
        c = *p;
        if(c >= '0' && c <= '7')
        {
          ++p;
          val <<= 3;
          val |=  c - '0';
        }
      }
      return val;
    }
    --p;
    return '\\';
  }
}

void_t Parser::pushState()
{
  State& state = states.append();
  state.p = p;
  state.currentLine = currentLine;
  state.currentFile = currentFile;
  state.token = token;
  state.lastComment = lastComment;
}

void_t Parser::popState()
{
  ASSERT(!states.isEmpty());
  const State& state = states.back();
  p = state.p;
  currentLine = state.currentLine;
  currentFile = state.currentFile;
  token = state.token;
  lastComment = state.lastComment;
  states.removeBack();
}

void_t Parser::dropState(size_t count)
{
  for(size_t i = 0; i < count; ++i)
  {
    ASSERT(!states.isEmpty());
    states.removeBack();
  }
}

Identifier* Parser::parseIdentifier()
{
  if(token.type != identifierType)
    return 0;
  Identifier* result = new Identifier;
  result->value = token.value;
  readToken();
  return result;
}

DecimalLiteral* Parser::parseDecimalLiteral()
{
  if(token.type != decimalType)
    return 0;
  DecimalLiteral* result = new DecimalLiteral;
  result->value = token.value;
  readToken();
  return result;
}

OctalLiteral* Parser::parseOctalLiteral()
{
  if(token.type != octalType)
    return 0;
  OctalLiteral* result = new OctalLiteral;
  result->value = token.value;
  readToken();
  return result;
}

HexadecimalLiteral* parseHexadecimalLiteral()
{
  if(token.type != hexadecimalType)
    return 0;
  HexadecimalLiteral* result = new HexadecimalLiteral;
  result->value = token.value;
  readToken();
  return result;
}

IntegerLiteral* Parser::parseIntegerLiteral()
{
  if(token.type != integerType)
    return 0;
  IntegerLiteral* result = new IntegerLiteral;
  result->value = token.value;
  readToken();
  return result;
}

CharacterLiteral* Parser::parseCharacterLiteral()
{
  if(token.type != characterType)
    return 0;
  CharacterLiteral* result = new CharacterLiteral;
  result->value = token.value;
  readToken();
  return result;
}

FloatingLiteral* Parser::parseFloatingLiteral()
{
  if(token.type != floatingType)
    return 0;
  FloatingLiteral* result = new FloatingLiteral;
  result->value = token.value;
  readToken();
  return result;
}

StringLiteral* Parser::parseStringLiteral()
{
  if(token.type != stringType)
    return 0;
  StringLiteral* result = new StringLiteral;
  result->value = token.value;
  readToken();
  return result;
}

SkipParenthesis* Parser::parseSkipParenthesis()
{
  pushState();
  size_t count = 0;
  for(;;)
  {
    if(token.value == ')')
    {
      if(count == 0)
      {
        dropState(1);
        return new SkipParenthesis;
      }
      --count;
    }
    if(token.value == '(')
      ++count;
    if(token.type == eofType)
    {
      popState();
      throw String("Unmatched (");
    }
    readToken();
  }
}
