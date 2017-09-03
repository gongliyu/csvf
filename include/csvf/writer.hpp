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

        writer& write_field(const std::string&);
        writer& write_record(const std::vector<std::string>&);
        writer& operator<<(const std::vector<std::string>& record);
        writer& flush();
        writer& close();

    private:
        std::string m_filename;
        std::ofstream m_stream;
        char m_sep{','};
    };
}

#endif
