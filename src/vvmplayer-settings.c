/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-settings.c
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

#define G_LOG_DOMAIN "vvmplayer-settings"


#include "config.h"

#include "vvmplayer-settings.h"

/**
 * SECTION: vvmplayer-settings
 * @title: VvmSettings
 * @short_description: The Application settings
 * @include: "vvmplayer-settings.h"
 *
 * A class that handles application specific settings, and
 * to store them to disk.
 */

struct _VvmSettings
{
  GObject    parent_instance;

  GSettings *app_settings;

  GtkSettings *gtk_settings;

  //App Settings
  int prefer_dark_theme;

  //Spam Contact
  char     *spam_contact;
  gboolean  spam_contact_enabled;

  //Modem Manager Settings
  char  *vvm_destination_number;
  char  *vvm_type;
  char  *default_number;
  char  *carrier_prefix;

  //Whether or not to activate VVM
  int    enable_vvm;

  unsigned int  stream_count;
  gboolean  speaker_active;

  //Status Settings
  char  *provision_status;
  int    mm_available;       //Boolean
  int    service_available;  //Boolean
  int    mailbox_active;     //Boolean

  //Don't need to display this
  char  *country_code;
};

G_DEFINE_TYPE (VvmSettings, vvmplayer_settings, G_TYPE_OBJECT)

void
vvm_settings_set_vvm_enabled (VvmSettings *self,
                              int          VVMEnabled)
{
  self->enable_vvm = VVMEnabled;
  if (self->enable_vvm)
    g_debug ("VVM Enabled is set to TRUE");
  else
    g_debug ("VVM Enabled is set to FALSE");
}

int
vvm_settings_get_vvm_enabled (VvmSettings *self)
{
  return self->enable_vvm;
}

void
vvm_settings_set_speaker_button_state (VvmSettings *self,
                                       gboolean     setting)
{
  self->speaker_active = setting;
  g_debug ("speaker button state is now %d", self->speaker_active);
}

unsigned int
vvm_settings_get_stream_count (VvmSettings *self)
{
  return self->stream_count;
}

void
vvm_settings_incriment_stream_count (VvmSettings *self)
{
  if (self->stream_count == 0 && self->speaker_active == FALSE)
    {
      g_autoptr(GError) error = NULL;
      gboolean ret;
      g_debug ("Setting audio profile to call");
      ret = call_audio_select_mode (CALL_AUDIO_MODE_CALL, &error);

      if (!ret && error)
        g_warning ("Failed to switch profile: %s", error->message);
    }

  self->stream_count = self->stream_count + 1;
  g_debug ("Stream count is imcrimented to %d", self->stream_count);
}


void
vvm_settings_decriment_stream_count (VvmSettings *self)
{
  if (self->stream_count == 0)
    {
      g_warning ("Stream count is already at 0!");
      return;
    }

  self->stream_count = self->stream_count - 1;
  g_debug ("Stream count is decrimented to %d", self->stream_count);

  if (self->stream_count == 0 && self->speaker_active == FALSE)
    {
      g_autoptr(GError) error = NULL;
      gboolean ret;
      g_debug ("Setting audio profile to back to default");
      ret = call_audio_select_mode (CALL_AUDIO_MODE_DEFAULT, &error);

      if (!ret && error)
        g_warning ("Failed to switch profile: %s", error->message);
    }
}

void
vvm_settings_set_mm_available (VvmSettings *self,
                               int          ModemManagerAvailable)
{
  self->mm_available = ModemManagerAvailable;
  if (self->mm_available)
    g_debug ("Modem Manager available through VVMD");
  else
    g_debug ("Modem Manager not available through VVMD");
}

int
vvmplayer_settings_get_dark_theme (VvmSettings *self)
{
  return self->prefer_dark_theme;
}

static void
adw_style_manager_set_dark_theme (int prefer_dark_theme)
{
  AdwStyleManager *adw_style_manager = adw_style_manager_get_default ();

  if (prefer_dark_theme)
    adw_style_manager_set_color_scheme (adw_style_manager, ADW_COLOR_SCHEME_PREFER_DARK);
  else
    adw_style_manager_set_color_scheme (adw_style_manager, ADW_COLOR_SCHEME_DEFAULT);
}

