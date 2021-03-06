#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <array>
#include <algorithm>
#include <cctype>
#include <csvf/reader.hpp>
#include <boost/format.hpp>

namespace
{
    template<typename T>
    std::basic_ostream<T>& operator<<(std::basic_ostream<T>& stream, csvf::reader::quote_rule_type rule)
    {
        switch(rule)
        {
        case csvf::reader::quote_rule_type::doubled:
            stream<<"doubled";
            break;
        case csvf::reader::quote_rule_type::escaped:
            stream<<"escaped";
            break;
        case csvf::reader::quote_rule_type::verbatim:
            stream<<"verbatim";
            break;
        case csvf::reader::quote_rule_type::none:
            stream<<"none";
            break;
        case csvf::reader::quote_rule_type::auto_detect:
            stream<<"auto_detect";
            break;
        default:
            std::logic_error("internal error");
        }
        return stream;
    }

    std::string sep2str(char sep)
    {
        if (sep==' ') return "blank";
        else if(sep=='\t') return "\\t";
        else if(sep=='\n') return "\\n";
        else return std::string(1,sep);
    }

    inline std::string make_source_error( std::string msg,
      char const* file, char const* function,
      std::size_t line
    ) {
        return std::string{} + file + "(" + std::to_string(line)
                                 + "): [" + function + "] " + msg;
    }
    
#define SOURCE_ERROR(...) make_source_error(__VA_ARGS__, __FILE__, __func__, __LINE__ )
    
}

namespace csvf
{
    reader::~reader()
    {
        m_file.close();
    }

    reader& reader::strip_if_bom()
    {
        assert(m_begin && m_end);
        if (m_verbose)
            std::cout << "detect and skip BOM" << std::endl;
        if(file_size() >= 3 &&
           std::memcmp(m_begin, "\xEF\xBB\xBF", 3) == 0)
        {
            m_begin += 3;
            if (m_verbose)
                std::cout<<" UTF-8 byte order mark EF BB BF found"
                    " at the start of the file and skipped."
                         <<std::endl;
        }
        else if(file_size() >= 4 &&
                std::memcmp(m_begin, "\x84\x31\x95\x33", 4) == 0)
        {
            m_begin += 4;
            if (m_verbose)
                std::cout<<"  GB-18030 byte order mark 84 31 95 33 "
                    "found at the start of the file and skipped."
                         <<std::endl;
        }
        else if (file_size() >= 2 &&
                 m_begin[0] + m_begin[1] == '\xFE' + '\xFF')
        {  // either 0xFE 0xFF or 0xFF 0xFE
            throw std::runtime_error(
                "File is encoded in UTF-16, this encoding is"
                " not supported by fread(). Please recode "
                "the file to UTF-8.");
        }
        if (m_end[-1] == '\x1A')
        {
            m_end--;
            if (m_verbose)
                std::cout<<"  Last byte of input found to be 0x1A "
                    "(Ctrl+Z) and removed.\n"<<std::endl;
        }
        m_pos = m_begin;
        return *this;
    }

    reader& reader::detect_eol()
    {
        if (m_verbose)
            std::cout<<"detect EOL"<<std::endl;
        m_eol.clear();
        const char *pos = m_begin;
        while (pos<m_end && *pos!='\n' && *pos!='\r') {
            if (*pos==m_quote)
                while (++pos<m_end && *pos!=m_quote) {};
            pos++;
        }

        if (pos>=m_end) {
            if (m_verbose) {
                std::cout<<"  Input ends before any \\r or \\n "
                    "observed. Input will be treated as a single row."
                         <<std::endl;
            }
            m_eol.push_back('\n');
        } else {
            m_eol.push_back(*pos);
            if(*pos=='\r') {
                if(pos+1<m_end && *(pos+1)=='\n') {
                    if (m_verbose)
                        std::cout<<"detected eol as \\r\\n (CRLF)"
                                 <<std::endl;
                    m_eol.push_back(*(pos+1));
                } else {
                    if(pos+1<m_end && *(pos+1)=='\r')
                        throw(std::runtime_error("line ending is\\r\\r\\n"));
                    if (m_verbose)
                        std::cout<<"detected eol as \\r only"
                                 <<std::endl;
                }
            }
            else if (*pos=='\n')
            {
                if(pos+1<m_end && *(pos+1)=='\r') {
                    if (m_verbose)
                        std::cout<<"detected eol as \\n\\r"<<std::endl;
                    m_eol.push_back('\r');
                }
                else if(m_verbose)
                    std::cout<<"detected eol as \\n"<<std::endl;
            }
            else
                throw std::logic_error("some logic error in the code");
        }
        return *this;
    }

