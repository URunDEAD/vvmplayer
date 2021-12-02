/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-settings.h
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
#include <libcallaudio.h>

#include "vvmplayer-log.h"

G_BEGIN_DECLS

#define VVMPLAYER_TYPE_SETTINGS (vvmplayer_settings_get_type ())

G_DECLARE_FINAL_TYPE (VvmSettings, vvmplayer_settings, VVMPLAYER, SETTINGS, GObject)

VvmSettings *vvm_settings_get_default                (void);
void         vvmplayer_settings_save                 (VvmSettings *self);

gboolean     vvmplayer_settings_get_is_first_run     (VvmSettings *self);
gboolean     vvmplayer_settings_get_window_maximized (VvmSettings *self);
void         vvmplayer_settings_set_window_maximized (VvmSettings *self,
                                                      gboolean     maximized);
void         vvmplayer_settings_get_window_geometry  (VvmSettings  *self,
                                                      GdkRectangle *geometry);
void         vvmplayer_settings_set_window_geometry  (VvmSettings  *self,
                                                      GdkRectangle *geometry);

void vvm_settings_load_mm_defaults (VvmSettings *self);
void vvm_settings_load_service_defaults (VvmSettings *self);

void vvm_settings_set_vvm_enabled (VvmSettings *self, int VVMEnabled);
int vvm_settings_get_vvm_enabled (VvmSettings *self);

void vvm_settings_incriment_stream_count (VvmSettings *self);
void vvm_settings_decriment_stream_count (VvmSettings *self);

unsigned int vvm_settings_get_stream_count (VvmSettings *self);

void vvm_settings_set_speaker_button_state (VvmSettings *self, gboolean setting);

void vvm_settings_set_vvm_type (VvmSettings *self, const char *VVMType);
char *vvm_settings_get_vvm_type (VvmSettings *self);

void vvm_settings_set_vvm_destination_number (VvmSettings *self,
                                              const char *VVMDestinationNumber);
char *vvm_settings_get_vvm_destination_number (VvmSettings *self);

void vvm_settings_set_vvm_default_number (VvmSettings *self,
                                          const char *DefaultModemNumber);
char *vvm_settings_get_vvm_default_number (VvmSettings *self);

void vvm_settings_set_vvm_provision_status (VvmSettings *self,
                                            const char *ProvisionStatus);
char *vvm_settings_get_vvm_provision_status (VvmSettings *self);

void vvm_settings_set_vvm_carrier_prefix (VvmSettings *self,
                                          const char *CarrierPrefix);
char *vvm_settings_get_vvm_carrier_prefix (VvmSettings *self);

void vvm_settings_set_mm_available  (VvmSettings *self,
                                     int ModemManagerAvailable);
int vvm_settings_get_mm_available (VvmSettings *self);

void vvm_settings_set_service_available (VvmSettings *self,
                                         int ServiceAvailable);
int vvm_settings_get_service_available (VvmSettings *self);

void vvm_settings_set_vvm_country_code (VvmSettings *self,
                                        const char *CountryCode);
const char *vvm_settings_get_vvm_country_code (VvmSettings *self);

void vvm_settings_set_mailbox_active (VvmSettings *self, int MailboxActive);
int vvm_settings_get_mailbox_active (VvmSettings *self);

int vvmplayer_settings_get_override_global_appearence_pref (VvmSettings *self);
void vvmplayer_settings_set_override_global_appearence_pref (VvmSettings *self,
                                           int override_global_appearence_pref);

void vvmplayer_settings_set_dark_theme (VvmSettings *self,
                                        int prefer_dark_theme);
int vvmplayer_settings_get_dark_theme (VvmSettings *self);

void vvmplayer_settings_set_default_dark_theme_pref (VvmSettings *self,
                                                   int default_dark_theme_pref);
int vvmplayer_settings_get_default_dark_theme_pref (VvmSettings *self);

gboolean vvmplayer_settings_get_spam_contact_enabled (VvmSettings *self);
void vvmplayer_settings_set_spam_contact_enabled (VvmSettings *self,
                                                  gboolean spam_contact_enabled);

char *vvm_settings_get_spam_contact (VvmSettings *self);
void  vvm_settings_set_spam_contact (VvmSettings *self,
                                     const char *spam_contact);

G_END_DECLS
