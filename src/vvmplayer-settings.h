#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VVM_TYPE_SETTINGS (vvm_settings_get_type ())

G_DECLARE_FINAL_TYPE (VvmSettings, vvm_settings, VVM, SETTINGS, GObject)

VvmSettings *vvm_settings_get_default (void);

void vvm_settings_load_mm_defaults (VvmSettings *self);
void vvm_settings_load_service_defaults (VvmSettings *self);

void vvm_settings_set_vvm_enabled (VvmSettings *self, int VVMEnabled);
int vvm_settings_get_vvm_enabled (VvmSettings *self);

void vvm_settings_set_vvm_type (VvmSettings *self, const char *VVMType);
char *vvm_settings_get_vvm_type (VvmSettings *self);

void vvm_settings_set_vvm_destination_number (VvmSettings *self,
                                              const char *VVMDestinationNumber);
char *vvm_settings_get_vvm_destination_number (VvmSettings *self);

void vvm_settings_set_vvm_default_number (VvmSettings *self,
                                          const char *DefaultModemNumber);
char *vvm_settings_get_vvm_default_number (VvmSettings *self);

void vvm_settings_set_vvm_provision_status (VvmSettings *self,
                                            const char *ProvisionStatus);
char *vvm_settings_get_vvm_provision_status (VvmSettings *self);

void vvm_settings_set_vvm_carrier_prefix (VvmSettings *self,
                                          const char *CarrierPrefix);
char *vvm_settings_get_vvm_carrier_prefix (VvmSettings *self);

void vvm_settings_set_mm_available  (VvmSettings *self,
                                     int ModemManagerAvailable);
int vvm_settings_get_mm_available (VvmSettings *self);

void vvm_settings_set_service_available (VvmSettings *self,
                                         int ServiceAvailable);
int vvm_settings_get_service_available (VvmSettings *self);

void vvm_settings_set_vvm_country_code (VvmSettings *self,
                                        const char *CountryCode);
const char *vvm_settings_get_vvm_country_code (VvmSettings *self);

void vvm_settings_set_mailbox_active (VvmSettings *self, int MailboxActive);
int vvm_settings_get_mailbox_active (VvmSettings *self);

G_END_DECLS