    reader& reader::strip_space()
    {
        while (m_begin<m_end && isspace(*m_begin)) m_begin++;
        return *this;
    }
    
    reader& reader::detect_sep_quote_rule_nfields()
    {
        if (m_verbose)
            std::cout<<"detect separator, quoting rule, and nFields"
                     <<std::endl;
        // candidate seps in order of preference
        std::vector<char> seps(0);
        if (m_sep=='\0')
            seps = {',' , '|', ';', '\t', ' '};
        else
            seps.push_back(m_sep);
        
        // the most number of lines with the same number of
        // fields, so far
        int topNumLines = 0;

        // how many fields that was to resolve ties
        int topNumFields = 1;

        assert(!m_eol.empty());
        char topSep = m_eol[0];
        
        quote_rule_type topquote_rule = quote_rule_type::doubled;
        int topNmax = 1; // maximum number of columns

        // remember where the winning jumpline from jump 0 ends,
        // to know its size excluding header
        constexpr int JUMPLINES = 100;

        // group info store the number of fields and number of lines
        // for each contiguous group with the same number of fields
        // the first and second element in pairs are number of fields
        // and number of lines respectively.
        std::vector<quote_rule_type> rules(0);
        if (m_quote_rule == quote_rule_type::auto_detect) {
            rules = {
                quote_rule_type::doubled, quote_rule_type::escaped,
                quote_rule_type::verbatim, quote_rule_type::none};
        } else {
            rules.push_back(m_quote_rule);
        }
        
        for (auto sep : seps)
        {
            m_sep = sep;
            for (auto rule : rules)
            {
                m_quote_rule = rule;

                // analyse contiguous group for current candidate
                // sep and quote rule
                std::vector<std::pair<int,int>> group_info(0);
                m_pos = m_begin;
                int thisLine=0, lastncol=-1;
                while (m_pos<m_end && thisLine++<JUMPLINES)
                {
                    int thisncol = -1;
                    try
                    {
                        skip_record(thisncol);
                    }
                    catch(std::exception& e)
                    {
                        group_info.clear(); break;
                    }
                    if (thisncol!=lastncol)
                        group_info.emplace_back(thisncol, 0);
                    group_info.back().second++;
                                    lastncol = thisncol;
                }

                if (group_info.empty()) continue;
                bool updated = false;
                int nmax = 0; // max nfields in this iteration

                // find the contiguous group with the maximum
                // number of lines
                for (auto info : group_info)
                {
                    if (info.first > nmax) nmax=info.first;
                    if (info.first>1 &&
                        (info.second>topNumLines ||
                          (info.second==topNumLines &&
                           info.first>topNumFields &&
                           sep!=' ')))
                    {
                        topNumLines = info.second;
                        topNumFields = info.first;
                        topSep = sep;
                        topquote_rule = rule;
                        updated = true;
                    }
                }
                if (updated) topNmax = nmax;
            }
        }
        
        m_quote_rule = topquote_rule;
        m_sep = topSep;
        m_pos = m_begin;
        if (m_fill)
            m_nfields = topNmax;
        else
        {
            // find the top line with the consistent number of fields.
            // There might be irregular header lines above it.
            m_nfields = topNumFields;
            int thisLine=-1;
            while (m_pos<m_end && ++thisLine<JUMPLINES)
            {
                const char *pos = m_pos;
                int nFields;
                skip_record(nFields);
                skip_if_eol();
                if (nFields == m_nfields)
                {
                    m_pos = m_begin = pos;
                    break;
                }
            }
        }
    
        if (m_nfields<1)
            throw std::runtime_error("Internal error");
        int nfields;
        const char *pos = m_pos;
        skip_record(nfields);
        m_pos = pos;
        if (!m_fill && nfields!=m_nfields)
            throw std::logic_error("Internal error");

        if (m_verbose)
        {
            if (m_sep==m_eol[0])
                std::cout<<"Failed to detect sep, treat file as single column csv"<<std::endl;
            else
                std::cout
                    <<"detected sep="<<sep2str(topSep)
                    <<" quote rule="<<topquote_rule
                    <<" top nlines="<<topNumLines
                    <<" top nfields="<<topNumFields
                    <<std::endl;
        }
        return *this;
    }

