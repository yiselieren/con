/*
 *  str_utils.cpp - Miscellaneous general std::string related routines
 *                  Most of interfaces inspired by Qt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "str_utils.h"

#define TELOPTS
#define TELCMDS
#include <arpa/telnet.h>

////////////////////////////////////////////////////////////////////////
char *str::vprintf(const char *format, va_list args)
{
    unsigned  max_str, max_buf = INIT_SIZE;
    char      *out_buf;

    while (1)
    {
        out_buf = new char[max_buf];
        max_str = max_buf - 1;

        if (::vsnprintf(out_buf, max_str, format, args) < (int)max_str)
            return out_buf;
        max_buf *= 2;
        if (max_buf > SIZE_LIMIT)
            return out_buf;
        delete [] out_buf;
    }
} // str::vprint

////////////////////////////////////////////////////////////////////////
char *str::printf(const char *format, ...)
{
    va_list   args;

    va_start(args, format);
    char *rc = vprintf(format, args);
    va_end(args);

    return rc;
} // str::print

////////////////////////////////////////////////////////////////////////
std::string str::sprintf(const char *format, ...)
{
    std::string rc;
    va_list     args;

    va_start(args, format);
    char *s = vprintf(format, args);
    va_end(args);
    rc = s;
    delete [] s;
    return rc;
} // str::sprintf

////////////////////////////////////////////////////////////////////////
std::string str::vsprintf(const char *format, va_list args)
{
    std::string rc;

    char *s = vprintf(format, args);
    rc = s;
    delete [] s;
    return rc;
} // str::vsprintf

////////////////////////////////////////////////////////////////////////
void str::sappend(std::string& str, const char *format, ...)
{
    va_list     args;

    va_start(args, format);
    char *s = vprintf(format, args);
    va_end(args);
    str += s;
    delete [] s;
} // str::sappend

////////////////////////////////////////////////////////////////////////
void str::vsappend(std::string& str, const char *format, va_list args)
{
    char *s = vprintf(format, args);
    str += s;
    delete [] s;
} // str::vsappend

////////////////////////////////////////////////////////////////////////
std::string str::escape(const std::string& str)
{
    const char  *p = str.c_str();
    std::string rc;

    if (str.empty())
        return rc;

    for (; *p; p++)
    {
        switch (*p)
        {
        case '\\':
            rc += "\\\\";
            break;
        case '\r':
            rc += "\\r";
            break;
        case '\n':
            rc += "\\n";
            break;
        case '\t':
            rc += "\\t";
            break;
        default:
            if (*p >= ' ' && *p <= '~')
                rc += *p;
            else
                rc += str::sprintf("\\x%02x", *p & 0xff);
        }
    }

    return rc;
} // escape

////////////////////////////////////////////////////////////////////////
std::string str::unescape(const std::string& str)
{
    enum Mode { NORMAL, ESCAPE, HEXA};
    const char   *p = str.c_str();
    char         c, *endp;
    std::string  rc;
    std::string  h;
    Mode         m = NORMAL;

    if (str.empty())
        return rc;

    for (; *p; p++)
    {
        switch (m)
        {
        case NORMAL:
            if (*p == '\\')
                m = ESCAPE;
            else
                rc += *p;
            break;
        case ESCAPE:
            switch (*p)
            {
            case '\\':
                rc += "\\";
                m = NORMAL;
                break;
            case 'r':
                rc += "\r";
                m = NORMAL;
                break;
            case 'n':
                rc += "\n";
                m = NORMAL;
                break;
            case 't':
                rc += "\t";
                m = NORMAL;
                break;
            case 'x':
                h.clear();
                m = HEXA;
                break;
            default:
                rc += "\\";
                rc += *p;
                m = NORMAL;
            }
            break;
        case HEXA:
            switch (h.length())
            {
            case 0:
            case 1:
                h += *p;
                break;
            default:
                c = strtol(h.c_str(), &endp, 16) & 0xff;
                if (*endp)
                {
                    rc += "\\x";
                    rc += h;
                }
                else
                    rc += c;
                h.clear();
                p--;
                m = NORMAL;
            }
            break;
        }
    }

    if (h.length() > 1)
    {
        c = strtol(h.c_str(), &endp, 16) & 0xff;
        if (*endp)
        {
            rc += "\\x";
            rc += h;
        }
        else
            rc += c;
    }

    return rc;
} // unescape

////////////////////////////////////////////////////////////////////////
static enum {
    R_NORMAL       = 0,
    R_TELNET_CMD   = 1,
    R_TELNET_OPT   = 2,
    R_TELNET_SB    = 3
} rstate = R_NORMAL;
char str::filter_telnet(const char c)
{
    unsigned int ch = c & 0xff;

    switch (rstate)
    {
    case R_NORMAL:
        if (ch == IAC)
            rstate = R_TELNET_CMD;
        else
            return c;
        break;
    case R_TELNET_CMD:
        if (TELCMD_OK(ch))
        {
            if (ch == IAC)
            {
                rstate = R_NORMAL;  // Two IACs means IAC as data
                return c;
            }
            else
                rstate = R_TELNET_OPT;
        }
        else
            rstate = R_NORMAL;     // Wrong telnet command
        break;
    case R_TELNET_OPT:
        if (TELOPT_OK(ch))
        {
            if (ch == SB)
                rstate = R_TELNET_SB;
            else
                rstate = R_NORMAL;  // end of "IAC, Command, Option" sequence
        }
        else
            rstate = R_NORMAL;     // Wrong telnet option
        break;
    case R_TELNET_SB:
        if (ch == SE)
            rstate = R_NORMAL;  // subnegotiation ends
        break;
    }
    return 0;
} // filter_telnet

////////////////////////////////////////////////////////////////////////
static int cstate = 1;
char str::filter_colors(const char c)
{
    if (cstate)
    {
        if (c == '\033')
            cstate = 0;
        else
            return c;
    }
    else
    {
        if (c == 'm')
            cstate = 1;
    }
    return 0;
} // filter_colors

////////////////////////////////////////////////////////////////////////
std::string str::trim_right(const std::string &s, const char *delimiters)
{
    std::string::size_type st = s.find_last_not_of(delimiters);
    if (!st || st == std::string::npos)
        return std::string();
    return s.substr(0, st + 1 >= s.length() ? std::string::npos : st + 1);
}

////////////////////////////////////////////////////////////////////////
std::string str::trim_left(const std::string &s, const char *delimiters)
{
    std::string::size_type st = s.find_first_not_of(delimiters);
    if (st == std::string::npos)
        return std::string();
    return s.substr(st);
}

////////////////////////////////////////////////////////////////////////
std::string str::trim(const std::string &s, const char *delimiters)
{
    std::string::size_type st1 = s.find_first_not_of(delimiters);
    if (st1 == std::string::npos)
        return std::string();

    std::string::size_type st2 = s.find_last_not_of(delimiters);
    if (st2 == std::string::npos)
        return std::string();
    if (st2 < st1)
        return std::string();
    return s.substr(st1, st2 + 1 >= s.size() ? std::string::npos : st2 + 1 - st1);
}

////////////////////////////////////////////////////////////////////////
bool str::equals(const std::string &str1, const std::string &str2)
{
    if (str1.length() != str2.length())
        return false;

    std::string::const_iterator i1 = str1.begin();
    std::string::const_iterator e1 = str1.end();
    std::string::const_iterator i2 = str2.begin();
    std::string::const_iterator e2 = str2.end();
    for (; i1 != e1 && i2 != e2; ++i1, ++i2)
        if (tolower(*i1) != tolower(*i2))
            return false;

    return true;
}

////////////////////////////////////////////////////////////////////////
bool str::starts_with(const std::string &str, const std::string &prefix)
{
    if (prefix.length() > str.length())
        return false;
    return strncasecmp(str.c_str(), prefix.c_str(), std::min(str.length(), prefix.length())) == 0;
}

////////////////////////////////////////////////////////////////////////
bool str::ends_with(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
        return false;
    return str::equals(str.substr(str.length() - suffix.length()), suffix);
}

////////////////////////////////////////////////////////////////////////
str::regexp::~regexp()
{
    if (_compiled)
        regfree(&_re);
    if (_error)
        delete [] _error;
    if (_pattern)
        free(_pattern);
}

////////////////////////////////////////////////////////////////////////
bool str::regexp::set_pattern(const char *pattern, const int flags)
{
    if (_error)
        delete [] _error;
    if (_pattern)
        free(_pattern);
    _pattern = strdup(pattern);

    int rc = regcomp(&_re, _pattern, flags | REG_EXTENDED);
    _compiled = true;

    if (rc)
    {
        const unsigned INIT_BUF = 1024;
        unsigned maxBuf = INIT_BUF;
        char *outBuf;

        while (true)
        {
            unsigned maxStr = maxBuf - 1;

            outBuf = new char[maxBuf];

            if (regerror(rc, &_re, outBuf, maxStr) < maxStr)
            {
                _error = outBuf;
                break;
            }

            delete [] outBuf;

            maxBuf *= 2;
        }
        _valid = false;
    }
    else
        _valid = true;

    return _valid;
}

////////////////////////////////////////////////////////////////////////
bool str::regexp::match(const std::string &s)
{
    // Regex is not valid (or never been compiled).
    if (!_compiled || !_valid)
        return false;

    _matches.clear();
    regmatch_t match[_re.re_nsub + 1];
    if (regexec(&_re, s.c_str(), _re.re_nsub + 1, match, 0))
        return false;

    for (unsigned i = 0; i < _re.re_nsub + 1; ++i)
    {
        if (match[i].rm_so == -1)
            break;
        _matches.push_back(s.substr(match[i].rm_so, match[i].rm_eo - match[i].rm_so));
    }
    return true;
}

////////////////////////////////////////////////////////////////////////
bool str::regexp::exec(const std::string &s)
{
    // Regex is not valid (or never been compiled).
    if (!_compiled || !_valid)
        return false;

    _matches.clear();
    return !regexec(&_re, s.c_str(), 0, NULL, 0);
}

////////////////////////////////////////////////////////////////////////
std::string str::regexp::cap(const unsigned n) const
{
    // Regex is not valid (or never been compiled).
    if (!_compiled || !_valid)
        return "";
    if (n >= _matches.size())
        return "";
    return _matches[n];
}

////////////////////////////////////////////////////////////////////////
template<class Container>
Container str::regexp::split(const std::string &s)
{
    Container container;
    regmatch_t match[1];

    // Special case - regex is not valid (or never been compiled).
    if (!_compiled || !_valid)
    {
        container.push_back(s);
        return container;
    }

    std::string str = s;
    while (!regexec(&_re, str.c_str(), 1, match, 0))
    {
        if (match[0].rm_so != -1  &&  match[0].rm_so > 0)
            container.push_back(str.substr(0, match[0].rm_so));
        str = str.substr(match[0].rm_eo);
        if (str.empty())
            return container;
    }

    container.push_back(str);
    return container;
}
