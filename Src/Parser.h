
#pragma once

#include <nstd/List.h>
#include <nstd/Pool.h>

#include "Namespace.h"
#include "Class.h"
#include "TypeName.h"

class Parser
{
public:
  Parser() : currentLine(1), currentNamespace(&rootNamespace) {}

  bool_t parse(const String& data);

  void_t getError(String& file, uint_t& line, String& message);

  void_t getClasses(List<Class*>& classes) {rootNamespace.getClasses(classes);}

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
    String file;
    int line;
  };

private:
  const char_t* p;
  uint_t currentLine;
  String currentFile;
  String error;
  Token lastToken;
  Token token;

  Namespace rootNamespace;
  Namespace* currentNamespace;

  Pool<TypeName> typeNamePool;
  Pool<Class> classPool;
  Pool<TemplateParameter> templateParameterPool;

private:
  bool_t readToken();
  bool_t readNextToken(Token& token);
  bool_t readNumberToken(Token& token);
  bool_t readIdentifierToken(Token& token);
  bool_t readCommentToken(Token& token);
  bool_t readTokenSkipComments();

private:
  static uint_t readPreprocessorInteger(const char_t*& p);
  static String readPreprocessorString(const char_t*& p);
  static char_t unescape(const char_t*& p);

private:
  bool_t parseBody();
  bool_t parseClass();
  bool_t parseTemplate();
  TypeName* parseTypeName();
  bool_t parseNamespaceBody();

};
