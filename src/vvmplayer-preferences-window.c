/* vvmplayer-preferences-window.h
 *
 * Copyright 2021-2022 Chris Talbot
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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "vvmplayer-preferences-window.h"
#include "vvmplayer-settings.h"
#include "vvmplayer-vvmd.h"

#define STATUS_GOOD "emblem-ok-symbolic"
#define STATUS_BAD "window-close-symbolic"

struct _VvmplayerPreferencesWindow
{
  AdwPreferencesWindow parent_instance;

  //Refresh
  GtkButton *refresh_settings_button;

  //Enable VVM
  GtkSwitch *enable_vvm_switch;

  AdwActionRow *enable_vvm_text;

  //Appearance Settings
  AdwActionRow *enable_dark_theme_row;
  GtkSwitch *enable_dark_theme_switch;

  //Status
  GtkImage *vvmd_enabled_image;
  GtkImage *mailbox_active_image;
  AdwActionRow *provision_status_text;

  //Activate
  GtkButton *activate_vvm_button;

  //Sync
  GtkButton *sync_vvm_button;

  //Modem Manager Settings
  GtkEntry *destination_number_text;
  GtkEntryBuffer *destination_number_text_buffer;
  GtkEntry *modem_number_text;
  GtkEntryBuffer *modem_number_text_buffer;
  AdwActionRow *carrier_prefix_text_row;
  GtkEntry *carrier_prefix_text;
  GtkEntryBuffer *carrier_prefix_text_buffer;
  AdwComboRow *vvm_type_combo_row;

  //Spam Contact
  GtkSwitch *spam_contact_switch;
  GtkEntry *spam_contact_text;
  GtkEntryBuffer *spam_contact_text_buffer;

  //Watch ID
  gulong vvm_settings_watch_id;
};

G_DEFINE_TYPE (VvmplayerPreferencesWindow, vvm_player_preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

static void update_settings_cb (VvmplayerPreferencesWindow *self);

static void
vvm_preferences_display_toast (const char                 *title,
                               VvmplayerPreferencesWindow *self)
{
     AdwToast *toast = adw_toast_new (title);
     adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self), toast);
     adw_toast_set_timeout (toast, 1);
}

static void
spam_contact_text_button_clicked_cb (GtkButton                  *btn,
                                     VvmplayerPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  const char *new_contact;

  new_contact = gtk_entry_buffer_get_text (self->spam_contact_text_buffer);
  vvm_settings_set_spam_contact (settings, new_contact);
}

static gboolean
spam_contact_switch_flipped_cb (GtkSwitch                  *widget,
                                gboolean                    contact_switch_bool,
                                VvmplayerPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();

  vvmplayer_settings_set_spam_contact_enabled (settings, contact_switch_bool);
  return FALSE;
}

static void
vvm_type_selected_cb (GObject                    *sender,
                      GParamSpec                 *pspec,
                      VvmplayerPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();
  int row_selected;

  row_selected = adw_combo_row_get_selected (self->vvm_type_combo_row);

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
      g_warning ("Error changing setting in vvmd!");
      adw_combo_row_set_selected (self->vvm_type_combo_row, VVM_TYPE_UNKNOWN);
    } else {

        if (row_selected == VVM_TYPE_ATTUSA) {
           adw_preferences_row_set_title (ADW_PREFERENCES_ROW (self->carrier_prefix_text_row),
                                                 "AT&amp;T USA Number (No dashes or spaces)");
           gtk_entry_set_placeholder_text(self->carrier_prefix_text,
                                                 "AT&T USA Number (No dashes or spaces)");

        } else {
           adw_preferences_row_set_title (ADW_PREFERENCES_ROW (self->carrier_prefix_text_row),
                                          "Carrier Prefix");
           gtk_entry_set_placeholder_text (self->carrier_prefix_text,
                                           "Carrier Prefix");
        }
    }
  } else {
     vvm_preferences_display_toast ("Cannot Find VVMD", self);
     adw_combo_row_set_selected (self->vvm_type_combo_row, VVM_TYPE_UNKNOWN);
  }
}

static gboolean
enable_theme_switch_flipped_cb (GtkSwitch                  *widget,
                                gboolean                    prefer_dark_theme,
                                VvmplayerPreferencesWindow *self)
{
  AdwStyleManager *adw_style_manager = adw_style_manager_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if (prefer_dark_theme)
    adw_style_manager_set_color_scheme (adw_style_manager, ADW_COLOR_SCHEME_FORCE_DARK);
  else
    adw_style_manager_set_color_scheme (adw_style_manager, ADW_COLOR_SCHEME_DEFAULT);

  vvmplayer_settings_set_dark_theme (settings, prefer_dark_theme);
  return FALSE;
}

static void
refresh_settings_button_clicked_cb (GtkButton                  *btn,
                                    VvmplayerPreferencesWindow *self)
{
    vvm_preferences_display_toast ("Refreshing Settings", self);
    update_settings_cb (self);
}

static void
set_destination_number (VvmplayerPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if (vvm_settings_get_mm_available (settings)) {
    const char *dest_number = NULL;

    dest_number = gtk_entry_buffer_get_text (self->destination_number_text_buffer);

    vvm_settings_set_vvm_destination_number (settings, dest_number);
    if (vvmplayer_vvmd_set_mm_setting (backend,
                                      "VVMDestinationNumber",
                                      dest_number) == FALSE) {

      vvm_preferences_display_toast ("Changing setting failed", self);
    }
  } else
    g_warning("VVMD Modem Manager settings not available!");
}

static void
set_default_modem_number (VvmplayerPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    const char *modem_number = NULL;

    modem_number = gtk_entry_buffer_get_text (self->modem_number_text_buffer);

    if (strlen (modem_number) < 1) {
      modem_number = "NULL";
    }

    vvm_settings_set_vvm_default_number (settings, modem_number);
    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "DefaultModemNumber",
                                      modem_number) == FALSE) {

      vvm_preferences_display_toast ("Changing setting failed", self);
    }
  } else
    g_warning("VVMD Modem Manager settings not available!");

}

static void
set_carrier_prefix (VvmplayerPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    const char *carrier_prefix = NULL;

    carrier_prefix = gtk_entry_buffer_get_text (self->carrier_prefix_text_buffer);

    vvm_settings_set_vvm_carrier_prefix (settings, carrier_prefix);
    if(vvmplayer_vvmd_set_mm_setting (backend,
                                      "CarrierPrefix",
                                      carrier_prefix) == FALSE) {

      g_warning ("Error changing setting in vvmd!");
      vvm_preferences_display_toast ("Changing setting failed", self);
    }
  } else
    g_warning("VVMD Modem Manager settings not available!");

}

static gboolean
enable_vvm_switch_timeout(gpointer user_data)
{
  VvmplayerPreferencesWindow *self = user_data;
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMActive = vvm_settings_get_mailbox_active (settings);
  int EnableVVMSwitch = gtk_switch_get_state (self->enable_vvm_switch);
  const char *toast_message = NULL;

  gtk_widget_set_sensitive (GTK_WIDGET (self->enable_vvm_switch), TRUE);
  update_settings_cb (self);

  if (!VVMActive && !EnableVVMSwitch)
    toast_message = "VVM Disabled";
  else if (!VVMActive && EnableVVMSwitch)
    toast_message = "Could not enable VVM";
  else if (VVMActive && !EnableVVMSwitch)
    toast_message = "Could not disable VVM";
  else if (VVMActive && EnableVVMSwitch)
    toast_message = "VVM Enabled";

  vvm_preferences_display_toast (toast_message, self);

  return FALSE;
}

static gboolean
enable_vvm_switch_flipped_cb (GtkSwitch                  *widget,
                              gboolean                    state,
                              VvmplayerPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  VvmVvmd *backend = vvm_vvmd_get_default ();

  if (VVMEnabled == state) {
    return FALSE;
  }

  gtk_widget_set_sensitive (GTK_WIDGET (widget), FALSE);

  if(vvm_settings_get_mm_available (settings) == FALSE) {
    g_debug ("Cannot find vvmd!");
    vvm_preferences_display_toast ("Cannot find vvmd", self);
    g_timeout_add_seconds (2, enable_vvm_switch_timeout, self);
    return FALSE;
  }



  if (state) {
    g_debug ("Enabling VVM");
    vvm_preferences_display_toast ("Enabling VVM...", self);

  } else {
    g_debug ("Disabling VVM");
    vvm_preferences_display_toast ("Disabling VVM...", self);
  }

  g_timeout_add_seconds (10, enable_vvm_switch_timeout, self);

  set_destination_number (self);
  set_default_modem_number (self);
  set_carrier_prefix (self);
  update_settings_cb (self);

  vvmplayer_vvmd_set_mm_vvm_enabled (backend, state);

  return FALSE;
}

static gboolean
activate_vvm_button_timeout(gpointer user_data)
{
  VvmplayerPreferencesWindow *self = user_data;
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMActive = vvm_settings_get_mailbox_active (settings);
  int EnableVVMSwitch = gtk_switch_get_state (self->enable_vvm_switch);
  const char *toast_message = NULL;

  gtk_widget_set_sensitive (GTK_WIDGET (self->activate_vvm_button), TRUE);
  update_settings_cb (self);

  if (!VVMActive && !EnableVVMSwitch)
    toast_message = "VVM Disabled";
  else if (!VVMActive && EnableVVMSwitch)
    toast_message = "Could not activate VVM";
  else if (VVMActive && !EnableVVMSwitch)
    toast_message = "Could not deactivate VVM";
  else if (VVMActive && EnableVVMSwitch)
    toast_message = "VVM Enabled";

  vvm_preferences_display_toast (toast_message, self);

  return FALSE;
}

static void
activate_vvm_button_clicked_cb (GtkButton     *btn,
                                VvmplayerPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  int MailboxActive = vvm_settings_get_mailbox_active (settings);

  VvmVvmd *backend = vvm_vvmd_get_default ();

  if(vvm_settings_get_mm_available (settings) == FALSE) {
    g_debug ("Cannot find VVMD!");
    vvm_preferences_display_toast ("Cannot find VVMD", self);
    return;
  }

  if (VVMEnabled == FALSE) {
    g_debug ("VVM not Active!");
    vvm_preferences_display_toast ("VVM not enabled in VVM Player", self);
    return;
  }

  if (MailboxActive == TRUE) {
    g_debug ("Mailbox already Active!");
    vvm_preferences_display_toast ("Mailbox already active", self);
    return;
  }
  g_debug ("Activating VVM.....");
  vvm_preferences_display_toast ("Activating VVM", self);
  gtk_widget_set_sensitive (GTK_WIDGET (self->activate_vvm_button), FALSE);
  g_timeout_add_seconds (10, activate_vvm_button_timeout, self);

  set_destination_number (self);
  set_default_modem_number (self);
  set_carrier_prefix (self);
  update_settings_cb (self);

  vvmplayer_vvmd_check_subscription_status (backend);
}

static gboolean
sync_vvm_button_timeout(gpointer user_data)
{
  VvmplayerPreferencesWindow *self = user_data;
  gtk_widget_set_sensitive (GTK_WIDGET (self->sync_vvm_button), TRUE);

  return FALSE;
}

static void
sync_vvm_button_clicked_cb (GtkButton                  *btn,
                            VvmplayerPreferencesWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();
  int VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  int MailboxActive = vvm_settings_get_mailbox_active (settings);
  VvmVvmd *backend = vvm_vvmd_get_default ();

  if(vvm_settings_get_mm_available (settings) == FALSE) {
    g_debug ("Cannot find vvmd!");
    vvm_preferences_display_toast ("Cannot find VVMD", self);
    return;
  }

  if (VVMEnabled == FALSE) {
    g_debug ("VVM not Active!");
    vvm_preferences_display_toast ("VVM not enabled", self);
    return;
  }

  if (MailboxActive == FALSE) {
    g_debug ("Mailbox not Active!");
    vvm_preferences_display_toast ("Mailbox not active", self);
    return;
  }

  g_debug ("Syncing VVM.....");
  vvm_preferences_display_toast ("Syncing...", self);
  gtk_widget_set_sensitive (GTK_WIDGET (self->sync_vvm_button), FALSE);
  g_timeout_add_seconds (10, sync_vvm_button_timeout, self);
  vvmplayer_vvmd_sync_vvm (backend);
}

static void
vvm_preferences_window_populate (VvmplayerPreferencesWindow *self)
{
  g_autofree char     *VVMDestinationNumber = NULL;
  g_autofree char     *VVMDefaultModemNumber = NULL;
  g_autofree char     *VVMProvisionStatus = NULL;
  g_autofree char     *VVMCarrierPrefix = NULL;
  g_autofree char     *VVMType = NULL;
  g_autofree char     *VVMSpamContact = NULL;
  int VVMEnabled;
  gboolean VVMSpamContactEnabled;
  int VVMType_encoded, vvm_type_combo_row_state;
  VvmSettings *settings = vvm_settings_get_default ();
  int prefer_dark_theme, enable_dark_theme_switch_state;

  //Enable VVM
  VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  gtk_switch_set_active (self->enable_vvm_switch, VVMEnabled);

  //Spam Contact
  VVMSpamContact = vvm_settings_get_spam_contact (settings);
  VVMSpamContactEnabled = vvmplayer_settings_get_spam_contact_enabled (settings);
  gtk_switch_set_active (self->spam_contact_switch, VVMSpamContactEnabled);
  gtk_entry_buffer_set_text (self->spam_contact_text_buffer, VVMSpamContact, -1);

  prefer_dark_theme = vvmplayer_settings_get_dark_theme (settings);

  enable_dark_theme_switch_state = gtk_switch_get_state (self->enable_dark_theme_switch);
  if (prefer_dark_theme != enable_dark_theme_switch_state)
    gtk_switch_set_active (self->enable_dark_theme_switch, prefer_dark_theme);

  //Modem Manager settings
  VVMDestinationNumber = vvm_settings_get_vvm_destination_number (settings);
  if (strstr (VVMDestinationNumber, "invalid") == NULL) { //Don't populate invalid settings
    gtk_entry_buffer_set_text (self->destination_number_text_buffer, VVMDestinationNumber, -1);
  } else {
    gtk_entry_buffer_set_text (self->destination_number_text_buffer, "", -1);
  }

  VVMDefaultModemNumber = vvm_settings_get_vvm_default_number (settings);
  if (g_strcmp0 (VVMDefaultModemNumber, "NULL") != 0) { //Don't populate invalid settings
    gtk_entry_buffer_set_text (self->modem_number_text_buffer, VVMDefaultModemNumber, -1);
  } else {
    gtk_entry_buffer_set_text (self->modem_number_text_buffer, "", -1);
  }

  VVMCarrierPrefix = vvm_settings_get_vvm_carrier_prefix (settings);
  if (strstr (VVMCarrierPrefix, "invalid") == NULL) { //Don't populate invalid settings
    gtk_entry_buffer_set_text (self->carrier_prefix_text_buffer, VVMCarrierPrefix, -1);
  } else {
    gtk_entry_buffer_set_text (self->carrier_prefix_text_buffer, "", -1);
  }

  VVMType = vvm_settings_get_vvm_type (settings);
  VVMType_encoded = vvmplayer_vvmd_encode_vvm_type (VVMType);
  g_debug("VVMType: %s, VVMType_encoded: %d", VVMType, VVMType_encoded);
  vvm_type_combo_row_state = adw_combo_row_get_selected (self->vvm_type_combo_row);
  if (vvm_type_combo_row_state != VVMType_encoded)
    adw_combo_row_set_selected (self->vvm_type_combo_row, VVMType_encoded);

  //Status
  if(vvm_settings_get_mm_available (settings)) {
    gtk_image_set_from_icon_name (self->vvmd_enabled_image, STATUS_GOOD);
  } else {
    gtk_image_set_from_icon_name (self->vvmd_enabled_image, STATUS_BAD);
  }

  if (vvm_settings_get_mailbox_active (settings)) {
    gtk_image_set_from_icon_name (self->mailbox_active_image, STATUS_GOOD);
  } else {
    gtk_image_set_from_icon_name (self->mailbox_active_image, STATUS_BAD);
  }

  VVMProvisionStatus = vvm_settings_get_vvm_provision_status (settings);
  VVMProvisionStatus = g_strdup_printf ("Provision Status: %s", VVMProvisionStatus);
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (self->provision_status_text), VVMProvisionStatus);

}

static void
vvm_preferences_refresh_settings (void)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  VvmSettings *settings = vvm_settings_get_default ();

  if(vvm_settings_get_mm_available (settings)) {
    if (vvmplayer_vvmd_get_mm_settings(backend) == FALSE) {
      g_debug ("Error getting VVMD Modem Manager Settings");
      vvm_settings_load_mm_defaults (settings);
    }
  } else {
    g_debug ("Loading VVMD MM Defaults");
    vvm_settings_load_mm_defaults (settings);
  }

  if (vvm_settings_get_service_available (settings)) {
    if (vvmplayer_vvmd_get_service_settings (backend) == FALSE) {
      g_debug ("Error getting VVMD Service Settings");
      vvm_settings_load_service_defaults (settings);
    }
  } else {
    g_debug ("Loading VVMD service Defaults");
    vvm_settings_load_service_defaults (settings);
  }
}

VvmplayerPreferencesWindow *
vvm_player_preferences_window_new (void)
{
  return g_object_new (ADW_TYPE_DEMO_PREFERENCES_WINDOW, NULL);
}

static void
vvm_player_preferences_window_class_init (VvmplayerPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/ui/vvmplayer-preferences-window.ui");

  //Refresh Page
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, refresh_settings_button);

  gtk_widget_class_bind_template_callback (widget_class, refresh_settings_button_clicked_cb);

  //Appearance Settings
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_dark_theme_switch);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_dark_theme_row);

  gtk_widget_class_bind_template_callback (widget_class, enable_theme_switch_flipped_cb);

  //Whether or not VVM is enabled
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_vvm_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_vvm_switch);

  gtk_widget_class_bind_template_callback (widget_class, enable_vvm_switch_flipped_cb);

  //Status Settings
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, vvmd_enabled_image);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, mailbox_active_image);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, provision_status_text);

  gtk_widget_class_bind_template_callback (widget_class, vvm_type_selected_cb);

  //Activate VVM
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, activate_vvm_button);

  gtk_widget_class_bind_template_callback (widget_class, activate_vvm_button_clicked_cb);

  //Sync VVM
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, sync_vvm_button);

  gtk_widget_class_bind_template_callback (widget_class, sync_vvm_button_clicked_cb);

  //Spam Contact
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, spam_contact_switch);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, spam_contact_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, spam_contact_text_buffer);

  gtk_widget_class_bind_template_callback (widget_class, spam_contact_switch_flipped_cb);
  gtk_widget_class_bind_template_callback (widget_class, spam_contact_text_button_clicked_cb);

  //Modem Manager Settings
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, destination_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, destination_number_text_buffer);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, modem_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, modem_number_text_buffer);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, carrier_prefix_text_row);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, carrier_prefix_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, carrier_prefix_text_buffer);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, vvm_type_combo_row);
}

static void
update_settings_cb (VvmplayerPreferencesWindow *self)
{
    vvm_preferences_refresh_settings ();
    vvm_preferences_window_populate (self);
}

static void
vvm_player_preferences_window_init (VvmplayerPreferencesWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();

  gtk_widget_init_template (GTK_WIDGET (self));
  vvm_preferences_refresh_settings ();
  vvm_preferences_window_populate (self);

  self->vvm_settings_watch_id = g_signal_connect_swapped (backend, "settings-updated",
                                                          G_CALLBACK (update_settings_cb), self);
}
