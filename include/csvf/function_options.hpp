#ifndef __FUNCTION_OPTIONS_HPP
#define __FUNCTION_OPTIONS_HPP

#include <map>
#include <type_traits>

#if __cplusplus < 201103L
    #error "function_options requires C++11 support"
#elif __cplusplus < 201701L
    #include <boost/any.hpp>
#endif

namespace
{
    template <typename To>
    To any_cast(const boost::any& from)
    {
        return boost::any_cast<To>(from);
    }

    template <> inline
    std::string any_cast<std::string>(const boost::any& from)
    {
        try {return boost::any_cast<std::string>(from);}
        catch(boost::bad_any_cast& e)
        {
            return boost::any_cast<const char*>(from);
        }
    }
}



#if __cplusplus >= 201701L
class function_options : public std::map<std::string,std::any>
#else
class function_options : public std::map<std::string,boost::any>
#endif
{
public:
    using type = function_options;
    
    type& parse(){ return *this;};

    template <typename T, typename... Args>
    typename
    std::enable_if<!std::is_convertible<T,const std::string&>::value,
                   type&>::type parse(T, Args...)
    {
        static_assert(std::is_convertible<T,const std::string&>::value,
                      "arguments of parse should be name-value pairs"
                      ", and name should be convertible to const std"
                      "::string. However, some of the names are not "
                      "convertible to const std::string& in the "
                      "current arguments.");
    };
    
    template <typename T, typename... Args>
    type& parse(const std::string& name, const T& value, Args... args)
    {
        //std::cout<<__FUNCTION__<<" "<<name<<std::endl;
        static_assert(sizeof...(Args) % 2 == 0,
                      "arguments of parse should be name-value"
                      " pairs. However, the current number of"
                      " arguments is an odd number.");
        
        emplace(name, value);
        parse(args...);
        return *this;
    };

    type& assign() {return *this;};

    template <typename T, typename... Args>
    typename
    std::enable_if<!std::is_convertible<T,const std::string&>::value,
                   type&>::type assign(T, Args...)
    {
        static_assert(std::is_convertible<T,const std::string&>::value,
                      "arguments of parse should be name-target pairs"
                      ", and name should be convertible to const std"
                      "::string. However, some of the names are not "
                      "convertible to const std::string& in the "
                      "current arguments.");
    };

    template <typename T, typename... Args>
    type& assign(const std::string& name, T& target, Args&... args)
    {
        //std::cout<<__FUNCTION__<<" "<<name<<std::endl;
        if (empty())
            return *this;
        static_assert(sizeof...(Args) % 2 == 0,
                      "arguments of assign should be name-target"
                      " pairs. However, the current number of"
                      " arguments is an odd number.");
        auto pos = find(name);
        if (pos!=end())
        {
            target = any_cast<T>(pos->second);
            erase(pos);
        }
        assign(args...);
        return *this;
    };
};

#endif
