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

  int    provision_status;
  int    enable_vvm;
  char  *vvm_destination_number;
  char  *vvm_type;
  char  *default_number;
  char  *carrier_prefix;
  int    vvmd_connected;
};

G_DEFINE_TYPE (VvmSettings, vvm_settings, G_TYPE_OBJECT)

static void
vvm_settings_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  VvmSettings *self = (VvmSettings *)object;

}

static void
vvm_settings_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  VvmSettings *self = (VvmSettings *)object;
}

static void
vvm_settings_constructed (GObject *object)
{
  VvmSettings *self = (VvmSettings *)object;

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

  G_OBJECT_CLASS (vvm_settings_parent_class)->finalize (object);
}

static void
vvm_settings_class_init (VvmSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = vvm_settings_get_property;
  object_class->set_property = vvm_settings_set_property;
  object_class->constructed = vvm_settings_constructed;
  object_class->finalize = vvm_settings_finalize;
}

static void
vvm_settings_load_defaults (VvmSettings *self)
{
  self->provision_status = VVM_PROVISION_STATUS_NOT_SET;
  self->enable_vvm = FALSE;
  self->vvmd_connected = FALSE;
  self->vvm_destination_number = g_strdup("number_invalid");
  self->vvm_type = g_strdup("type_invalid");
  self->default_number = NULL;
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
