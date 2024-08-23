#pragma once

#include <Windows.h>
#include <kbd.h>

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

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

struct WCharLiteral {
    wchar_t value;
    WCharLiteral(wchar_t value) : value(value) {}
};

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, WCharLiteral literal) {
    switch (literal.value) {
    case 0: stream << "0"; break;
    case '\t': stream << "L'\\t'"; break;
    case '\r': stream << "L'\\r'"; break;
    case '\n': stream << "L'\\n'"; break;
    case '\'': stream << "L'\\''"; break;
    case WCH_NONE: stream << STRINGIFY(WCH_NONE); break;
    case WCH_DEAD: stream << STRINGIFY(WCH_DEAD); break;
    case WCH_LGTR: stream << STRINGIFY(WCH_LGTR); break;
    default:
        char utf8[6] = {};
        if (literal.value >= 0x20
            && (literal.value < 0xD800 || literal.value > 0xDFFF) // Surrogate
            && (literal.value < 0xE000 || literal.value > 0xF8FF) // Private use
            && WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &literal.value, 1, &utf8[0], sizeof(utf8), nullptr, nullptr)) {
            stream << "L'";
            stream << utf8;
            stream << "'";
        }
        else {
            stream << HexLiteral((USHORT)literal.value);
        }
        break;
    }
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

