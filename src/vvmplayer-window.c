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

#include "vvmplayer-config.h"
#include "vvmplayer-window.h"
#include "vvmplayer-preferences-window.h"
#include "vvmplayer-vvmd.h"
#include "vvmplayer-voicemail-window.h"

struct _VvmplayerWindow
{
  GtkApplicationWindow  parent_instance;

  /* Template widgets */
  HdyHeaderBar  *header_bar;
  GtkListBox     *visual_voicemail;
  GtkLabel      *no_visual_voicemail_label;

  GListModel *voicemails;

  GtkWidget  *add_row;
};

G_DEFINE_TYPE (VvmplayerWindow, vvmplayer_window, GTK_TYPE_APPLICATION_WINDOW)

static void
vvmplayer_window_show_about_dialog (VvmplayerWindow *self)
{
  static const gchar *authors[] = {
    "Chris Talbot <chris@talbothome.com>",
    NULL
  };

  gtk_show_about_dialog (GTK_WINDOW (self),
                         "website", "https://gitlab.com/kop316/vvmplayer",
                         "copyright", "© 2021 Chris Talbot",
                         "license-type", GTK_LICENSE_GPL_3_0,
                         "authors", authors,
                         NULL);
}

static void
vvmplayer_window_show_settings_dialog (VvmplayerWindow *self)
{
  VvmPreferencesWindow *preferences = vvm_preferences_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (preferences), GTK_WINDOW (self));
  gtk_widget_show (GTK_WIDGET (preferences));
}

void
vvmplayer_window_add_row (VvmplayerWindow *self)
{

 GtkWidget *voicemail = voicemail_window_new ();

  gtk_list_box_prepend (GTK_LIST_BOX (self->visual_voicemail),
                        GTK_WIDGET (voicemail));

// works only when everything else is commented out
  gtk_list_box_prepend (GTK_LIST_BOX (self->visual_voicemail),
                        GTK_WIDGET (self->add_row));

}


static void
vvmplayer_window_class_init (VvmplayerWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-window.ui");
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, visual_voicemail);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, no_visual_voicemail_label);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, add_row);

  gtk_widget_class_bind_template_callback (widget_class, vvmplayer_window_show_about_dialog);
  gtk_widget_class_bind_template_callback (widget_class, vvmplayer_window_show_settings_dialog);
}

static void
vvmplayer_window_init (VvmplayerWindow *self)
{
  VvmVvmd *backend = vvm_vvmd_get_default ();
  gtk_widget_init_template (GTK_WIDGET (self));
  vvmplayer_vvmd_set_mm_vvm_list_box (backend, self);

  gtk_list_box_bind_model (GTK_LIST_BOX (self->visual_voicemail),
                           self->voicemails,
                           (GtkListBoxCreateWidgetFunc)voicemail_window_new,
                           self, NULL);

 /*****DEBUG***/
  vvmplayer_window_add_row (self);
  /*****DEBUG***/


}
