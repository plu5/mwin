#include <Windows.h>

HMENU hmenu_cast(int id) {
    return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
}
