
#pragma once

#include <nstd/String.h>

class Rules;

class Reader
{
public:
  bool_t read(const String& filePath, Rules& rules);

  const String& getError() const {return error;}

private:
  String error;
  Rules* rules;

private:
  bool_t getToken(const char_t*& p, String& token);
  bool_t handleLine(const String& line);
};



