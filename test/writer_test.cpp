#define BOOST_TEST_MODULE writer test
#include <boost/test/included/unit_test.hpp>

#include <vector>
#include <string>
#include <csvf/reader.hpp>
#include <csvf/writer.hpp>

BOOST_AUTO_TEST_CASE(write_record)
{
    std::string fname("__tmp1.csv");
    csvf::writer writer(fname);
    std::vector<std::string> content{"hello", "world", "hi"};
    for (int i=0; i <3; i++)
        writer.write_record(content);
    writer.flush();

    csvf::reader reader(fname);
    BOOST_TEST(reader.read_record()==content,
               boost::test_tools::per_element());
    BOOST_TEST(reader.read_record()==content,
               boost::test_tools::per_element());
    BOOST_TEST(reader.read_record()==content,
               boost::test_tools::per_element());
    BOOST_TEST(!reader);
}

BOOST_AUTO_TEST_CASE(write_record_operator)
{
    std::string fname("__tmp1.csv");
    csvf::writer writer(fname);
    std::vector<std::string> content{"hello", "world", "hi"};
    for (int i=0; i <3; i++)
        writer<<content;
    writer.flush();

    csvf::reader reader(fname);
    BOOST_TEST(reader.read_record()==content,
               boost::test_tools::per_element());
    BOOST_TEST(reader.read_record()==content,
               boost::test_tools::per_element());
    BOOST_TEST(reader.read_record()==content,
               boost::test_tools::per_element());
    BOOST_TEST(!reader);
}

