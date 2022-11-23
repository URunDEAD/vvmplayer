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
  GtkSwitch *override_global_pref_switch;

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

  //Timeouts
  int refresh_settings_button_timeout;
  int activate_vvm_button_clicked_timeout;
  int sync_vvm_button_clicked_timeout;
  int enable_vvm_switch_clicked_timeout;

  //Watch ID
  gulong vvm_settings_watch_id;
};

G_DEFINE_TYPE (VvmplayerPreferencesWindow, vvm_player_preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

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
     adw_combo_row_set_selected (self->vvm_type_combo_row, VVM_TYPE_UNKNOWN);
  }
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
  int override_global_appearance_pref;
  GtkSettings *gtk_settings = gtk_settings_get_default ();

  //Enable VVM
  VVMEnabled = vvm_settings_get_vvm_enabled (settings);
  gtk_switch_set_active (self->enable_vvm_switch, VVMEnabled);

  //Spam Contact
  VVMSpamContact = vvm_settings_get_spam_contact (settings);
  VVMSpamContactEnabled = vvmplayer_settings_get_spam_contact_enabled (settings);
  gtk_switch_set_active (self->spam_contact_switch, VVMSpamContactEnabled);
  gtk_entry_buffer_set_text (self->spam_contact_text_buffer, VVMSpamContact, -1);

  //Appearance Settings
  override_global_appearance_pref = vvmplayer_settings_get_override_global_appearance_pref (settings);
  gtk_switch_set_active (self->override_global_pref_switch, override_global_appearance_pref);

  g_object_get (gtk_settings, "gtk-application-prefer-dark-theme", &prefer_dark_theme, NULL);
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
      g_warning ("Error getting VVMD Modem Manager Settings");
      vvm_settings_load_mm_defaults (settings);
    }
  } else {
    g_debug ("Loading VVMD MM Defaults");
    vvm_settings_load_mm_defaults (settings);
  }

  if (vvm_settings_get_service_available (settings)) {
    if (vvmplayer_vvmd_get_service_settings (backend) == FALSE) {
      g_warning ("Error getting VVMD Service Settings");
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

  //Appearance Settings
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, override_global_pref_switch);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_dark_theme_switch);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_dark_theme_row);

  //gtk_widget_class_bind_template_callback (widget_class, override_global_pref_switch_flipped_cb);
  //gtk_widget_class_bind_template_callback (widget_class, enable_theme_switch_flipped_cb);

  //Whether or not VVM is enabled
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_vvm_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, enable_vvm_switch);

  //gtk_widget_class_bind_template_callback (widget_class, enable_vvm_switch_flipped_cb);

  //Status Settings
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, vvmd_enabled_image);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, mailbox_active_image);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, provision_status_text);

  //Activate VVM
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, activate_vvm_button);

  //gtk_widget_class_bind_template_callback (widget_class, activate_vvm_button_clicked_cb);

  //Sync VVM
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, sync_vvm_button);

  //gtk_widget_class_bind_template_callback (widget_class, sync_vvm_button_clicked_cb);

  //Spam Contact
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, spam_contact_switch);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, spam_contact_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, spam_contact_text_buffer);

  //gtk_widget_class_bind_template_callback (widget_class, spam_contact_switch_flipped_cb);
  //gtk_widget_class_bind_template_callback (widget_class, spam_contact_text_button_clicked_cb);

  //Modem Manager Settings
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, destination_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, destination_number_text_buffer);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, modem_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, modem_number_text_buffer);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, carrier_prefix_text_row);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, carrier_prefix_text);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, carrier_prefix_text_buffer);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerPreferencesWindow, vvm_type_combo_row);

  gtk_widget_class_bind_template_callback (widget_class, vvm_type_selected_cb);
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
