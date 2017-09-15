#ifndef __CSVF_READER_HPP
#define __CSVF_READER_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <stdexcept>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/utility/string_view.hpp>

#include "options.hpp"

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
        using field_type = boost::string_view;
        using record_type = std::vector<field_type>;
        enum class quote_rule_type {doubled, escaped, verbatim, none};

        reader() = default;
        virtual ~reader();

        template <typename... Args>
        explicit reader(const std::string& fname, Args... args)
        {
            open(fname, args...);
        }

        template <typename... Args>
        reader& open(const std::string& fname, Args... args)
        {
            m_filename = fname;
            m_file.close();
            m_file.open(fname);
            auto opts = options::parse(args...);
            m_sep = '\0';
            m_eol.clear();
            m_nfields = -1;
            options::assign(opts,
                            "sep", m_sep,
                            "fill", m_fill,
                            "quote_rule", m_quote_rule,
                            "strip_white", m_strip_white,
                            "quote", m_quote,
                            //"skip_blank_lines", m_skip_blank_lines,
                            "verbose", m_verbose);
            m_begin = m_file.data();
            m_end = m_file.data() + m_file.size();
            strip_if_bom();
            detect_eol();
            strip_space();
            detect_sep_quote_rule_nfields();
            return *this;
        }
        
        operator bool() const
        {
            return m_pos<m_end;
        }
        
        reader& strip_if_bom();

        reader& detect_eol();

        reader& strip_space();

        reader& detect_sep_quote_rule_nfields();

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

        bool is_white() const;
        bool is_sep() const;
        bool is_eol() const;
        bool is_end() const;
        bool is_eol_or_end() const;

        reader& read_field(std::string& field);
        field_type read_field();

        reader& read_record(record_type& record);
        record_type read_record();
        
        std::vector<char> eol() const
        {
            return m_eol;
        };
        
        size_type file_size() const
        {
            return m_file.size();
        };

        char sep() const
        {
            return m_sep;
        }

        quote_rule_type quote_rule() const
        {
            return m_quote_rule;
        }

        int nfields() const
        {
            return m_nfields;
        }

        std::vector<std::string> field_names() const
        {
            return m_field_names;
        }
        
    protected:
        std::string m_filename;
        boost::iostreams::mapped_file_source m_file;
        const char *m_pos = nullptr;
        const char *m_begin = nullptr;
        const char *m_end = nullptr;
        int m_nfields = -1;

        quote_rule_type m_quote_rule{quote_rule_type::doubled};
        
        std::vector<char> m_eol;
        char m_sep{'\0'};
        bool m_fill{true};

        bool m_strip_white{true};
        bool m_skip_blank_lines{true};
        char m_quote{'"'};
        //bool m_blankIsANAString{false};
        std::vector<std::string> m_NAStrings{{"NA"}};
        std::vector<std::string> m_field_names{};
        bool m_verbose{true};
    };
}


#endif
