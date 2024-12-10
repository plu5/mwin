#include "ui/grab_dialog.h"
#include "ui/button.h" // create_cb, create_btn
#include "utility/win32_geometry.h" // get_size, get_relative_rect
#include "utility/win32_painting.h" // get_window_font, paint_text
#include "constants.h" // UM::grabbed message id
#include <windowsx.h> // Button_GetCheck

void GrabDialog::initialise(HWND parent_hwnd_, int label_foreground_) {
    parent_hwnd = parent_hwnd_;
    label_foreground = label_foreground_;

    WndCoordinates geom = {x, y, w, h};
    hwnd = create_window<GrabDialog>
        (*this, hinst, &geom, WS_POPUPWINDOW | WS_CAPTION | WS_SIZEBOX,
         parent_hwnd, false, true);

    for (const auto& field : fields) {
        auto fgeom = calculate_field_geometry(field);
        if (field.cb) {
            *field.cb = create_cb(field.label, fgeom.x, fgeom.y,
                                  fgeom.w, fgeom.h, -1, hwnd, hinst);
            if (field.checked_initially)
                Button_SetCheck(*field.cb, BST_CHECKED);
        } else if (field.wlist) {
            field.wlist->initialise(hwnd, hinst, fgeom);
        } else if (field.btn) {
            if (*field.btn == refresh_btn) {
                *field.btn = create_btn
                    (field.label, fgeom.x, fgeom.y, fgeom.w, fgeom.h, -1, hwnd,
                     hinst, true, WS_VISIBLE | BS_ICON, refresh_icon.get());
                continue;
            }
            *field.btn = create_btn(field.label, fgeom.x, fgeom.y,
                                    fgeom.w, fgeom.h, -1, hwnd, hinst);
        }
    }

    set_window_font(hwnd, font.get());
}

void GrabDialog::show(int x_, int y_) {
    x = x_;
    y = y_;
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE);
    windows_list.refresh();
    ShowWindow(hwnd, SW_SHOW);
    EnableWindow(parent_hwnd, false);
}

void GrabDialog::hide() {
    EnableWindow(parent_hwnd, true);
    ShowWindow(hwnd, SW_HIDE);
}

void GrabDialog::update_wh() {
    auto s = get_size(hwnd);
    w = s.w;
    h = s.h;
}

// if less than threshold, taken to be an offset of whole
inline int normalise(int value, int whole, int threshold=0) {
    return value < threshold ? whole + value : value;
}

WndCoordinates GrabDialog::calculate_field_geometry(const GrabField& field) {
    update_wh();
    return {normalise(field.x, w), normalise(field.y, h),
        normalise(field.w, w, 1), normalise(field.h, h, 1)};
}

LRESULT GrabDialog::proc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_GETMINMAXINFO: {
        auto pmmi = reinterpret_cast<PMINMAXINFO>(lp);
        pmmi->ptMinTrackSize.x = min_w;
        pmmi->ptMinTrackSize.y = min_h;
        break;
    }

    case WM_CTLCOLORSTATIC: {
        auto hdc = reinterpret_cast<HDC>(wp);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, label_foreground);
        return reinterpret_cast<LRESULT>(bg.get());
    }

    case WM_CTLCOLORLISTBOX: {
        auto hdc = reinterpret_cast<HDC>(wp);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, label_foreground);
        return reinterpret_cast<LRESULT>(list_bg.get());
    }

    case WM_SIZE: {
        for (const auto& field : fields) {
            auto fgeom = calculate_field_geometry(field);
            if (field.wlist) {
                field.wlist->resize(fgeom.w, fgeom.h);
            } else if (field.btn) {
                SetWindowPos(*field.btn, NULL, fgeom.x, fgeom.y,
                             0, 0, SWP_NOSIZE);
            }
        }
        break;
    }

    case WM_COMMAND:
        if (HIWORD(wp) == BN_CLICKED) {
            auto hwnd_ = reinterpret_cast<HWND>(lp);
            if (hwnd_ == cancel_btn) {
                hide();
            } else if (hwnd_ == ok_btn) {
                confirm();
            } else if (hwnd_ == refresh_btn) {
                windows_list.refresh();
            }
        } else if (HIWORD(wp) == LBN_DBLCLK) {
            confirm_if_selected();
        }
        break;

    case WM_KEYDOWN:
        switch (wp) {
        case VK_RETURN:
            confirm_if_selected();
            break;
        case VK_ESCAPE:
            hide();
            break;
        }
        break;

    case WM_CLOSE:
        hide();
        return 1;
    }
    return super::proc(msg, wp, lp);
}

void GrabDialog::paint() {
    paint_info_label(dc2.get());

    super::paint();
}

void GrabDialog::paint_info_label(HDC hdc) {
    auto& last_cb = geometry_cb;
    auto rect = get_relative_rect(last_cb, hwnd);
    auto move_down = (rect.bottom - rect.top) + marg;
    OffsetRect(&rect, 0, move_down);
    paint_text(hdc, info_label, label_foreground, &rect);
}

void GrabDialog::confirm() {
    auto i = windows_list.selected();
    if (i != -1) {
        for (auto& grabbable : grabbables) {
            if (Button_GetCheck(*grabbable.cb) == BST_CHECKED) {
                grabbable.set_data(windows_list, i);
            } else {
                grabbable.reset_data();
            }
        }
        PostMessage(parent_hwnd, UM::grabbed, 0, 0);
    }
    hide();
}

void GrabDialog::confirm_if_selected() {
    if (windows_list.selected() != -1) confirm();
}
