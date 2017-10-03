#include <string>
#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>
#include <csvf/combine.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    std::vector<std::string> infnames;
    std::string outfname;
    bool header = false;
    std::string by("record");
    
    po::options_description desc;
    desc.add_options()
        ("help,h", "help message")
        ("input,i", po::value<std::vector<std::string>>(&infnames)->required(), "input file to be splitted")
        ("output,o", po::value<std::string>(&outfname)->required(), "result splitted files")
        ("header", po::value<bool>(&header), "whether input file contains header")
        ("by", po::value<std::string>(&by), "split by field or by record")
        ;

    po::positional_options_description pd;
    pd.add("output", 1).add("output", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc)
              .positional(pd).run(), vm);
    notify(vm);

    if (by == "record")
        csvf::combine_by_record(infnames, outfname, header);
    else if (by == "field")
        csvf::combine_by_field(infnames, outfname);
    else {
        std::cout<<"option \"by\" should be either \"record\" or \"field\""<<std::endl;
        return 1;
    }
}