    reader& reader::skip_if_white()
    {
        // skip space if it is not separator
        // and skip tab if it is not separator
        if (m_sep==' ')
            while (*m_pos=='\t') m_pos++;
        else if (m_sep=='\t')
            while (*m_pos==' ') m_pos++;
        else
            while (*m_pos==' ' || *m_pos=='\t') m_pos++;
        return *this;
    }


    reader& reader::skip_field_content(
        const char **nwBegin, const char**nwEnd)
    {
        if (is_end())
            throw std::runtime_error("end of file");
        // The current position should be at the begin of a field
        if(m_pos!=m_begin && *(m_pos-1)!=m_sep &&
               *(m_pos-1)!=m_eol.back())
            throw bad_format("Wrong field begin");

        if (m_strip_white) skip_if_white();
        bool quoted = false;

        // record actual content begin, remove quote char later
        if(nwBegin) *nwBegin = m_pos; 
      

        if (*m_pos!=m_quote || m_quote_rule==quote_rule_type::none)
        {
            // not quoted, looking for sep or eol
            while (m_pos<m_end && *m_pos!=m_sep && *m_pos!=m_eol[0])
                m_pos++;
        }
        else
        {
            int eolCount = 0;
            quoted = true;
            switch(m_quote_rule)
            {
            case quote_rule_type::doubled:
                // doubled quote, looking for closing quote
                while (++m_pos<m_end && eolCount<100)
                {
                    eolCount += (*m_pos==m_eol[0]);
                    if (*m_pos==m_quote)
                    {
                        if (m_pos+1<m_end && *(m_pos+1)==m_quote)
                            m_pos++;
                        else
                            break; // closing quote or last char
                    }
                }
                if (m_pos>=m_end || *m_pos!=m_quote)
                    throw std::logic_error("unknown quoting style");
                break;

            case quote_rule_type::escaped:
                while (++m_pos<m_end && *m_pos!=m_quote && eolCount<100)
                {
                    eolCount += (*m_pos==m_eol[0]);
                    m_pos += (*m_pos=='\\');
                }
                if (m_pos>=m_end || *m_pos!=m_quote)
                    throw std::logic_error("unknown quoting style");
                break;

            case quote_rule_type::verbatim:
                // verbatim quoting, looking for quote followed by
                // sep or eol
                quoted = false;
                while (++m_pos<m_end && *m_pos!=m_eol[0])
                {
                    if (*m_pos==m_quote &&
                        (m_pos+1>=m_end || *(m_pos+1)==m_sep ||
                         *(m_pos+1)==m_eol[0]))
                    {
                        quoted = true; break;
                    }
                    if (*m_pos==m_sep)
                    {
                        // first sep encountered, we need to
                        // decide whether it is a field end
                        // we look into the following sequence,
                        // if there is quote+sep, that quote+sep
                        // should be field end, otherwise, the current
                        // sep is field end.
                        const char *pos = m_pos;
                        while (++pos<m_end && *pos!=m_eol[0])
                        {
                            if (*pos==m_quote &&
                                (pos+1>=m_end || *(pos+1)==m_sep ||
                                 *(pos+1)==m_eol[0]))
                            {
                                m_pos = pos;  quoted=true; break;
                            }
                        }
                        break;
                    }// endif
                }//end while
            break;
            default:
                assert(false);
            }//end switch
        }//end if...else
        // we can only land on sep, quote, EOL, or EOF
        assert(m_pos>=m_end || *m_pos==m_sep ||
               *m_pos==m_eol[0] || *m_pos==m_quote);
              
        // record content end
        if(nwEnd) *nwEnd = m_pos;
        
        if (quoted)
        {
            assert(*m_pos==m_quote);
            assert(!nwBegin || **nwBegin==m_quote);
            assert(!nwEnd || **nwEnd==m_quote);
            m_pos++;

            // *nwEnd should be on char after quote
            if (nwEnd) (*nwEnd)++;
            if (m_strip_white) skip_if_white();
        }
        if (m_pos<m_end && *m_pos!=m_eol[0] && *m_pos!=m_sep)
            throw std::logic_error("parsing field failed.");
        assert(m_pos>=m_end || *m_pos==m_eol[0] || *m_pos==m_sep);
        
        return *this;
    }// end function

