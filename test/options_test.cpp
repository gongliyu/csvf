#define BOOST_TEST_MODULE options test
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <iomanip>
#include <csvf/options.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
    struct A {
        A()=default;
        A(int m1) : m_m1(m1) {};
        int m_m1;
    };
    
    auto opts = options::parse(
        "a", 1, "b", 'b', "c", "hello", "d", 2);

    int a;
    char b;
    std::string c;
    A d;
    options::assign(opts, "a", a, "b", b, "c", c, "d", d.m_m1);
    BOOST_TEST(a==1);
    BOOST_TEST(b=='b');
    BOOST_TEST(c=="hello");
    BOOST_TEST(d.m_m1==2);
}
