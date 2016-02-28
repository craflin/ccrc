
#include "Namespace.h"
#include "Class.h"

String Namespace::getFullName() const
{
  if(parent)
  {
    String parentName = parent->getFullName();
    if(!parentName.isEmpty())
      return parentName + "::" + name;
  }
  return name;
}

void_t Namespace::addClass(Class& _class)
{
  classes.append(_class.getName(), &_class);
}

void_t Namespace::getClasses(List<Class*>& classes)
{
  for(HashMap<String, Class*>::Iterator i = this->classes.begin(), end = this->classes.end(); i != end; ++i)
  {
    Class* _class = *i;
    classes.append(_class);
    _class->getClasses(classes);
  }
}
