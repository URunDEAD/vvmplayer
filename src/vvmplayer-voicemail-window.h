#pragma once

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <handy.h>

G_BEGIN_DECLS

#define GTK_TYPE_VOICEMAIL_WINDOW (voicemail_window_get_type())

G_DECLARE_FINAL_TYPE (VoicemailWindow, voicemail_window, VVM, VOICEMAIL_WINDOW, HdyActionRow)

GtkWidget *voicemail_window_new (void);

G_END_DECLS
