#pragma once

std::wstring load_wstr_resource(HINSTANCE hInstance, UINT uID) {
    /**
     * Colin Finck
     * https://github.com/enlyze/Wizard-2020/blob/clang/src/utils.cpp @ d69945b
     *
     * Windows NT 4.0 does not support determining the string length via
     * LoadStringW.  We can only do that manually via FindResourceW.
     *
     * Even worse, the format of STRINGTABLEs is different from any other
     * resource type:
     * * The ID of each string is divided by 16 and this determines the
     *   resource ID.
     * * Each of these resource IDs has a binary block containing information
     *   about the 16 strings managed by this block.
     * * Each string begins with a 2-byte length field and is followed by the
     *   UTF-16 string bytes.
     * * Unused strings in a block just have a zero size. Unused blocks don't
     *   make it into the compiled resource file at all.
     */

    UINT BlockId = uID / 16 + 1;
    UINT StringIdInBlock = uID % 16;

    HRSRC hFindResource = FindResourceW
        (hInstance, MAKEINTRESOURCEW(BlockId), RT_STRING);
    if (hFindResource == nullptr) {
        return std::wstring();
    }

    HGLOBAL hLoadResource = LoadResource(hInstance, hFindResource);
    if (hLoadResource == nullptr) {
        return std::wstring();
    }

    USHORT* pResource = reinterpret_cast<USHORT*>(LockResource(hLoadResource));
    if (!pResource) {
        return std::wstring();
    }

    // Check the length of each string in the block before our string, and skip
    // it to reach our string.
    for (UINT i = 0; i < StringIdInBlock; i++) {
        USHORT StringLength = *pResource;
        pResource++;
        pResource += StringLength;
    }

    // We are now in front of the length field of our string.
    // Extract the length and the actual string.
    USHORT StringLength = *pResource;
    std::wstring wstr(reinterpret_cast<PCWSTR>(pResource + 1), StringLength);

    // Strings in resources may or may not be NUL-terminated. Remove any
    // trailing NUL character that we have just mistakenly copied to wstr.
    const auto it = wstr.crbegin();
    if (it != wstr.crend() && *it == L'\0') {
        wstr.pop_back();
    }

    return wstr;
}
