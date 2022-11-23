/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-contact.h
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

/* See https://gitlab.gnome.org/GNOME/evolution-data-server/-/issues/332#note_1107764 */
#define EDS_DISABLE_DEPRECATED

#include <glib-object.h>
#include <libebook-contacts/libebook-contacts.h>

G_BEGIN_DECLS

#define VVM_TYPE_CONTACT (vvm_contact_get_type ())

G_DECLARE_FINAL_TYPE (VvmContact, vvm_contact, VVM, CONTACT, GObject)

VvmContact        *vvm_contact_new                   (EContact          *contact,
                                                         EVCardAttribute   *attr);
void               vvm_contact_set_name              (VvmContact     *self,
                                                         const char        *name);
const char        *vvm_contact_get_value             (VvmContact     *self);
void               vvm_contact_set_value             (VvmContact     *self,
                                                         const char        *value);
const char        *vvm_contact_get_value_type        (VvmContact     *self);
const char        *vvm_contact_get_uid               (VvmContact     *self);
gboolean           vvm_contact_is_dummy              (VvmContact     *self);
const char        *vvm_contact_get_name              (VvmContact *self);
gboolean           vvm_contact_matches               (VvmContact     *self,
                                                      const char     *needle);

G_END_DECLS
