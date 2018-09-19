#ifndef CPPUNITWSTRINGSUPPORT_H
#define CPPUNITWSTRINGSUPPORT_H

#include <iostream>       // std::cout, std::hex
#include <string>         // std::string, std::u32string
#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8
#include <cstdint>        // std::uint_least32_t



namespace CppUnit {
    template<>
    struct assertion_traits<std::wstring>
    {
        static bool equal(const std::wstring& x, const std::wstring& y)
        {
            return x == y;
        }
        
        static std::string toString(const std::wstring& ws)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
            return cv.to_bytes(ws);
        }
    };
};

#endif // CPPUNITWSTRINGSUPPORT_H
