
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>

class Production
{
public:
  struct Data
  {
    Production* subProduction;
    uint_t lineIndex;
  };

public:
  HashMap<String, Data> productions;
};

class Rule
{
public:
  Rule() : productionLines(0) {}

public:
  Production productionRoot;
  uint_t productionLines;
};

class Rules
{
public:
  HashMap<String, Rule*> rules;
};