    reader& reader::skip_field()
    {
        skip_field_content();
        skip_if_sep();
        return *this;
    }
    
    reader& reader::skip_sep()
    {
        assert(*m_pos==m_sep);
        if (m_sep == m_eol[0]) return *this;

        m_pos++;
        if (m_sep==' ')
            while (m_pos<m_end && *m_pos==' ')
                m_pos++;
        return *this;
    }

    reader& reader::skip_if_sep()
    {
        if (m_sep!=m_eol[0] && *m_pos==m_sep) skip_sep();
        return *this;
    }

    reader& reader::skip_eol()
    {
        assert(*m_pos==m_eol[0]);
        m_pos += m_eol.size();
        assert(m_pos<=m_end);
        return *this;
    }

    reader& reader::skip_if_eol()
    {
        if (*m_pos==m_eol[0])
            skip_eol();
        return *this;
    }

    reader& reader::skip_record(int& nfields)
    {
        nfields = 0;
        while (!is_eol_or_end()) {
            skip_field();
            nfields++;
        }
        skip_if_eol();

        if (m_skip_blank_lines) {
            while (!is_end() &&
                   (is_eol() || (m_strip_white?is_white():false))) {
                if (m_strip_white) skip_if_white();
                skip_if_eol();
                if (m_strip_white) skip_if_white();
            }
        }

        return *this;
    }

    reader& reader::skip_record()
    {
        int nfield;
        return skip_record(nfield);
    }

    reader& reader::read_field(std::string& field)
    {
        field = static_cast<std::string>(read_field());
        return *this;
    }

    reader::field_type reader::read_field()
    {
        const char *contentBegin, *contentEnd;
        skip_field_content(&contentBegin, &contentEnd);
        skip_if_sep();
        assert(contentEnd >= contentBegin);
        if (contentEnd > contentBegin)
            return reader::field_type(contentBegin, contentEnd-contentBegin);
        else
            return reader::field_type("");
    }

    reader& reader::read_record(reader::record_type& content)
    {
        content.resize(nfields());
        int i=0;
        while (!is_eol_or_end())
        {
            if (i>=nfields())
                throw bad_format("read_record: nfield not match.");
            content[i] = read_field();
            i++;
        }
        assert(i<=nfields());
        if (i<nfields())
        {
            if (m_fill)
                std::fill(content.begin()+i, content.end(), "");
            else
                throw bad_format("nfields not match.");
        }
        skip_if_eol();

        if (m_skip_blank_lines) {
            while (!is_end() &&
                   (is_eol() || (m_strip_white?is_white():false))) {
                if (m_strip_white) skip_if_white();
                skip_if_eol();
                if (m_strip_white) skip_if_white();
            }
        }
        
        return *this;
    }

    reader::record_type reader::read_record()
    {
        reader::record_type record;
        read_record(record);
        return record;
    }

    bool reader::is_white() const
    {
        if (m_sep==' ') return *m_pos=='\t';
        else if (m_sep=='\t') return *m_pos==' ';
        else return *m_pos==' ' || *m_pos=='\t';
    }

    bool reader::is_sep() const
    {
        return m_pos<m_end && m_sep!=m_eol[0] && *m_pos==m_sep;
    }

    bool reader::is_eol() const
    {
        return m_pos<m_end && *m_pos==m_eol[0];
    }

    bool reader::is_end() const
    {
        return m_pos>=m_end;
    }

    bool reader::is_begin() const
    {
        return m_pos<=m_begin;
    }

    bool reader::is_eol_or_end() const
    {
        return m_pos>=m_end || *m_pos==m_eol[0];
    }

    reader& reader::anywhere_to_next_record_begin()
    {
        int ntries = 0;
        const char *pos = m_pos; // record current position
        while (ntries++<30) {
            // seek to an EOL
            if (is_end()) {
                return *this;
            }

            // move over EOL
            const char *record_begin = m_pos; // record begin
            int ngood_records = 0;
            while (ngood_records<5)
            {
                int nfields;
                try {
                    skip_record(nfields);
                } catch (...) {
                    break;
                }
                if (nfields != m_nfields) break;
                ngood_records++;
            }
            
            if (ngood_records == 5 || ngood_records > 0 && is_end()) {
                // succeed
                m_pos = record_begin;
                break;
            } else {
                // fail, try next
                m_pos = record_begin;
                // seek to eol for next possible record begin
                while (!is_eol_or_end())  m_pos++;
                skip_if_eol();
            }
        }
        return *this;
    }

