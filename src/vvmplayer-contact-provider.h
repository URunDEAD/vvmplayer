/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-contact-provider.h
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

#include <glib-object.h>
#include <gdk/gdk.h>

#include "vvmplayer-contact.h"

G_BEGIN_DECLS

#define VVM_TYPE_EDS (vvm_eds_get_type ())

G_DECLARE_FINAL_TYPE (VvmEds, vvm_eds, VVM, EDS, GObject)

VvmEds     *vvm_eds_get_default            (void);
gboolean       vvm_eds_is_ready       (VvmEds  *self);
VvmContact *vvm_eds_find_by_number    (VvmEds  *self,
                                       const char *phone_number);
GListModel *vvm_eds_get_model         (VvmEds *self);
int vvm_eds_get_is_ready (VvmEds *self);

G_END_DECLS
