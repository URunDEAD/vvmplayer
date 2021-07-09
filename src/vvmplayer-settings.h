#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

// Refer to "OMTP_VVM_Specification v1.3, Section 2.8
enum vvmd_provision_status {
	VVM_PROVISION_STATUS_NOT_SET,		//vvmd only, not a VVM Specification
	VVM_PROVISION_STATUS_NEW,
	VVM_PROVISION_STATUS_READY,
	VVM_PROVISION_STATUS_PROVISIONED,
	VVM_PROVISION_STATUS_UNKNOWN,
	VVM_PROVISION_STATUS_BLOCKED,
};

#define VVM_TYPE_SETTINGS (vvm_settings_get_type ())

G_DECLARE_FINAL_TYPE (VvmSettings, vvm_settings, VVM, SETTINGS, GObject)

VvmSettings *vvm_settings_get_default                  (void);

G_END_DECLS
