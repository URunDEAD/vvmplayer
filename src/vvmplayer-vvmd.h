/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-vvmd.h
 *
 * Copyright 2021-2022 Chris Talbot <chris@talbothome.com>
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
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include "vvmplayer-window.h"
#include "vvmplayer-settings.h"
#include "vvmplayer-voicemail-window.h"
#include "vvmplayer-contact-provider.h"
#include "vvmplayer-notification.h"

G_BEGIN_DECLS

#define VVMD_SERVICE	                "org.kop316.vvm"
#define VVMD_PATH	                "/org/kop316/vvm"
#define VVMD_MODEMMANAGER_PATH          VVMD_PATH    "/modemmanager"
#define VVMD_MANAGER_INTERFACE	        VVMD_SERVICE ".Manager"
#define VVMD_SERVICE_INTERFACE	        VVMD_SERVICE ".Service"
#define VVMD_MESSAGE_INTERFACE	        VVMD_SERVICE ".Message"
#define VVMD_MODEMMANAGER_INTERFACE	VVMD_SERVICE ".ModemManager"

#define VVM_TYPE_VVMD (vvm_vvmd_get_type ())

G_DECLARE_FINAL_TYPE (VvmVvmd, vvm_vvmd, VVM, VVMD, GObject)

enum vvm_type {
	VVM_TYPE_UNKNOWN,
	VVM_TYPE_CVVM,
	VVM_TYPE_ATTUSA,
	VVM_TYPE_OTMP,
	VVM_TYPE_VVM_THREE,
};

VvmVvmd *vvm_vvmd_get_default                  (void);

void vvmplayer_vvmd_set_mm_vvm_list_box (VvmVvmd *self,
                                         VvmWindow    *visual_voicemail);

int vvmplayer_vvmd_get_mm_settings (VvmVvmd *self);
int vvmplayer_vvmd_set_mm_setting (VvmVvmd *self,
                                   const char *setting_to_change,
                                   const char *updated_setting);

int vvmplayer_vvmd_set_mm_vvm_enabled (VvmVvmd *self,
                                       int enabled);

int vvmplayer_vvmd_get_service_settings (VvmVvmd *self);

int vvmplayer_vvmd_check_subscription_status (VvmVvmd *self);

int vvmplayer_vvmd_sync_vvm (VvmVvmd *self);

int vvmplayer_vvmd_encode_vvm_type (const char *vvm_type);

char *vvmplayer_vvmd_decode_vvm_type (int vvm_type);

void vvmplayer_vvmd_mark_message_read (GDBusProxy      *message_proxy,
                                       char            *objectpath,
                                       gpointer         user_data);

void vvmplayer_vvmd_delete_vvm (VvmVvmd     *self,
                           GDBusProxy  *message_proxy,
                           char        *objectpath,
                           gconstpointer user_data);

void vvmplayer_vvmd_decrement_unread (VvmVvmd *self);
void vvmplayer_vvmd_increment_unread (VvmVvmd *self);


G_END_DECLS
