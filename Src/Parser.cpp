
#include <nstd/Debug.h>

#include "Parser.h"

bool_t Parser::parse(const String& data)
{
  p = data;

  lastToken.type = eofType;
  token.type = eofType;
  readNextToken(nextToken);
  readToken();

  if(!parseNamespaceBody())
    return false;

  return true;
}

void_t Parser::getError(String& file, uint_t& line, String& message)
{
  file = token.file;
  line = token.line;
  message = error;
}

bool_t Parser::readToken()
{
  lastToken = token;
  token = nextToken;
  return readNextToken(nextToken);
}

bool_t Parser::readNextToken(Token& token)
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
      return readNumberToken(token);
    token.type = operatorType;
    if(*p == '+')
    {
      if(p[1] == '+')
        return p += 2, token.value = "++", true;
      if(p[1] == '=')
        return p += 2, token.value = "+=", true;
    }
    else if(*p == '-')
    {
      if(p[1] == '-')
        return p += 2, token.value = "--", true;
      if(p[1] == '=')
        return p += 2, token.value = "-=", true;
      if(p[1] == '>')
      {
        if(p[2] == '*')
          return p += 3, token.value = "->*", true;
        return p += 2, token.value = "->", true;
      }
    }
    return token.value = String(p++, 1), true;
  case '/':
    if(p[1] == '/' || p[1] == '*')
      return readCommentToken(token);
    if(p[1] == '=')
      return p += 2, token.type = operatorType, token.value = "/=", true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '=':
  case '!':
  case '^':
  case '*':
  case '%':
    if(p[1] == '=')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '>':
    if(p[1] == '>' && p[2] == '=')
      return token.type = operatorType, token.value = String(p, 3), p += 3, true;
    if(p[1] == '=' || p[1] == '>')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '<':
    if(p[1] == '<' && p[2] == '=')
      return token.type = operatorType, token.value = String(p, 3), p += 3, true;
    if(p[1] == '=' || p[1] == '<')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '&':
    if(p[1] == '&' || p[1] == '=')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '|':
    if(p[1] == '|' || p[1] == '=')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '.':
    if(p[1] == '*')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
    return token.type = operatorType, token.value = String(p++, 1), true;
  case '#':
    {
      const char_t* start = p;
      p = String::findOneOf(p, "\r\n");
      if(!p)
        p = start + String::length(start);
      return token.value = String(start, p - start), token.type = preprocessorType, true;
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
          return error = "New line in constant", false;
        case '\\':
          if(p[1])
          {
            p += 2;
            break;
          }
          // no break
        case '\0':
          return p = start, error = "Unmatched '", false;
        default:
          ++p;
        }
    endloop3:;
      return token.value = String(start, p - start), true;
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
          return error = "New line in constant", false;
        case '\\':
          if(p[1])
          {
            p += 2;
            break;
          }
          // no break
        case '\0':
          return p = start, error = "Unmatched \"", false;
        default:
          ++p;
        }
    endloop4:;
      return token.value = String(start, p - start), true;
    }
    break;
  case '\0':
    return token.type = eofType, token.value.clear(), true;
  default:
    if(String::isDigit(*p))
      return readNumberToken(token);
    if(String::isAlpha(*p) || *p == '_')
      return readIdentifierToken(token);
    return token.type = operatorType, token.value = String(p++, 1), true;
  }
}

bool_t Parser::readNumberToken(Token& token)
{
  ASSERT(*p == '+' || *p == '-' || String::isDigit(*p));
  const char_t* start = p;
  if(*p == '+' || *p == '-')
  {
    ++p;
    if(!String::isDigit(*p))
      return error = "Expected digit", false;
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
        return error = "Expected digit", false;
      ++p;
      while(String::isDigit(*p))
        ++p;
    }
    token.value = String(start, p - start);
    token.type = realType;
    return true;
  }
  token.value = String(start, p - start);
  token.type = integerType;
  return true;
}

bool_t Parser::readIdentifierToken(Token& token)
{
  ASSERT(String::isAlpha(*p) || *p == '_');
  const char_t* start = p;
  ++p;
  while(String::isAlpha(*p) || *p == '_' || String::isDigit(*p))
    ++p;
  token.value = String(start, p - start);
  token.type = identifierType;
  return true;
}

