#pragma once

#include <gtk/gtk.h>
#include <handy.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
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
                                 GstDiscoverer    *discoverer);

void voicemail_window_set_contact_name (VoicemailWindow *self,
                                        const char *contact_name);
char *voicemail_window_get_number (VoicemailWindow *self);
char *voicemail_window_get_objectpath (VoicemailWindow *self);
GDateTime *voicemail_window_get_datetime (VoicemailWindow *self);

G_END_DECLS
