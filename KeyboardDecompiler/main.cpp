#include <Windows.h>
#include <kbd.h>
#include <string>
#include <string_view>
#include <filesystem>
#include <exception>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <span>

#include "ostream_helpers.h"

using namespace std;

static const char* dllExportName = "KbdLayerDescriptor";
static string_view indent = "    ";

void WriteVirtualKeyToBitTable(const VK_TO_BIT table[], string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(VK_TO_BIT) << " " << name << "[] = {" << "\n";
    for (auto iterator = table; iterator->Vk != 0; ++iterator) {
        stream << indent << "{ ";
        stream << VirtualKeyLiteral(iterator->Vk);
        stream << ", ";
        switch (iterator->ModBits) {
        case KBDSHIFT: stream << STRINGIFY(KDBSHIFT); break;
        case KBDCTRL: stream << STRINGIFY(KBDCTRL); break;
        case KBDALT: stream << STRINGIFY(KDBALT); break;
        default: stream << HexLiteral(iterator->ModBits); break;
        }
        stream << " }" << "\n";
    }
    stream << indent << "{ 0, 0 }" << "\n";
    stream << "};\n\n";
}

void WriteModifiersStruct(const MODIFIERS& modifiers, string_view name, ostream& stream) {
    if (modifiers.pVkToBit) WriteVirtualKeyToBitTable(modifiers.pVkToBit, "vk_to_bits", stream);

    stream << "static " << STRINGIFY(MODIFIERS) << " " << name << " = {" << "\n";
    stream << indent << ".pVkToBit = " << (modifiers.pVkToBit ? "vk_to_bits" : "nullptr") << ",\n";
    stream << indent << ".wMaxModBits = " << modifiers.wMaxModBits << ",\n";
    stream << indent << ".ModNumber = {\n";
    for (WORD i = 0; i < modifiers.wMaxModBits; ++i) {
        stream << indent << indent << (unsigned)modifiers.ModNumber[i] << ",\n";
    }
    stream << indent << "}\n";
    stream << "};\n\n";
}

void WriteVirtualCodeToWideCharTable(const VK_TO_WCHARS1 table[], string_view name, BYTE modifications, BYTE cbSize, ostream& stream) {
    stream << "static " << "VK_TO_WCHARS" << (UINT)modifications << " " << name << "[] = {" << "\n";
    for (auto iterator = table; ; iterator = (const VK_TO_WCHARS1*)((const BYTE*)iterator + cbSize)) {
        stream << indent << "{ " << VirtualKeyLiteral(iterator->VirtualKey);

        // TODO: Decompose attributes according to:
        //#define CAPLOK      0x01
        //#define SGCAPS      0x02
        //#define CAPLOKALTGR 0x04
        //// KANALOK is for FE
        //#define KANALOK     0x08
        //#define GRPSELTAP   0x80
        stream << ", " << HexLiteral(iterator->Attributes);
        
        stream << ", " << "{ ";
        for (BYTE i = 0; i < modifications; ++i) {
            if (i > 0) stream << ", ";
            stream << WCharLiteral(iterator->wch[i]);
        }
        stream << " }";
        
        stream << " }";
        if (iterator->VirtualKey != 0) stream << ",";
        stream << "\n";
        if (iterator->VirtualKey == 0) break;
    }
    stream << "};\n\n";
}

void WriteVirtualCodeToWideCharTables(const VK_TO_WCHAR_TABLE table[], string_view name, ostream& stream) {
    for (auto iterator = table; iterator->pVkToWchars; ++iterator) {
        std::string tableName = "vk_to_wchar";
        tableName += (char)('0' + iterator->nModifications);
        WriteVirtualCodeToWideCharTable(iterator->pVkToWchars, tableName, iterator->nModifications, iterator->cbSize, stream);
    }

    stream << "static " << STRINGIFY(VK_TO_WCHAR_TABLE) << " " << name << "[] = {" << "\n";
    for (auto iterator = table; iterator->pVkToWchars; ++iterator) {
        stream << indent << "{ ";

        std::string tableName = "vk_to_wchar";
        tableName += (char)('0' + iterator->nModifications);
        stream << "(PVK_TO_WCHARS1)" << tableName;
        stream << ", " << (USHORT)iterator->nModifications;
        stream << ", " << "sizeof(" << tableName << "[0]" << ")";
        stream << " }";
        stream << "\n";
    }
    stream << indent << "{ nullptr, 0, 0 }\n";
    stream << "};\n\n";
}

