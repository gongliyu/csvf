#ifndef __CSVF_READER_HPP
#define __CSVF_READER_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstddef>
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

    /**
     * class to read csv file
     */
    class reader
    {
    public:
        /** unsigned interge type to represent size */
        using size_type = size_t;

        /**
         * type to represent field content
         *
         * The field type is std::string_view if the compiler supports
         * it. Otherwise, it will be boost::string_view.
         */
        using field_type = boost::string_view;

        /**
         * type to represent record content
         */
        using record_type = std::vector<field_type>;

        /**
         * enum class to represent quote rule
         * 
         * @details there are four possible values for quote rule:
         *
         * 1. doubled \n Quotes inside fields are doubled. For
         * example: <<...,"hello ""world""",...>> 
         * 
         * 2. escaped \n Quotes inside fields are escaped with a
         * backslash. For example: <<...,"hello \"world\"",...>>
         *
         * 3. verbatim \n Quotes inside fields are not escaped, but
         * appears verbatim. In this case, the reader will check the
         * following character to see if the quote marks the end of
         * field. If the quote is followed by the field separator,
         * then it is the end of field, otherwise it is assumed to be
         * quote inside field. For example: <<...,"hello
         * "world"",...>>
         *
         * 4. none \n Fields are not quoted at all. Any quote characters appearing anywhere inside fields will be treated as any other regular characters. For example: <<...,hello "world",...>>
         */
        enum class quote_rule_type {
            doubled,
            escaped,
            verbatim,
            none
        };

        /** @name Constructors and destructors
         */
        /**@{*/
        /** default constructor */
        reader() = default;

        /** virtual destructor */
        virtual ~reader();

        /**
         * constructor a reader object from a file this constructor
         * will call member function open to open the file specified
         * by fname.
         *
         * @param fname a string specifies the path of the csv file
         * @args other arguments passed to open
         */
        template <typename... Args>
        explicit reader(const std::string& fname, Args... args)
        {
            open(fname, args...);
        }
        /**@}*/

        /**
         * open a csv file
         *
         * @param fname a string specifies the path of the csv file
         * @args options in string-value pairs
         *
         * @example 
         */
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
            ptrdiff_t begin_offset=-1, end_offset=-1;
            options::assign(opts,
                            "begin_offset", begin_offset,
                            "end_offset", end_offset,
                            "quote_rule", m_quote_rule,
                            "eol", m_eol,
                            "sep", m_sep,
                            "fill", m_fill,
                            "strip_white", m_strip_white,
                            "skip_blank_lines", m_skip_blank_lines,
                            "quote", m_quote,
                            "verbose", m_verbose);

            m_begin = m_file.data();
            m_end = m_file.data() + m_file.size();
            strip_if_bom();
            detect_eol();
            strip_space();
            detect_sep_quote_rule_nfields();

            if (begin_offset > 0) {
                m_begin = m_file.data() + begin_offset;
            }

            if (end_offset > 0) {
                m_end = m_file.data() + end_offset;
            }
                
            
            return *this;
        }

        /**
         * @return whether the end is reached
         */
        operator bool() const
        {
            return m_pos<m_end;
        }

        /**  
         * if there is bom bytes at the end of file, move the end
         * pointer backward to skip the bom bytes
         */
        reader& strip_if_bom();

        /**
         * detect end of line characters from the content of the file
         */
        reader& detect_eol();

        /**
         * if there are white spaces at the beginning of the file,
         * move the begin pointer forward to skip the space
         */
        reader& strip_space();

        /**
         * detect separator character, quote rule and number of fields
         * from the content
         */
        reader& detect_sep_quote_rule_nfields();

        /**
         * if the current point is at a white space character, move
         * the current pointer forward to skip it
         */
        reader& skip_if_white();

        /**
         * move the current pointer forward to skip field content
         * where the current pointer landed. 
         *
         * @param actualBegin record the actual beginning of field
         * content. See details for more information.
         *
         * @param actualEnd record the actual end of field
         * content. See details for more information.
         *
         * @note the current pointer must be at the beginning of a
         * field, otherwise an exception will be thrown
         *
         * @details if actualBegin and actualEnd is not a nullptr,
         * they will record the pointer to the address of the begin
         * and end of the field content, that's say [actualBegin,
         * actualEnd) will be the content. If m_strip_white is true,
         * the content will not contain heading and trailing space,
         * which means the interval will be shrinked.
         */
        reader& skip_field_content(
            const char**actualBegin=nullptr,
            const char**actualEnd=nullptr);

        /**
         * move the current pointer forward to skip the current field,
         * including the field separator
         *
         * @note the current pointer must be at the beginning of a
         * field, otherwise an exception will be thrown
         */
        reader& skip_field();

        /**
         * If the current position is a separator, skip
         * it. Otherwise, throw an exception.
         */
        reader& skip_sep();

        /**
         * If the current position is a separator, skip it.
         */
        reader& skip_if_sep();

        /**
         * If the current position is an end of line character, skip
         * it. Otherwise, throw an exception.
         */
        reader& skip_eol();

        /**
         * If the current position is an end of line character, skip
         * it.
         */
        reader& skip_if_eol();

        /**
         * Skip a record, including the EOL
         * @param nfields record the number of fields in the record
         *
         * @note The current position must be at the beginning of the
         * record, otherwise, an exception will be thrown.
         */
        /**@{*/
        reader& skip_record(int& nfields);
        reader& skip_record();
        /**@}*/

        /**
         * Check if the current position is a white space, separator,
         * end of line character. Return a bool value to indicate the
         * result.
         */
        /**@{*/
        bool is_white() const;
        bool is_sep() const;
        bool is_eol() const;
        bool is_end() const;
        bool is_begin() const;
        bool is_eol_or_end() const;
        /**@}*/

        /**
         * Read field content
         *
         * @note the current position must be at the beginning of the
         * field, otherwise, an exception will be thrown.
         */
        /**@{*/
        /**
         * @param field variable to store the field content
         */
        reader& read_field(std::string& field);
        /**
         * @return field content
         */
        field_type read_field();
        /**@}*/

        /**
         * read a record
         *
         * @note the current position must be at the beginning of the
         * record, otherwise, an exception will be thrown.
         */
        /**@{*/
        
        /**
         * @param record variable to store the content of the record
         */
        reader& read_record(record_type& record);
        /**
         * @return the content of the record
         */
        record_type read_record();
        /**@}*/

        /**
         * Move current position to the beginning of next record from
         * anywhere landed.
         */
        reader& anywhere_to_next_record_begin();

        /**
         * Get the end of line characters of this reader
         *
         * @return end of line characters
         */
        std::vector<char> eol() const
        {
            return m_eol;
        };

        /**
         * Get the file size
         *
         * @return file size
         */
        size_type file_size() const
        {
            return m_file.size();
        };

        /**
         * Get the separator character of this reader
         *
         * @return separator character
         */
        char sep() const
        {
            return m_sep;
        }

        /**
         * Get the quote rule of this reader
         *
         * @return quote rule
         */
        quote_rule_type quote_rule() const
        {
            return m_quote_rule;
        }

        /**
         * Get the number of fields detected by this reader
         *
         * @return number of fields
         */
        int nfields() const
        {
            return m_nfields;
        }

        /**
         * Get the field names detected by this reader
         *
         * @return field names
         */
        std::vector<std::string> field_names() const
        {
            return m_field_names;
        }

        ptrdiff_t begin_offset() const
        {
            return m_begin - m_file.data();
        }

        ptrdiff_t end_offset() const
        {
            return m_end - m_file.data();
        }

        ptrdiff_t pos_offset() const
        {
            return m_pos - m_file.data();
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
