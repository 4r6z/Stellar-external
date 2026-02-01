#pragma once

#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <iostream>

extern std::uintptr_t virtualaddy; // Declare external linkage for the variable

namespace codes {
    constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x824, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x825, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

    constexpr ULONG security = 0x192ba72c;
}

typedef struct _Request {
    INT32 security;

    HANDLE process_id;

    PVOID address;
    PVOID buffer;

    SIZE_T size;
    SIZE_T* ret_size;
} Request;

namespace memory {
    extern HANDLE driver_handle; // Declare external linkage for the variable
    extern INT32 process_id;     // Declare external linkage for the variable
    extern HANDLE process_id1;   // Declare external linkage for the variable
    extern uintptr_t BaseAddy;   // Declare external linkage for the variable

    inline bool find_driver() {
        driver_handle = CreateFileW((L"\\\\.\\3f12wqbhm"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
            return false;

        return true;
    }

    inline bool is_valid(std::uint64_t address) {
        if (address <= 0x400000 || address == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(address) == nullptr || address > 0x7FFFFFFFFFFFFFFF)
            return false;

        return true;
    }

    inline void read_physical(PVOID address, PVOID buffer, DWORD size) {
        if (!is_valid((std::uint64_t)address))
            return;

        Request arguments = { 0 };

        arguments.security = codes::security;
        arguments.process_id = (HANDLE)process_id;
        arguments.address = address;
        arguments.buffer = buffer;
        arguments.size = size;

        DeviceIoControl(driver_handle, codes::read, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
    }

    inline INT32 find_process(LPCTSTR process_name) {
        PROCESSENTRY32 pt;
        HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        pt.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hsnap, &pt)) {
            do {
                if (!lstrcmpi(pt.szExeFile, process_name)) {
                    CloseHandle(hsnap);
                    process_id = pt.th32ProcessID;
                    return pt.th32ProcessID;
                }
            } while (Process32Next(hsnap, &pt));
        }
        CloseHandle(hsnap);

        return { NULL };
    }
}

template <typename T>
inline T read(uint64_t address) {
    if (!memory::is_valid(address))
        return T();

    T buffer{ };

    memory::read_physical((PVOID)address, &buffer, sizeof(T));
    return buffer;
}

template <typename T>
inline T write(uint64_t address, T buffer) {
    return T();
}

template<typename T>
inline bool read_array(uintptr_t address, T out[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        out[i] = read<T>(address + i * sizeof(T));
    }
    return true;
}

#endif
