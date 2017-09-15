#ifndef __CSVF_WRITER_HPP
#define __CSVF_WRITER_HPP

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <vector>
#include <string>

namespace csvf
{
    class writer
    {
    public:
        using size_type = size_t;
        writer() = default;
        virtual ~writer(){};

        template <typename... Args>
        explicit writer(const std::string& fname, Args... args)
        {
            open(fname, args...);
        };

        template <typename... Args>
        writer& open(const std::string& fname, Args... args)
        {
            m_filename = fname;
            m_stream.open(fname);
        };

        template <typename RecordType>
        writer& write_record(const RecordType& record)
        {
            int n = record.size();
            int i = 0;
            for (auto& field : record)
            {
                i++;
                m_stream<<field;
                m_stream<<(i==n?'\n':m_sep);
            }
            return *this;
        };

        template <typename RecordType>
        writer& operator<<(const RecordType& record)
        {
            return write_record(record);
        }
        
        writer& flush();
        writer& close();

    private:
        std::string m_filename;
        std::ofstream m_stream;
        char m_sep{','};
    };
}

#endif
