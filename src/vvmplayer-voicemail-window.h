/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-voicemail-window.h
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
#include <handy.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include "vvmplayer-vvmd.h"

G_BEGIN_DECLS

enum vvmd_lifetime_status {
	VVM_LIFETIME_STATUS_UNKNOWN,
	VVM_LIFETIME_STATUS_NOT_READ,
	VVM_LIFETIME_STATUS_READ
};

#define GTK_TYPE_VOICEMAIL_WINDOW (voicemail_window_get_type())

G_DECLARE_FINAL_TYPE (VoicemailWindow, voicemail_window, VVM, VOICEMAIL_WINDOW, HdyExpanderRow)

GtkWidget *voicemail_window_new (GVariant   *message_t,
                                 GDBusProxy *message_proxy,
                                 GstDiscoverer    *discoverer);

void voicemail_window_set_contact_name (VoicemailWindow *self,
                                        const char *contact_name);
char *voicemail_window_get_number (VoicemailWindow *self);
char *voicemail_window_get_objectpath (VoicemailWindow *self);
GDateTime *voicemail_window_get_datetime (VoicemailWindow *self);

G_END_DECLS
