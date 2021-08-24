/*
 * Copyright (c) 2021 EKA2L1 Team.
 * 
 * This file is part of EKA2L1 project.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <common/algorithm.h>
#include <common/log.h>
#include <qt/displaywidget.h>
#include <drivers/graphics/graphics.h>

#include <QWindow>
#include <QOpenGLContext>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

display_window::display_window(QWindow *parent, display_widget *container)
    : QWindow(parent) 
    , display_context_(nullptr)
    , shared_display_context_(nullptr)   // For vulkan this may stay null all over
    , offscreen_surface_(nullptr)
    , container_(container)
    , main_made_(false) {
    setSurfaceType(QWindow::OpenGLSurface);
    create();

    static constexpr std::int32_t TOTAL_TRY = 3;

    display_context_ = new QOpenGLContext;

    QSurfaceFormat::OpenGLContextProfile total_profiles_to_try[TOTAL_TRY] = { QSurfaceFormat::CoreProfile, QSurfaceFormat::NoProfile, QSurfaceFormat::NoProfile };
    int total_minor_to_try[TOTAL_TRY] = { 2, 1, 0 };

    bool success = false;

    for (int i = 0; i < TOTAL_TRY; i++) {
        QSurfaceFormat format_needed;
        format_needed.setProfile(total_profiles_to_try[i]);
        format_needed.setVersion(3, total_minor_to_try[i]);
        format_needed.setSwapBehavior(QSurfaceFormat::DefaultSwapBehavior);
        format_needed.setSwapInterval(0);

        display_context_->setFormat(format_needed);
        display_context_->setParent(this);

        if (display_context_->create()) {
            success = true;
            break;
        }
    }

    if (!success) {
        LOG_ERROR(eka2l1::FRONTEND_UI, "Unable to create required OpenGL context (needed OpenGL 3.0 at minimum!");

        delete display_context_;
        display_context_ = nullptr;
    }
}

display_window::~display_window() {
    display_context_->doneCurrent();

    if (shared_display_context_)
        delete shared_display_context_;

    if (display_context_)
        delete display_context_;

    if (offscreen_surface_)
        delete offscreen_surface_;
}

void display_window::init(eka2l1::vec2 size) {
    // This is usally called from another thread :D
    // Create shared contexts
    shared_display_context_ = new QOpenGLContext;
    shared_display_context_->setShareContext(display_context_);
    shared_display_context_->setFormat(display_context_->format());

    if (!shared_display_context_->create()) {
        LOG_ERROR(eka2l1::FRONTEND_UI, "Unable to create shared OpenGL context for worker thread!");
    }

    offscreen_surface_ = new QOffscreenSurface(nullptr);

    offscreen_surface_->setParent(display_context_->parent());
    offscreen_surface_->setFormat(display_context_->format());
    offscreen_surface_->create();
}

void display_window::make_current() {
    shared_display_context_->makeCurrent(offscreen_surface_);
}

void display_window::done_current() {
    shared_display_context_->doneCurrent();
}

void display_window::swap_buffer() {
    display_context_->swapBuffers(this);
}

bool display_window::event(QEvent *event) {
    if (event->type() == QEvent::UpdateRequest) {
        if (container_->repaint_request) {
            if (!main_made_) {
                main_made_ = true;
            }

            display_context_->makeCurrent(this);
            container_->repaint_request();
        }

        requestUpdate();
        return true;
    }

    return QWindow::event(event);
}

void display_window::exposeEvent(QExposeEvent *event) {
    requestUpdate();
    QWindow::exposeEvent(event);
}

display_widget::display_widget(QWidget *parent)
    : QWidget(parent)
    , userdata_(nullptr)
    , win_(nullptr)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);

    setMouseTracking(false);
    
    win_ = new display_window(qobject_cast<QWindow*>(parent), this);
    win_container_ = createWindowContainer(win_, this);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(win_container_);

    setLayout(layout);
}

display_widget::~display_widget() {
    delete win_container_;
    delete win_;
}

void display_widget::init(std::string title, eka2l1::vec2 size, const std::uint32_t flags) {
    if (win_) {
        win_->init(size);
    }

    resize(size.x, size.y);
    change_title(title);
}

void display_widget::make_current() {
    if (win_) {
        win_->make_current();
    }
}

void display_widget::done_current() {
    if (win_) {
        win_->done_current();
    }
}

void display_widget::swap_buffer() {
    if (win_) {
        win_->swap_buffer();
    }
}

void display_widget::poll_events() {
    // Not to our business
}

void display_widget::shutdown() {
    // todo
}

void display_widget::set_fullscreen(const bool is_fullscreen) {
    if (is_fullscreen) {
        showFullScreen();
    } else {
        showNormal();
    }
}

bool display_widget::should_quit() {
    return false;
}

void display_widget::change_title(std::string title) {
    setWindowTitle(QString::fromUtf8(title.c_str()));
}

eka2l1::vec2 display_widget::window_size() {
    return eka2l1::vec2(width(), height());
}

eka2l1::vec2 display_widget::window_fb_size() {
    const qreal pixel_ratio = devicePixelRatioF();
    return eka2l1::vec2(width() * pixel_ratio, height() * pixel_ratio);
}

eka2l1::vec2d display_widget::get_mouse_pos() {
    return eka2l1::vec2d{ 0, 0 };
}

bool display_widget::get_mouse_button_hold(const int mouse_btt) {
    return false;
}

void display_widget::set_userdata(void *userdata) {
    userdata_ = userdata;
}

void *display_widget::get_userdata() {
    return userdata_;
}

bool display_widget::set_cursor(eka2l1::drivers::cursor *cur) {
    return true;
}

void display_widget::cursor_visiblity(const bool visi) {
}

bool display_widget::cursor_visiblity() {
    return true;
}

void display_widget::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        return;
    }

    if (button_pressed) {
        button_pressed(userdata_, event->key());
    }
}

void display_widget::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        return;
    }

    if (button_released) {
        button_released(userdata_, event->key());
    }
}

void display_widget::mousePressEvent(QMouseEvent *event) {
    const qreal pixel_ratio = devicePixelRatioF();

    if (raw_mouse_event) {
        const int button = eka2l1::common::find_most_significant_bit_one(event->button());
        raw_mouse_event(userdata_, eka2l1::vec2(event->pos().x() * pixel_ratio, event->pos().y() * pixel_ratio), button, 0);
    }

    if (touch_pressed && (event->button() == Qt::LeftButton)) {
        touch_pressed(userdata_, eka2l1::vec2(event->pos().x() * pixel_ratio, event->pos().y() * pixel_ratio));
    }
}

void display_widget::mouseReleaseEvent(QMouseEvent *event) {
    if (raw_mouse_event) {
        const qreal pixel_ratio = devicePixelRatioF();

        const int button = eka2l1::common::find_most_significant_bit_one(event->button());
        raw_mouse_event(userdata_, eka2l1::vec2(event->pos().x() * pixel_ratio, event->pos().y() * pixel_ratio), button, 2);
    }

    if (touch_released && (event->button() == Qt::LeftButton)) {
        touch_released(userdata_);
    }
}

void display_widget::mouseMoveEvent(QMouseEvent *event) {
    // Even with mouse tracking disabled, they still keep coming
    if (event->buttons() == Qt::NoButton) {
        return;
    }

    const qreal pixel_ratio = devicePixelRatioF();

    if (raw_mouse_event) {
        const int button = eka2l1::common::find_most_significant_bit_one(event->buttons());
        raw_mouse_event(userdata_, eka2l1::vec2(event->pos().x() * pixel_ratio, event->pos().y() * pixel_ratio), button, 1);
    }

    if (touch_move && (event->buttons() & Qt::LeftButton)) {
        touch_move(userdata_, eka2l1::vec2(event->pos().x() * pixel_ratio, event->pos().y() * pixel_ratio));
    }
}