void WriteDeadKeyTable(const DEADKEY table[], string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(DEADKEY) << " " << name << "[] = {" << "\n";
    for (auto iterator = table; iterator->dwBoth; ++iterator) {
        stream << indent << "DEADTRANS(";

        stream << WCharLiteral(LOWORD(iterator->dwBoth));
        stream << ", " << WCharLiteral(HIWORD(iterator->dwBoth));
        stream << ", " << WCharLiteral(iterator->wchComposed);
        stream << ", " << HexLiteral((USHORT)iterator->uFlags);

        stream << ")";
        stream << "\n";
    }
    stream << indent << "{ 0, 0, 0 }\n";
    stream << "};\n\n";
}

void WriteKeyNamesTable(const VSC_LPWSTR table[], string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(VSC_LPWSTR) << " " << name << "[] = {" << "\n";
    for (auto iterator = table; iterator->vsc != 0; ++iterator) {
        stream << indent << "{ " << HexLiteral(iterator->vsc) << ", " << WStrLiteral(iterator->pwsz) << " }" << "\n";
    }
    stream << indent << "{ 0, 0 }" << "\n";
    stream << "};\n\n";
}

void WriteScanCodesToVirtualKeyArray(std::span<const USHORT> values, string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(USHORT) << " " << name << "[] = {" << "\n";
    for (size_t i = 0; i < values.size(); ++i) {
        stream << indent << "/* " << HexLiteral((BYTE)i) << " */ " << VirtualKeyLiteral(values[i]);
        if (i + 1 != values.size()) stream << ",";
        stream << "\n";
    }
    stream << "};\n\n";
}

void WriteScanCodesToVirtualKeyTable(const VSC_VK table[], string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(VSC_VK) << " " << name << "[] = {" << "\n";
    for (auto iterator = table; iterator->Vsc != 0; ++iterator) {
        stream << indent << "{ " << HexLiteral(iterator->Vsc) << ", " << VirtualKeyLiteral(iterator->Vk) << " }" << "\n";
    }
    stream << indent << "{ 0, 0 }" << "\n";
    stream << "};\n\n";
}

void WriteTables(const KBDTABLES& tables, string_view name, ostream& stream) {
    if (tables.pCharModifiers) WriteModifiersStruct(*tables.pCharModifiers, "char_modifiers", stream);
    if (tables.pVkToWcharTable) WriteVirtualCodeToWideCharTables(tables.pVkToWcharTable, "vk_to_wchar", stream);
    if (tables.pDeadKey) WriteDeadKeyTable(tables.pDeadKey, "dead_keys", stream);

    if (tables.pKeyNames) WriteKeyNamesTable(tables.pKeyNames, "key_names", stream);
    if (tables.pKeyNamesExt) WriteKeyNamesTable(tables.pKeyNamesExt, "key_names_ext", stream);
    // TODO: pKeyNamesDead

    if (tables.pusVSCtoVK) WriteScanCodesToVirtualKeyArray(std::span<const USHORT>(tables.pusVSCtoVK, tables.bMaxVSCtoVK), "scancode_to_vk", stream);
    if (tables.pVSCtoVK_E0) WriteScanCodesToVirtualKeyTable(tables.pVSCtoVK_E0, "scancode_to_vk_e0", stream);
    if (tables.pVSCtoVK_E1) WriteScanCodesToVirtualKeyTable(tables.pVSCtoVK_E1, "scancode_to_vk_e1", stream);
}

void WriteKeyboardSource(const KBDTABLES& tables, ostream& stream) {
    stream << "#include <Windows.h>" << "\n";
    stream << "#include <kbd.h>" << "\n";
    stream << "\n";

    WriteTables(tables, "kbd_tables", stream);
}

const KBDTABLES& LoadKeyboard(filesystem::path path) {
    HMODULE module = LoadLibraryW(path.native().c_str());
    if (!module) throw exception("Failed to load keyboard layout binary.");

    auto dllExport = (PKBDTABLES(*)())GetProcAddress(module, dllExportName);
    if (!dllExport) throw exception("Keyboard layout binary contains no expected export.");

    PKBDTABLES tables = dllExport();
    if (!dllExport) throw exception("Keyboard layout binary returned null table.");

    return *tables;
}

int main() {
    SetConsoleCP(CP_UTF8);
    auto& tables = LoadKeyboard(L"C:\\Windows\\System32\\Colemak2.dll");
    WriteKeyboardSource(tables, cout);
}
