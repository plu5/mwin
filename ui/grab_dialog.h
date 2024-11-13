#pragma once

#include <Windows.h>
#include <string> // std::wstring
#include <optional> // std::optional
#include "ui/base.h" // Window, create_window
#include "core/rules.h" // RuleFieldType
#include "core/coords.h" // WndCoordinates
#include "ui/windows_list.h" // WindowsList
#include "utility/win32_painting.h" // Icon

struct GrabField {
    RuleFieldType type = RuleFieldType::none;
    HWND* cb = nullptr;
    WindowsList* wlist = nullptr;
    HWND* btn = nullptr;
    std::wstring label = L"";
    int x = 0, y = 0, w = 0, h = 0;
    bool checked_initially = false;
};

struct GrabData {
    std::optional<std::string> wnd_title = std::nullopt;
    std::optional<std::string> wnd_exe = std::nullopt;
    std::optional<WndCoordinates> geometry = std::nullopt;
};

struct Grabbable {
    WindowFieldType type = WindowFieldType::none;
    HWND* cb = nullptr;
    std::optional<std::string>* data_str;
    std::optional<WndCoordinates>* data_coords;

    inline void set_data(WindowsList& windows_list, size_t i) {
        auto data = windows_list.get(type, i);
        if (data.str) *data_str = data.str;
        else if (data.coords) *data_coords = data.coords;
    }

    inline void reset_data() {
        if (data_str) data_str->reset();
        if (data_coords) data_coords->reset();
    }
};

class GrabDialog : public Window {
    using super = Window;
public:
    HWND hwnd = NULL;
    int x = 0, y = 0, w = 250, h = 500;
    GrabData data;
    WindowsList windows_list;
    GrabDialog
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst), refresh_icon(hinst, IDI_REFRESH)
    {};
    void initialise(HWND parent_hwnd_, int label_foreground_);
    void show(int x_, int y_);
    void hide();
protected:
    int min_w = 170, min_h = 185;
    HWND parent_hwnd = NULL;
    HWND wnd_title_cb = NULL;
    HWND wnd_exe_cb = NULL;
    HWND geometry_cb = NULL;
    HWND refresh_btn = NULL;
    Icon refresh_icon;
    HWND cancel_btn = NULL;
    HWND ok_btn = NULL;
    int marg = 5;
    int edit_height = 20;
    int btn_size = 25;
    int btn_wsize = 50;
    int refresh_btn_size = 21;
    std::vector<GrabField> fields = {
        {.type = RuleFieldType::wnd_title, .cb = &wnd_title_cb,
         .label = L"Window title", .x = marg, .y = marg,
         .w = -marg*2, .h = edit_height, .checked_initially = true},
        {.type = RuleFieldType::wnd_exe, .cb = &wnd_exe_cb,
         .label = L"Exe path", .x = marg, .y = edit_height + marg,
         .w = -marg*2, .h = edit_height, .checked_initially = true},
        {.type = RuleFieldType::coords, .cb = &geometry_cb,
         .label = L"Geometry", .x = marg, .y = 2*edit_height + marg,
         .w = -marg*2, .h = edit_height},
        {.btn = &refresh_btn, .x = -refresh_btn_size-marg,
         .y = 3*edit_height + 9,
         .w = refresh_btn_size, .h = refresh_btn_size},
        {.wlist = &windows_list, .x = 0, .y = 4*edit_height + 3*marg,
         .h = -(4*edit_height + 3*marg)-btn_size-marg*2},
        {.btn = &cancel_btn, .label = L"Cancel",
         .x = -btn_wsize-marg, .y = -btn_size-marg,
         .w = btn_wsize, .h = btn_size},
        {.btn = &ok_btn, .label = L"OK",
         .x = -btn_wsize*2-marg*2, .y = -btn_size-marg,
         .w = btn_wsize, .h = btn_size},
    };
    std::vector<Grabbable> grabbables = {
        {.type = WindowFieldType::title, .cb = &wnd_title_cb,
         .data_str = &data.wnd_title},
        {.type = WindowFieldType::path, .cb = &wnd_exe_cb,
         .data_str = &data.wnd_exe},
        {.type = WindowFieldType::geometry, .cb = &geometry_cb,
         .data_coords = &data.geometry},
    };
    int label_foreground = 0;
    void update_wh();
    WndCoordinates calculate_field_geometry(GrabField field);
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    void paint();
    std::wstring info_label = L"Grab from window:";
    void paint_info_label(HDC hdc);
    void confirm();
    void confirm_if_selected();
};
