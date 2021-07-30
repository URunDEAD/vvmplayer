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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "vvmplayer-settings.h"
#include "vvmplayer-log.h"

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
  gboolean   first_run;

  GtkSettings *gtk_settings;

  //Modem Manager Settings
  char  *vvm_destination_number;
  char  *vvm_type;
  char  *default_number;
  char  *carrier_prefix;

  //Whether or not to activate VVM
  int    enable_vvm;

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
vvm_settings_set_vvm_enabled (VvmSettings *self, int VVMEnabled)
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
vvm_settings_set_mm_available (VvmSettings *self, int ModemManagerAvailable)
{
  self->mm_available = ModemManagerAvailable;
  if (self->mm_available)
    g_debug ("Modem Manager available through VVMD");
  else
    g_debug ("Modem Manager not available through VVMD");

}

int
vvm_settings_get_mm_available (VvmSettings *self)
{
  return self->mm_available;
}

void
vvm_settings_set_vvm_type (VvmSettings *self, const char *VVMType)
{
  g_free(self->vvm_type);
  self->vvm_type = g_strdup(VVMType);
  g_debug ("VVM Type is set to %s", self->vvm_type);

}

char
*vvm_settings_get_vvm_type (VvmSettings *self)
{
  return g_strdup(self->vvm_type);
}

void
vvm_settings_set_vvm_destination_number (VvmSettings *self,
                                         const char *VVMDestinationNumber)
{
  g_free(self->vvm_destination_number);
  self->vvm_destination_number = g_strdup(VVMDestinationNumber);
  g_debug ("VVM Destination Number is set to %s", self->vvm_destination_number);

}

char
*vvm_settings_get_vvm_destination_number (VvmSettings *self)
{
  return g_strdup(self->vvm_destination_number);
}

void
vvm_settings_set_vvm_carrier_prefix (VvmSettings *self,
                                     const char *CarrierPrefix)
{
  g_free(self->carrier_prefix);
  self->carrier_prefix = g_strdup(CarrierPrefix);
  g_debug ("VVM Carrier Prefix is set to %s", self->carrier_prefix);

}

char
*vvm_settings_get_vvm_carrier_prefix (VvmSettings *self)
{
  return g_strdup(self->carrier_prefix);
}

void
vvm_settings_set_vvm_default_number (VvmSettings *self,
                                     const char *DefaultModemNumber)
{

  g_free(self->default_number);
  self->default_number = g_strdup(DefaultModemNumber);

  g_debug ("VVM Modem Default Number is set to %s", self->default_number);

}

char
*vvm_settings_get_vvm_default_number (VvmSettings *self)
{
  return g_strdup(self->default_number);
}

void
vvm_settings_set_vvm_provision_status (VvmSettings *self,
                                       const char *ProvisionStatus)
{
  g_free(self->provision_status);
  self->provision_status = g_strdup(ProvisionStatus);
  g_debug ("VVM ProvisionStatus is set to %s", self->provision_status);

}

char
*vvm_settings_get_vvm_provision_status (VvmSettings *self)
{
  return g_strdup(self->provision_status);
}

void
vvm_settings_set_service_available (VvmSettings *self, int ServiceAvailable)
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
                                   const char *CountryCode)
{
  g_free(self->country_code);
  self->country_code = g_strdup(CountryCode);
  g_debug ("VVM Modem Country Code is set to %s", self->country_code);

}

const char
*vvm_settings_get_vvm_country_code (VvmSettings *self)
{
  return self->country_code;
}

void
vvm_settings_set_mailbox_active (VvmSettings *self, int MailboxActive)
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
  g_free(self->vvm_destination_number);
  self->vvm_destination_number = g_strdup("Destination Number invalid");
  g_free(self->vvm_type);
  self->vvm_type = g_strdup("Unknown");
  g_free(self->default_number);
  self->default_number = g_strdup("NULL");
  g_free(self->provision_status);
  self->provision_status = g_strdup("Unknown");
  g_free(self->carrier_prefix);
  self->carrier_prefix = g_strdup("Carrier Prefix invalid");
}

