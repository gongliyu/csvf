#define BOOST_TEST_MODULE combine test
#include <boost/test/included/unit_test.hpp>

#include <string>
#include <boost/filesystem.hpp>

#include <csvf/combine.hpp>

BOOST_AUTO_TEST_CASE(combine_by_record_noheader)
{
    std::vector<std::string> infnames{"combine1.csv", "combine2.csv","combine3.csv"};
    std::string outfname = boost::filesystem::unique_path().native();

    csvf::combine_by_record(infnames, outfname, false);
    csvf::reader reader(outfname);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"1","1","1"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"2","2","2"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"3","3","3"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"b","b","b"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"c","c","c"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"e","e","e"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"f","f","f"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"g","g","g"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);
}

BOOST_AUTO_TEST_CASE(combine_by_record_header)
{
    std::vector<std::string> infnames{"combine1.csv", "combine2.csv","combine3.csv"};
    std::string outfname = boost::filesystem::unique_path().native();

    csvf::combine_by_record(infnames, outfname, true);
    csvf::reader reader(outfname);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"1","1","1"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"2","2","2"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"3","3","3"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"b","b","b"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"c","c","c"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"e","e","e"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"f","f","f"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"g","g","g"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);
}

BOOST_AUTO_TEST_CASE(combine_by_field)
{
    std::vector<std::string> infnames{"combine1.csv", "combine2.csv","combine3.csv"};
    std::string outfname = boost::filesystem::unique_path().native();

    csvf::combine_by_field(infnames, outfname);
    csvf::reader reader(outfname);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","a","a","a","a","a","a","a","a"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"1","1","1","a","a","a","e","e","e"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"2","2","2","b","b","b","f","f","f"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"3","3","3","c","c","c","g","g","g"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);
}

BOOST_AUTO_TEST_CASE(combine_by_field_notmatch)
{
    std::vector<std::string> infnames{"combine1.csv", "ch11b.dat"};
    std::string outfname = boost::filesystem::unique_path().native();

    BOOST_CHECK_THROW(csvf::combine_by_field(infnames, outfname), csvf::bad_format);
}
