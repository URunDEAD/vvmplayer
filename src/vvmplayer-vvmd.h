#pragma once

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include "vvmplayer-window.h"
#include "vvmplayer-settings.h"
#include "vvmplayer-voicemail-window.h"
#include "vvmplayer-contact-provider.h"

G_BEGIN_DECLS

#define VVM_TYPE_VVMD (vvm_vvmd_get_type ())

G_DECLARE_FINAL_TYPE (VvmVvmd, vvm_vvmd, VVM, VVMD, GObject)

enum vvm_type {
	VVM_TYPE_UNKNOWN,
	VVM_TYPE_CVVM,
	VVM_TYPE_ATTUSA,
	VVM_TYPE_OTMP,
	VVM_TYPE_VVM_THREE,
};

VvmVvmd *vvm_vvmd_get_default                  (void);

void vvmplayer_vvmd_set_mm_vvm_list_box (VvmVvmd *self,
                                         VvmplayerWindow    *visual_voicemail);

int vvmplayer_vvmd_get_mm_settings (VvmVvmd *self);
int vvmplayer_vvmd_set_mm_setting (VvmVvmd *self,
                                   const char *setting_to_change,
                                   const char *updated_setting);

int vvmplayer_vvmd_set_mm_vvm_enabled (VvmVvmd *self,
                                       int enabled);

int vvmplayer_vvmd_get_service_settings (VvmVvmd *self);

int vvmplayer_vvmd_check_subscription_status (VvmVvmd *self);

int vvmplayer_vvmd_encode_vvm_type (const char *vvm_type);

char *vvmplayer_vvmd_decode_vvm_type (int vvm_type);

void vvmplayer_vvmd_mark_message_read (GDBusProxy      *message_proxy,
                                       char            *objectpath,
                                       gpointer         user_data);

void vvmplayer_vvmd_delete_vvm (VvmVvmd     *self,
                           GDBusProxy  *message_proxy,
                           char        *objectpath,
                           gconstpointer user_data);

G_END_DECLS
