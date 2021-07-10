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

#include "vvmplayer-preferences-window.h"
#include "vvmplayer-settings.h"
#include "vvmplayer-vvmd.h"

#define STATUS_GOOD "gtk-apply"
#define STATUS_BAD "gtk-no"

struct _VvmPreferencesWindow
{
  HdyPreferencesWindow parent_instance;

  //Modem Manager Settings
  GtkEntry *destination_number_text;

  //Status
  GtkImage *vvmd_enabled_image;

  //Demos
  GtkEntry *demo_text;
  GtkSwitch *demo_switch;

};

G_DEFINE_TYPE (VvmPreferencesWindow, vvm_preferences_window, HDY_TYPE_PREFERENCES_WINDOW)

VvmPreferencesWindow *
vvm_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_VVM_PREFERENCES_WINDOW, NULL);
}

static void
vvm_preferences_set_gtkimage_icon (GtkImage *image_to_set, const char *icon) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, icon);
  g_object_set_property (G_OBJECT (image_to_set), "icon-name", &a);
}

static void
vvm_preferences_window_populate (VvmPreferencesWindow *self)
{
  g_autofree char     *VVMDestinationNumber = NULL;
  VvmSettings *settings = vvm_settings_get_default ();


  //Modem Manager settings
  VVMDestinationNumber = vvm_settings_get_vvm_destination_number (settings);
  if (strstr (VVMDestinationNumber, "invalid") == NULL) { //Don't populate invalid settings
    gtk_entry_set_text (self->destination_number_text, VVMDestinationNumber);
  }

  if(vvm_settings_get_mm_available (settings) &&
     vvm_settings_get_service_available (settings)) {
    vvm_preferences_set_gtkimage_icon (self->vvmd_enabled_image, STATUS_GOOD);
  } else {
    vvm_preferences_set_gtkimage_icon (self->vvmd_enabled_image, STATUS_BAD);
  }

  //Demo
  g_autofree gchar *name = g_strdup("Hello!!!!");

  gtk_entry_set_text (self->demo_text, name);
  gtk_switch_set_active (self->demo_switch,
                         TRUE);
}

static void
demo_switch_signal_flipped_cb (GtkSwitch *widget,
                               gboolean   state,
                               VvmPreferencesWindow *self)
{

  if (state)
    g_debug("State is now true!");
  else
    g_debug("State is now false!");

}

static void
activation_vvm_switch_flipped_cb (GtkSwitch *widget,
                                  gboolean   state,
                                  VvmPreferencesWindow *self)
{

  if (state)
    g_debug("Activation of VVM is set to true!");
  else
    g_debug("Activation of VVM is set to false!");

}

static void
demo_button_clicked_cb (GtkButton     *btn,
                               VvmPreferencesWindow *self)
{
  g_debug ("Button was pressed");
  g_debug ("I got here: Showed %s", gtk_entry_get_text (self->demo_text));
}

static void
destination_number_button_clicked_cb (GtkButton     *btn,
                                      VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    const char *dest_number = NULL;

    dest_number = gtk_entry_get_text (self->destination_number_text);

    vvm_settings_set_vvm_destination_number (settings, dest_number);
    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "VVMDestinationNumber",
                                      dest_number) == FALSE) {

      g_autofree char *result = g_strdup("Action Failed");

      g_error("Error changing setting in vvmd!");
      gtk_entry_set_text (self->destination_number_text, result);
    }
  } else {
    g_autofree char *result = g_strdup("Action Failed");

    g_warning("VVMD Modem Manager settings not available!");
    gtk_entry_set_text (self->destination_number_text, result);
  }
}

static void
vvm_preferences_window_class_init (VvmPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-preferences-window.ui");

  //Modem Manager Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, destination_number_text);

  gtk_widget_class_bind_template_callback (widget_class, destination_number_button_clicked_cb);
/*
  char  *provision_status;
  char  *vvm_destination_number;
  char  *vvm_type;
  char  *default_number;
  char  *carrier_prefix;
*/

  //Whether or not to activate VVM
/*
  int    enable_vvm;
*/
  gtk_widget_class_bind_template_callback (widget_class, activation_vvm_switch_flipped_cb);


  //Status Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, vvmd_enabled_image);
/*
  int    mm_available;
  int    service_available;
  int    mailbox_active;
*/

  //Demos only
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, demo_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, demo_switch);
  gtk_widget_class_bind_template_callback (widget_class, demo_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, demo_switch_signal_flipped_cb);
}

static void
vvm_preferences_window_init (VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    if (vvmplayer_vvmd_get_mm_settings(backend) == FALSE) {
      g_error("Error getting VVMD Modem Manager Settings");
      vvm_settings_load_mm_defaults (settings);
    }
  } else {
    g_debug("Loading VVMD MM Defaults");
    vvm_settings_load_mm_defaults (settings);
  }

  if(vvm_settings_get_service_available (settings)) {
    if (vvmplayer_vvmd_get_service_settings (backend) == FALSE) {
      g_error("Error getting VVMD Service Settings");
      vvm_settings_load_service_defaults (settings);
    }
  } else {
    g_debug("Loading VVMD service Defaults");
    vvm_settings_load_service_defaults (settings);
  }

  gtk_widget_init_template (GTK_WIDGET (self));
  vvm_preferences_window_populate (self);

}
