/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-preferences-window.c
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

#define G_LOG_DOMAIN "vvmplayer-preferences-window"

#ifdef HAVE_CONFIG_H
# include "config.h"
# include "version.h"
#endif

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

  //Enable VVM
  GtkSwitch *enable_vvm_switch;
  GtkSwitch *enable_dark_theme_switch;
  HdyActionRow *enable_vvm_text;

  //Status
  GtkImage *vvmd_enabled_image;
  GtkImage *mailbox_active_image;
  HdyActionRow *provision_status_text;

  //Activate
  GtkButton *activate_vvm_button;

  //Sync
  GtkButton *sync_vvm_button;

  //Modem Manager Settings
  GtkEntry *destination_number_text;
  GtkEntry *modem_number_text;
  HdyActionRow *carrier_prefix_text_row;
  GtkEntry *carrier_prefix_text;
  HdyComboRow *vvm_type_combo_row;

  //Timeouts
  int refresh_settings_button_timeout;
  int activate_vvm_button_clicked_timeout;
  int sync_vvm_button_clicked_timeout;
  int enable_vvm_switch_clicked_timeout;
};

G_DEFINE_TYPE (VvmPreferencesWindow, vvm_preferences_window, HDY_TYPE_PREFERENCES_WINDOW)

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
vvm_preferences_set_button_content (GtkButton *btn, const char *title) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, title);
  g_object_set_property (G_OBJECT (btn), "label", &a);
}

static void
vvm_preferences_refresh_settings (void)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    if (vvmplayer_vvmd_get_mm_settings(backend) == FALSE) {
      g_error ("Error getting VVMD Modem Manager Settings");
      vvm_settings_load_mm_defaults (settings);
    }
  } else {
    g_debug ("Loading VVMD MM Defaults");
    vvm_settings_load_mm_defaults (settings);
  }

  if (vvm_settings_get_service_available (settings)) {
    if (vvmplayer_vvmd_get_service_settings (backend) == FALSE) {
      g_error("Error getting VVMD Service Settings");
      vvm_settings_load_service_defaults (settings);
    }
  } else {
    g_debug ("Loading VVMD service Defaults");
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
  g_autofree char     *VVMType = NULL;
  int VVMEnabled;
  int VVMType_encoded, vvm_type_combo_row_state;
  VvmSettings *settings = vvm_settings_get_default ();
  int prefer_dark_theme, enable_dark_theme_switch_state;
  GtkSettings *gtk_settings = gtk_settings_get_default ();

  //Enable VVM
  VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  gtk_switch_set_active (self->enable_vvm_switch, VVMEnabled);

  g_object_get (gtk_settings, "gtk-application-prefer-dark-theme", &prefer_dark_theme, NULL);
  enable_dark_theme_switch_state = gtk_switch_get_state (self->enable_dark_theme_switch);
  if (prefer_dark_theme != enable_dark_theme_switch_state)
    gtk_switch_set_active (self->enable_dark_theme_switch, prefer_dark_theme);

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

  VVMType = vvm_settings_get_vvm_type (settings);
  VVMType_encoded = vvmplayer_vvmd_encode_vvm_type (VVMType);
  g_debug("VVMType: %s, VVMType_encoded: %d", VVMType, VVMType_encoded);
  vvm_type_combo_row_state = hdy_combo_row_get_selected_index (self->vvm_type_combo_row);
  if (vvm_type_combo_row_state != VVMType_encoded)
    hdy_combo_row_set_selected_index (self->vvm_type_combo_row, VVMType_encoded);

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
  VVMProvisionStatus = g_strdup_printf ("Provision Status: %s", VVMProvisionStatus);
  vvm_preferences_set_hdyactionrow_text (self->provision_status_text, VVMProvisionStatus);

}

static gboolean
refresh_settings_button_timeout (gpointer user_data)
{
  VvmPreferencesWindow *self = user_data;
  g_debug("Removing timeout to refresh_settings_button_clicked_cb ()");
  self->refresh_settings_button_timeout = FALSE;

  return FALSE;
}

static void
refresh_settings_button_clicked_cb (GtkButton     *btn,
                                    VvmPreferencesWindow *self)
{

  if (self->refresh_settings_button_timeout == FALSE) {
    g_debug ("Refreshing Settings.....");
    g_debug ("Adding timeout to refresh_settings_button_clicked_cb ()");
    //Adding one hundred millisecond timeout
    g_timeout_add (100,refresh_settings_button_timeout, self);

    vvm_preferences_refresh_settings ();
    vvm_preferences_window_populate (self);

    self->refresh_settings_button_timeout = TRUE;

  } else {
    g_debug ("refresh_settings_button_clicked_cb() timed out!");
  }
}

