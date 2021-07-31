/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-preferences-window.h
 *
 * Copyright 2021 Chris Talbot <chris@talbothome.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s):
 *   Chris Talbot <chris@talbothome.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <handy.h>

G_BEGIN_DECLS

#define HDY_TYPE_VVM_PREFERENCES_WINDOW (vvm_preferences_window_get_type())

G_DECLARE_FINAL_TYPE (VvmPreferencesWindow, vvm_preferences_window, HDY, VVM_PREFERENCES_WINDOW, HdyPreferencesWindow)

VvmPreferencesWindow *vvm_preferences_window_new (void);

G_END_DECLS
