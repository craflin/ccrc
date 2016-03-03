
#pragma once

#include <nstd/String.h>
#include <nstd/List.h>

class TypeName
{
public:
  enum Flags
  {
    constFlag = 0x01,
    volatileFlag = 0x02,
    unsignedFlag = 0x04,
    signedFlag = 0x08,
    longFlag = 0x10,
    shortFlag = 0x20,
    longLongFlag = 0x40,
    referenceFlag = 0x80,
  };

public:
  TypeName() : flags(0) {}

  void_t setName(const String& name) {this->name = name;}
  const String& getName() const {return name;}

  void_t addTemplateParam(TypeName* param) {templateParams.append(param);}
  void_t swapTemplateParams(TypeName& other) {templateParams.swap(other.templateParams);}

  void_t setFlags(uint_t flags) {this->flags = flags;}
  void_t addPointeeFlags(uint_t flags) {pointeeFlags.prepend(flags);}

  void_t addNamespaceName(TypeName& namespaceTypeName) {namespaceTypes.append(&namespaceTypeName);}

private:
  String name;
  List<TypeName*> templateParams;
  List<uint_t> pointeeFlags;
  uint_t flags;
  List<TypeName*> namespaceTypes;
};
