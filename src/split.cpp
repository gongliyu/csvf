
#include <cassert>
#include <csvf/split.hpp>

namespace csvf
{
    void split_by_record(
        const std::string& in,
        const std::vector<std::string>& outs,
        bool header,
        std::function<int(int,int)> policy)
    {
        csvf::reader reader(in);
        int nfields = reader.nfields();
        
        int nwriters = outs.size();
        std::vector<csvf::writer> writers(nwriters);
        for (int i=0; i<nwriters; i++)
            writers[i].open(outs[i]);

        std::vector<std::string> content(nfields);
        if (header)
        {
            reader.read_record(content);
            for (int i=0; i<nwriters; i++)
                writers[i].write_record(content);
        }

        int i=0;
        while (reader)
        {
            reader.read_record(content);
            assert(policy(i, nwriters)<nwriters);
            writers[policy(i, nwriters)].write_record(content);
            i++;
        }
    }

    void split_by_field(
        const std::string& in,
        const std::vector<std::string>& outs,
        std::function<int(int,int)> policy)
    {
        csvf::reader reader(in);
        int nfields = reader.nfields();
        
        int nwriters = outs.size();
        std::vector<csvf::writer> writers(nwriters);
        for (int i=0; i<nwriters; i++)
            writers[i].open(outs[i]);

        std::vector<std::string> content(nfields);

        while (reader)
        {
            reader.read_record(content);
            int i=0;
            std::vector<std::vector<std::string>> contents(nwriters);
            for (auto& field : content)
            {
                assert(policy(i, nwriters)<nwriters);
                contents[policy(i, nwriters)].push_back(field);
                i++;
            }
            for (int i=0; i<nwriters; i++)
                writers[i].write_record(contents[i]);
        }
    }
}
