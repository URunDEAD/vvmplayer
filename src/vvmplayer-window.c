/* vvmplayer-window.c
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

#include "config.h"

#include "vvmplayer-window.h"

struct _VvmplayerWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkListBox          *visual_voicemail;
  GtkToggleButton     *speaker_output_button;
  GtkImage            *speaker_output_image;
};

G_DEFINE_FINAL_TYPE (VvmplayerWindow, vvmplayer_window, ADW_TYPE_APPLICATION_WINDOW)

void vvmplayer_window_set_speaker_button (VvmplayerWindow *self, gboolean setting)
{
  gtk_toggle_button_set_active (self->speaker_output_button,
                                setting);
}

void
vvmplayer_window_add_row (VvmplayerWindow *self, GtkWidget *voicemail)
{
  if (gtk_widget_get_parent (voicemail) == NULL) {
    gtk_list_box_prepend (GTK_LIST_BOX (self->visual_voicemail),
                          GTK_WIDGET (voicemail));
  } else {
    g_warning ("Voicemail already has a parent!");
  }
}

static void
speaker_output_button_clicked_cb (GtkToggleButton     *btn,
                                  VvmplayerWindow *self)
{
  gboolean toggled;
//  VvmSettings *settings = vvm_settings_get_default ();
  toggled = gtk_toggle_button_get_active (btn);
//  vvm_settings_set_speaker_button_state (settings, toggled);

  //TODO FIXME
  if (toggled) {
    g_debug ("Button Toggled");
//    call_audio_select_mode_async (CALL_AUDIO_MODE_DEFAULT,
//                                  select_mode_complete,
//                                  NULL);
  } else {
    g_debug ("Button Untoggled");
//    if (vvm_settings_get_stream_count (settings) > 0) {
//      g_debug ("Setting audio profile to call");
//      call_audio_select_mode_async (CALL_AUDIO_MODE_CALL,
//                                    select_mode_complete,
//                                    NULL);
//    } else {
//      g_debug ("No stream is playing, waiting to change profile.");
//    }
  }
}

static void
vvmplayer_window_class_init (VvmplayerWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/ui/vvmplayer-window.ui");
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, visual_voicemail);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, speaker_output_image);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, speaker_output_button);

  gtk_widget_class_bind_template_callback (widget_class, speaker_output_button_clicked_cb);
}

static void
vvmplayer_window_init (VvmplayerWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
