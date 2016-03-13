
#pragma once

#include <nstd/String.h>

namespace Parser
{
  bool_t parse(const String& data);

  void_t getError(String& file, uint_t& line, String& message);

  enum TokenType
  {
    integerType,
    decimalType,
    floatingType,
    octalType,
    hexadecimalType,
    operatorType,
    stringType,
    characterType,
    eofType,
    identifierType,
    keywordType,
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

  extern Token token;

  void_t readToken();
  void_t readTokenRaw();
  void_t readNumberToken();
  void_t readIdentifierOrKeywordToken();
  void_t readCommentToken();

  uint_t readPreprocessorInteger(const char_t*& p);
  String readPreprocessorString(const char_t*& p);
  char_t unescape(const char_t*& p);

  void_t pushState();
  void_t popState();
  void_t dropState(size_t count);

  struct Identifier
  {
    String value;
  };

  struct DecimalLiteral
  {
    String value;
  };

  struct OctalLiteral
  {
    String value;
  };

  struct HexadecimalLiteral
  {
    String value;
  };

  struct IntegerLiteral
  {
    String value;
  };

  struct CharacterLiteral
  {
    String value;
  };

  struct FloatingLiteral
  {
    String value;
  };

  struct StringLiteral
  {
    String value;
  };

  Identifier* parseIdentifier();
  DecimalLiteral* parseDecimalLiteral();
  OctalLiteral* parseOctalLiteral();
  HexadecimalLiteral* parseHexadecimalLiteral();
  IntegerLiteral* parseIntegerLiteral();
  CharacterLiteral* parseCharacterLiteral();
  FloatingLiteral* parseFloatingLiteral();
  StringLiteral* parseStringLiteral();
};
