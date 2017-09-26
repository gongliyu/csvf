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
    /**
     * a class to write csv files
     */
    class writer
    {
    public:
        /** type to represent size */
        using size_type = size_t;

        /**
         * Constructors and destructors
         */
        /**@{*/
        /** default constructor */
        writer() = default;
        /** virtual destructor */
        virtual ~writer(){};

        /**
         * construct writer object and associate it with a file
         *
         * @param fname file name
         * @param args other arguments passed to open
         *
         * @details This constructor will call open to open the file
         * specified by fname to write csv content.
         */
        template <typename... Args>
        explicit writer(const std::string& fname, Args... args)
        {
            open(fname, args...);
        };
        /**@}*/

        /**
         * Open a file to write csv content
         * 
         * @param fname file name specify which file to write @param
         * string-value pairs of options to open the file. See details
         * for more information.
         *
         */
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
