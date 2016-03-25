/*
void _invalid_parameter_noinfoxx_test(int a);

void __cdecl _invalid_parameter_noinfoxx(int a);
*/
/*
namespace Test
{

class Reflected
{
};

class MetaInfo;

class MyObject : public Reflected
{
  class SubClass
  {
  };

public: // implements Reflected
  virtual const MetaInfo& getMetaInfo();
};

};

struct A
{
  int a;
} a;
*/

namespace X
{
namespace Test
{
  class Class1
  {
  };

  class Class2 : public Class1
  {
  };

  struct Struct1
  {
  };

  template <class A, unsigned U> class TemplateClass1
  {
  };
  
  enum Enum1
  {
    XX_a,
    XX_b,
  };
};
};

/*

#include <list>

class B
{
  int a;
};

class C : public MyTemplateObject<B>, public B
{
};

template <class T> class D : public MyTemplateObject<T>
{
};
*/