void
vvmplayer_settings_set_dark_theme (VvmSettings *self,
                                   int          prefer_dark_theme)
{
  self->prefer_dark_theme = prefer_dark_theme;

  /* Set the setting right away */
  g_settings_set_boolean (self->app_settings, "dark-theme", self->prefer_dark_theme);
  g_settings_apply (self->app_settings);
  adw_style_manager_set_dark_theme (self->prefer_dark_theme);
}

char
*
vvm_settings_get_spam_contact (VvmSettings *self)
{
  return g_strdup (self->spam_contact);
}

void
vvm_settings_set_spam_contact (VvmSettings *self,
                               const char  *spam_contact)
{
  g_free (self->spam_contact);
  self->spam_contact = g_strdup (spam_contact);
  g_debug ("Spam Contact is set to %s", self->spam_contact);

  /* Set the setting right away */
  g_settings_set_string (self->app_settings, "spam-contact", self->spam_contact);
  g_settings_apply (self->app_settings);
}

gboolean
vvmplayer_settings_get_spam_contact_enabled (VvmSettings *self)
{
  return self->spam_contact_enabled;
}

void
vvmplayer_settings_set_spam_contact_enabled (VvmSettings *self,
                                             gboolean     spam_contact_enabled)
{
  self->spam_contact_enabled = spam_contact_enabled;

  /* Set the setting right away */
  g_settings_set_boolean (self->app_settings, "spam-contact-enabled", self->spam_contact_enabled);
  g_settings_apply (self->app_settings);
}

int
vvm_settings_get_mm_available (VvmSettings *self)
{
  return self->mm_available;
}

void
vvm_settings_set_vvm_type (VvmSettings *self,
                           const char  *VVMType)
{
  g_free (self->vvm_type);
  self->vvm_type = g_strdup (VVMType);
  g_debug ("VVM Type is set to %s", self->vvm_type);
}

char
*
vvm_settings_get_vvm_type (VvmSettings *self)
{
  return g_strdup (self->vvm_type);
}

void
vvm_settings_set_vvm_destination_number (VvmSettings *self,
                                         const char  *VVMDestinationNumber)
{
  g_free (self->vvm_destination_number);
  self->vvm_destination_number = g_strdup (VVMDestinationNumber);
  g_debug ("VVM Destination Number is set to %s", self->vvm_destination_number);
}

char
*
vvm_settings_get_vvm_destination_number (VvmSettings *self)
{
  return g_strdup (self->vvm_destination_number);
}

void
vvm_settings_set_vvm_carrier_prefix (VvmSettings *self,
                                     const char  *CarrierPrefix)
{
  g_free (self->carrier_prefix);
  self->carrier_prefix = g_strdup (CarrierPrefix);
  g_debug ("VVM Carrier Prefix is set to %s", self->carrier_prefix);
}

char
*
vvm_settings_get_vvm_carrier_prefix (VvmSettings *self)
{
  return g_strdup (self->carrier_prefix);
}

void
vvm_settings_set_vvm_default_number (VvmSettings *self,
                                     const char  *DefaultModemNumber)
{
  g_free (self->default_number);
  self->default_number = g_strdup (DefaultModemNumber);

  g_debug ("VVM Modem Default Number is set to %s", self->default_number);
}

char
*
vvm_settings_get_vvm_default_number (VvmSettings *self)
{
  return g_strdup (self->default_number);
}

void
vvm_settings_set_vvm_provision_status (VvmSettings *self,
                                       const char  *ProvisionStatus)
{
  g_free (self->provision_status);
  self->provision_status = g_strdup (ProvisionStatus);
  g_debug ("VVM ProvisionStatus is set to %s", self->provision_status);
}

char
*
vvm_settings_get_vvm_provision_status (VvmSettings *self)
{
  return g_strdup (self->provision_status);
}

