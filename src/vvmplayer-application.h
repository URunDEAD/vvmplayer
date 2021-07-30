/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-application.h
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

#include <gtk/gtk.h>

#include "vvmplayer-settings.h"

G_BEGIN_DECLS

#define VVMPLAYER_APPLICATION_DEFAULT() ((VvmApplication *)g_application_get_default ())
#define VVMPLAYER_TYPE_APPLICATION (vvmplayer_application_get_type ())

G_DECLARE_FINAL_TYPE (VvmApplication, vvmplayer_application, VVMPLAYER, APPLICATION, GtkApplication)

VvmApplication *vvmplayer_application_new    (void);

G_END_DECLS
