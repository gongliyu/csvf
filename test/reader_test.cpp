#define BOOST_TEST_MODULE reader test
#include <boost/test/included/unit_test.hpp>

#include <vector>
#include <string>
#include <csvf/reader.hpp>


BOOST_AUTO_TEST_CASE(get_file_size)
{
    BOOST_TEST(csvf::reader("2008head.csv").get_file_size() == 47754u);
}

BOOST_AUTO_TEST_CASE(detect_eol)
{
    BOOST_TEST(csvf::reader("2008head.csv").get_eol()==
               std::vector<char>{'\n'},
               boost::test_tools::per_element());
    BOOST_TEST(csvf::reader("doublequote_newline.csv").get_eol()==
               std::vector<char>{'\n'},
               boost::test_tools::per_element());
    BOOST_TEST(csvf::reader("russellCRLF.csv").get_eol()==
               std::vector<char>({'\r', '\n'}),
               boost::test_tools::per_element());
    BOOST_CHECK_THROW(csvf::reader("russellCRCRLF.csv").get_eol(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(detect_sep)
{
    BOOST_TEST(csvf::reader("2008head.csv").get_sep()==',');
    BOOST_TEST(csvf::reader("doublequote_newline.csv").get_sep()==',');
    BOOST_TEST(csvf::reader("issue_773_fread.txt").get_sep()=='|');
    BOOST_TEST(csvf::reader("ch11b.dat").get_sep()==' ');
}

BOOST_AUTO_TEST_CASE(detect_quote_rule)
{
    BOOST_TEST(static_cast<int>(csvf::reader("doublequote_newline.csv").get_quote_rule())==static_cast<int>(csvf::reader::quote_rule::doubled));
    BOOST_TEST(static_cast<int>(csvf::reader("unescaped.csv").get_quote_rule())==static_cast<int>(csvf::reader::quote_rule::verbatim));
}

BOOST_AUTO_TEST_CASE(detect_nfields)
{
    BOOST_TEST(csvf::reader("doublequote_newline.csv").get_nfields()==2);
    BOOST_TEST(csvf::reader("unescaped.csv").get_nfields()==3);
    BOOST_TEST(csvf::reader("allchar.csv").get_nfields()==2);
    BOOST_TEST(csvf::reader("ch11b.dat").get_nfields()==5);
}


BOOST_AUTO_TEST_CASE(read_record)
{
    csvf::reader reader("ch11b.dat");
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"001","307","0930","36.58","0"}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"002","307","0940","36.73","0"}),
        boost::test_tools::per_element());
    
    reader.open("unescaped.csv");
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"\"No\"","\"Comment\"","\"Type\""}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"\"0\"","\"he said:\"wonderful.\"\"","\"A\""}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"\"1\"","\"The problem is: reading table, and also \"a problem, yes.\" keep going on.\"","\"A\""}),
        boost::test_tools::per_element());
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"","",""}),
        boost::test_tools::per_element());
    BOOST_TEST(!reader);
    BOOST_TEST(reader.is_end());
}

