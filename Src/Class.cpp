
#include "Class.h"

String Class::getFullName() const
{
  String result = Namespace::getFullName();
  // add template stuff..
  return result;
}
