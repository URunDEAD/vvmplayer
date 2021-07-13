/* vvmplayer-voicemail-window.c
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

#include "vvmplayer-voicemail-window.h"

struct _VoicemailWindow
{
  HdyActionRow parent_instance;

};

G_DEFINE_TYPE (VoicemailWindow, voicemail_window, HDY_TYPE_ACTION_ROW)

GtkWidget *
voicemail_window_new (void)
{
  return g_object_new (GTK_TYPE_VOICEMAIL_WINDOW, NULL);
}

static void
voicemail_window_finalize (GObject *object)
{
  VoicemailWindow *self = (VoicemailWindow *)object;

  G_OBJECT_CLASS (voicemail_window_parent_class)->finalize (object);

}


static void
voicemail_window_class_init (VoicemailWindowClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-voicemail-window.ui");

  object_class->finalize = voicemail_window_finalize;

}

static void
voicemail_window_init (VoicemailWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
