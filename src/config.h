/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/


#ifndef CONFIG_H
#define CONFIG_H

#define ADD_TAB "Add Tab"
#define RENAME_TAB "Rename Tab"
#define CLOSE_TAB "Close Tab"
#define NEW_WINDOW "New Window"

#define QUIT "Quit"
#define PREFERENCES "Preferences..."

#define TAB_NEXT "Next Tab"
#define TAB_PREV "Previous Tab"

#define CLEAR_TERMINAL "Clear Active Terminal"

#define SPLIT_HORIZONTAL "Split Terminal Horizontally"
#define SPLIT_VERTICAL "Split Terminal Vertically"

#define SUB_COLLAPSE "Collapse Subterminal"
#define SUB_NEXT "Next Subterminal"
#define SUB_PREV "Previous Subterminal"

#define MOVE_LEFT "Move Tab Left"
#define MOVE_RIGHT "Move Tab Right"

#define COPY_SELECTION "Copy Selection"
#define PASTE_CLIPBOARD "Paste Clipboard"
#define PASTE_SELECTION "Paste Selection"

#define ZOOM_IN "Zoom in"
#define ZOOM_OUT "Zoom out"
#define ZOOM_RESET "Zoom reset"

#define FIND "Find"

#define TOGGLE_MENU "Toggle Menu"
#define TOGGLE_BOOKMARKS "Toggle Bookmarks"

#define HIDE_WINDOW_BORDERS "Hide Window Borders"
#define SHOW_TAB_BAR "Show Tab Bar"
#define FULLSCREEN "Fullscreen"

/* Some defaults for QTerminal application */

#define DEFAULT_WIDTH                  800
#define DEFAULT_HEIGHT                 600
#define DEFAULT_FONT                   "Monospace"

// ACTIONS
#define CLEAR_TERMINAL_SHORTCUT        "Ctrl+Shift+X"
#define TAB_PREV_SHORTCUT	       "Shift+Left"
#define TAB_NEXT_SHORTCUT	       "Shift+Right"
#define SUB_PREV_SHORTCUT	       "Shift+Down"
#define SUB_NEXT_SHORTCUT	       "Shift+Up"

#ifdef Q_WS_MAC
// It's tricky - Ctrl is "command" key on mac's keyboards
#define COPY_SELECTION_SHORTCUT      "Ctrl+C"
#define PASTE_CLIPBOARD_SHORTCUT      "Ctrl+V"
#define FIND_SHORTCUT                "Ctrl+F"
#define NEW_WINDOW_SHORTCUT          "Ctrl+N"
#define ADD_TAB_SHORTCUT               "Ctrl+T"
#define CLOSE_TAB_SHORTCUT             "Ctrl+W"
#define TOGGLE_MENU_SHORTCUT            "Ctrl+M"
#define TOGGLE_BOOKMARKS_SHORTCUT       "Ctrl+B"
#else
#define COPY_SELECTION_SHORTCUT      "Ctrl+Shift+C"
#define PASTE_CLIPBOARD_SHORTCUT      "Ctrl+Shift+V"
#define PASTE_SELECTION_SHORTCUT      "Shift+Ins"
#define FIND_SHORTCUT                 "Ctrl+Shift+F"
#define NEW_WINDOW_SHORTCUT           "Ctrl+Shift+N"
#define ADD_TAB_SHORTCUT               "Ctrl+Shift+T"
#define CLOSE_TAB_SHORTCUT             "Ctrl+Shift+W"
#define TOGGLE_MENU_SHORTCUT            "Ctrl+Shift+M"
#define TOGGLE_BOOKMARKS_SHORTCUT       "Ctrl+Shift+B"
#endif

#define ZOOM_IN_SHORTCUT               "Ctrl++"
#define ZOOM_OUT_SHORTCUT              "Ctrl+-"
#define ZOOM_RESET_SHORTCUT              "Ctrl+0"

#define MOVE_LEFT_SHORTCUT             "Shift+Alt+Left"
#define MOVE_RIGHT_SHORTCUT            "Shift+Alt+Right"

#define RENAME_SESSION_SHORTCUT        "Shift+Alt+S"

#define FULLSCREEN_SHORTCUT           "F11"

// XON/XOFF features:

#define FLOW_CONTROL_ENABLED		false
#define FLOW_CONTROL_WARNING_ENABLED	false

#endif
