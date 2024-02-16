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

#include <glib/gi18n.h>
#include "config.h"
#include "vvmplayer-window.h"
#include "vvmplayer-voicemail-window.h"
#include "vvmplayer-settings.h"

struct _VvmplayerWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkWidget           *header_bar;
  GtkListBox          *visual_voicemail;
  GtkToggleButton     *speaker_output_button;
  GtkImage            *speaker_output_image;

  AdwToastOverlay *toast_overlay;
  AdwToast *toast;

  GtkScrolledWindow   *scroll_window;

  AdwStatusPage *window_status_page;

  //Watch ID
  gulong vvm_settings_watch_id;
  gulong window_closed_id;
};

G_DEFINE_TYPE (VvmplayerWindow, vvmplayer_window, ADW_TYPE_APPLICATION_WINDOW)

void
vvmplayer_window_set_speaker_button (VvmplayerWindow *self,
                                     gboolean         setting)
{
  gtk_toggle_button_set_active (self->speaker_output_button,
                                setting);
}

void
vvmplayer_window_add_row (VvmplayerWindow *self,
                          GtkWidget       *voicemail)
{
  if (gtk_widget_get_parent (voicemail) == NULL)
    gtk_list_box_prepend (GTK_LIST_BOX (self->visual_voicemail),
                          GTK_WIDGET (voicemail));
  else
    g_warning ("Voicemail already has a parent!");
}

static void
select_mode_complete (gboolean success,
                      GError  *error,
                      gpointer data)
{
  if (error)
    {
      g_warning ("Failed to select audio mode: %s", error->message);
      g_error_free (error);
    }
}

static void
speaker_output_button_clicked_cb (GtkToggleButton *btn,
                                  VvmplayerWindow *self)
{
  gboolean toggled;
  VvmSettings *settings = vvm_settings_get_default ();
  toggled = gtk_toggle_button_get_active (btn);
  vvm_settings_set_speaker_button_state (settings, toggled);

  if (toggled)
    {
      g_debug ("Button Toggled");
      call_audio_select_mode_async (CALL_AUDIO_MODE_DEFAULT,
                                    select_mode_complete,
                                    NULL);
    }
  else {
      g_debug ("Button Untoggled");
      if (vvm_settings_get_stream_count (settings) > 0)
        {
          g_debug ("Setting audio profile to call");
          call_audio_select_mode_async (CALL_AUDIO_MODE_CALL,
                                        select_mode_complete,
                                        NULL);
        }
      else
        g_debug ("No stream is playing, waiting to change profile.");
    }
}

static void
update_settings_cb (VvmplayerWindow *self)
{
  VvmSettings *settings = vvm_settings_get_default ();

  const char *vvm_disabled_text =
  _("Go to Preferences to "
  "enable and see "
  "<a href=\"https://gitlab.com/kop316/vvmplayer/-/wikis/home\">"
  "here</a> for "
  "configuration help.");

  const char *no_vvmd_text =
  _("Please install or enable "
  "<a href=\"https://gitlab.com/kop316/vvmd/\">"
  "vvmd</a>");


  if (vvm_settings_get_mm_available (settings) == FALSE)
    {
      adw_status_page_set_title (self->window_status_page, _("vvmd not active"));
      adw_status_page_set_description (self->window_status_page, no_vvmd_text);
  } else if (vvm_settings_get_mailbox_active (settings) == TRUE)
    {
      adw_status_page_set_title (self->window_status_page, _("No Voicemails!"));
      adw_status_page_set_description (self->window_status_page, "");
  } else
    {
      adw_status_page_set_title (self->window_status_page, _("Visual Voicemail disabled"));
      adw_status_page_set_description (self->window_status_page, vvm_disabled_text);
    }

}

static void
main_window_closed_cb (VvmplayerWindow *self)
{
  GtkAdjustment *vadjustment = NULL;

  gtk_list_box_unselect_all (GTK_LIST_BOX (self->visual_voicemail));

  /* Reset scrolling to the top */
  vadjustment = gtk_scrolled_window_get_vadjustment (self->scroll_window);
  gtk_adjustment_set_value (vadjustment,
                            gtk_adjustment_get_lower (vadjustment));
}

static void
vvmplayer_window_finalize (GObject *object)
{
  VvmplayerWindow *self = (VvmplayerWindow *)object;
  VvmVvmd *backend = vvm_vvmd_get_default ();

  g_signal_handler_disconnect (g_application_get_default (),
                               self->window_closed_id);

  g_signal_handler_disconnect (backend,
                               self->vvm_settings_watch_id);

  G_OBJECT_CLASS (vvmplayer_window_parent_class)->finalize (object);
}

static void
vvmplayer_window_class_init (VvmplayerWindowClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/ui/vvmplayer-window.ui");
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, visual_voicemail);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, speaker_output_image);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, speaker_output_button);

  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, toast_overlay);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, scroll_window);
  gtk_widget_class_bind_template_child (widget_class, VvmplayerWindow, window_status_page);
  gtk_widget_class_bind_template_callback (widget_class, speaker_output_button_clicked_cb);

  object_class->finalize = vvmplayer_window_finalize;
}

static void
vvmplayer_window_init (VvmplayerWindow *self)
{
  g_autoptr(GError) error = NULL;
  VvmVvmd *backend = vvm_vvmd_get_default ();
  gtk_widget_init_template (GTK_WIDGET (self));
  vvmplayer_vvmd_set_mm_vvm_list_box (backend, self);

  if (!call_audio_init (&error))
    g_warning ("Failed to init libcallaudio: %s", error->message);

  self->window_closed_id = g_signal_connect_swapped (g_application_get_default (),
                                                     "main-window-closed",
                                                     G_CALLBACK (main_window_closed_cb),
                                                     self);

  self->vvm_settings_watch_id = g_signal_connect_swapped (backend, "settings-updated",
                                                          G_CALLBACK (update_settings_cb), self);
}
