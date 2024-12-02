#include "ui/status.h"
#include "constants.h" // Theme
#include "utility/win32_geometry.h" // get_rect
#include "utility/win32_painting.h" // paint_text, get_window_font
#include "utility/string_conversion.h" // string_to_wstring

#include <plog/Log.h>
#include <plog/Init.h>

namespace plog {
    class StatusFormatter {
    public:
        static util::nstring header() {
            return util::nstring();
        }

        static util::nstring format(const Record& record) {
            util::nostringstream ss;
            ss << record.getMessage() << "\n";
            return ss.str();
        }
    };

    template<class Formatter>
    class StatusAppender : public IAppender {
    public:
        void write(const Record& record) PLOG_OVERRIDE {
            util::nstring str = Formatter::format(record);
            if (status) status->set(static_cast<std::wstring>(str));
        }

        void attach_status(Status* status_) {
            status = status_;
        }
    private:
        Status* status;
    };
}

void Status::initialise(HWND parent_hwnd_, HINSTANCE hinst_) {
    parent_hwnd = parent_hwnd_;
    hinst = hinst_;
    static plog::StatusAppender<plog::StatusFormatter> appender;
    auto logger = plog::get<0>();
    if (logger) {
        logger->addAppender(&appender);
        appender.attach_status(this);
    }
}

void Status::adjust_size(RECT rect_) {
    rect = rect_;
    tooltip.change_rect(rect);
}

void Status::set(const std::wstring& s) {
    current_message = s;
    InvalidateRect(parent_hwnd, &rect, false);
    tooltip.initialise(parent_hwnd, current_message, rect, tooltip_width);
}

void Status::set_font(HFONT font_) {
    font = font_;
}

void Status::paint(HDC hdc) {
    if (!current_message.empty())
        paint_text(hdc, current_message, Theme::fg, &rect, font,
                   DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
}