bool_t Parser::readCommentToken(Token& token)
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
    return true;
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
      }
    }
  endloop2:;
    token.value = String(p, p - start);
    token.type = commentType;
    return true;
  }
}

bool_t Parser::readTokenSkipComments()
{
  do
  {
    if(!readToken())
      return false;
  } while(token.type == commentType);
  return true;
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

bool_t Parser::parseBody()
{
  for(;;)
  {
    switch(token.type)
    {
    case operatorType:
      if(token.value == "{")
      {
        if(!readToken())
          return false;
        if(!parseBody())
          return false;
        if(token.value != "}")
          return error = "Expected }", false;
        break;
      }
      if(token.value == "}")
        return true;
      break;
    case eofType:
      return error = "Unexpected end of file", false;
    default:
      break;
    }
    if(!readToken())
      return false;
  }
}

Class* Parser::parseClass()
{
  ASSERT(token.value == "class" || token.value == "struct");

  String comment;
  if(lastToken.type == commentType)
    comment = lastToken.value;

  if(!readTokenSkipComments())
    return 0;

  if(token.value == "{") // anonymous class
  {
    Class& _class = classPool.append();
    _class.setParent(*currentNamespace);

    if(!readToken())
      return 0;

    if(!parseBody())
      return 0;

    if(token.value != "}")
      return error = "Expected }", 0;

    if(!readToken())
      return 0;

    return &_class;
  }

  if(token.type != identifierType)
    return error = "Expected identifer", 0;

  Class& _class = classPool.append();
  _class.setParent(*currentNamespace);
  _class.setName(token.value);
  _class.setComment(comment);

  if(!readTokenSkipComments())
    return 0;

  if(token.value == ":")
  {
    if(!readTokenSkipComments())
      return 0;

    do
    {
      if(token.value == "public" || token.value == "protected" ||token.value == "private")
        if(!readTokenSkipComments())
          return 0;

      TypeName* typeName = parseTypeName();
      if(!typeName)
        return 0;
      _class.addBaseType(*typeName);

      if(!readTokenSkipComments())
          return 0;
    } while(token.value == ",");
  }

  if(token.value == "{")
  {
    if(!readToken())
      return 0;

    currentNamespace = &_class;
    if(!parseNamespaceBody())
      return 0;
    currentNamespace = _class.getParent();

    if(token.value != "}")
      return error = "Expected }", 0;

    if(!readToken())
      return 0;
  }

  return &_class;
}

TypeName* Parser::parseTypeName()
{
  if(token.type != identifierType)
    return error = "Expected identifer", 0;

  TypeName& typeName = typeNamePool.append();
  typeName.setName(token.value);

  if(token.value == "<")
  {
    do
    {
      if(!readTokenSkipComments())
        return false;

      TypeName* templateParam = parseTypeName();
      if(!templateParam)
        return 0;
      typeName.addTemplateParam(templateParam);
    } while(token.value == ",");

    if(token.value == ">>")
      token.value = ">";
    else
    {
      if(token.value != ">")
        return error = "Expected >", 0;

      readTokenSkipComments();
    }
  }
  return &typeName;
}

bool_t Parser::parseNamespaceBody()
{
  for(;;)
  {
    switch(token.type)
    {
    case preprocessorType:
      if(token.value.startsWith("#line "))
      {
        const char_t* p = token.value;
        p += 5;
        currentLine = readPreprocessorInteger(p) - 1;
        currentFile = readPreprocessorString(p);
      }
      break;
    case identifierType:
      if(token.value == "class" || token.value == "struct")
      {
        Class* _class = parseClass();
        if(!_class)
          return false;
        if(!_class->getName().isEmpty())
          currentNamespace->addClass(*_class);
        continue;
      }
      else if(token.value == "template")
      {
      }
      else if(token.value == "enum")
      {
      }
      else if(token.value == "namespace")
      {
      }
      else if(token.value == "typedef")
      {
      }
      break;
    case operatorType:
      if(token.value == "}")
        return true;
    }
    if(!readToken())
      return false;
  }
}