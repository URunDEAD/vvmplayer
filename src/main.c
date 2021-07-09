/* main.c
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

#include <glib/gi18n.h>

#include "vvmplayer-config.h"
#include "vvmplayer-window.h"
#include "vvmplayer-settings.h"
#include "vvmplayer-vvmd.h"

typedef struct
{
  GtkApplication parent_instance;
  VvmSettings *settings;
  VvmVvmd *backend;

} VvmPlayer;

typedef GtkApplicationClass VvmPlayerClass;

GType vvm_player_get_type (void);
G_DEFINE_TYPE (VvmPlayer, vvm_player, GTK_TYPE_APPLICATION)

static void
on_startup (GtkApplication *app)
{
  g_assert (GTK_IS_APPLICATION (app));
  hdy_init();
}

static void
on_activate (GtkApplication *app)
{
	GtkWindow *window;

	/* It's good practice to check your parameters at the beginning of the
	 * function. It helps catch errors early and in development instead of
	 * by your users.
	 */
	g_assert (GTK_IS_APPLICATION (app));

	/* Get the current window or create one if necessary. */
	window = gtk_application_get_active_window (app);
	if (window == NULL)
		window = g_object_new (VVMPLAYER_TYPE_WINDOW,
		                       "application", app,
		                       "default-width", 600,
		                       "default-height", 300,
		                       NULL);

	/* Ask the window manager/compositor to present the window. */
	gtk_window_present (window);
}

static void
vvm_player_finalize (GObject *object)
{
  G_OBJECT_CLASS (vvm_player_parent_class)->finalize (object);
}

static void
vvm_player_init (VvmPlayer *vvm_player)
{
}

static void
vvm_player_class_init (VvmPlayerClass *class)
{
  //GApplicationClass *application_class = G_APPLICATION_CLASS (class);
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = vvm_player_finalize;

}

static VvmPlayer *
vvm_player_new (void)
{
  VvmPlayer *vvm_player;

  g_set_application_name ("VVM Player");

  vvm_player = g_object_new (vvm_player_get_type (),
                            "application-id", "org.kop316.vvmplayer",
                            "flags", G_APPLICATION_FLAGS_NONE,
                            "inactivity-timeout", 30000,
                            "register-session", TRUE,
                            NULL);

  vvm_player->settings = vvm_settings_get_default ();
  vvm_player->backend = vvm_vvmd_get_default ();

  return vvm_player;
}

int
main (int   argc,
      char *argv[])
{
	VvmPlayer *vvm_player;
	int ret;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	vvm_player = vvm_player_new ();
	g_signal_connect (G_APPLICATION (vvm_player), "activate", G_CALLBACK (on_activate), NULL);
	g_signal_connect (G_APPLICATION (vvm_player), "startup", G_CALLBACK (on_startup), NULL);

	ret = g_application_run (G_APPLICATION (vvm_player), argc, argv);

	return ret;
}
