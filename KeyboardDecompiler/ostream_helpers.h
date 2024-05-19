#pragma once

#include <Windows.h>
#include <kbd.h>

#include <iostream>
#include <string>
#include <string_view>

#define STRINGIFY(x) #x

template<typename U>
struct HexLiteral {
    U value;
    HexLiteral(U value) : value(value) {}
};

template<typename U>
std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, HexLiteral<U> literal) {
    stream << "0x";

    auto oldFlags = stream.flags();
    auto oldWidth = stream.width();
    auto oldFill = stream.fill();
    stream.setf(std::ios::hex, std::ios::basefield);
    stream.setf(std::ios::uppercase);
    stream.width(sizeof(U) * 2);
    stream.fill('0');

    if (sizeof(U) == 1) stream << (unsigned)literal.value;
    else stream << literal.value;

    stream.flags(oldFlags);
    stream.width(oldWidth);
    stream.fill(oldFill);
    return stream;
}

struct WStrLiteral {
    const wchar_t* value;
    WStrLiteral(const wchar_t* value) : value(value) {}
};

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, WStrLiteral literal) {
    if (literal.value) {
        stream << "L\"";
        for (auto wc : std::wstring_view(literal.value))
            stream << (char)wc;
        stream << "\"";
    }
    else {
        stream << "nullptr";
    }
    return stream;
}

struct VirtualKeyLiteral {
    USHORT value;
    VirtualKeyLiteral(USHORT value) : value(value) {}
};

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, VirtualKeyLiteral literal) {
    auto value = literal.value;
    if ((value >= 'A' && value <= 'Z') || (value >= '0' && value <= '9'))
        stream << '\'' << (char)value << '\'';
    else {
        auto virtualKeyCode = (BYTE)value;
        if (virtualKeyCode == 0xFF) stream << "VK__none_";
        else stream << HexLiteral(virtualKeyCode);

        if (value & KBDEXT) stream << " | " << STRINGIFY(KBDEXT);
        if (value & KBDMULTIVK) stream << " | " << STRINGIFY(KBDMULTIVK);
        if (value & KBDSPECIAL) stream << " | " << STRINGIFY(KBDSPECIAL);
    }
    return stream;
}