
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

class Class;

class Namespace
{
public:
  Namespace() : parent(0) {}

  void_t setName(const String& name) {this->name = name;}
  const String& getName() const {return name;}

  virtual String getFullName() const;

  void_t setParent(Namespace& parent) {this->parent = &parent;}
  Namespace* getParent() const {return parent;}

  void_t addClass(Class& _class);
  void_t getClasses(List<Class*>& classes);
  Class* getLastClass() {return classes.back();}

  void_t addNamespace(Namespace& _namespace) {namespaces.append(_namespace.getName(), &_namespace);}

private:
  String name;
  Namespace* parent;
  HashMap<String, Class*> classes;
  HashMap<String, Namespace*> namespaces;
};