static void
enable_theme_switch_flipped_cb (GtkSwitch *widget,
                                gboolean   prefer_dark_theme,
                                VvmPreferencesWindow *self)
{
  GtkSettings *gtk_settings = gtk_settings_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  g_object_set (gtk_settings, "gtk-application-prefer-dark-theme", prefer_dark_theme, NULL);
  vvmplayer_settings_set_dark_theme (settings, prefer_dark_theme);
}

static void
destination_number_button_clicked_cb (GtkButton     *btn,
                                      VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  /*
   * You will need to modify activate_vvm_button_clicked_cb() and
   * enable_vvm_switch_flipped_cb() if you do anything with *btn
   */

  if (vvm_settings_get_mm_available (settings)) {
    const char *dest_number = NULL;

    dest_number = gtk_entry_get_text (self->destination_number_text);

    vvm_settings_set_vvm_destination_number (settings, dest_number);
    if (vvmplayer_vvmd_set_mm_setting (backend,
                                      "VVMDestinationNumber",
                                      dest_number) == FALSE) {

      g_autofree char *result = g_strdup ("Action Failed");

      g_error ("Error changing setting in vvmd!");
      gtk_entry_set_text (self->destination_number_text, result);
    }
  } else {
    g_autofree char *result = g_strdup ("Cannot find vvmd");

    g_warning("VVMD Modem Manager settings not available!");
    gtk_entry_set_text (self->destination_number_text, result);
  }
}

static void
default_modem_number_button_clicked_cb (GtkButton     *btn,
                                        VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  /*
   * You will need to modify activate_vvm_button_clicked_cb() and
   * enable_vvm_switch_flipped_cb() if you do anything with *btn
   */

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
    g_autofree char *result = g_strdup ("Cannot find vvmd");

    g_warning("VVMD Modem Manager settings not available!");
    gtk_entry_set_text (self->modem_number_text, result);
  }
}

static void
carrier_prefix_button_clicked_cb (GtkButton     *btn,
                                  VvmPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  /*
   * You will need to modify activate_vvm_button_clicked_cb() and
   * enable_vvm_switch_flipped_cb() if you do anything with *btn
   */

  if(vvm_settings_get_mm_available (settings)) {
    const char *carrier_prefix = NULL;

    carrier_prefix = gtk_entry_get_text (self->carrier_prefix_text);

    vvm_settings_set_vvm_default_number (settings, carrier_prefix);
    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "CarrierPrefix",
                                      carrier_prefix) == FALSE) {

      g_autofree char *result = g_strdup ("Action Failed");

      g_error("Error changing setting in vvmd!");
      gtk_entry_set_text (self->destination_number_text, result);
    }
  } else {
    g_autofree char *result = g_strdup ("Cannot find vvmd");

    g_warning("VVMD Modem Manager settings not available!");
    gtk_entry_set_text (self->carrier_prefix_text, result);
  }
}

static gboolean
enable_vvm_switch_timeout(gpointer user_data)
{
  VvmPreferencesWindow *self = user_data;

  self->enable_vvm_switch_clicked_timeout = FALSE;
  vvm_preferences_set_hdyactionrow_text (self->enable_vvm_text, "Enable VVM");

  refresh_settings_button_clicked_cb (self->refresh_settings_button, self);

  return FALSE;
}

static void
enable_vvm_switch_flipped_cb (GtkSwitch *widget,
                              gboolean   state,
                              VvmPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMEnabled = vvm_settings_get_vvm_enabled (settings);

  if (VVMEnabled == state) {
    return;
  }

  if (self->enable_vvm_switch_clicked_timeout == FALSE) {
    VvmVvmd *backend = vvm_vvmd_get_default ();
    self->enable_vvm_switch_clicked_timeout = TRUE;

    if(vvm_settings_get_mm_available (settings) == FALSE) {
      g_warning("Cannot find VVMD!");
      vvm_preferences_set_hdyactionrow_text(self->enable_vvm_text, "Cannot find vvmd");
      g_timeout_add_seconds (2, enable_vvm_switch_timeout, self);
      gtk_switch_set_active (self->enable_vvm_switch, FALSE);
      return;
    }

    g_timeout_add_seconds(10, enable_vvm_switch_timeout, self);

    if (state) {
      g_debug ("Enabling VVM");
      vvm_preferences_set_hdyactionrow_text(self->enable_vvm_text, "Enabling VVM...");

      /* In case the user did not hit apply, just refresh all settings */
      destination_number_button_clicked_cb (NULL, self);
      default_modem_number_button_clicked_cb (NULL, self);
      carrier_prefix_button_clicked_cb (NULL, self);

    } else {
      g_debug ("Disabling VVM");
      vvm_preferences_set_hdyactionrow_text(self->enable_vvm_text, "Disabling VVM...");
    }

    vvmplayer_vvmd_set_mm_vvm_enabled (backend, state);

  } else {
    g_debug ("enable_vvm_switch_clicked_cb() timed out!");
    gtk_switch_set_active (self->enable_vvm_switch, VVMEnabled);
  }

}

