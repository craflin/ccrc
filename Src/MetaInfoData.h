
#pragma once

#include "MetaTypeDecl.h"

class MetaInfoData
{
public:
  MetaTypeDecl& getMetaTypeDecl(const String& name)
  {
    HashMap<String, MetaTypeDecl>::Iterator it = declarations.find(name);
    if(it == declarations.end())
      return declarations.append(name, MetaTypeDecl());
    return *it;
  }

  MetaTypeDecl* getLastMetaTypeDecl() {return declarations.isEmpty() ? 0 : &declarations.back();}

  void_t print()
  {
    for(HashMap<String, MetaTypeDecl>::Iterator i = declarations.begin(), end = declarations.end(); i != end; ++i)
      i->print();
  }

public:
  HashMap<String, MetaTypeDecl> declarations;
};
