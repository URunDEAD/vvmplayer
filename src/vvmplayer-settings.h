#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VVM_TYPE_SETTINGS (vvm_settings_get_type ())

G_DECLARE_FINAL_TYPE (VvmSettings, vvm_settings, VVM, SETTINGS, GObject)

VvmSettings *vvm_settings_get_default                  (void);

G_END_DECLS
