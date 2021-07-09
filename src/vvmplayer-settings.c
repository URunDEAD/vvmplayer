/* vvmplayer-window.c
 *
 * Copyright 2021 Chris Talbot
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
 */

#include "vvmplayer-settings.h"

struct _VvmSettings
{
  GObject     parent_instance;

  GtkSettings *gtk_settings;

  char  *provision_status;
  int    enable_vvm;
  char  *vvm_destination_number;
  char  *vvm_type;
  char  *default_number;
  char  *carrier_prefix;
  int    vvmd_connected;
};

G_DEFINE_TYPE (VvmSettings, vvm_settings, G_TYPE_OBJECT)

void
vvm_settings_set_vvm_enabled (VvmSettings *self, int VVMEnabled)
{
  self->enable_vvm = VVMEnabled;
  if (self->enable_vvm)
    g_debug ("VVM Enabled is set to TRUE");
  else
    g_debug ("VVM Enabled is set to FALSE");

}

void
vvm_settings_set_vvm_type (VvmSettings *self, const char *VVMType)
{
  g_free(self->vvm_type);
  self->vvm_type = g_strdup(VVMType);
  g_debug ("VVM Type is set to %s", self->vvm_type);

}

void
vvm_settings_set_vvm_destination_number (VvmSettings *self,
                                         const char *VVMDestinationNumber)
{
  g_free(self->vvm_destination_number);
  self->vvm_destination_number = g_strdup(VVMDestinationNumber);
  g_debug ("VVM Destination Number is set to %s", self->vvm_destination_number);

}

void
vvm_settings_set_vvm_carrier_prefix (VvmSettings *self,
                                     const char *CarrierPrefix)
{
  g_free(self->carrier_prefix);
  self->carrier_prefix = g_strdup(CarrierPrefix);
  g_debug ("VVM Carrier Prefix is set to %s", self->carrier_prefix);

}

void
vvm_settings_set_vvm_default_number (VvmSettings *self,
                                     const char *DefaultModemNumber)
{

  g_free(self->default_number);
  self->default_number = g_strdup(DefaultModemNumber);

  g_debug ("VVM Modem Default Number is set to %s", self->default_number);

}

void
vvm_settings_set_vvm_provision_status (VvmSettings *self,
                                       const char *ProvisionStatus)
{
  g_free(self->provision_status);
  self->provision_status = g_strdup(ProvisionStatus);
  g_debug ("VVM ProvisionStatus is set to %s", self->provision_status);

}

static void
vvm_settings_constructed (GObject *object)
{
  //VvmSettings *self = (VvmSettings *)object;

  G_OBJECT_CLASS (vvm_settings_parent_class)->constructed (object);

}

static void
vvm_settings_finalize (GObject *object)
{
  VvmSettings *self = (VvmSettings *)object;

  g_object_unref (self->gtk_settings);
  g_free(self->vvm_destination_number);
  g_free(self->vvm_type);
  g_free(self->default_number);
  g_free(self->carrier_prefix);
  g_free(self->provision_status);

  G_OBJECT_CLASS (vvm_settings_parent_class)->finalize (object);
}

static void
vvm_settings_class_init (VvmSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = vvm_settings_constructed;
  object_class->finalize = vvm_settings_finalize;
}

static void
vvm_settings_load_defaults (VvmSettings *self)
{

  self->enable_vvm = FALSE;
  self->vvmd_connected = FALSE;
  self->vvm_destination_number = g_strdup("Destination Number Invalid");
  self->vvm_type = g_strdup("Type Invalid");
  self->default_number = g_strdup("Default Number Invalid");
  self->provision_status = g_strdup("Provision Status Invalid");
  self->carrier_prefix = g_strdup("//VVM");
}

static void
vvm_settings_init (VvmSettings *self)
{
  self->gtk_settings = gtk_settings_get_default ();
}

VvmSettings *
vvm_settings_get_default (void)
{
  static VvmSettings *self;

  if (!self)
    {
      self = g_object_new (VVM_TYPE_SETTINGS, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
      vvm_settings_load_defaults (self);
    }
  return self;
}
