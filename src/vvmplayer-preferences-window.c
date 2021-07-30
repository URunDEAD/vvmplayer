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
  HdyActionRow *enable_vvm_text;

  //Status
  GtkImage *vvmd_enabled_image;
  GtkImage *mailbox_active_image;
  HdyActionRow *provision_status_text;

  //Activate
  GtkButton *activate_vvm_button;

  //Modem Manager Settings
  GtkEntry *destination_number_text;
  GtkEntry *modem_number_text;
  GtkEntry *carrier_prefix_text;
  HdyComboRow *vvm_type_combo_row;

  //Timeouts
  int refresh_settings_button_timeout;
  int activate_vvm_button_clicked_timeout;
  int enable_vvm_switch_clicked_timeout;
};

G_DEFINE_TYPE (VvmPreferencesWindow, vvm_preferences_window, HDY_TYPE_PREFERENCES_WINDOW)

static void
refresh_settings_button_clicked_cb (GtkButton     *btn,
                                    VvmPreferencesWindow *self)
{
  g_debug("refresh_settings_button_clicked_cb() call back!");
}

static void
enable_vvm_switch_flipped_cb (GtkSwitch *widget,
                              gboolean   state,
                              VvmPreferencesWindow *self)
{
  g_debug ("enable_vvm_switch_clicked_cb() call back!");
}

static void
activate_vvm_button_clicked_cb (GtkButton     *btn,
                                VvmPreferencesWindow *self)
{
  g_debug ("activate_vvm_button_clicked_cb() call back!");
}

static void
destination_number_button_clicked_cb (GtkButton     *btn,
                                      VvmPreferencesWindow *self)
{
  g_debug ("destination_number_button_clicked_cb() call back!");
}

static void
default_modem_number_button_clicked_cb (GtkButton     *btn,
                                        VvmPreferencesWindow *self)
{
  g_debug ("default_modem_number_button_clicked_cb() call back!");
}

static void
carrier_prefix_button_clicked_cb (GtkButton     *btn,
                                  VvmPreferencesWindow *self)
{
  g_debug ("carrier_prefix_button_clicked_cb() call back!");
}

static void
vvm_type_selected_cb (GObject              *sender,
                      GParamSpec           *pspec,
                      VvmPreferencesWindow *self)
{
  g_debug ("vvm_type_selected_cb() call back!");
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

  gtk_widget_class_bind_template_callback (widget_class, refresh_settings_button_clicked_cb);

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

  //Modem Manager Settings
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, destination_number_text);
  gtk_widget_class_bind_template_child (widget_class, VvmPreferencesWindow, modem_number_text);
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
  //vvm_preferences_refresh_settings ();
  //vvm_preferences_window_populate (self);


}
