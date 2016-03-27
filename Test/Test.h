
/** some comment for Class1 */
class Class1
{
};


void _invalid_parameter_noinfoxx_test(int a);

void __cdecl _invalid_parameter_noinfoxx(int a);

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
  class Class2 : public Reflected
  {
  };

  /** some comment for Class3 */
  class Class3 : public Class2
  {
    /** some in class comment */
  
    /** some comment for func1 */
    void func1(int);
  };

  struct Struct1
  {
  };

  template <class A, unsigned U> class TemplateClass1
  {
    template <class Z> class SubTemplateClass1
    {
    };
  };

  class Class4
  {
    TemplateClass1<Class2, 3> func2(int a);
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