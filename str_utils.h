/*
 *  str_utils.h - Miscellaneous general std::string related routines
 *                Most of interfaces inspired by Qt
 *
 */

#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <stdarg.h>
#include <sys/types.h>
#include <regex.h>

#include <string>
#include <vector>
#include <list>

namespace str
{
    const size_t INIT_SIZE = 256;
    const size_t SIZE_LIMIT = 1048576;

    // don't forget to delete char* ptr returned from this!!!
    char *printf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

    // don't forget to delete char* ptr returned from this!!!
    char *vprintf(const char *format, va_list args);

    std::string sprintf(const char *format, ...) __attribute__ ((format(printf, 1, 2)));
    std::string vsprintf(const char *format, va_list args);

    std::string escape(const std::string& str);
    std::string unescape(const std::string& str);

    void sappend(std::string& str, const char *format, ...) __attribute__ ((format (printf, 2, 3)));
    void vsappend(std::string& str, const char *format, va_list args);

    char filter_telnet(const char c);
    char filter_colors(const char c);

    bool equals(const std::string &str1, const std::string &str2);
    bool starts_with(const std::string &str, const std::string &prefix);
    bool ends_with(const std::string &str, const std::string &suffix);

    std::string trim_right(const std::string &s, const char *delimiters = " \t\r\n");
    std::string trim_left(const std::string &s, const char *delimiters = " \t\r\n");
    std::string trim(const std::string &s, const char *delimiters = " \t\r\n");

    class regexp
    {

    public:

        regexp()
            : _pattern(0)
            , _error(0)
            , _valid(false)
            , _compiled(false)
            , _re()
            , _matches()
            { }
        regexp(const char *pattern, const int flag = 0)
            : _pattern(0)
            , _error(0)
            , _valid(false)
            , _compiled(false)
            , _re()
            , _matches()
        {
            set_pattern(pattern, flag);
        }

        ~regexp();

        const char *error() const    { return _error;          }
        const char *pattern() const  { return _pattern;        }
        bool compiled() const        { return _compiled;       }
        bool valid() const           { return _valid;          }
        unsigned cap_count() const   { return _matches.size(); }

        bool set_pattern(const char *pattern, const int flags = 0);
        bool match(const std::string &s);
        bool exec(const std::string &s);

        std::string cap(const unsigned n) const;

        template<class Container> Container split(const std::string &s);

        std::list<std::string> split_l(const std::string &s)   { return split<std::list<std::string> >(s);   }
        std::vector<std::string> split_v(const std::string &s) { return split<std::vector<std::string> >(s); }

    private:

        char                     *_pattern;
        char                     *_error;
        bool                     _valid;
        bool                     _compiled;
        regex_t                  _re;
        std::vector<std::string> _matches;
    };
};

#endif
