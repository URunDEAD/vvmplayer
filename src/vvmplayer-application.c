/* vvmplayer-application.c
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
#include "version.h"

#include "vvmplayer-application.h"
#include "vvmplayer-preferences-window.h"
#include "vvmplayer-settings.h"
#include "vvmplayer-window.h"
#include "vvmplayer-vvmd.h"

struct _VvmplayerApplication
{
  AdwApplication parent_instance;
  gboolean        daemon;
  gboolean        daemon_running;
  GtkWindow      *main_window;
  gboolean        app_debug;

  VvmVvmd      *backend;
  VvmSettings  *settings;
};

G_DEFINE_TYPE (VvmplayerApplication, vvmplayer_application, ADW_TYPE_APPLICATION)

enum {
  SIGNAL_MAIN_WINDOW_CLOSED,
  N_SIGNALS
};

static guint signals[N_SIGNALS];

VvmplayerApplication *
vvmplayer_application_new (const char        *application_id,
                           GApplicationFlags  flags)
{
  g_return_val_if_fail (application_id != NULL, NULL);

  return g_object_new (VVMPLAYER_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static int
vvmplayer_handle_local_options (GApplication *application,
                                GVariantDict *options)
{
  VvmplayerApplication *self = (VvmplayerApplication *)application;
  if (g_variant_dict_contains (options, "version")) {
    g_print ("%s, git version: %s\n", PACKAGE_VERSION, PACKAGE_VCS_VERSION);
    return 0;
  }

  if (g_variant_dict_contains (options, "debug")) {
    g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);
    self->app_debug = TRUE;
  }

  self->daemon = FALSE;
  self->app_debug = FALSE;
  if (g_variant_dict_contains (options, "daemon")) {
    /* Hold application only the first time daemon mode is set */
    if (!self->daemon)
      g_application_hold (application);

    self->daemon = TRUE;
    g_debug ("Application marked as daemon");
  }

  return -1;
}

static void
vvmplayer_application_startup (GApplication *application)
{
  VvmplayerApplication *self = (VvmplayerApplication *)application;

  g_info ("%s %s, git version: %s", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_VCS_VERSION);

  G_APPLICATION_CLASS (vvmplayer_application_parent_class)->startup (application);

  g_set_application_name ("Visual Voicemail");
  gtk_window_set_default_icon_name (PACKAGE_ID);

  self->backend = vvm_vvmd_get_default ();
  self->settings = vvm_settings_get_default ();
}

static gboolean
on_widget_deleted(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  /* Passing app through g_signal_connect causes a segfault, so just find the default */
  VvmplayerApplication *self = (VvmplayerApplication *)g_application_get_default ();
  g_autoptr (GError) error = NULL;
  gboolean ret;

  /* If we are not running in daemon mode, hide the window and reset speaker button */
  if (self->daemon)
    vvmplayer_window_set_speaker_button (VVMPLAYER_WINDOW (self->main_window), FALSE);

  g_signal_emit (self, signals[SIGNAL_MAIN_WINDOW_CLOSED], 0);

  /* Set audio profile back to default whether you are quitting or closing the window */
  g_debug ("Setting audio profile to back to default");
  ret = call_audio_select_mode (CALL_AUDIO_MODE_DEFAULT, &error);

  if (!ret && error)
    g_warning ("Failed to switch profile: %s", error->message);

  return FALSE;
}

static void
vvmplayer_application_activate (GApplication *app)
{
  VvmplayerApplication *self = (VvmplayerApplication *)app;

  if (!self->main_window) {
    self->main_window = g_object_new (VVMPLAYER_TYPE_WINDOW, "application", app, NULL);
    g_object_add_weak_pointer (G_OBJECT (self->main_window), (gpointer *)&self->main_window);

    g_signal_connect(G_OBJECT(self->main_window),
                     "close_request",
                     G_CALLBACK(on_widget_deleted),
                     NULL);
  }

  if ((!self->daemon && !self->daemon_running) || (self->daemon_running)) {
    gtk_window_present (GTK_WINDOW (self->main_window));
  } else {
    self->daemon_running = TRUE;
    gtk_window_set_hide_on_close (GTK_WINDOW (self->main_window), self->daemon_running);
  }


}

static void
vvmplayer_application_finalize (GObject *object)
{
  VvmplayerApplication *self = (VvmplayerApplication *)object;

  g_debug ("disposing application");

  g_clear_object (&self->settings);
  g_clear_object (&self->backend);

  G_OBJECT_CLASS (vvmplayer_application_parent_class)->finalize (object);
}

static void
vvmplayer_application_class_init (VvmplayerApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  signals [SIGNAL_MAIN_WINDOW_CLOSED] =
    g_signal_new ("main-window-closed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

  object_class->finalize = vvmplayer_application_finalize;

  app_class->handle_local_options = vvmplayer_handle_local_options;
  app_class->startup = vvmplayer_application_startup;
  app_class->activate = vvmplayer_application_activate;
}

static void
vvmplayer_application_show_preferences (GSimpleAction *action,
                  GVariant      *state,
                  gpointer       user_data)
{
  GtkApplication *app = GTK_APPLICATION (user_data);

  GtkWindow *window = gtk_application_get_active_window (app);
  VvmplayerPreferencesWindow *preferences = vvm_player_preferences_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (preferences), window);
  gtk_window_present (GTK_WINDOW (preferences));
}

static void
vvmplayer_application_about_action (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
  static const char *developers[] = {"Chris Talbot", NULL};
  static const char *artists[] = {"Asiya Morris <aitrisart@gmail.com>", NULL};
  VvmplayerApplication *self = (VvmplayerApplication *)user_data;
  GtkWindow *window = NULL;

  g_assert (VVMPLAYER_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  adw_show_about_window (window,
                         "application-name", "Visual Voicemail",
                         "application-icon", PACKAGE_ID,
                         "version", PACKAGE_VCS_VERSION,
                         "developers", developers,
                         "artists", artists,
                         "copyright", "© 2021-2022 Chris Talbot",
                         "website", "https://gitlab.com/kop316/vvmplayer",
                         "issue-url", "https://gitlab.com/kop316/vvmplayer/-/issues",
                         "license-type", GTK_LICENSE_GPL_3_0,
                         NULL);
}

static GOptionEntry cmd_options[] = {
  {
    "daemon", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL,
    ("Whether to present the main window on startup"),
    NULL
  },
  { "debug", 'd', 0, G_OPTION_ARG_NONE, G_OPTION_ARG_NONE,
     "Enable debugging output"
  },
  {
    "version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL,
    ("Show release version"), NULL
  },
  { NULL }
};

static const GActionEntry app_actions[] = {
  { "about", vvmplayer_application_about_action },
  { "preferences", vvmplayer_application_show_preferences, NULL, NULL, NULL },
};

static void
vvmplayer_application_init (VvmplayerApplication *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   app_actions,
                                   G_N_ELEMENTS (app_actions),
                                   self);
  g_application_add_main_option_entries (G_APPLICATION (self), cmd_options);
}

