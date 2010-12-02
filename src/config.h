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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef CONFIG_H
#define CONFIG_H

#define ADD_TAB "AddTab"
#define CLOSE_TAB "CloseTab"
#define TAB_RIGHT "TabRight"
#define TAB_LEFT "TabLeft"
#define SUB_NEXT "SubterminalNext"
#define SUB_PREV "SubterminalPrev"
#define MOVE_LEFT "MoveLeft"
#define MOVE_RIGHT "MoveRight"
#define COPY_SELECTION "CopySelection"
#define PASTE_SELECTION "PasteSelection"
#define RENAME_SESSION  "RenameSession"


/* Some defaults for QTerminal application */

#define DEFAULT_WIDTH                  800
#define DEFAULT_HEIGHT                 600

// ACTIONS
//#define ADD_TAB_SHORTCUT	       "Ctrl+N"
//#define CLOSE_TAB_SHORTCUT         "Ctrl+W"
#define TAB_LEFT_SHORTCUT	       "Shift+Left"
#define TAB_RIGHT_SHORTCUT	       "Shift+Right"
#define SUB_NEXT_SHORTCUT	       "Shift+Up"
#define SUB_PREV_SHORTCUT	       "Shift+Down"

#define COPY_SELECTION_SHORTCUT      "Ctrl+Insert"
#define PASTE_SELECTION_SHORTCUT      "Shift+Insert"

#define MOVE_LEFT_SHORTCUT             "Shift+Alt+Left"
#define MOVE_RIGHT_SHORTCUT            "Shift+Alt+Right"

#define RENAME_SESSION_SHORTCUT        "Shift+Alt+S"

// XON/XOFF features:

#define FLOW_CONTROL_ENABLED		false
#define FLOW_CONTROL_WARNING_ENABLED	false

#endif
