
void _invalid_parameter_noinfoxx_test(int a);

void __cdecl _invalid_parameter_noinfoxx(int a);

class MyObject : public Reflected
{
  class SubClass
  {
  };

public: // implements Reflected
  virtual const MetaInfo& getMetaInfo();
};

struct 
{
  int a;
} a;

template <class A> class MyTemplateObject
{
};

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