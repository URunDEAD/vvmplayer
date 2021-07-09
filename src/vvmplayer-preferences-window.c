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

struct _HdyVvmPreferencesWindow
{
  HdyPreferencesWindow parent_instance;

};

G_DEFINE_TYPE (HdyVvmPreferencesWindow, hdy_vvm_preferences_window, HDY_TYPE_PREFERENCES_WINDOW)

HdyVvmPreferencesWindow *
hdy_vvm_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_VVM_PREFERENCES_WINDOW, NULL);
}

static void
return_to_preferences_cb (HdyVvmPreferencesWindow *self)
{
  hdy_preferences_window_close_subpage (HDY_PREFERENCES_WINDOW (self));
}

static void
hdy_vvm_preferences_window_class_init (HdyVvmPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-preferences-window.ui");

  gtk_widget_class_bind_template_callback (widget_class, return_to_preferences_cb);
}

static void
hdy_vvm_preferences_window_init (HdyVvmPreferencesWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