const char* getVKConstant(BYTE value) {
#define VK_CONSTANTS_CASE(name) case name: return #name;
    switch (value) {
        VK_CONSTANTS_CASE(VK_LBUTTON) // 0x01
        VK_CONSTANTS_CASE(VK_RBUTTON) // 0x02
        VK_CONSTANTS_CASE(VK_CANCEL) // 0x03
        VK_CONSTANTS_CASE(VK_BACK) // 0x08
        VK_CONSTANTS_CASE(VK_TAB) // 0x09
        VK_CONSTANTS_CASE(VK_CLEAR) // 0x0C
        VK_CONSTANTS_CASE(VK_RETURN) // 0x0D
        VK_CONSTANTS_CASE(VK_SHIFT) // 0x10
        VK_CONSTANTS_CASE(VK_CONTROL) // 0x11
        VK_CONSTANTS_CASE(VK_MENU) // 0x12
        VK_CONSTANTS_CASE(VK_PAUSE) // 0x13
        VK_CONSTANTS_CASE(VK_CAPITAL) // 0x14
        VK_CONSTANTS_CASE(VK_KANA) // 0x15
        // VK_CONSTANTS_CASE(VK_HANGUL) // 0x15
        VK_CONSTANTS_CASE(VK_IME_ON) // 0x16
        VK_CONSTANTS_CASE(VK_JUNJA) // 0x17
        VK_CONSTANTS_CASE(VK_FINAL) // 0x18
        // VK_CONSTANTS_CASE(VK_HANJA) // 0x19
        VK_CONSTANTS_CASE(VK_KANJI) // 0x19
        VK_CONSTANTS_CASE(VK_IME_OFF) // 0x1A
        VK_CONSTANTS_CASE(VK_ESCAPE) // 0x1B
        VK_CONSTANTS_CASE(VK_CONVERT) // 0x1C
        VK_CONSTANTS_CASE(VK_NONCONVERT) // 0x1D
        VK_CONSTANTS_CASE(VK_ACCEPT) // 0x1E
        VK_CONSTANTS_CASE(VK_MODECHANGE) // 0x1F
        VK_CONSTANTS_CASE(VK_SPACE) // 0x20
        VK_CONSTANTS_CASE(VK_PRIOR) // 0x21
        VK_CONSTANTS_CASE(VK_NEXT) // 0x22
        VK_CONSTANTS_CASE(VK_END) // 0x23
        VK_CONSTANTS_CASE(VK_HOME) // 0x24
        VK_CONSTANTS_CASE(VK_LEFT) // 0x25
        VK_CONSTANTS_CASE(VK_UP) // 0x26
        VK_CONSTANTS_CASE(VK_RIGHT) // 0x27
        VK_CONSTANTS_CASE(VK_DOWN) // 0x28
        VK_CONSTANTS_CASE(VK_SELECT) // 0x29
        VK_CONSTANTS_CASE(VK_PRINT) // 0x2A
        VK_CONSTANTS_CASE(VK_EXECUTE) // 0x2B
        VK_CONSTANTS_CASE(VK_SNAPSHOT) // 0x2C
        VK_CONSTANTS_CASE(VK_INSERT) // 0x2D
        VK_CONSTANTS_CASE(VK_DELETE) // 0x2E
        VK_CONSTANTS_CASE(VK_HELP) // 0x2F
        VK_CONSTANTS_CASE(VK_LWIN) // 0x5B
        VK_CONSTANTS_CASE(VK_RWIN) // 0x5C
        VK_CONSTANTS_CASE(VK_APPS) // 0x5D
        VK_CONSTANTS_CASE(VK_SLEEP) // 0x5F
        VK_CONSTANTS_CASE(VK_NUMPAD0) // 0x60
        VK_CONSTANTS_CASE(VK_NUMPAD1) // 0x61
        VK_CONSTANTS_CASE(VK_NUMPAD2) // 0x62
        VK_CONSTANTS_CASE(VK_NUMPAD3) // 0x63
        VK_CONSTANTS_CASE(VK_NUMPAD4) // 0x64
        VK_CONSTANTS_CASE(VK_NUMPAD5) // 0x65
        VK_CONSTANTS_CASE(VK_NUMPAD6) // 0x66
        VK_CONSTANTS_CASE(VK_NUMPAD7) // 0x67
        VK_CONSTANTS_CASE(VK_NUMPAD8) // 0x68
        VK_CONSTANTS_CASE(VK_NUMPAD9) // 0x69
        VK_CONSTANTS_CASE(VK_MULTIPLY) // 0x6A
        VK_CONSTANTS_CASE(VK_ADD) // 0x6B
        VK_CONSTANTS_CASE(VK_SEPARATOR) // 0x6C
        VK_CONSTANTS_CASE(VK_SUBTRACT) // 0x6D
        VK_CONSTANTS_CASE(VK_DECIMAL) // 0x6E
        VK_CONSTANTS_CASE(VK_DIVIDE) // 0x6F
        VK_CONSTANTS_CASE(VK_F1) // 0x70
        VK_CONSTANTS_CASE(VK_F2) // 0x71
        VK_CONSTANTS_CASE(VK_F3) // 0x72
        VK_CONSTANTS_CASE(VK_F4) // 0x73
        VK_CONSTANTS_CASE(VK_F5) // 0x74
        VK_CONSTANTS_CASE(VK_F6) // 0x75
        VK_CONSTANTS_CASE(VK_F7) // 0x76
        VK_CONSTANTS_CASE(VK_F8) // 0x77
        VK_CONSTANTS_CASE(VK_F9) // 0x78
        VK_CONSTANTS_CASE(VK_F10) // 0x79
        VK_CONSTANTS_CASE(VK_F11) // 0x7A
        VK_CONSTANTS_CASE(VK_F12) // 0x7B
        VK_CONSTANTS_CASE(VK_F13) // 0x7C
        VK_CONSTANTS_CASE(VK_F14) // 0x7D
        VK_CONSTANTS_CASE(VK_F15) // 0x7E
        VK_CONSTANTS_CASE(VK_F16) // 0x7F
        VK_CONSTANTS_CASE(VK_F17) // 0x80
        VK_CONSTANTS_CASE(VK_F18) // 0x81
        VK_CONSTANTS_CASE(VK_F19) // 0x82
        VK_CONSTANTS_CASE(VK_F20) // 0x83
        VK_CONSTANTS_CASE(VK_F21) // 0x84
        VK_CONSTANTS_CASE(VK_F22) // 0x85
        VK_CONSTANTS_CASE(VK_F23) // 0x86
        VK_CONSTANTS_CASE(VK_F24) // 0x87
        VK_CONSTANTS_CASE(VK_NUMLOCK) // 0x90
        VK_CONSTANTS_CASE(VK_SCROLL) // 0x91
        VK_CONSTANTS_CASE(VK_LSHIFT) // 0xA0
        VK_CONSTANTS_CASE(VK_RSHIFT) // 0xA1
        VK_CONSTANTS_CASE(VK_LCONTROL) // 0xA2
        VK_CONSTANTS_CASE(VK_RCONTROL) // 0xA3
        VK_CONSTANTS_CASE(VK_LMENU) // 0xA4
        VK_CONSTANTS_CASE(VK_RMENU) // 0xA5
        VK_CONSTANTS_CASE(VK_BROWSER_BACK) // 0xA6
        VK_CONSTANTS_CASE(VK_BROWSER_FORWARD) // 0xA7
        VK_CONSTANTS_CASE(VK_BROWSER_REFRESH) // 0xA8
        VK_CONSTANTS_CASE(VK_BROWSER_STOP) // 0xA9
        VK_CONSTANTS_CASE(VK_BROWSER_SEARCH) // 0xAA
        VK_CONSTANTS_CASE(VK_BROWSER_FAVORITES) // 0xAB
        VK_CONSTANTS_CASE(VK_BROWSER_HOME) // 0xAC
        VK_CONSTANTS_CASE(VK_VOLUME_MUTE) // 0xAD
        VK_CONSTANTS_CASE(VK_VOLUME_DOWN) // 0xAE
        VK_CONSTANTS_CASE(VK_VOLUME_UP) // 0xAF
        VK_CONSTANTS_CASE(VK_MEDIA_NEXT_TRACK) // 0xB0
        VK_CONSTANTS_CASE(VK_MEDIA_PREV_TRACK) // 0xB1
        VK_CONSTANTS_CASE(VK_MEDIA_STOP) // 0xB2
        VK_CONSTANTS_CASE(VK_MEDIA_PLAY_PAUSE) // 0xB3
        VK_CONSTANTS_CASE(VK_LAUNCH_MAIL) // 0xB4
        VK_CONSTANTS_CASE(VK_LAUNCH_MEDIA_SELECT) // 0xB5
        VK_CONSTANTS_CASE(VK_LAUNCH_APP1) // 0xB6
        VK_CONSTANTS_CASE(VK_LAUNCH_APP2) // 0xB7
        VK_CONSTANTS_CASE(VK_OEM_1) // 0xBA
        VK_CONSTANTS_CASE(VK_OEM_PLUS) // 0xBB
        VK_CONSTANTS_CASE(VK_OEM_COMMA) // 0xBC
        VK_CONSTANTS_CASE(VK_OEM_MINUS) // 0xBD
        VK_CONSTANTS_CASE(VK_OEM_PERIOD) // 0xBE
        VK_CONSTANTS_CASE(VK_OEM_2) // 0xBF
        VK_CONSTANTS_CASE(VK_OEM_3) // 0xC0
        VK_CONSTANTS_CASE(VK_OEM_4) // 0xDB
        VK_CONSTANTS_CASE(VK_OEM_5) // 0xDC
        VK_CONSTANTS_CASE(VK_OEM_6) // 0xDD
        VK_CONSTANTS_CASE(VK_OEM_7) // 0xDE
        VK_CONSTANTS_CASE(VK_OEM_8) // 0xDF
        VK_CONSTANTS_CASE(VK_OEM_AX) // 0xE1
        VK_CONSTANTS_CASE(VK_OEM_102) // 0xE2
        VK_CONSTANTS_CASE(VK_ICO_HELP) // 0xE3
        VK_CONSTANTS_CASE(VK_ICO_00) // 0xE4
        VK_CONSTANTS_CASE(VK_PROCESSKEY) // 0xE5
        VK_CONSTANTS_CASE(VK_ICO_CLEAR) // 0xE6
        VK_CONSTANTS_CASE(VK_PACKET) // 0xE7
        VK_CONSTANTS_CASE(VK_OEM_RESET) // 0xE9
        VK_CONSTANTS_CASE(VK_OEM_JUMP) // 0xEA
        VK_CONSTANTS_CASE(VK_OEM_PA1) // 0xEB
        VK_CONSTANTS_CASE(VK_OEM_PA2) // 0xEC
        VK_CONSTANTS_CASE(VK_OEM_PA3) // 0xED
        VK_CONSTANTS_CASE(VK_OEM_WSCTRL) // 0xEE
        VK_CONSTANTS_CASE(VK_OEM_CUSEL) // 0xEF
        VK_CONSTANTS_CASE(VK_OEM_ATTN) // 0xF0
        VK_CONSTANTS_CASE(VK_OEM_FINISH) // 0xF1
        VK_CONSTANTS_CASE(VK_OEM_COPY) // 0xF2
        VK_CONSTANTS_CASE(VK_OEM_AUTO) // 0xF3
        VK_CONSTANTS_CASE(VK_OEM_ENLW) // 0xF4
        VK_CONSTANTS_CASE(VK_OEM_BACKTAB) // 0xF5
        VK_CONSTANTS_CASE(VK_ATTN) // 0xF6
        VK_CONSTANTS_CASE(VK_CRSEL) // 0xF7
        VK_CONSTANTS_CASE(VK_EXSEL) // 0xF8
        VK_CONSTANTS_CASE(VK_EREOF) // 0xF9
        VK_CONSTANTS_CASE(VK_PLAY) // 0xFA
        VK_CONSTANTS_CASE(VK_ZOOM) // 0xFB
        VK_CONSTANTS_CASE(VK_NONAME) // 0xFC
        VK_CONSTANTS_CASE(VK_PA1) // 0xFD
        VK_CONSTANTS_CASE(VK_OEM_CLEAR) // 0xFE
        default: return nullptr;
    }
#undef VK_CONSTANTS_CASE
}

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, VirtualKeyLiteral literal) {
    auto value = literal.value;
    if ((value >= 'A' && value <= 'Z') || (value >= '0' && value <= '9'))
        stream << '\'' << (char)value << '\'';
    else {
        auto virtualKeyCode = (BYTE)value;
        auto constant = getVKConstant(virtualKeyCode);
        if (constant != nullptr) stream << constant;
        else stream << HexLiteral(virtualKeyCode);

        if (value & KBDEXT) stream << " | " << STRINGIFY(KBDEXT);
        if (value & KBDMULTIVK) stream << " | " << STRINGIFY(KBDMULTIVK);
        if (value & KBDSPECIAL) stream << " | " << STRINGIFY(KBDSPECIAL);
    }
    return stream;
}

struct VKToWCharsAttributesLiteral {
    BYTE value;
    VKToWCharsAttributesLiteral(BYTE value) : value(value) {}
};

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, VKToWCharsAttributesLiteral literal) {
    if (literal.value == 0) {
        stream << "0";
        return stream;
    }

    auto remaining = literal.value;
    bool first = true;
    auto handle = [&remaining, &first, &stream]( BYTE flag, const char* name) {
        if (!(remaining & flag)) return;
        if (!first) stream << " | ";
        stream << name;
        first = false;
        remaining &= ~flag;
    };

    handle(CAPLOK, STRINGIFY(CAPLOK));
    handle(SGCAPS, STRINGIFY(SGCAPS));
    handle(CAPLOKALTGR, STRINGIFY(CAPLOKALTGR));
    handle(KANALOK, STRINGIFY(KANALOK));
    handle(GRPSELTAP, STRINGIFY(GRPSELTAP));
    if (remaining) {
        if (!first) stream << " | ";
        stream << HexLiteral(remaining);
    }

    return stream;
}