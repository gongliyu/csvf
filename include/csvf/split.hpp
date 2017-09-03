#ifndef __CSVF_SPLIT_HPP
#define __CSVF_SPLIT_HPP

#include <functional>
#include <csvf/reader.hpp>
#include <csvf/writer.hpp>

namespace csvf
{
    void split_by_record(
        const std::string& in,
        const std::vector<std::string>& outs,
        bool header=false,
        std::function<int(int,int)> policy=[](int i, int n){
            return i%n;}
        );

    void split_by_field(
        const std::string& in,
        const std::vector<std::string>& outs,
        std::function<int(int,int)> policy=[](int i, int n){
            return i%n;}
        );
}

#endif
