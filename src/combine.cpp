#include <cassert>
#include <csvf/combine.hpp>

namespace csvf {
    void combine_by_record(
        const std::vector<std::string>& ins,
        const std::string& out,
        bool header)
    {
        int nfields = -1;
        csvf::writer writer(out);

        bool header_written = false;
        for (auto& fname : ins) {
            csvf::reader reader(fname);

            if (nfields < 0) {
                nfields = reader.nfields();
            }

            if (header) {
                if (header_written) {
                    reader.skip_record();
                } else {
                    header_written = true;
                }
            }

            // if header was already written to the output file, skip
            // header of this input file
            while (reader) {
                writer.write_record(reader.read_record());
            }
        }
    }

    void combine_by_field(
        const std::vector<std::string>& ins,
        const std::string& out)
    {
        int nreaders = ins.size();
        std::vector<csvf::reader> readers(nreaders);
        for (int i=0; i<nreaders; i++)
            readers[i].open(ins[i]);

        csvf::writer writer(out);

        while (true) {
            csvf::reader::record_type record;
            int nvalid = 0;
            for (auto& reader : readers)
                nvalid += reader;
            if (nvalid == 0) break;
            else if (nvalid != nreaders)
                throw csvf::bad_format("# of records in input files do not match");
            
            for (int i=0; i<nreaders; i++) {
                auto tmp = readers[i].read_record();
                record.insert(record.end(), tmp.begin(), tmp.end());
            }
            writer.write_record(record);
        }
    }
}