void
vvm_settings_set_service_available (VvmSettings *self,
                                    int          ServiceAvailable)
{
  self->service_available = ServiceAvailable;
  if (self->service_available)
    g_debug ("VVMD Service available");
  else
    g_debug ("VVMD Service not available");
}

int
vvm_settings_get_service_available (VvmSettings *self)
{
  return self->service_available;
}

void
vvm_settings_set_vvm_country_code (VvmSettings *self,
                                   const char  *CountryCode)
{
  g_free (self->country_code);
  self->country_code = g_strdup (CountryCode);
  g_debug ("VVM Modem Country Code is set to %s", self->country_code);
}

const char
*
vvm_settings_get_vvm_country_code (VvmSettings *self)
{
  return self->country_code;
}

void
vvm_settings_set_mailbox_active (VvmSettings *self,
                                 int          MailboxActive)
{
  self->mailbox_active = MailboxActive;
  if (self->mailbox_active)
    g_debug ("VVM Mailbox active");
  else
    g_debug ("VVM Mailbox not active");
}

int
vvm_settings_get_mailbox_active (VvmSettings *self)
{
  return self->mailbox_active;
}

void
vvm_settings_load_mm_defaults (VvmSettings *self)
{
  self->mm_available = FALSE;
  self->enable_vvm = FALSE;
  g_free (self->vvm_destination_number);
  self->vvm_destination_number = g_strdup ("Destination Number invalid");
  g_free (self->vvm_type);
  self->vvm_type = g_strdup ("Unknown");
  g_free (self->default_number);
  self->default_number = g_strdup ("NULL");
  g_free (self->provision_status);
  self->provision_status = g_strdup ("Unknown");
  g_free (self->carrier_prefix);
  self->carrier_prefix = g_strdup ("Carrier Prefix invalid");
}

void
vvm_settings_load_service_defaults (VvmSettings *self)
{
  self->mailbox_active = FALSE;
  g_free (self->country_code);
  self->country_code = g_strdup ("Country Code invalid");
}

static void
vvmplayer_settings_dispose (GObject *object)
{
  VvmSettings *self = (VvmSettings *)object;

  g_debug ("Disposing settings");

  g_settings_set_string (self->app_settings, "version", PACKAGE_VERSION);
  g_settings_set_string (self->app_settings, "spam-contact", self->spam_contact);
  g_settings_set_boolean (self->app_settings, "spam-contact-enabled", self->spam_contact_enabled);
  g_settings_set_boolean (self->app_settings, "dark-theme", self->prefer_dark_theme);

  g_settings_apply (self->app_settings);
  g_free (self->vvm_destination_number);
  g_free (self->vvm_type);
  g_free (self->default_number);
  g_free (self->carrier_prefix);
  g_free (self->provision_status);
  g_free (self->spam_contact);

  G_OBJECT_CLASS (vvmplayer_settings_parent_class)->dispose (object);
}

static void
vvmplayer_settings_class_init (VvmSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = vvmplayer_settings_dispose;
}

static void
vvmplayer_settings_init (VvmSettings *self)
{
  g_autofree char *version = NULL;
  self->app_settings = g_settings_new (PACKAGE_ID);
  version = g_settings_get_string (self->app_settings, "version");
  self->spam_contact = g_settings_get_string (self->app_settings, "spam-contact");
  self->spam_contact_enabled = g_settings_get_boolean (self->app_settings, "spam-contact-enabled");
  self->prefer_dark_theme = g_settings_get_boolean (self->app_settings, "dark-theme");

  adw_style_manager_set_dark_theme (self->prefer_dark_theme);

  g_settings_delay (self->app_settings);
}

/**
 * vvmplayer_settings_new:
 *
 * Create a new #VvmSettings
 *
 * Returns: (transfer full): A #VvmSettings.
 * Free with g_object_unref().
 */
VvmSettings *
vvm_settings_get_default (void)
{
  static VvmSettings *self;

  if (!self)
    {
      self = g_object_new (VVMPLAYER_TYPE_SETTINGS, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
      vvm_settings_load_mm_defaults (self);
      vvm_settings_load_service_defaults (self);
      self->stream_count = 0;
      self->speaker_active = FALSE;
    }
  return self;
}
