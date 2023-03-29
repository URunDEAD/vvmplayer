/* vvmplayer-debug-info.h
 *
 * Copyright 2017–2022 Purism SPC
 *           2021-2023 Chris Talbot
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "version.h"

#include "vvmplayer-debug-info.h"

/* Copied and adapted from https://gitlab.gnome.org/GNOME/libadwaita/-/raw/main/demo/adw-demo-debug-info.c */
static void
get_gtk_info (const char **backend,
              const char **renderer)
{
  GdkDisplay *display = gdk_display_get_default ();
  GdkSurface *surface;
  GskRenderer *gsk_renderer;

  if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkX11Display"))
    *backend = "X11";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkWaylandDisplay"))
    *backend = "Wayland";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkBroadwayDisplay"))
    *backend = "Broadway";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkWin32Display"))
    *backend = "Windows";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkMacosDisplay"))
    *backend = "macOS";
  else
    *backend = G_OBJECT_TYPE_NAME (display);

  surface = gdk_surface_new_toplevel (display);
  gsk_renderer = gsk_renderer_new_for_surface (surface);
  if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskVulkanRenderer"))
    *renderer = "Vulkan";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskGLRenderer"))
    *renderer = "GL";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskCairoRenderer"))
    *renderer = "Cairo";
  else
    *renderer = G_OBJECT_TYPE_NAME (gsk_renderer);

  gsk_renderer_unrealize (gsk_renderer);
  g_object_unref (gsk_renderer);
  gdk_surface_destroy (surface);
}

char *
vvm_generate_debug_info (void)
{
  GString *string = g_string_new (NULL);
  g_string_append_printf (string, "vvmplayer: %s\n", PACKAGE_VCS_VERSION);
  g_string_append (string, "\n");

  g_string_append (string, "Compiled against:\n");
  g_string_append_printf (string, "- GLib: %d.%d.%d\n", GLIB_MAJOR_VERSION,
                                                        GLIB_MINOR_VERSION,
                                                        GLIB_MICRO_VERSION);
  g_string_append_printf (string, "- GTK: %d.%d.%d\n", GTK_MAJOR_VERSION,
                                                       GTK_MINOR_VERSION,
                                                       GTK_MICRO_VERSION);
  g_string_append_printf (string, "- LibAdwaita: %d.%d.%d\n", ADW_MAJOR_VERSION,
                                                              ADW_MINOR_VERSION,
                                                              ADW_MICRO_VERSION);
  g_string_append (string, "\n");

  g_string_append (string, "Running against:\n");
  g_string_append_printf (string, "- GLib: %d.%d.%d\n", glib_major_version,
                                                        glib_minor_version,
                                                        glib_micro_version);
  g_string_append_printf (string, "- GTK: %d.%d.%d\n", gtk_get_major_version (),
                                                       gtk_get_minor_version (),
                                                       gtk_get_micro_version ());
  g_string_append_printf (string, "- Libadwaita: %d.%d.%d\n", adw_get_major_version (),
                                                              adw_get_minor_version (),
                                                              adw_get_micro_version ());
  g_string_append (string, "\n");

  {
    char *os_name = g_get_os_info (G_OS_INFO_KEY_NAME);
    char *os_version = g_get_os_info (G_OS_INFO_KEY_VERSION);

    g_string_append (string, "System:\n");
    g_string_append_printf (string, "- Name: %s\n", os_name);
    g_string_append_printf (string, "- Version: %s\n", os_version);
    g_string_append (string, "\n");

    g_free (os_name);
    g_free (os_version);
  }
  {
    const char *backend, *renderer;

    get_gtk_info (&backend, &renderer);

    g_string_append (string, "GTK:\n");
    g_string_append_printf (string, "- GDK backend: %s\n", backend);
    g_string_append_printf (string, "- GSK renderer: %s\n", renderer);
    g_string_append (string, "\n");
  }
  {
    const char *desktop = g_getenv ("XDG_CURRENT_DESKTOP");
    const char *session_desktop = g_getenv ("XDG_SESSION_DESKTOP");
    const char *session_type = g_getenv ("XDG_SESSION_TYPE");
    const char *lang = g_getenv ("LANG");
    const char *builder = g_getenv ("INSIDE_GNOME_BUILDER");
    const char *gtk_debug = g_getenv ("GTK_DEBUG");
    const char *gtk_theme = g_getenv ("GTK_THEME");
    const char *adw_debug_color_scheme = g_getenv ("ADW_DEBUG_COLOR_SCHEME");
    const char *adw_debug_high_contrast = g_getenv ("ADW_DEBUG_HIGH_CONTRAST");
    const char *adw_disable_portal = g_getenv ("ADW_DISABLE_PORTAL");

    g_string_append (string, "Environment:\n");
    g_string_append_printf (string, "- Desktop: %s\n", desktop);
    g_string_append_printf (string, "- Session: %s (%s)\n", session_desktop,
                                                            session_type);
    g_string_append_printf (string, "- Language: %s\n", lang);
    g_string_append_printf (string, "- Running inside Builder: %s\n", builder ? "yes" : "no");

    if (gtk_debug)
      g_string_append_printf (string, "- GTK_DEBUG: %s\n", gtk_debug);
    if (gtk_theme)
      g_string_append_printf (string, "- GTK_THEME: %s\n", gtk_theme);
    if (adw_debug_color_scheme)
      g_string_append_printf (string, "- ADW_DEBUG_COLOR_SCHEME: %s\n", adw_debug_color_scheme);
    if (adw_debug_high_contrast)
      g_string_append_printf (string, "- ADW_DEBUG_HIGH_CONTRAST: %s\n", adw_debug_high_contrast);
    if (adw_disable_portal)
      g_string_append_printf (string, "- ADW_DISABLE_PORTAL: %s\n", adw_disable_portal);
  }

  return g_string_free (string, FALSE);
}
