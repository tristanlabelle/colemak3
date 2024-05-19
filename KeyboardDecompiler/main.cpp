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

void WriteKeyNamesTable(const VSC_LPWSTR* iterator, string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(VSC_LPWSTR) << " " << name << "[] = {" << "\n";
    for (;; ++iterator) {
        stream << indent << "{ " << HexLiteral(iterator->vsc) << ", " << WStrLiteral(iterator->pwsz) << " }";
        if (iterator->vsc > 0) stream << ",";
        stream << "\n";
        if (iterator->vsc == 0) break;
    }
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

void WriteScanCodesToVirtualKeyTable(const VSC_VK* iterator, string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(VSC_VK) << " " << name << "[] = {" << "\n";
    for (;; ++iterator) {
        stream << indent << "{ " << HexLiteral(iterator->Vsc) << ", " << VirtualKeyLiteral(iterator->Vk) << " }";
        if (iterator->Vsc != 0) stream << ",";
        stream << "\n";
        if (iterator->Vsc == 0) break;
    }
    stream << "};\n\n";
}

void WriteVirtualKeyToBitTable(const VK_TO_BIT* iterator, string_view name, ostream& stream) {
    stream << "static " << STRINGIFY(VK_TO_BIT) << " " << name << "[] = {" << "\n";
    for (;; ++iterator) {
        stream << indent << "{ " << VirtualKeyLiteral(iterator->Vk) << ", ";
        switch (iterator->ModBits) {
            case KBDSHIFT: stream << STRINGIFY(KDBSHIFT); break;
            case KBDCTRL: stream << STRINGIFY(KBDCTRL); break;
            case KBDALT: stream << STRINGIFY(KDBALT); break;
            default: stream << HexLiteral(iterator->ModBits); break;
        }
        stream << " }";
        if (iterator->Vk != 0) stream << ",";
        stream << "\n";
        if (iterator->Vk == 0) break;
    }
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

void WriteTables(const KBDTABLES& tables, string_view name, ostream& stream) {
    if (tables.pKeyNames) WriteKeyNamesTable(tables.pKeyNames, "key_names", stream);
    if (tables.pKeyNamesExt) WriteKeyNamesTable(tables.pKeyNamesExt, "key_names_ext", stream);
    if (tables.pusVSCtoVK) WriteScanCodesToVirtualKeyArray(std::span<const USHORT>(tables.pusVSCtoVK, tables.bMaxVSCtoVK), "scancode_to_vk", stream);
    if (tables.pVSCtoVK_E0) WriteScanCodesToVirtualKeyTable(tables.pVSCtoVK_E0, "scancode_to_vk_e0", stream);
    if (tables.pVSCtoVK_E1) WriteScanCodesToVirtualKeyTable(tables.pVSCtoVK_E1, "scancode_to_vk_e1", stream);
    if (tables.pCharModifiers) WriteModifiersStruct(*tables.pCharModifiers, "char_modifiers", stream);
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
    auto& tables = LoadKeyboard(L"C:\\Windows\\System32\\Colemak2.dll");
    WriteKeyboardSource(tables, cout);
}
