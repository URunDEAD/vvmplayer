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

  //Refresh
  GtkButton *refresh_settings_button;

  //Modem Manager Settings
  GtkEntry *destination_number_text;
  GtkEntry *modem_number_text;
  GtkEntry *carrier_prefix_text;

  //Status
  GtkImage *vvmd_enabled_image;
  GtkImage *mailbox_active_image;
  HdyActionRow *provision_status_text;

  //Demos
  GtkEntry *demo_text;
  GtkSwitch *demo_switch;


  int refresh_settings_button_timeout;
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
vvm_preferences_set_hdyactionrow_text (HdyActionRow *hdy_row, const char *text) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, text);
  g_object_set_property (G_OBJECT (hdy_row), "title", &a);
}

static void
vvm_preferences_refresh_settings (void)
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
}

static void
vvm_preferences_window_populate (VvmPreferencesWindow *self)
{
  g_autofree char     *VVMDestinationNumber = NULL;
  g_autofree char     *VVMDefaultModemNumber = NULL;
  g_autofree char     *VVMProvisionStatus = NULL;
  g_autofree char     *VVMCarrierPrefix = NULL;
  VvmSettings *settings = vvm_settings_get_default ();


  //Modem Manager settings
  VVMDestinationNumber = vvm_settings_get_vvm_destination_number (settings);
  if (strstr (VVMDestinationNumber, "invalid") == NULL) { //Don't populate invalid settings
    gtk_entry_set_text (self->destination_number_text, VVMDestinationNumber);
  } else {
    gtk_entry_set_text (self->destination_number_text, "");
  }

  VVMDefaultModemNumber = vvm_settings_get_vvm_default_number (settings);
  if (g_strcmp0 (VVMDefaultModemNumber, "NULL") != 0) { //Don't populate invalid settings
    gtk_entry_set_text (self->modem_number_text, VVMDefaultModemNumber);
  } else {
    gtk_entry_set_text (self->modem_number_text, "");
  }

  VVMCarrierPrefix = vvm_settings_get_vvm_carrier_prefix (settings);
  if (strstr (VVMCarrierPrefix, "invalid") == NULL) { //Don't populate invalid settings
    gtk_entry_set_text (self->carrier_prefix_text, VVMCarrierPrefix);
  } else {
    gtk_entry_set_text (self->carrier_prefix_text, "");
  }

  //Status
  if(vvm_settings_get_mm_available (settings) &&
     vvm_settings_get_service_available (settings)) {
    vvm_preferences_set_gtkimage_icon (self->vvmd_enabled_image, STATUS_GOOD);
  } else {
    vvm_preferences_set_gtkimage_icon (self->vvmd_enabled_image, STATUS_BAD);
  }

  if (vvm_settings_get_mailbox_active (settings)) {
    vvm_preferences_set_gtkimage_icon (self->mailbox_active_image, STATUS_GOOD);
  } else {
    vvm_preferences_set_gtkimage_icon (self->mailbox_active_image, STATUS_BAD);
  }

  VVMProvisionStatus = vvm_settings_get_vvm_provision_status (settings);
  VVMProvisionStatus = g_strdup_printf("Provision Status: %s", VVMProvisionStatus);
  vvm_preferences_set_hdyactionrow_text(self->provision_status_text, VVMProvisionStatus);

}

static void
enable_vvm_switch_flipped_cb (GtkSwitch *widget,
                                  gboolean   state,
                                  VvmPreferencesWindow *self)
{

  if (state)
    g_debug("Enable of VVM is set to true!");
  else
    g_debug("Enable of VVM is set to false!");

}

