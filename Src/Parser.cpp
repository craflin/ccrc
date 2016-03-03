
#include <nstd/Debug.h>

#include "Parser.h"

bool_t Parser::parse(const String& data)
{
  p = data;

  token.type = eofType;
  readToken();

  if(!parseNamespaceBody(true))
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
  return readNextToken(token);
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
  case ':':
    if(p[1] == ':')
      return token.type = operatorType, token.value = String(p, 2), p += 2, true;
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
        else
          ++p;
      }
    }
  endloop2:;
    token.value = String(start, p - start);
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

bool_t Parser::parseParenthesize()
{
  for(;;)
  {
    switch(token.type)
    {
    case operatorType:
      if(token.value == "(")
      {
        if(!readToken())
          return false;
        if(!parseParenthesize())
          return false;
        if(token.value != ")")
          return error = "Expected )", false;
        break;
      }
      if(token.value == ")")
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

bool_t Parser::parseClass(Class*& _class)
{
  _class = 0;
  ASSERT(token.value == "class" || token.value == "struct");

  String comment;
  if(lastToken.type == commentType)
    comment = lastToken.value;

  if(!readTokenSkipComments())
    return false;

  if(token.value == "{") // anonymous class
  {
    if(!readToken())
      return false;

    if(!parseBody())
      return false;

    if(token.value != "}")
      return error = "Expected }", false;

    if(!readToken())
      return false;

    return true;
  }

  if(token.type != identifierType)
    return error = "Expected identifer", false;

  String className = token.value;

  if(!readTokenSkipComments())
    return false;

  List<TypeName*> baseTypes;

  if(token.value == ":")
  {
    do
    {
      if(!readTokenSkipComments())
        return false;

      if(token.value == "public" || token.value == "protected" ||token.value == "private")
        if(!readTokenSkipComments())
          return false;

      TypeName* typeName = parseTypeName();
      if(!typeName)
        return false;
      baseTypes.append(typeName);
    } while(token.value == ",");
  }

  if(token.value == "{")
  {
    if(!readToken())
      return false;

    _class = &classPool.append();
    _class->setParent(*currentNamespace);
    _class->setName(className);
    _class->setComment(comment);
    for(List<TypeName*>::Iterator i = baseTypes.begin(), end = baseTypes.end(); i != end; ++i)
      _class->addBaseType(**i);
    currentNamespace->addClass(*_class);

    currentNamespace = _class;
    if(!parseNamespaceBody())
      return false;
    currentNamespace = _class->getParent();

    if(token.value != "}")
      return error = "Expected }", false;

    if(!readToken())
      return false;
  }

  return true;
}

bool_t Parser::parseTemplate()
{
  ASSERT(token.value == "template");

  String comment;
  if(lastToken.type == commentType)
    comment = lastToken.value;

  if(!readTokenSkipComments())
    return false;

  if(token.value != "<")
    return true;

  if(!readTokenSkipComments())
    return false;

  if(token.value == ">")
    return true;

  List<TemplateParameter*> templateParams;

  for(;;)
  {
    if(token.value == "typename" || token.value == "class")
    {
      if(!readTokenSkipComments())
        return false;

      if(token.type != identifierType)
        return error = "Expected identifier", false;

      TemplateParameter& param = templateParameterPool.append();
      param.setName(token.value);

      if(!readTokenSkipComments())
        return false;

      if(token.value == "=")
      {
        if(!readTokenSkipComments())
          return false;

        TypeName* type = parseTypeName();
        if(!type)
          return false;
        param.setDefaultType(type);
      }

      templateParams.append(&param);
    }
    else
    {
      TypeName* type = parseTypeName();
      if(!type)
        return false;

      if(token.value == ">" || token.value == ">>" || token.value == ",")
        return true; 

      if(token.type != identifierType)
        return error = "Expected identifier", false;

      TemplateParameter& param = templateParameterPool.append();
      param.setType(*type);
      param.setName(token.value);

      if(!readTokenSkipComments())
        return false;

      if(token.value == "=")
      {
        if(!readTokenSkipComments())
          return false;

        int k =32;
        //param.setDefaultValue(??);
      }


    }

    if(token.value == ",")
    {
      if(!readTokenSkipComments())
        return false;
      continue;
    }
    break;
  }

  if(token.value == ">>")
      token.value = ">";
  else
  {
    if(token.value != ">")
      return error = "Expected >", false;
    if(!readTokenSkipComments())
      return false;
  }

  if(token.value != "class" && token.value != "struct")
    return true;

  Class* _class;
  if(!parseClass(_class))
    return false;

  if(_class)
    for(List<TemplateParameter*>::Iterator i = templateParams.begin(), end = templateParams.end(); i != end; ++i)
      _class->addTemplateParam(**i);

  return true;
}

bool_t Parser::parseNamespace()
{
  ASSERT(token.value == "namespace");

  if(!readTokenSkipComments())
    return false;

  if(token.type != identifierType)
    return error = "Expected identifier", false;

  Namespace& _namespace = namespacePool.append();
  _namespace.setName(token.value);
  _namespace.setParent(*currentNamespace);
  currentNamespace->addNamespace(_namespace);

  if(!readTokenSkipComments())
    return false;

  if(token.value != "{")
    return error = "Expected {", false;

  if(!readTokenSkipComments())
    return false;

  currentNamespace = &_namespace;
  if(!parseNamespaceBody())
    return false;
  currentNamespace = currentNamespace->getParent();

  if(token.value != "}")
    return error = "Expected }", false;

  if(!readTokenSkipComments())
    return false;

  return true;
}

TypeName* Parser::parseTypeName()
{
  if(token.type != identifierType)
    return error = "Expected identifer", 0;

  int_t flags = 0;
  for(;;)
  {
    if(token.value == "const")
    {
      flags |= TypeName::constFlag;
      goto next;
    }
    if(token.value == "volatile")
    {
      flags |= TypeName::volatileFlag;
      goto next;
    }
    if(token.value == "unsigned")
    {
      flags |= TypeName::unsignedFlag;
      goto next;
    }
    if(token.value == "signed")
    {
      flags |= TypeName::signedFlag;
      goto next;
    }
    if(token.value == "long")
    {
      flags |= flags & TypeName::longFlag ? TypeName::longLongFlag : TypeName::longFlag;
      goto next;
    }
    if(token.value == "short")
    {
      flags |= TypeName::shortFlag;
      goto next;
    }
    break;
  next:;
    if(!readTokenSkipComments())
      return false;
  };

  if(token.type != identifierType)
    return error = "Expected identifer", 0;

  TypeName& typeName = typeNamePool.append();
  typeName.setName(token.value);

  for(;;)
  {
    if(!readTokenSkipComments())
      return false;

    if(token.value == "<")
    {
      for(;;)
      {
        if(!readTokenSkipComments())
          return false;

        if(token.value == "typename" || token.value == "class" )
          if(!readTokenSkipComments())
            return false;

        TypeName* templateParam = parseTypeName();
        if(!templateParam)
          return 0;
        typeName.addTemplateParam(templateParam);

        if(token.value == ",")
          continue;
        break;
      }

      if(token.value == ">>")
        token.value = ">";
      else
      {
        if(token.value != ">")
          return error = "Expected >", 0;

        if(!readTokenSkipComments())
          return false;
      }
    }

    if(token.value != "::")
      break;

    if(!readTokenSkipComments())
      return false;

    if(token.type != identifierType)
      return error = "Expected identifer", 0;

    TypeName& namespaceTypeName = typeNamePool.append();
    namespaceTypeName.setName(typeName.getName());
    namespaceTypeName.swapTemplateParams(typeName);
    typeName.addNamespaceName(namespaceTypeName);
    typeName.setName(token.value);
  }

  for(;;)
  {
    for(;;)
    {
      if(token.value == "const")
      {
        flags |= TypeName::constFlag;
        goto next2;
      }
      if(token.value == "volatile")
      {
        flags |= TypeName::volatileFlag;
        goto next2;
      }
      if(token.value == "unsigned")
      {
        flags |= TypeName::unsignedFlag;
        goto next2;
      }
      if(token.value == "signed")
      {
        flags |= TypeName::signedFlag;
        goto next2;
      }
      if(token.value == "long")
      {
        flags |= flags & TypeName::longFlag ? TypeName::longLongFlag : TypeName::longFlag;
        goto next2;
      }
      if(token.value == "short")
      {
        flags |= TypeName::shortFlag;
        goto next2;
      }
      break;
    next2:;
      if(!readTokenSkipComments())
          return false;
    };
  
    if(token.value == "*")
    {
      typeName.addPointeeFlags(flags);
      flags = 0;
    }
    else if(token.value == "&")
      flags |= TypeName::referenceFlag;
    else
      break;
  }

  typeName.setFlags(flags);
  return &typeName;
}

bool_t Parser::parseNamespaceBody(bool_t allowEof)
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
        Class* _class;
        if(!parseClass(_class))
          return false;
        continue;
      }
      else if(token.value == "template")
      {
        if(!parseTemplate())
          return false;
        continue;
      }
      else if(token.value == "enum")
      {
      }
      else if(token.value == "namespace")
      {
        if(!parseNamespace())
          return false;
        continue;
      }
      else if(token.value == "typedef")
      {
      }
      break;
    case operatorType:
      if(token.value == "(")
      {
        if(!readToken())
          return false;
        if(!parseParenthesize())
          return false;
        if(token.value != ")")
          return error = "Expected )", false;
        if(!readToken())
          return false;
        continue;
      }
      if(token.value == "{")
      {
        if(!readToken())
          return false;
        if(!parseBody())
          return false;
        if(token.value != "}")
          return error = "Expected }", false;
        if(!readToken())
          return false;
        continue;
      }
      
      if(token.value == "}")
      {
        if(allowEof)
          return error = "Unexpected }", false;
        return true;
      }
      break;
    case eofType:
      if(allowEof)
        return true;
      return error = "Unexpected end of file", false;
    }
    if(!readToken())
      return false;
  }
}
