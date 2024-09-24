#include "exe_path.h"
#include <Windows.h>
#include <memory>
#include <iostream>
#include <locale>
#include <codecvt>
#include "utility/string_conversion.h"

// PolarBear https://stackoverflow.com/a/71903802/18396947
// Converts relative name like "..\test.exe" to its full form like
//  "C:\project\test.exe".
std::basic_string<TCHAR> get_full_name(TCHAR const* name) {
    // First we need to get a length of the full name string
    const DWORD full_name_length{GetFullPathName(name, 0, NULL, NULL)};
    if (full_name_length == 0) {
        // GetFullPathName call failed. Maybe you want to throw an exception.
        return std::basic_string<TCHAR>{};
    }

    // Now, when we know the length, we create a buffer with correct size
    //  and write the full name into it
    std::unique_ptr<TCHAR[]> full_name_buffer{new TCHAR[full_name_length]};
    const DWORD res = GetFullPathName
        (name, full_name_length, full_name_buffer.get(), NULL);
    if (res == 0) {
        // GetFullPathName call failed. Maybe you want to throw an exception.
        return std::basic_string<TCHAR>{};
    }

    // The full name has been successfully written to the buffer.
    return std::basic_string<TCHAR>(full_name_buffer.get());
}

// Resolves short path like "C:\GIT-RE~1\TEST_G~1\test.exe" into its long form
//  like "C:\git-repository\test_project\test.exe"
std::basic_string<TCHAR> get_long_name(TCHAR const* name) {
    // First we need to get a length of the long name string
    const DWORD long_name_length{GetLongPathName(name, 0, NULL)};
    if (long_name_length == 0) {
        // GetLongPathName call failed. Maybe you want to throw an exception.
        return std::basic_string<TCHAR>{};
    }

    // Now, when we know the length, we create a buffer with correct size
    //  and write the full name into it
    std::unique_ptr<TCHAR[]> long_name_buffer{new TCHAR[long_name_length]};
    const DWORD res = GetLongPathName
        (name, long_name_buffer.get(), long_name_length);
    if (res == 0) {
        // GetLongPathName call failed. Maybe you want to throw an exception.
        return std::basic_string<TCHAR>{};
    }

    // The long name has been successfully written to the buffer.
    return std::basic_string<TCHAR>(long_name_buffer.get());
}

std::basic_string<TCHAR> get_current_executable_full_name() {
    // We start with MAX_PATH because it is most likely that the path doesn't
    //  exceed 260 characters.
    DWORD path_buffer_size = MAX_PATH;
    std::unique_ptr<TCHAR[]> path_buffer{new TCHAR[path_buffer_size]};

    while (true) {
        const auto bytes_written = GetModuleFileName
            (NULL, path_buffer.get(), path_buffer_size);
        const auto last_error = GetLastError();

        if (bytes_written == 0) {
            // GetModuleFileName call failed. Maybe you want to throw
            //  an exception.
            return std::basic_string<TCHAR>{};
        }

        if (last_error == ERROR_INSUFFICIENT_BUFFER) {
            // There is not enough space in our buffer to fit the path.
            // We need to increase the buffer and try again.
            path_buffer_size *= 2;
            path_buffer.reset(new TCHAR[path_buffer_size]);
            continue;
        }

        // GetModuleFileName has successfully written the executable name
        //  to the buffer.
        // Now we need to convert it to a full long name
        std::basic_string<TCHAR> full_name = get_full_name(path_buffer.get());
        return get_long_name(full_name.c_str());
    }
}

std::basic_string<TCHAR> get_current_executable_root_w() {
    auto exe_path = get_current_executable_full_name();
    return exe_path.substr(0, exe_path.find_last_of(L"\\"));
}

std::string get_current_executable_root() {
    std::wstring ws = get_current_executable_root_w();
    return wstring_to_string(ws);
}