    std::vector<ptrdiff_t> reader::chunk_uniformly(double& estimated_total_nrecords, int nchunks, int npositions, int nrecords_per_position)
    {
        const char *pos_original = m_pos;

        // sample positions, including the begin and end
        if (npositions < 2)
            npositions = std::max(101, 10*nchunks);
        std::vector<const char*> positions(0);
        positions.push_back(m_begin);
        for (int i=1; i<npositions-1; i++) {
            m_pos = m_begin + i*(m_end-m_begin)/(npositions-1);
            anywhere_to_next_record_begin();
            if (!(is_end() || m_pos==positions.back())) {
                positions.push_back(m_pos);
            }
        }

        // estimate density using nrecords_per_sample records
        std::vector<double> density(0);
        for (auto pos : positions) {
            m_pos = pos;
            double n = 0;
            while (!(is_end() || n > nrecords_per_position)) {
                skip_record();
                n++;
            }
            density.push_back(n/(m_pos-pos));
        }
        assert(positions.size()==density.size());


        // we have not yet implemented
        // anywhere_to_previous_record_begin() yet, so we assume the
        // density of m_end is the same as the sample point before it.
        positions.push_back(m_end);
        density.push_back(density.back());
        npositions = positions.size();

        // calculate the cumulative function by trapezoidal estimation
        std::vector<double> cumul(npositions, 0);
        for (int i=1; i<npositions; i++) {
            cumul[i] = cumul[i-1] + 0.5*(positions[i]-positions[i-1])*(density[i-1]+density[i]);
        }

        // std::cout<<"positions:"<<std::endl;
        // for (int i=0; i<positions.size(); i++)
        //     std::cout<<i+1<<":"<<positions[i]-m_file.data()<<"\t";
        // std::cout<<std::endl;

        // std::cout<<"density:"<<std::endl;
        // for (int i=0; i<density.size(); i++)
        //     std::cout<<i+1<<":"<<density[i]<<"\t";
        // std::cout<<std::endl;

        // std::cout<<"cumulative:"<<std::endl;
        // for (int i=0; i<cumul.size(); i++)
        //     std::cout<<i+1<<":"<<cumul[i]<<"\t";
        // std::cout<<std::endl;

        // find the quantiles
        estimated_total_nrecords = cumul.back();
        
        std::vector<ptrdiff_t> offsets(0);
        if (nchunks+1 >= npositions) {
            // there less npositions than required chunks, then return
            // the samples to user because this is the best we can do
            std::transform(positions.begin(), positions.end(),
                           std::back_inserter(offsets),
                           [this](const char* pos)->ptrdiff_t {
                               return pos-this->m_file.data();
                           });
        } else {
            offsets.push_back(positions[0]-m_file.data());
            int i=1;
            double quantile = estimated_total_nrecords / nchunks;
            double pre_diff = estimated_total_nrecords;
            while (quantile < estimated_total_nrecords) {
                double diff = std::abs(cumul[i]-quantile);
                if (diff > pre_diff) {
                    offsets.push_back(positions[i-1]-m_file.data());
                    quantile += estimated_total_nrecords / nchunks;
                }
                pre_diff = diff;
                i++;
            }
            offsets.push_back(m_end-m_file.data());
        }
        m_pos = pos_original;
        return std::move(offsets);
    }

    std::vector<ptrdiff_t> reader::chunk_uniformly(int nchunks, int npositions, int nrecords_per_position)
    {
        double tmp;
        return chunk_uniformly(tmp, nchunks, npositions, nrecords_per_position);
    }

    
    void reader::init_defaults()
    {
        m_pos = nullptr;
        m_begin = nullptr;
        m_end = nullptr;
        m_nfields = -1;
        m_quote_rule = quote_rule_type::auto_detect;
        m_eol.clear();
        m_sep = '\0';
        m_fill = true;
        m_strip_white = true;
        m_skip_blank_lines = true;
        m_quote = '"';
        m_verbose = true;
    }
}
