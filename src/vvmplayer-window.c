/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-window.c
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

#define G_LOG_DOMAIN "vvmplayer-window"

#ifdef HAVE_CONFIG_H
# include "config.h"
# include "version.h"
#endif

#include <glib/gi18n.h>
#include <libcallaudio.h>

#include "vvmplayer-window.h"
#include "vvmplayer-log.h"
#include "vvmplayer-preferences-window.h"
#include "vvmplayer-vvmd.h"

struct _VvmWindow
{
  GtkApplicationWindow parent_instance;

  VvmSettings *settings;

  GtkWidget   *menu_button;
  GtkListBox  *visual_voicemail;
  GtkToggleButton *speaker_output_button;
  gboolean         toggled;
  GtkImage    *speaker_output_image;
};

G_DEFINE_TYPE (VvmWindow, vvmplayer_window, GTK_TYPE_APPLICATION_WINDOW)


enum {
  PROP_0,
  PROP_SETTINGS,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
vvmplayer_window_show_settings_dialog (VvmWindow *self)
{
  VvmPreferencesWindow *preferences = vvm_preferences_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (preferences), GTK_WINDOW (self));
  gtk_widget_show (GTK_WIDGET (preferences));
}

static void
vvmplayer_window_show_about (VvmWindow *self)
{
  static const gchar *authors[] = {
    "Chris Talbot <chris@talbothome.com>",
    NULL
  };

  static const gchar *artists[] = {
    "Asiya Morris <aitrisart@gmail.com>",
    NULL
  };

  g_assert (VVMPLAYER_IS_WINDOW (self));

  /*
   * If “program-name” is not set, it is retrieved from
   * g_get_application_name().
   */
  gtk_show_about_dialog (GTK_WINDOW (self),
                         "website", "https://gitlab.com/kop316/vvmplayer",
                         "version", PACKAGE_VCS_VERSION,
                         "copyright", "Copyright © 2021 Chris Talbot",
                         "license-type", GTK_LICENSE_GPL_3_0,
                         "authors", authors,
                         "artists", artists,
                         "logo-icon-name", PACKAGE_ID,
                         "translator-credits", _("translator-credits"),
                         NULL);
}

static void
speaker_output_button_clicked_cb (GtkToggleButton     *btn,
                                  VvmWindow *self)
{
  g_autoptr (GError) error = NULL;
  gboolean ret;
  //gboolean toggled;
  //toggled = gtk_toggle_button_get_active (self->speaker_output_button);

  /* HACK: gtk_toggle_button_get_active only returns FALSE */
  if (self->toggled)
    self->toggled = FALSE;
  else
    self->toggled = TRUE;

  ret = call_audio_enable_speaker (self->toggled, &error);
  if (!ret && error)
    g_warning ("Failed to %sable speaker: %s",
               self->toggled ? "en" : "dis",
               error->message);
  else
    g_debug ("%sabled speaker",
             self->toggled ? "En" : "Dis");

}

static void
vvmplayer_window_unmap (GtkWidget *widget)
{
  VvmWindow *self = (VvmWindow *)widget;
  GtkWindow *window = (GtkWindow *)widget;
  GdkRectangle geometry;
  gboolean is_maximized;

  is_maximized = gtk_window_is_maximized (window);
  vvmplayer_settings_set_window_maximized (self->settings, is_maximized);

  if (!is_maximized)
    {
      gtk_window_get_size (window, &geometry.width, &geometry.height);
      vvmplayer_settings_set_window_geometry (self->settings, &geometry);
    }

  GTK_WIDGET_CLASS (vvmplayer_window_parent_class)->unmap (widget);
}

static void
vvmplayer_window_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  VvmWindow *self = (VvmWindow *)object;

  switch (prop_id)
    {
    case PROP_SETTINGS:
      self->settings = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

void
vvmplayer_window_add_row (VvmWindow *self, GtkWidget *voicemail)
{
  gtk_list_box_prepend (GTK_LIST_BOX (self->visual_voicemail),
                        GTK_WIDGET (voicemail));

}

static void
vvmplayer_window_constructed (GObject *object)
{
  VvmWindow *self = (VvmWindow *)object;
  GtkWindow *window = (GtkWindow *)object;
  GdkRectangle geometry;

  vvmplayer_settings_get_window_geometry (self->settings, &geometry);
  gtk_window_set_default_size (window, geometry.width, geometry.height);

  if (vvmplayer_settings_get_window_maximized (self->settings))
    gtk_window_maximize (window);

  G_OBJECT_CLASS (vvmplayer_window_parent_class)->constructed (object);
}

static void
vvmplayer_window_finalize (GObject *object)
{
  VvmWindow *self = (VvmWindow *)object;

  VVMPLAYER_TRACE_MSG ("finalizing window");

  g_object_unref (self->settings);

  G_OBJECT_CLASS (vvmplayer_window_parent_class)->finalize (object);
}

static void
vvmplayer_window_class_init (VvmWindowClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = vvmplayer_window_set_property;
  object_class->constructed  = vvmplayer_window_constructed;
  object_class->finalize     = vvmplayer_window_finalize;

  widget_class->unmap = vvmplayer_window_unmap;

  /**
   * VvmWindow:settings:
   *
   * The Application Settings
   */
  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings",
                         "Settings",
                         "The Application Settings",
                         VVMPLAYER_TYPE_SETTINGS,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/kop316/vvmplayer/"
                                               "ui/vvmplayer-window.ui");

  gtk_widget_class_bind_template_child (widget_class, VvmWindow, menu_button);
  gtk_widget_class_bind_template_child (widget_class, VvmWindow, visual_voicemail);
  gtk_widget_class_bind_template_child (widget_class, VvmWindow, speaker_output_image);
  gtk_widget_class_bind_template_child (widget_class, VvmWindow, speaker_output_button);
  gtk_widget_class_bind_template_callback (widget_class, vvmplayer_window_show_about);
  gtk_widget_class_bind_template_callback (widget_class, vvmplayer_window_show_settings_dialog);
  gtk_widget_class_bind_template_callback (widget_class, speaker_output_button_clicked_cb);
}

static void
vvmplayer_window_init (VvmWindow *self)
{
  g_autoptr (GError) error = NULL;
  VvmVvmd *backend = vvm_vvmd_get_default ();
  gtk_widget_init_template (GTK_WIDGET (self));
  vvmplayer_vvmd_set_mm_vvm_list_box (backend, self);

  if (!call_audio_init (&error)) {
      g_warning ("Failed to init libcallaudio: %s", error->message);
  }

  /* speaker_output_button_clicked_cb () changes TRUE to FALSE */
  self->toggled = TRUE;
  speaker_output_button_clicked_cb (self->speaker_output_button, self);

}

GtkWidget *
vvmplayer_window_new (GtkApplication *application,
                      VvmSettings    *settings)
{
  g_assert (GTK_IS_APPLICATION (application));
  g_assert (VVMPLAYER_IS_SETTINGS (settings));

  return g_object_new (VVMPLAYER_TYPE_WINDOW,
                       "application", application,
                       "settings", settings,
                       NULL);
}
