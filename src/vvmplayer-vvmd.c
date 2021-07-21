/* vvmplayer-window.c
 *
 * Copyright 2021 Chris Talbot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vvmplayer-vvmd.h"
#include "vvmplayer-contact-provider.h"

#define VVMD_SERVICE	                "org.kop316.vvm"
#define VVMD_PATH	                "/org/kop316/vvm"
#define VVMD_MODEMMANAGER_PATH          VVMD_PATH    "/modemmanager"
#define VVMD_MANAGER_INTERFACE	        VVMD_SERVICE ".Manager"
#define VVMD_SERVICE_INTERFACE	        VVMD_SERVICE ".Service"
#define VVMD_MESSAGE_INTERFACE	        VVMD_SERVICE ".Message"
#define VVMD_MODEMMANAGER_INTERFACE	VVMD_SERVICE ".ModemManager"

struct _VvmVvmd
{
  GObject     parent_instance;

  VvmplayerWindow    *visual_voicemail;

  GDBusConnection  *connection;
  unsigned int      vvmd_watch_id;
  GDBusProxy       *modemmanager_proxy;
  unsigned int      vvmd_manager_proxy_remove_watch_id;
  unsigned int      vvmd_manager_proxy_add_watch_id;
  unsigned int      vvmd_service_proxy_watch_id;
  GDBusProxy       *manager_proxy;
  GDBusProxy       *service_proxy;

  GPtrArray        *vvm_arr;
  VvmEds *contacts;
};

G_DEFINE_TYPE (VvmVvmd, vvm_vvmd, G_TYPE_OBJECT)

int
vvmplayer_vvmd_encode_vvm_type (const char *vvm_type)
{

 if (g_strcmp0(vvm_type, "Unknown") == 0) {
   return VVM_TYPE_UNKNOWN;
 } else if (g_strcmp0(vvm_type, "cvvm") == 0) {
   return VVM_TYPE_CVVM;
 } else if (g_strcmp0(vvm_type, "AT&TUSAProprietary") == 0) {
   return VVM_TYPE_ATTUSA;
 } else if (g_strcmp0(vvm_type, "otmp") == 0) {
   return VVM_TYPE_OTMP;
 } else if (g_strcmp0(vvm_type, "vvm3") == 0) {
   return VVM_TYPE_VVM_THREE;
 } else {
   return VVM_TYPE_UNKNOWN;
 }
 return VVM_TYPE_UNKNOWN;
}

char
*vvmplayer_vvmd_decode_vvm_type (int vvm_type)
{

  switch(vvm_type) {
    case VVM_TYPE_UNKNOWN:
      return g_strdup("Unknown");

    case VVM_TYPE_CVVM:
      return g_strdup("cvvm");

    case VVM_TYPE_ATTUSA:
      return g_strdup("AT&TUSAProprietary");

    case VVM_TYPE_OTMP:
      return g_strdup("otmp");

    case VVM_TYPE_VVM_THREE:
      return g_strdup("vvm3");

    default:
      return g_strdup("Unknown");

  }
  return g_strdup("Unknown");
}

void
vvmplayer_vvmd_set_mm_vvm_list_box (VvmVvmd *self,
                                    VvmplayerWindow    *visual_voicemail)
{
  self->visual_voicemail = visual_voicemail;
}

int
vvmplayer_vvmd_set_mm_setting (VvmVvmd *self,
                               const char *setting_to_change,
                               const char *updated_setting)
{

  g_autoptr (GError) error = NULL;
  GVariant		*new_setting;

  new_setting = g_variant_new_parsed("(%s, <%s>)",
                                     setting_to_change,
                                     updated_setting);

  g_dbus_proxy_call_sync (self->modemmanager_proxy,
                          "ChangeSettings",
                           new_setting,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           &error);

  if (error != NULL) {
    g_warning ("Error setting %s to %s: Error is %s",
               setting_to_change,
               updated_setting,
               error->message);
    return FALSE;
  }
  return TRUE;

}

int
vvmplayer_vvmd_set_mm_vvm_enabled (VvmVvmd *self,
                                   int enabled)
{

  g_autoptr (GError) error = NULL;
  GVariant		*new_setting;

  new_setting = g_variant_new_parsed("(%s, <%b>)",
                                     "VVMEnabled",
                                     enabled);

  g_dbus_proxy_call_sync (self->modemmanager_proxy,
                          "ChangeSettings",
                           new_setting,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           &error);

  if (error != NULL) {
    g_warning ("Error Enabling or disabling VVM!");
    return FALSE;
  }
  return TRUE;

}

int
vvmplayer_vvmd_check_subscription_status (VvmVvmd *self)
{

  g_autoptr (GError) error = NULL;

  g_dbus_proxy_call_sync (self->modemmanager_proxy,
                          "CheckSubscriptonStatus",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           &error);

  if (error != NULL) {
    g_warning ("Error Checking Subscription Status!");
    return FALSE;
  }
  return TRUE;

}

int
vvmplayer_vvmd_get_service_settings (VvmVvmd *self)
{
  g_autoptr (GError) error = NULL;
  GVariant *ret;

  ret = g_dbus_proxy_call_sync (self->service_proxy,
                                "GetProperties",
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

  if (error != NULL) {
      g_warning ("Error in Proxy call: %s\n", error->message);
      return FALSE;
  } else {
    GVariantDict dict;
    GVariant *all_settings;
    int MailBoxActive;
    g_autofree char     *ModemCountryCode = NULL;
    VvmSettings *settings = vvm_settings_get_default ();

    g_variant_get (ret, "(@a{?*})", &all_settings);
    g_variant_dict_init (&dict, all_settings);
    if (g_variant_dict_lookup (&dict, "ModemCountryCode", "s", &ModemCountryCode)) {
      vvm_settings_set_vvm_country_code (settings, ModemCountryCode);
    }
    if (g_variant_dict_lookup (&dict, "MailBoxActive", "b", &MailBoxActive)) {
      vvm_settings_set_mailbox_active (settings, MailBoxActive);
    }

  }
    return TRUE;
}

static void
vvmplayer_vvmd_delete_vvm_cb (GObject      *interface,
                              GAsyncResult *result,
                              gpointer     *user_data)
{
  g_autoptr (GError) error = NULL;

  if (g_dbus_proxy_call_finish (G_DBUS_PROXY (interface),
                                result,
                                &error)) {
    g_debug ("VVM deleted");
  } else {
    g_debug ("Couldn't delete VVM as read- error: %s", error ? error->message : "unknown");
  }
}

static int
vvm_props_match_by_obj_path (char *old_objectpath,
                             gconstpointer  objectpath)
{
  return g_strcmp0 (old_objectpath, (char *)objectpath) == 0;
}

void
vvmplayer_vvmd_delete_vvm (VvmVvmd     *self,
                           GDBusProxy  *message_proxy,
                           char        *objectpath,
                           gpointer     user_data)
{
    guint index;

    if (g_ptr_array_find_with_equal_func (self->vvm_arr,
                                          objectpath,
                                          (GEqualFunc)vvm_props_match_by_obj_path,
                                          &index)) {
      g_ptr_array_remove_index (self->vvm_arr, index);
      g_debug ("Deleted VVM from pointer array, new length: %d", self->vvm_arr->len);
    } else {
      g_debug ("Could not find MMS in pointer array");
      return;
    }
  g_debug ("Deleting VVM with Object Path: %s", objectpath);

  g_dbus_proxy_call (message_proxy,
                     "Delete",
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     (GAsyncReadyCallback) vvmplayer_vvmd_delete_vvm_cb,
                     user_data);

  return;

}

static void
vvmplayer_vvmd_mark_message_read_cb (GObject      *interface,
                                     GAsyncResult *result,
                                     gpointer     *user_data)
{
  g_autoptr (GError) error = NULL;

  if (g_dbus_proxy_call_finish (G_DBUS_PROXY (interface),
                                result,
                                &error)) {
    g_debug ("VVM marked as read");
  } else {
    g_debug ("Couldn't mark VVM as read- error: %s", error ? error->message : "unknown");
  }
}

void
vvmplayer_vvmd_mark_message_read (GDBusProxy      *message_proxy,
                                  char            *objectpath,
                                  gpointer         user_data)
{
  g_debug ("Marking VVM read with Object Path: %s", objectpath);

  g_dbus_proxy_call (message_proxy,
                     "MarkRead",
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     (GAsyncReadyCallback) vvmplayer_vvmd_mark_message_read_cb,
                     user_data);
}

static void
vvmplayer_vvmd_receive_message (VvmVvmd   *self,
                                GVariant *message_t)
{
  GtkWidget  *voicemail;
  GVariant   *properties;
  char       *objectpath;
  GDBusProxy *message_proxy;

  g_variant_get (message_t, "(o@a{?*})", &objectpath, &properties);
  g_debug ("VVM Object Path: %s", objectpath);

  if (self->vvm_arr->len != 0) {
    guint index;

    if (g_ptr_array_find_with_equal_func (self->vvm_arr,
                                          objectpath,
                                          (GEqualFunc)vvm_props_match_by_obj_path,
                                          &index)) {
      g_debug ("MMS Already exists! skipping...");
      return;
    } else {
      g_debug ("MMS Does not exist yet! adding...");
    }
  } else {
    g_debug ("No stored MMSes exists yet!");
  }

  message_proxy =
    g_dbus_proxy_new_sync (self->connection,
                           G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                           NULL,
                           VVMD_SERVICE,
                           objectpath,
                           VVMD_MESSAGE_INTERFACE,
                           NULL,
                           NULL);

  if (message_proxy == NULL) {
    g_warning ("Error connecting to the message Proxy!");
  }

  voicemail = voicemail_window_new (message_t, message_proxy, self);



  if (voicemail == NULL) {
    g_warning ("There was an error with making the VVM!");
  } else {
    g_ptr_array_add (self->vvm_arr, objectpath);
    g_debug ("Adding VVM to pointer array, new length: %d, objectpath: %s", self->vvm_arr->len, objectpath);
    vvmplayer_window_add_row (self->visual_voicemail, voicemail);
  }
}

static void
vvmplayer_vvmd_get_new_vvm_cb (GDBusConnection   *connection,
                               const char     *sender_name,
                               const char     *object_path,
                               const char     *interface_name,
                               const char     *signal_name,
                               GVariant       *parameters,
                               gpointer       user_data)
{
  VvmVvmd *self = user_data;

  g_debug ("%s", __func__);
  vvmplayer_vvmd_receive_message (self, parameters);
}

static void
vvmplayer_vvmd_get_all_vvm_cb (GObject      *service,
                            GAsyncResult *res,
                            gpointer      user_data)
{
  VvmVvmd *self = user_data;
  g_autoptr (GError) error = NULL;
  GVariant *ret;

  g_debug ("%s", __func__);

  ret = g_dbus_proxy_call_finish (self->service_proxy,
                                  res,
                                  &error);

  if (error != NULL) {
      g_warning ("Error in Proxy call: %s\n", error->message);
  } else {
    GVariant *msg_pack = g_variant_get_child_value (ret, 0);
    GVariantIter iter;
    unsigned long num;

    if ((num = g_variant_iter_init (&iter, msg_pack))) {
      GVariant *message_t;
      g_debug ("Have %lu VVM (s) to process", num);

      while ((message_t = g_variant_iter_next_value (&iter))) {
        vvmplayer_vvmd_receive_message (self, message_t);
      }
    }
  }
}

static void
vvmplayer_vvmd_get_service_cb (GObject      *service,
                               GAsyncResult *res,
                               gpointer      user_data)
{
  VvmVvmd *self = user_data;
  g_autoptr (GError) error = NULL;

  self->service_proxy = g_dbus_proxy_new_finish (res, &error);
  if (error != NULL) {
    g_warning ("Error in VVMD Service Proxy call: %s\n", error->message);
  } else {
    g_debug ("Got VVMD Service");

    if (vvmplayer_vvmd_get_service_settings (self) == FALSE) {
      g_error("Error getting VVMD Service Settings");

    } else {
      VvmSettings *settings = vvm_settings_get_default ();
      vvm_settings_set_service_available (settings, TRUE);

      self->vvmd_service_proxy_watch_id = g_dbus_connection_signal_subscribe (self->connection,
                                        VVMD_SERVICE,
                                        VVMD_SERVICE_INTERFACE,
                                        "MessageAdded",
                                        VVMD_MODEMMANAGER_PATH,
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        (GDBusSignalCallback)vvmplayer_vvmd_get_new_vvm_cb,
                                        self,
                                        NULL);


    if (self->vvmd_service_proxy_watch_id) {
      g_debug ("Listening for new VVMs");
    } else {
      g_warning ("Failed to connect 'MessageAdded' signal");
    }

    g_dbus_proxy_call (self->service_proxy,
                       "GetMessages",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       (GAsyncReadyCallback)vvmplayer_vvmd_get_all_vvm_cb,
                       self);

    }

  }
}

static void
vvmplayer_vvmd_connect_to_service (VvmVvmd         *self,
                                   GVariant        *service)
{
  char               *servicepath, *serviceidentity;
  GVariant           *properties;
  GVariantDict        dict;


  g_variant_get (service, "(o@a{?*})", &servicepath, &properties);
  g_debug ("Service Path: %s", servicepath);

  g_variant_dict_init (&dict, properties);
  if (!g_variant_dict_lookup (&dict, "Identity", "s", &serviceidentity)) {
    g_warning ("Could not get Service Identity!");
    serviceidentity = NULL;
    return;
  }
  g_debug ("Identity: %s", serviceidentity);
  if (g_strcmp0 (servicepath, VVMD_MODEMMANAGER_PATH) == 0) {
    g_dbus_proxy_new (self->connection,
                      G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                      NULL,
                      VVMD_SERVICE,
                      servicepath,
                      VVMD_SERVICE_INTERFACE,
                      NULL,
                      vvmplayer_vvmd_get_service_cb,
                      self);
  }
}

static void
vvmplayer_vvmd_service_added_cb (GDBusConnection *connection,
                              const char      *sender_name,
                              const char      *object_path,
                              const char      *interface_name,
                              const char      *signal_name,
                              GVariant        *parameters,
                              gpointer         user_data)
{
  VvmVvmd *self = user_data;
  g_autoptr (GError) error = NULL;
  g_debug ("%s", __func__);
  g_debug ("Service Added g_variant: %s", g_variant_print (parameters, TRUE));
  vvmplayer_vvmd_connect_to_service (self, parameters);
}

static void
vvmplayer_vvmd_remove_service (VvmVvmd *self)
{
  if (G_IS_OBJECT (self->service_proxy)) {
    g_debug ("Removing Service!");
    g_object_unref (self->service_proxy);
    g_dbus_connection_signal_unsubscribe (self->connection,
                                          self->vvmd_service_proxy_watch_id);

  } else {
    g_warning ("No Service to remove!");
  }
}

static void
vvmplayer_vvmd_service_removed_cb (GDBusConnection *connection,
                                const char      *sender_name,
                                const char      *object_path,
                                const char      *interface_name,
                                const char      *signal_name,
                                GVariant        *parameters,
                                gpointer         user_data)
{
  VvmVvmd *self = user_data;
  g_autoptr (GError) error = NULL;

  g_debug ("%s", __func__);
  g_debug ("Service Removed g_variant: %s", g_variant_print (parameters, TRUE));
  vvmplayer_vvmd_remove_service (self);
}

static void
vvmplayer_vvmd_get_manager_cb (GObject      *manager,
                               GAsyncResult *res,
                               gpointer      user_data)
{
  VvmVvmd *self = user_data;
  g_autoptr (GError) error = NULL;

  self->manager_proxy = g_dbus_proxy_new_finish (res, &error);
  if (error != NULL) {
    g_warning ("Error in VVMD Manager Proxy call: %s\n", error->message);
  } else {
    GVariant *all_services, *service_pack;
    GVariantIter iter;
    unsigned long num;
    g_debug ("Got VVMD Manager");

    self->vvmd_manager_proxy_add_watch_id = g_dbus_connection_signal_subscribe (self->connection,
                                        VVMD_SERVICE,
                                        VVMD_MANAGER_INTERFACE,
                                        "ServiceAdded",
                                        VVMD_PATH,
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        (GDBusSignalCallback)vvmplayer_vvmd_service_added_cb,
                                        self,
                                        NULL);

    self->vvmd_manager_proxy_remove_watch_id = g_dbus_connection_signal_subscribe (self->connection,
                                        VVMD_SERVICE,
                                        VVMD_MANAGER_INTERFACE,
                                        "ServiceRemoved",
                                        VVMD_PATH,
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        (GDBusSignalCallback)vvmplayer_vvmd_service_removed_cb,
                                        self,
                                        NULL);

    all_services = g_dbus_proxy_call_sync (self->manager_proxy,
                                           "GetServices",
                                           NULL,
                                           G_DBUS_CALL_FLAGS_NONE,
                                           -1,
                                           NULL,
                                           NULL);

    service_pack = g_variant_get_child_value (all_services, 0);
    if ((num = g_variant_iter_init (&iter, service_pack))) {
      GVariant *service;

      while ((service = g_variant_iter_next_value (&iter))) {
        vvmplayer_vvmd_connect_to_service (self, service);
      }
    }
  }
}

int
vvmplayer_vvmd_get_mm_settings (VvmVvmd *self)
{
  g_autoptr (GError) error = NULL;
  GVariant *ret;
  g_debug ("Got VVMD Modem Manager");


  ret = g_dbus_proxy_call_sync (self->modemmanager_proxy,
                                "ViewSettings",
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

  if (error != NULL) {
    g_warning ("Error in Proxy call: %s\n", error->message);
    return FALSE;
  } else {
    VvmSettings *settings = vvm_settings_get_default ();
	  GVariantDict dict;
	  GVariant *all_settings;
	  int VVMEnabled;
	  g_autofree char     *VVMType = NULL;
	  g_autofree char     *VVMDestinationNumber = NULL;
	  g_autofree char     *CarrierPrefix = NULL;
	  g_autofree char     *DefaultModemNumber = NULL;
	  g_autofree char     *ProvisionStatus = NULL;

	  g_variant_get (ret, "(@a{?*})", &all_settings);
	  g_variant_dict_init (&dict, all_settings);
	  if (g_variant_dict_lookup (&dict, "VVMEnabled", "b", &VVMEnabled)) {
	    vvm_settings_set_vvm_enabled (settings, VVMEnabled);
	  }
	  if (g_variant_dict_lookup (&dict, "VVMType", "s", &VVMType)) {
	    g_debug("VVMType is:%s", VVMType);
	    vvm_settings_set_vvm_type (settings, VVMType);
	  }
	  if (g_variant_dict_lookup (&dict, "VVMDestinationNumber", "s", &VVMDestinationNumber)) {
	    vvm_settings_set_vvm_destination_number (settings, VVMDestinationNumber);
	  }
	  if (g_variant_dict_lookup (&dict, "CarrierPrefix", "s", &CarrierPrefix)) {
	    vvm_settings_set_vvm_carrier_prefix (settings, CarrierPrefix);
	  }
	  if (g_variant_dict_lookup (&dict, "DefaultModemNumber", "s", &DefaultModemNumber)) {
	    vvm_settings_set_vvm_default_number (settings, DefaultModemNumber);
	  }
	  if (g_variant_dict_lookup (&dict, "ProvisionStatus", "s", &ProvisionStatus)) {
	    vvm_settings_set_vvm_provision_status (settings, ProvisionStatus);
	  }
	}
  return TRUE;
}

static void
vvmplayer_vvmd_get_modemmanager_cb (GObject      *simple,
                                    GAsyncResult *res,
                                    gpointer      user_data)
{
  VvmVvmd *self = user_data;
  g_autoptr (GError) error = NULL;

  /*
   * There is a race condition where if vvmd loads after VVM Player,
   * the dbus is not set up in time for it to do this proxy call. I am sleeping
   * for 1 second here to fix that.
   */
  //TODO: Probably a better way to handle doing this?
  sleep (1);

  self->modemmanager_proxy = g_dbus_proxy_new_finish (res, &error);
  if (error != NULL) {
    g_warning ("Error in VVMD Modem Manager Proxy call: %s\n", error->message);
  } else {

    if (vvmplayer_vvmd_get_mm_settings(self) == FALSE) {
      g_error("Error getting VVMD Modem Manager Settings");

    } else {
      VvmSettings *settings = vvm_settings_get_default ();
	    vvm_settings_set_mm_available (settings, TRUE);

  		g_dbus_proxy_new (self->connection,
  		                  G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
	  	                  NULL,
	  	                  VVMD_SERVICE,
	  	                  VVMD_PATH,
	  	                  VVMD_MANAGER_INTERFACE,
	  	                  NULL,
	  	                  vvmplayer_vvmd_get_manager_cb,
	  	                  self);

    }
  }
}

