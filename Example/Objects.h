
#include <Reflected.h>

class MyInterface1
{
public:
  /**
   * @invokable
   * Example function
   * @param p0 Some description
   */
  virtual int func1(int p0) = 0;

  /**
   * @invokable
   * Example function
   * @param p0 Some description
   */
  virtual int func2(int p0) = 0;
};

/** some comment for MyObject1 */
class MyObject1 : public Reflected
{
public:
  virtual const Reflected::Type& getReflectedType() const;

public:
  /**
   * @invokable
   * Example function
   * @param p0 Some description
   */
  int func3(int p0);
};

namespace SomeSpace1
{

class MyObject2 : public MyInterface1, public Reflected
{
public:
  virtual const Reflected::Type& getReflectedType() const;

public:
  virtual int func1(int p0) {return 0;}
  virtual int func2(int p0) {return 0;}

public:
  /**
   * @invokable
   * Example function
   * @param p0 Some description
   */
  int func4(int p0);
};

}

namespace SomeSpace2
{
  using namespace SomeSpace1;

class MyObject3 : public MyObject1
{
public:
  virtual const Reflected::Type& getReflectedType() const;

public:
  /**
   * @invokable
   * Example function
   * @param p0 Some description
   */
  int func5(int p0);
};

class MyObject4 : public MyObject2
{
public:
  virtual const Reflected::Type& getReflectedType() const;

public:
  /**
   * @invokable
   * Example function
   * @param p0 Some description
   */
  int func6(int p0);

  int func7();
};

class MyObject5 : public Reflected
{
public:
  virtual const Reflected::Type& getReflectedType() const;
};
};
