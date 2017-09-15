
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

        csvf::reader::record_type record(nfields);
        if (header)
        {
            reader.read_record(record);
            for (int i=0; i<nwriters; i++)
                writers[i].write_record(record);
        }

        int i=0;
        while (reader)
        {
            reader.read_record(record);
            assert(policy(i, nwriters)<nwriters);
            writers[policy(i, nwriters)].write_record(record);
            i++;
        }
    }

    void split_by_field(
        const std::string& in,
        const std::vector<std::string>& outs,
        std::function<int(int,int)> policy)
    {
        // open input file, detect nfields
        csvf::reader reader(in);
        int nfields = reader.nfields();

        // open output files
        int nwriters = outs.size();
        std::vector<csvf::writer> writers(nwriters);
        for (int i=0; i<nwriters; i++)
            writers[i].open(outs[i]);

        // calculate field map and nfields for outputs
        std::vector<int> field_map(nfields);
        std::vector<int> nfields_out(nwriters, 0);
        for (int i=0; i<nfields; i++)
        {
            auto idx_out = policy(i, nwriters);
            assert(idx_out<nwriters);
            field_map[i] = idx_out;
            nfields_out[idx_out]++;
        }

        // initialize records for outputs
        std::vector<reader::record_type> records(nwriters);
        for (int i=0; i<nwriters; i++)
            records[i].resize(nfields_out[i]);
        std::vector<int> idx_fields(nwriters, 0);
        reader::record_type record(nfields);

        while (reader)
        {
            reader.read_record(record);
            std::fill(std::begin(idx_fields), std::end(idx_fields), 0);
            int i = 0;
            for (auto& field : record)
            {
                auto idx_out = field_map[i++];
                records[idx_out][idx_fields[idx_out]++] = field;
            }
            for (int i=0; i<nwriters; i++)
                writers[i].write_record(records[i]);
        }
    }
}
