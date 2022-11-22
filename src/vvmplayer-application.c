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
#include "vvmplayer-window.h"

struct _VvmplayerApplication
{
  AdwApplication parent_instance;
};

G_DEFINE_TYPE (VvmplayerApplication, vvmplayer_application, ADW_TYPE_APPLICATION)

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

static void
vvmplayer_application_activate (GApplication *app)
{
  GtkWindow *window;

  g_assert (VVMPLAYER_IS_APPLICATION (app));

  window = gtk_application_get_active_window (GTK_APPLICATION (app));
  if (window == NULL)
    window = g_object_new (VVMPLAYER_TYPE_WINDOW,
                           "application", app,
                           NULL);

  gtk_window_present (window);
}

static void
vvmplayer_application_class_init (VvmplayerApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = vvmplayer_application_activate;
}

static void
vvmplayer_application_about_action (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
  static const char *developers[] = {"Chris Talbot", NULL};
  static const char *artists[] = {"Asiya Morris <aitrisart@gmail.com>", NULL};
  VvmplayerApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (VVMPLAYER_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  adw_show_about_window (window,
                         "application-name", GETTEXT_PACKAGE,
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

static void
vvmplayer_application_quit_action (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       user_data)
{
  VvmplayerApplication *self = user_data;

  g_assert (VVMPLAYER_IS_APPLICATION (self));

  g_application_quit (G_APPLICATION (self));
}

static const GActionEntry app_actions[] = {
  { "quit", vvmplayer_application_quit_action },
  { "about", vvmplayer_application_about_action },
};

static void
vvmplayer_application_init (VvmplayerApplication *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   app_actions,
                                   G_N_ELEMENTS (app_actions),
                                   self);
  gtk_application_set_accels_for_action (GTK_APPLICATION (self),
                                         "app.quit",
                                         (const char *[]) { "<primary>q", NULL });
}

