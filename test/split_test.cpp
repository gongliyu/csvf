#define BOOST_TEST_MODULE split test
#include <boost/test/included/unit_test.hpp>

#include <string>
#include <boost/filesystem.hpp>

#include <csvf/split.hpp>


BOOST_AUTO_TEST_CASE(split_by_record)
{
    std::string infname = "simple.csv";
    std::vector<std::string> outfnames(0);
    outfnames.push_back(boost::filesystem::unique_path().native());
    outfnames.push_back(boost::filesystem::unique_path().native());
    outfnames.push_back(boost::filesystem::unique_path().native());
    csvf::split_by_record(infname, outfnames);

    csvf::reader reader(outfnames[0]);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"hello","hi","nihao"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"1","2","3"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);

    reader.open(outfnames[1]);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"nihao", "hello","hi"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"3","1","2"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);
    for (auto& fname : outfnames)
        boost::filesystem::remove(boost::filesystem::path(fname));
}


BOOST_AUTO_TEST_CASE(split_by_field)
{
    std::string infname = "simple.csv";
    std::vector<std::string> outfnames(0);
    outfnames.push_back(boost::filesystem::unique_path().native());
    outfnames.push_back(boost::filesystem::unique_path().native());
    csvf::split_by_field(infname, outfnames);

    csvf::reader reader(outfnames[0]);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"hello","nihao"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"nihao","hi"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"hi","hello"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"1","3"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"3","2"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"2","1"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);

    reader.open(outfnames[1]);
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"hi"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"hello"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"nihao"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"2"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"1"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"3"}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);
    
    for (auto& fname : outfnames)
        boost::filesystem::remove(boost::filesystem::path(fname));
}

