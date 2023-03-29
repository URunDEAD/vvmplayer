/* vvmplayer-debug-info.h
 *
 * Copyright 2017–2022 Purism SPC
 *           2021-2023 Chris Talbot
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <adwaita.h>

/* Copied and adapted from https://gitlab.gnome.org/GNOME/libadwaita/-/raw/main/demo/adw-demo-debug-info.c */
G_BEGIN_DECLS

char *vvm_generate_debug_info (void);

G_END_DECLS
