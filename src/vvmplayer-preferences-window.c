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

struct _VvmPreferencesWindow
{
  HdyPreferencesWindow parent_instance;

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
vvm_preferences_window_populate (VvmPreferencesWindow *self)
{

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
vvm_preferences_window_class_init (VvmPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-preferences-window.ui");

  //Modem Manager Settings
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
  gtk_widget_init_template (GTK_WIDGET (self));

  vvm_preferences_window_populate (self);

}
