# Colemak3

This project aims to provide source code and eventually binary distributions implementing the [Colemak](https://colemak.com/) keyboard layout on Windows.

The current [default implementation](https://colemak.com/Windows) of Colemak is an opaque `Colemak2.dll` binary file, which does not include the caps to backspace remapping and of which no arm64 version exists.

Since keyboard layouts on Windows are DLLs exporting static data structures, it should be possible to decompile the existing DLL into C++ code, tweak it as necessary, and rebuild it for all architectures.
