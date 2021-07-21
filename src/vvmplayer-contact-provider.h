#pragma once

#include <glib-object.h>
#include <gdk/gdk.h>

#include "vvmplayer-contact.h"

G_BEGIN_DECLS

#define VVM_TYPE_EDS (vvm_eds_get_type ())

G_DECLARE_FINAL_TYPE (VvmEds, vvm_eds, VVM, EDS, GObject)

VvmEds     *vvm_eds_get_default            (void);
gboolean       vvm_eds_is_ready       (VvmEds  *self);
VvmContact *vvm_eds_find_by_number    (VvmEds  *self,
                                       const char *phone_number);
GListModel *vvm_eds_get_model         (VvmEds *self);

G_END_DECLS
