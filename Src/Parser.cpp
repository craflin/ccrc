
#include <nstd/Debug.h>

#include "Parser.h"

bool_t Parser::parse(const String& data)
{
  p = data;

  try
  {
    readToken();
    //root = parseTranslationUnit();
  }
  catch(const String& error)
  {
    this->error = error;
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
      token.type = charType;
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
      return readIdentifierToken();
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
    token.type = realType;
    return;
  }
  token.value = String(start, p - start);
  token.type = integerType;
  return;
}

void_t Parser::readIdentifierToken()
{
  ASSERT(String::isAlpha(*p) || *p == '_');
  const char_t* start = p;
  ++p;
  while(String::isAlpha(*p) || *p == '_' || String::isDigit(*p))
    ++p;
  token.value = String(start, p - start);
  token.type = identifierType;
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
