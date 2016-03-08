
#pragma once

#include <nstd/List.h>
#include <nstd/Pool.h>

#include "Namespace.h"
#include "Class.h"
#include "TypeName.h"

class Parser
{
public:
  Parser() : currentLine(1) {}

  bool_t parse(const String& data);

  void_t getError(String& file, uint_t& line, String& message);

  //void_t getClasses(List<Class*>& classes) {rootNamespace.getClasses(classes);}

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
  Token token;
  String lastComment;

private:
  void_t readToken();
  void_t readTokenRaw();
  void_t readNumberToken();
  void_t readIdentifierToken();
  void_t readCommentToken();

private:
  struct Identifier;
  struct DecimalLiteral;
  struct OctalLiteral;
  struct HexadecimalLiteral;
  struct IntegerLiteral;
  struct CharacterLiteral;
  struct FloatingLiteral;
  struct StringLiteral;

private:
  static uint_t readPreprocessorInteger(const char_t*& p);
  static String readPreprocessorString(const char_t*& p);
  static char_t unescape(const char_t*& p);

private:
#include "ParserGenerated.h"
};