static gboolean
activate_vvm_button_timeout(gpointer user_data)
{
  VvmPreferencesWindow *self = user_data;
  self->activate_vvm_button_clicked_timeout = FALSE;
  vvm_preferences_set_button_content (self->activate_vvm_button, "Activate");

  refresh_settings_button_clicked_cb (self->refresh_settings_button, self);
  return FALSE;
}

static void
activate_vvm_button_clicked_cb (GtkButton     *btn,
                                VvmPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  int MailboxActive = vvm_settings_get_mailbox_active (settings);

  if (self->activate_vvm_button_clicked_timeout == FALSE) {
    VvmVvmd *backend = vvm_vvmd_get_default ();
    self->activate_vvm_button_clicked_timeout = TRUE;

    if(vvm_settings_get_mm_available (settings) == FALSE) {
      g_warning ("Cannot find vvmd!");
      vvm_preferences_set_button_content (self->activate_vvm_button, "Cannot find vvmd");
      g_timeout_add_seconds (2, activate_vvm_button_timeout, self);
      return;
    }

    if (VVMEnabled == FALSE) {
      g_warning ("VVM not Active!");
      vvm_preferences_set_button_content (self->activate_vvm_button, "VVM not enabled in VVM Player");
      g_timeout_add_seconds (2, activate_vvm_button_timeout, self);
      return;
    }

    if (MailboxActive == TRUE) {
      g_warning ("Mailbox already Active!");
      vvm_preferences_set_button_content (self->activate_vvm_button, "Mailbox already active");
      g_timeout_add_seconds (2, activate_vvm_button_timeout, self);
      return;
    }
    g_debug ("Activating VVM.....");
    g_timeout_add_seconds (10, activate_vvm_button_timeout, self);

    /* In case the user did not hit apply, just refresh all settings */
    destination_number_button_clicked_cb (NULL, self);
    default_modem_number_button_clicked_cb (NULL, self);
    carrier_prefix_button_clicked_cb (NULL, self);

    vvm_preferences_set_button_content (self->activate_vvm_button, "Activating...");

    vvmplayer_vvmd_check_subscription_status (backend);

  } else {
    g_debug("activate_vvm_button_clicked_cb() timed out!");
  }
}

static gboolean
sync_vvm_button_timeout(gpointer user_data)
{
  VvmPreferencesWindow *self = user_data;
  self->sync_vvm_button_clicked_timeout = FALSE;
  vvm_preferences_set_button_content (self->sync_vvm_button, "Sync");

  return FALSE;
}

static void
sync_vvm_button_clicked_cb (GtkButton     *btn,
                                VvmPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  int MailboxActive = vvm_settings_get_mailbox_active (settings);

  if (self->sync_vvm_button_clicked_timeout == FALSE) {
    VvmVvmd *backend = vvm_vvmd_get_default ();
    g_debug ("Syncing VVM.....");
    self->sync_vvm_button_clicked_timeout = TRUE;

    if(vvm_settings_get_mm_available (settings) == FALSE) {
      g_warning ("Cannot find VVMD!");
      vvm_preferences_set_button_content (self->sync_vvm_button, "Cannot find VVMD");
      g_timeout_add_seconds (2, sync_vvm_button_timeout, self);
      return;
    }

    if (VVMEnabled == FALSE) {
      g_warning ("VVM not Active!");
      vvm_preferences_set_button_content (self->sync_vvm_button, "VVM not enabled in VVM Player");
      g_timeout_add_seconds (2, sync_vvm_button_timeout, self);
      return;
    }

    if (MailboxActive == FALSE) {
      g_warning ("Mailbox not Active!");
      vvm_preferences_set_button_content (self->sync_vvm_button, "Mailbox not active");
      g_timeout_add_seconds (2, sync_vvm_button_timeout, self);
      return;
    }

    vvm_preferences_set_button_content (self->sync_vvm_button, "Syncing...");

    g_debug ("Adding timeout to sync_vvm_button_clicked_cb()");
    g_timeout_add_seconds (10, sync_vvm_button_timeout, self);

    vvmplayer_vvmd_sync_vvm (backend);

  } else {
    g_debug ("sync_vvm_button_clicked_cb() timed out!");
  }
}

