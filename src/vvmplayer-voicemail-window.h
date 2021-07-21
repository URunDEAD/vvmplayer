#pragma once

#include <gtk/gtk.h>
#include <handy.h>
#include <gst/gst.h>
#include "vvmplayer-vvmd.h"

G_BEGIN_DECLS

enum vvmd_lifetime_status {
	VVM_LIFETIME_STATUS_UNKNOWN,
	VVM_LIFETIME_STATUS_NOT_READ,
	VVM_LIFETIME_STATUS_READ
};

#define GTK_TYPE_VOICEMAIL_WINDOW (voicemail_window_get_type())

G_DECLARE_FINAL_TYPE (VoicemailWindow, voicemail_window, VVM, VOICEMAIL_WINDOW, HdyExpanderRow)

GtkWidget *voicemail_window_new (GVariant   *message_t,
                                 GDBusProxy *message_proxy,
                                 gpointer    user_data);

G_END_DECLS
