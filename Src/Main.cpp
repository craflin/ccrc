
#include <nstd/Console.h>
#include <nstd/Process.h>
#include <nstd/Debug.h>
#include <nstd/List.h>

void_t usage(char_t* argv[])
{
  Console::errorf("Usage: %s <header> <source> -o <output>\n", argv[0]);
}

class Parser
{
public:
  bool_t parse(const String& data)
  {
    p = data;
    line = 1;

    Token token;
    for(;;)
    {
      if(!readToken(token))
        return false;
      switch(token.type)
      {
      case commentType:
        lastComment = token.value;
        break;
      case preprocessorType:
        break;
      case identifierType:
        if(!readIdentifier(token))
          return false;
        break;
      case operatorType:
        if(token.value== "}")
          return error = "Unexpected }", false;
        break;
      }
    }

    return true;
  }

private:
  enum TokenType
  {
    integerType,
    realType,
    operatorType,
    stringType,
    charType,
    eofType,
    identifierType,
    commentType,
    preprocessorType,
  };

  struct Token
  {
    String value;
    TokenType type;
  };

private:
  const char_t* p;
  uint_t line;
  String error;
  String lastComment;

private:
  bool_t readToken(Token& token)
  {
    for(;;)
      switch(*p)
      {
      case '\r':
        ++p;
        ++line;
        if(*p == '\n')
          ++p;
        break;
      case '\n':
        ++p;
        ++line;
        break;
      default:
        if(String::isSpace(*p))
          ++p;
        else
          goto endloop;
      }
  endloop:;

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
        return token.value = String(p, p - start), token.type = preprocessorType, true;
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
              p += 2;
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
              p += 2;
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

  bool_t readNumberToken(Token& token)
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

  bool_t readIdentifierToken(Token& token)
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

  bool_t readCommentToken(Token& token)
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
          ++line;
          ++p;
          if(*p == '\n')
            ++p;
          break;
        case '\n':
          ++line;
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

  bool_t peekToken(Token& token)
  {
    const char* p = this->p;
    int_t line = this->line;
    if(!readToken(token))
      return false;
    this->p = p;
    this->line = line;
    return true;
  }

  bool_t readTokenSkipComments(Token& token)
  {
    do
    {
      if(!readToken(token))
        return false;
    } while(token.type == commentType);
    return true;
  }

  bool_t readBody()
  {
    Token token;
    for(;;)
    {
      if(!readToken(token))
        return false;
      switch(token.type)
      {
      case operatorType:
        if(token.value == "{")
        {
          if(!readBody())
            return false;
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
    }
  }

  bool_t readIdentifier(Token& token)
  {
    if(token.value == "class")
    {
      Token token;
      if(!readTokenSkipComments(token))
        return false;

      if(token.type != identifierType)
        return error = "Expected identifer", false;

      String className = token.value;

      if(!readTokenSkipComments(token))
        return false;

      if(token.value == "{")
        return readBody();

      if(token.value != ":")
        return error = "Expected :", false;

      if(!readTokenSkipComments(token))
        return false;

      List<String> parents;

      for(;;)
      {
        if(token.value == "public" || token.value == "protected" ||token.value == "private")
          if(!readTokenSkipComments(token))
            return false;

        if(token.type != identifierType)
          return error = "Expected identifer", false;

        parents.append(token.value);

        if(!readTokenSkipComments(token))
            return false;

        if(token.value == ",")
          continue;
        break;
      }

      if(token.value != "{")
          return error = "Expected {", false;

      if(!readClassBody())
        return false;
      return true;
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
    return true;
  }

  bool_t readClassBody()
  {
    return true;
  }
};

int_t main(int_t argc, char_t* argv[])
{
  String headerFile;
  String sourceFile;
  String outputFile;
  {
    Process::Option options[] = {
        {'o', "output", Process::argumentFlag},
        {'h', "help", Process::optionFlag},
    };
    Process::Arguments arguments(argc, argv, options);
    int_t character;
    String argument;
    while(arguments.read(character, argument))
      switch(character)
      {
      case 'o':
        outputFile = argument;
        break;
      case 0:
        if(headerFile.isEmpty())
          headerFile = argument;
        else if(sourceFile.isEmpty())
          sourceFile = argument;
        else
          return usage(argv), 1;
        break;
      case '?':
        Console::errorf("Unknown option: %s.\n", (const char_t*)argument);
        return 1;
      case ':':
        Console::errorf("Option %s required an argument.\n", (const char_t*)argument);
        return 1;
      default:
        return usage(argv), 1;
      }
  }

  if(headerFile.isEmpty() || sourceFile.isEmpty() || outputFile.isEmpty())
    return usage(argv), 1;

  // find a compiler
  String path = Process::getEnvironmentVariable("PATH");
  //List<String> paths;
  //path.split(';', paths);
  String compiler = "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\cl.exe";

  // use preprocessor
  String input;
  {
    Process process;
    if(!process.open(compiler + " /E /C " + sourceFile))
      return 1;
    char_t buffer[4096];
    String data;
    for(ssize_t i;;)
    {
      i = process.read(buffer, sizeof(buffer));
      if(i <= 0)
        break;
      data.attach(buffer, i);
      input += data;
    }
    uint32_t exitCode;
    if(!process.join(exitCode))
      return 1;
    if(exitCode != 0)
      return 1;
  }

  // parse data
  Parser parser;
  if(!parser.parse(input))
    return 1;

  // generate output


  return 0;
}