static void
vvm_preferences_set_placeholder_text (GtkEntry *placeholder, const char *text) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, text);
  g_object_set_property (G_OBJECT (placeholder), "placeholder-text", &a);
}

static void
vvm_type_selected_cb (GObject              *sender,
                      GParamSpec           *pspec,
                      VvmPreferencesWindow *self)
{
  //HdyComboRow *row = HDY_COMBO_ROW (sender);
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();
  int row_selected;

  row_selected = hdy_combo_row_get_selected_index (self->vvm_type_combo_row);

  if (row_selected == VVM_TYPE_UNKNOWN) {
    //This is not a valid vvmd type, so don't set it
    return;
  }

  if(vvm_settings_get_mm_available (settings)) {
    g_autofree char *vvm_type = NULL;


    vvm_type = vvmplayer_vvmd_decode_vvm_type (row_selected);

    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "VVMType",
                                      vvm_type) == FALSE) {
      g_error("Error changing setting in vvmd!");
      hdy_combo_row_set_selected_index (self->vvm_type_combo_row, VVM_TYPE_UNKNOWN);
    } else {

        if (row_selected == VVM_TYPE_ATTUSA) {
           vvm_preferences_set_hdyactionrow_text(self->carrier_prefix_text_row,
                                                 "AT&T USA Number (No dashes or spaces)");
           vvm_preferences_set_placeholder_text(self->carrier_prefix_text,
                                                 "AT&T USA Number (No dashes or spaces)");

        } else {
           vvm_preferences_set_hdyactionrow_text(self->carrier_prefix_text_row, "Carrier Prefix");
           vvm_preferences_set_placeholder_text(self->carrier_prefix_text,
                                                 "Carrier Prefix");
        }
    }
  } else {
     hdy_combo_row_set_selected_index (self->vvm_type_combo_row, VVM_TYPE_UNKNOWN);
  }

}

static void
lists_page_init (VvmPreferencesWindow *self)
{

  GListStore *list_store;
  HdyValueObject *obj;

  list_store = g_list_store_new (HDY_TYPE_VALUE_OBJECT);

  obj = hdy_value_object_new_string ("Unknown");
  g_list_store_insert (list_store, VVM_TYPE_UNKNOWN, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("cvvm");
  g_list_store_insert (list_store, VVM_TYPE_CVVM, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("AT&T USA");
  g_list_store_insert (list_store, VVM_TYPE_ATTUSA, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("otmp");
  g_list_store_insert (list_store, VVM_TYPE_OTMP, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("vvm3");
  g_list_store_insert (list_store, VVM_TYPE_VVM_THREE, obj);
  g_clear_object (&obj);

  hdy_combo_row_bind_name_model (self->vvm_type_combo_row,
                                 G_LIST_MODEL (list_store),
                                (HdyComboRowGetNameFunc) hdy_value_object_dup_string,
                                 NULL, NULL);
}

VvmPreferencesWindow *
vvm_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_VVM_PREFERENCES_WINDOW, NULL);
}

static void
vvm_preferences_window_class_init (VvmPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/kop316/vvmplayer/"
                                               "ui/vvmplayer-preferences-window.ui");

  //Refresh Page
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, refresh_settings_button);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, enable_dark_theme_switch);

  gtk_widget_class_bind_template_callback (widget_class, refresh_settings_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, enable_theme_switch_flipped_cb);

  //Whether or not VVM is enabled
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, enable_vvm_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, enable_vvm_switch);

  gtk_widget_class_bind_template_callback (widget_class, enable_vvm_switch_flipped_cb);

  //Status Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, vvmd_enabled_image);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, mailbox_active_image);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, provision_status_text);

  //Activate VVM
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, activate_vvm_button);
  gtk_widget_class_bind_template_callback (widget_class, activate_vvm_button_clicked_cb);


  //Sync VVM
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, sync_vvm_button);
  gtk_widget_class_bind_template_callback (widget_class, sync_vvm_button_clicked_cb);

  //Modem Manager Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, destination_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, modem_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, carrier_prefix_text_row);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, carrier_prefix_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, vvm_type_combo_row);

  gtk_widget_class_bind_template_callback (widget_class, destination_number_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, default_modem_number_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, carrier_prefix_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, vvm_type_selected_cb);
}

static void
vvm_preferences_window_init (VvmPreferencesWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  lists_page_init (self);
  vvm_preferences_refresh_settings ();
  vvm_preferences_window_populate (self);


}
