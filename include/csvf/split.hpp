#ifndef __CSVF_SPLIT_HPP
#define __CSVF_SPLIT_HPP

#include <csvf/reader.hpp>
#include <csvf/writer.hpp>

namespace csvf
{
    template <typename DistributePolicy>
    void split_by_row(
        const std::string& in,
        const std::vector<std::string>& outs,
        bool header=false,
        DistributePolicy policy=[](int i, int n){return i % n;})
    {
        csvf::reader reader(in);
        int nfields = reader.get_nfields();
        
        int nwriters = outs.size();
        std::vector<csvf::writer> writers(nwriters);
        for (int i=0; i<nwriters; i++)
            nwriters.open(outs[i]);

        std::vector<std::string> content(nfields);
        if (header)
        {
            reader.read_record(content);
            for (int i=0; i<nwriters; i++)
                writers[i].write_record(content);
        }

        
    }
}

#endif
