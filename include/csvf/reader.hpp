#ifndef __CSVF_READER_HPP
#define __CSVF_READER_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <stdexcept>
#include <boost/iostreams/device/mapped_file.hpp>

#include "function_options.hpp"

namespace csvf
{
    class bad_format : public std::logic_error
    {
    public:
        bad_format(const std::string& what) : std::logic_error(what)
        {
        }
    };
    
    class reader
    {
    public:
        using size_type = size_t;
        enum class quote_rule {doubled, escaped, verbatim, none};
        reader(const reader&) = delete;
        reader& operator=(const reader&) = delete;
        
        template <typename... Args>
        explicit reader(const std::string& fname, Args... args):
            m_filename(fname), m_file(fname)
        {
            function_options fopts;
            fopts.parse(args...);
            std::cout<<"fopts size(): "<<fopts.size()<<std::endl;
            fopts.assign("Sep", m_sep);
                                     // "Fill", m_fill,
                                     // "quote_rule", m_quote_rule,
                                     // "StripWhite", m_stripWhite,
                                     // "Quote", m_quote,
                                     // "Verbose", m_verbose);
            init();
                                     
        }
        


        operator bool() const
        {
            return m_pos<m_end;
        }
        
        ~reader();

        reader& init();

        reader& strip_if_bom();

        reader& detect_eol();

        reader& detect_sep_quote_rule_nfields();

        reader& detect_field_names();

        reader& skip_if_white();
        
        reader& skip_field_content(
            const char**actualBegin=nullptr,
            const char**actualEnd=nullptr);

        reader& skip_field();

        reader& skip_sep();

        reader& skip_if_sep();

        reader& skip_eol();

        reader& skip_if_eol();

        reader& skip_record(int&);

        reader& skip_record();

        reader& read_field(std::string& content);

        bool is_sep() const;
        bool is_eol() const;
        bool is_end() const;
        bool is_eol_or_end() const;

        std::string read_field();

        reader& read_record(std::vector<std::string>& content);
        std::vector<std::string> read_record();
        
        std::vector<char> get_eol() const
        {
            return m_eol;
        };
        
        size_type get_file_size() const
        {
            return m_file.size();
        };

        char get_sep() const
        {
            return m_sep;
        }

        quote_rule get_quote_rule() const
        {
            return m_quote_rule;
        }

        int get_nfields() const
        {
            return m_nfields;
        }

        std::vector<std::string> get_field_names() const
        {
            return m_field_names;
        }
        
    private:
        const std::string m_filename;

        boost::iostreams::mapped_file_source m_file;
        const char *m_pos = nullptr;
        const char *m_begin = nullptr;
        const char *m_end = nullptr;
        int m_nfields = -1;

        std::vector<char> m_eol;
        char m_sep{'\0'};
        bool m_fill{false};
        quote_rule m_quote_rule{quote_rule::doubled};
        bool m_strip_white{false};
        char m_quote{'"'};
        bool m_blankIsANAString{false};
        std::vector<std::string> m_NAStrings{{"NA"}};
        std::vector<std::string> m_field_names{};
        bool m_verbose{true};
    };
}


#endif
