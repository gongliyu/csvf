#ifndef __CSVF_COMBINE_HPP
#define __CSVF_COMBINE_HPP

#include <functional>
#include <csvf/reader.hpp>
#include <csvf/writer.hpp>

namespace csvf
{
    void combine_by_record(
        const std::vector<std::string>& ins,
        const std::string& out,
        bool header=false
        );

    void combine_by_field(
        const std::vector<std::string>& ins,
        const std::string& out
        );
}

#endif
