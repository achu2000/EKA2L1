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

#ifndef DISPLAY_WIDGET_H
#define DISPLAY_WIDGET_H

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QWidget>
#include <QWindow>

#include <functional>

#include <drivers/graphics/emu_window.h>

class display_widget;

class display_window : public QWindow {
protected:
    QOpenGLContext *display_context_;
    QOpenGLContext *shared_display_context_;

    QOffscreenSurface *offscreen_surface_;

    display_widget *container_;

    bool main_made_;

public:
    explicit display_window(QWindow *parent, display_widget *container);
    ~display_window();

    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

    void init(eka2l1::vec2 size);
    void make_current();
    void done_current();
    void swap_buffer();
};

class display_widget : public QWidget, public eka2l1::drivers::emu_window {
    Q_OBJECT

private:
    display_window *win_;
    QWidget *win_container_;

    void *userdata_;

public:
    explicit display_widget(QWidget *parent = nullptr);
    ~display_widget();

    void init(std::string title, eka2l1::vec2 size, const std::uint32_t flags) override;

    void make_current() override;
    void done_current() override;
    void swap_buffer() override;
    void poll_events() override;
    void shutdown() override;
    void set_fullscreen(const bool is_fullscreen) override;

    bool should_quit() override;

    void change_title(std::string title) override;

    eka2l1::vec2 window_size() override;
    eka2l1::vec2 window_fb_size() override;
    eka2l1::vec2d get_mouse_pos() override;

    bool get_mouse_button_hold(const int mouse_btt) override;
    void set_userdata(void *userdata) override;
    void *get_userdata() override;

    bool set_cursor(eka2l1::drivers::cursor *cur) override;
    void cursor_visiblity(const bool visi) override;
    bool cursor_visiblity() override;

    QPaintEngine *paintEngine() const override {
        return nullptr;
    }

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // DISPLAY_WIDGET_H
