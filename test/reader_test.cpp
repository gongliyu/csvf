#define BOOST_TEST_MODULE reader test
#include <boost/test/included/unit_test.hpp>

#include <vector>
#include <string>
#include <csvf/reader.hpp>


BOOST_AUTO_TEST_CASE(detect_eol)
{
    BOOST_TEST(csvf::reader("simple.csv").eol()==
               std::vector<char>{'\n'},
               boost::test_tools::per_element());
    BOOST_TEST(csvf::reader("CRLF.csv").eol()==
               std::vector<char>({'\r', '\n'}),
               boost::test_tools::per_element());
    BOOST_CHECK_THROW(csvf::reader("russellCRCRLF.csv").eol(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(detect_sep)
{
    BOOST_TEST(csvf::reader("simple.csv").sep()==',');
    BOOST_TEST(csvf::reader("doublequote_newline.csv").sep()==',');
    BOOST_TEST(csvf::reader("sep_vline.csv").sep()=='|');
    BOOST_TEST(csvf::reader("sep_blank.dat").sep()==' ');
}

BOOST_AUTO_TEST_CASE(detect_quote_rule)
{
    BOOST_TEST(static_cast<int>(csvf::reader("doublequote_newline.csv").quote_rule())==static_cast<int>(csvf::reader::quote_rule_type::doubled));
    BOOST_TEST(static_cast<int>(csvf::reader("unescaped.csv").quote_rule())==static_cast<int>(csvf::reader::quote_rule_type::verbatim));
}

BOOST_AUTO_TEST_CASE(detect_nfields)
{
    BOOST_TEST(csvf::reader("doublequote_newline.csv").nfields()==2);
    BOOST_TEST(csvf::reader("unescaped.csv").nfields()==3);
    BOOST_TEST(csvf::reader("allchar.csv").nfields()==2);
    BOOST_TEST(csvf::reader("sep_blank.dat").nfields()==5);
}

BOOST_AUTO_TEST_CASE(read_record_onecolumn)
{
    csvf::reader reader("onecolumn.csv");
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

BOOST_AUTO_TEST_CASE(blank_lines)
{
    csvf::reader reader("blank_lines.txt");
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","b","c"}),
        boost::test_tools::per_element());
    for (int i=0; i<5; i++) reader.skip_record();
    for (int i=0; i<3; i++) {
        BOOST_TEST(
            reader.read_record()==
            std::vector<std::string>({"1","2","3"}),
            boost::test_tools::per_element());
    }
    BOOST_TEST(!reader);

    reader.open("blank_lines2.txt");
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","b","c"}),
        boost::test_tools::per_element());
    for (int i=0; i<5; i++) {
        BOOST_TEST(
            reader.read_record()==
            std::vector<std::string>({"1","2","3"}),
            boost::test_tools::per_element());
    }
    BOOST_TEST(!reader);

    reader.open("blank_lines3.txt");
    BOOST_TEST(
        reader.read_record()==
        std::vector<std::string>({"a","b","c"}),
        boost::test_tools::per_element());
    for (int i=0; i<5; i++) {
        BOOST_TEST(
            reader.read_record()==
            std::vector<std::string>({"1","2","3"}),
            boost::test_tools::per_element());
    }
    BOOST_TEST(!reader);
}

BOOST_AUTO_TEST_CASE(anywhere_to_next_record_begin)
{
    csvf::reader reader("simple.csv");
    reader.skip_field().anywhere_to_next_record_begin();
    auto offset = reader.pos_offset();
    BOOST_TEST(offset==15);
    reader.skip_field().anywhere_to_next_record_begin();
    offset = reader.pos_offset();
    BOOST_TEST(offset==30);
    reader.skip_field().anywhere_to_next_record_begin();
    offset = reader.pos_offset();
    BOOST_TEST(offset==45);
    reader.skip_field().anywhere_to_next_record_begin();
    offset = reader.pos_offset();
    BOOST_TEST(offset==51);
    reader.skip_field().anywhere_to_next_record_begin();
    offset = reader.pos_offset();
    BOOST_TEST(offset==57);
    reader.skip_field().anywhere_to_next_record_begin();
    BOOST_TEST(!reader);
}

BOOST_AUTO_TEST_CASE(chunk)
{
    csvf::reader reader("chunk.csv");
    std::vector<ptrdiff_t> offsets = reader.chunk(2, 81, 1);
    BOOST_TEST(static_cast<double>(offsets[1]-offsets[0])/(offsets[2]-offsets[1])/3==1.0, boost::test_tools::tolerance(0.01));

    reader.open("simple2.csv");
    offsets = reader.chunk(2, 11, 1);
    BOOST_TEST(
        offsets==std::vector<ptrdiff_t>({0,50,100}),
        boost::test_tools::per_element());
}
