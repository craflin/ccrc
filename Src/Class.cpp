
#include "Class.h"

String Class::getFullName() const
{
  String result = Namespace::getFullName();
  if(!templateParameters.isEmpty())
  {
    result += "<";
    for(List<TemplateParameter*>::Iterator i = templateParameters.begin(), end = templateParameters.end();;)
    {
      result += (*i)->getName();
      if(++i != end)
        result += ", ";
      else
        break;
    }
    result += ">";
  }
  return result;
}