static void
carrier_prefix_button_clicked_cb (GtkButton     *btn,
                                  VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    const char *carrier_prefix = NULL;

    carrier_prefix = gtk_entry_get_text (self->carrier_prefix_text);

    vvm_settings_set_vvm_default_number (settings, carrier_prefix);
    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "CarrierPrefix",
                                      carrier_prefix) == FALSE) {

      g_autofree char *result = g_strdup("Action Failed");

      g_error("Error changing setting in vvmd!");
      gtk_entry_set_text (self->destination_number_text, result);
    }
  } else {
    g_autofree char *result = g_strdup("Action Failed");

    g_warning("VVMD Modem Manager settings not available!");
    gtk_entry_set_text (self->carrier_prefix_text, result);
  }
}

static void
default_modem_number_button_clicked_cb (GtkButton     *btn,
                                        VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    const char *modem_number = NULL;

    modem_number = gtk_entry_get_text (self->modem_number_text);

    if (strlen (modem_number) < 1) {
      modem_number = "NULL";
    }

    vvm_settings_set_vvm_default_number (settings, modem_number);
    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "DefaultModemNumber",
                                      modem_number) == FALSE) {

      g_autofree char *result = g_strdup("Action Failed");

      g_error("Error changing setting in vvmd!");
      gtk_entry_set_text (self->destination_number_text, result);
    }
  } else {
    g_autofree char *result = g_strdup("Action Failed");

    g_warning("VVMD Modem Manager settings not available!");
    gtk_entry_set_text (self->modem_number_text, result);
  }
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
vvm_preferences_set_button_content (GtkButton *btn, const char *title) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, title);
  g_object_set_property (G_OBJECT (btn), "label", &a);
}

static gboolean
refresh_settings_button_timeout(gpointer user_data)
{
  VvmPreferencesWindow *self = user_data;
  g_debug("Removing timeout to refresh_settings_button_clicked_cb()");
  self->refresh_settings_button_timeout = FALSE;
  vvm_preferences_set_button_content (self->refresh_settings_button, "Apply");

  return FALSE;
}
static void
activate_vvm_button_clicked_cb (GtkButton     *btn,
                                VvmPreferencesWindow *self)
{

  g_debug("Activate VVM Button Clicked!");
}

static void
refresh_settings_button_clicked_cb (GtkButton     *btn,
                                    VvmPreferencesWindow *self)
{

  if (self->refresh_settings_button_timeout == FALSE) {
    g_debug("Refreshing Settings.....");

    vvm_preferences_refresh_settings ();
    vvm_preferences_window_populate (self);

    self->refresh_settings_button_timeout = TRUE;
    vvm_preferences_set_button_content (self->refresh_settings_button, "Refreshing...");

    g_debug("Adding timeout to refresh_settings_button_clicked_cb()");
    //Adding two second timeout
    g_timeout_add_seconds(2,refresh_settings_button_timeout, self);
  } else {
    g_debug("refresh_settings_button_clicked_cb() timed out!");
  }
}

static void
vvm_preferences_window_class_init (VvmPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-preferences-window.ui");

  //Whether or not VVM is enabled
/*
  int    enable_vvm;
*/
  gtk_widget_class_bind_template_callback (widget_class, enable_vvm_switch_flipped_cb);

  //Refresh Page
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, refresh_settings_button);
  gtk_widget_class_bind_template_callback (widget_class, refresh_settings_button_clicked_cb);

  //Modem Manager Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, destination_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, modem_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, carrier_prefix_text);


  gtk_widget_class_bind_template_callback (widget_class, destination_number_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, default_modem_number_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, carrier_prefix_button_clicked_cb);
/*
  char  *vvm_type;
*/

  //Activate VVM
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, refresh_settings_button);
  gtk_widget_class_bind_template_callback (widget_class, activate_vvm_button_clicked_cb);

  //Status Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, vvmd_enabled_image);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, mailbox_active_image);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, provision_status_text);

}

static void
vvm_preferences_window_init (VvmPreferencesWindow *self)
{
  vvm_preferences_refresh_settings ();
  gtk_widget_init_template (GTK_WIDGET (self));
  vvm_preferences_window_populate (self);

}
