#pragma once

#include <handy.h>

G_BEGIN_DECLS

#define HDY_TYPE_VVM_PREFERENCES_WINDOW (vvm_preferences_window_get_type())

G_DECLARE_FINAL_TYPE (VvmPreferencesWindow, vvm_preferences_window, HDY, VVM_PREFERENCES_WINDOW, HdyPreferencesWindow)

VvmPreferencesWindow *vvm_preferences_window_new (void);

G_END_DECLS
