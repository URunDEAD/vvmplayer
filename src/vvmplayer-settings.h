#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VVM_TYPE_SETTINGS (vvm_settings_get_type ())

G_DECLARE_FINAL_TYPE (VvmSettings, vvm_settings, VVM, SETTINGS, GObject)

VvmSettings *vvm_settings_get_default                  (void);
void vvm_settings_set_vvm_enabled (VvmSettings *self, int VVMEnabled);
void vvm_settings_set_vvm_type (VvmSettings *self, const char *VVMType);
void vvm_settings_set_vvm_destination_number (VvmSettings *self,
                                              const char *VVMDestinationNumber);
void vvm_settings_set_vvm_default_number (VvmSettings *self,
                                          const char *DefaultModemNumber);
void vvm_settings_set_vvm_provision_status (VvmSettings *self,
                                            const char *ProvisionStatus);
void vvm_settings_set_vvm_carrier_prefix (VvmSettings *self,
                                          const char *CarrierPrefix);

G_END_DECLS