void
vvm_settings_load_service_defaults (VvmSettings *self)
{
  self->mailbox_active = FALSE;
  g_free(self->country_code);
  self->country_code = g_strdup("Country Code invalid");
}

static void
vvmplayer_settings_dispose (GObject *object)
{
  VvmSettings *self = (VvmSettings *)object;

  VVMPLAYER_TRACE_MSG ("disposing settings");

  g_settings_set_string (self->app_settings, "version", PACKAGE_VERSION);

  g_settings_apply (self->app_settings);
  g_object_unref (self->gtk_settings);
  g_free(self->vvm_destination_number);
  g_free(self->vvm_type);
  g_free(self->default_number);
  g_free(self->carrier_prefix);
  g_free(self->provision_status);

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

  if (!g_str_equal (version, PACKAGE_VERSION))
    self->first_run = TRUE;

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
    }
  return self;
}

/**
 * vvmplayer_settings_save:
 * @self: A #VvmSettings
 *
 * Save modified settings to disk.  By default,
 * the modified settings are saved to disk only
 * when #VvmSettings is disposed.  Use this
 * to force save to disk.
 */
void
vvmplayer_settings_save (VvmSettings *self)
{
  g_return_if_fail (VVMPLAYER_IS_SETTINGS (self));

  g_settings_apply (self->app_settings);
}

/**
 * vvmplayer_settings_get_is_first_run:
 * @self: A #VvmSettings
 *
 * Get if the application has ever launched after install
 * or update.
 *
 * Returns: %TRUE for the first launch of application after
 * install or update.  %FALSE otherwise.
 */
gboolean
vvmplayer_settings_get_is_first_run (VvmSettings *self)
{
  g_return_val_if_fail (VVMPLAYER_IS_SETTINGS (self), FALSE);

  return self->first_run;
}

/**
 * vvmplayer_settings_get_window_maximized:
 * @self: A #VvmSettings
 *
 * Get the window maximized state as saved in @self.
 *
 * Returns: %TRUE if maximized.  %FALSE otherwise.
 */
gboolean
vvmplayer_settings_get_window_maximized (VvmSettings *self)
{
  g_return_val_if_fail (VVMPLAYER_IS_SETTINGS (self), FALSE);

  return g_settings_get_boolean (self->app_settings, "window-maximized");
}

/**
 * vvmplayer_settings_set_window_maximized:
 * @self: A #VvmSettings
 * @maximized: The window state to save
 *
 * Set the window maximized state in @self.
 */
void
vvmplayer_settings_set_window_maximized (VvmSettings *self,
                                         gboolean     maximized)
{
  g_return_if_fail (VVMPLAYER_IS_SETTINGS (self));

  g_settings_set_boolean (self->app_settings, "window-maximized", !!maximized);
}

/**
 * vvmplayer_settings_get_window_geometry:
 * @self: A #VvmSettings
 * @geometry: (out): A #GdkRectangle
 *
 * Get the window geometry as saved in @self.
 */
void
vvmplayer_settings_get_window_geometry (VvmSettings  *self,
                                        GdkRectangle *geometry)
{
  g_return_if_fail (VVMPLAYER_IS_SETTINGS (self));
  g_return_if_fail (geometry != NULL);

  g_settings_get (self->app_settings, "window-size", "(ii)",
                  &geometry->width, &geometry->height);
  geometry->x = geometry->y = -1;
}

/**
 * vvmplayer_settings_set_window_geometry:
 * @self: A #VvmSettings
 * @geometry: A #GdkRectangle
 *
 * Set the window geometry in @self.
 */
void
vvmplayer_settings_set_window_geometry (VvmSettings  *self,
                                        GdkRectangle *geometry)
{
  g_return_if_fail (VVMPLAYER_IS_SETTINGS (self));
  g_return_if_fail (geometry != NULL);

  g_settings_set (self->app_settings, "window-size", "(ii)",
                  geometry->width, geometry->height);
}
