#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VVM_TYPE_VVMD (vvm_vvmd_get_type ())

G_DECLARE_FINAL_TYPE (VvmVvmd, vvm_vvmd, VVM, VVMD, GObject)

VvmVvmd *vvm_vvmd_get_default                  (void);

G_END_DECLS