static void
vvmd_appeared_cb (GDBusConnection *connection,
                  const char      *name,
                  const char      *name_owner,
                  gpointer         user_data)
{
  VvmVvmd *self = user_data;
  g_assert (G_IS_DBUS_CONNECTION (connection));
  self->connection = connection;
  g_debug("VVMD Appeared");

  g_dbus_proxy_new (self->connection,
                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                    NULL,
                    VVMD_SERVICE,
                    VVMD_PATH,
                    VVMD_MODEMMANAGER_INTERFACE,
                    NULL,
                    vvmplayer_vvmd_get_modemmanager_cb,
                    self);

}

static void
vvmd_vanished_cb (GDBusConnection *connection,
                  const char      *name,
                  gpointer        user_data)
{
  VvmVvmd *self = user_data;
  g_assert (G_IS_DBUS_CONNECTION (connection));
  VvmSettings *settings = vvm_settings_get_default ();

  g_debug ("VVMD vanished");
  vvm_settings_set_mm_available (settings, FALSE);
  vvm_settings_set_service_available (settings, FALSE);

  if (G_IS_OBJECT (self->service_proxy)) {
    vvmplayer_vvmd_remove_service (self);
  }
  if (G_IS_OBJECT (self->manager_proxy)) {
    g_object_unref (self->manager_proxy);
    g_dbus_connection_signal_unsubscribe (self->connection,
                                          self->vvmd_manager_proxy_add_watch_id);
    g_dbus_connection_signal_unsubscribe (self->connection,
                                          self->vvmd_manager_proxy_remove_watch_id);
  }

  if (G_IS_DBUS_CONNECTION (self->connection)) {
    g_dbus_connection_unregister_object (self->connection,
                                         self->vvmd_watch_id);
  }

}


