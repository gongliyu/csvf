#ifndef __OPTIONS_HPP_55e671f8_49db_40c8_a21d_cf638f227531
#define __OPTIONS_HPP_55e671f8_49db_40c8_a21d_cf638f227531

#if __cplusplus < 201103L
    #error "options requires C++11 support"
#endif

#include <map>
#include <type_traits>

#if __cplusplus < 201701L
    #include <boost/any.hpp>
    #define __IMPL_NAMESPACE boost
#else
    #define __IMPL_NAMESPACE std
#endif



namespace options
{
    //************************************************************
    // many cast
    //************************************************************
    template <typename To>
    To many_cast(const __IMPL_NAMESPACE::any& from)
    {
       return __IMPL_NAMESPACE::any_cast<To>(from);
    }

    template <typename To, typename T, typename... Args>
    To many_cast(const __IMPL_NAMESPACE::any& from)
    {
        try
        {
            return __IMPL_NAMESPACE::any_cast<To>(from);
        }
        catch(const __IMPL_NAMESPACE::bad_any_cast& e)
        {
            return __IMPL_NAMESPACE::any_cast<T,Args...>(from);
        }
    }

    //************************************************************
    // many_static_cast
    //************************************************************
    template <typename To>
    To many_static_cast(const __IMPL_NAMESPACE::any& from)
    {
       return __IMPL_NAMESPACE::any_cast<To>(from);
    }

    template <typename To, typename T, typename... Args>
    typename std::enable_if<(sizeof...(Args)>0),To>::type
    many_static_cast(const __IMPL_NAMESPACE::any& from)
    {
        try
        {
            return static_cast<To>(
                __IMPL_NAMESPACE::any_cast<To>(from));
        }
        catch(const __IMPL_NAMESPACE::bad_any_cast& e)
        {
            return static_cast<To>(
                __IMPL_NAMESPACE::any_cast<Args...>(from));
        }
    }

    //************************************************************
    // option_cast
    //************************************************************
    template <typename To>
    To option_cast(const __IMPL_NAMESPACE::any& from)
    {
        return many_cast<To>(from);
    }

    template <> inline
    std::string option_cast<std::string>(
        const __IMPL_NAMESPACE::any& from)
    {
        return many_cast<std::string, const char*>(from);
    }

    //************************************************************
    // parse
    //************************************************************
    inline
    std::map<std::string,__IMPL_NAMESPACE::any> parse()
    {
        return std::map<std::string,__IMPL_NAMESPACE::any>();
    }

    template <typename T, typename... Args>
    std::map<std::string,__IMPL_NAMESPACE::any> parse(
        const std::string& name,
        const T& value, Args... args)
    {
        static_assert(sizeof...(Args) % 2 == 0,
                      "arguments of parse should be name-value"
                      " pairs. However, the current number of"
                      " arguments is an odd number.");
        return parse(args...).emplace(name, value);
    }

    //************************************************************
    // assign
    //************************************************************
    template <typename T>
    void assign(
        std::map<std::string,__IMPL_NAMESPACE::any>& opts,
        const std::string& name,
        T& to)
    {
        auto pos = opts.find(name);
        if (pos!=opts.end())
        {
            to = option_cast<T>(pos->second);
            opts.erase(pos);
        }
    }
    
    template <typename T, typename... Args>
    void assign(
        std::map<std::string,__IMPL_NAMESPACE::any>& opts,
        const std::string& name,
        T& to, Args... args)
    {
        assign(opts, name, to);
        assign(opts, args...);
    }
};

#endif