static void
vvm_vvmd_constructed (GObject *object)
{
  //VvmVvmd *self = (VvmVvmd *)object;

  G_OBJECT_CLASS (vvm_vvmd_parent_class)->constructed (object);

}

static void
vvm_vvmd_finalize (GObject *object)
{
  VvmVvmd *self = (VvmVvmd *)object;
  if (self->vvm_arr && self->vvm_arr->len) {
    g_ptr_array_unref (self->vvm_arr);
  }
  G_OBJECT_CLASS (vvm_vvmd_parent_class)->finalize (object);
}

static void
vvm_vvmd_class_init (VvmVvmdClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = vvm_vvmd_constructed;
  object_class->finalize = vvm_vvmd_finalize;
}

static void
vvm_vvmd_init (VvmVvmd *self)
{
    self->vvm_arr = g_ptr_array_new_with_free_func (g_free);
}

static void
vvm_vmd_eds_is_ready (VvmVvmd *self)
{
  g_debug("EDS IS READY!");
  VvmContact *test;

  //TODO: We have loaded contacts, now you need to load them into the window

  test = vvm_eds_find_by_number(self->contacts, "+1XXXXXXXXXX");

  if (test != NULL) {
    g_debug("Found contact!");
    g_debug("Contact: %s", vvm_contact_get_name (test));
  } else {
    g_debug("Did not find contact!");
  }

}

VvmVvmd *
vvm_vvmd_get_default (void)
{
  static VvmVvmd *self;

  if (!self)
    {
      self = g_object_new (VVM_TYPE_VVMD, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
      self->vvmd_watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                              VVMD_SERVICE,
                                              G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                              (GBusNameAppearedCallback)vvmd_appeared_cb,
                                              (GBusNameVanishedCallback)vvmd_vanished_cb,
                                              self, NULL);

      self->contacts = vvm_eds_get_default ();
      g_signal_connect_object (self->contacts, "notify::is-ready",
                               G_CALLBACK (vvm_vmd_eds_is_ready), self,
                               G_CONNECT_SWAPPED);

    }
  return self;
}
