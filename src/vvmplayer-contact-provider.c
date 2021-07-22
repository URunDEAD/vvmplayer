/* vvm-contact-provider.c
 *
 * Copyright 2020 Purism SPC
 *           2021 Chris Talbot
 *
 * Author(s):
 *   Mohammed Sadiq <sadiq@sadiqpk.org>
 *   Chris Talbot   <chris@talbothome.com>
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


/* See https://gitlab.gnome.org/GNOME/evolution-data-server/-/issues/332#note_1107764 */
#define EDS_DISABLE_DEPRECATED

#include <libebook/libebook.h>

#include "vvmplayer-contact-provider.h"

#define PHONE_SEXP    "(contains 'phone' '')"

struct _VvmEds
{
  GObject           parent_instance;

  ESourceRegistry  *source_registry;
  GCancellable     *cancellable;

  /* contacts to be saved to contacts_list */
  GPtrArray        *contacts_array;

  GListStore       *eds_view_list;
  GListStore       *contacts_list;
  guint             providers_to_load;
  gboolean          is_ready;
};

G_DEFINE_TYPE (VvmEds, vvm_eds, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_IS_READY,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
eds_find_contact_index (VvmEds  *self,
                        const char *uid,
                        guint      *position,
                        guint      *count)
{
  const char *old_uid;
  guint n_items, i;
  gboolean match;

  g_assert (VVM_IS_EDS (self));
  g_assert (position && count);
  g_assert (uid && *uid);

  n_items = g_list_model_get_n_items (G_LIST_MODEL (self->contacts_list));
  *position = *count = 0;

  for (i = 0; i < n_items; i++) {
    g_autoptr(VvmContact) contact = NULL;

    contact = g_list_model_get_item (G_LIST_MODEL (self->contacts_list), i);
    old_uid = vvm_contact_get_uid (contact);
    match = g_str_equal (old_uid, uid);

    /* The list is sorted.  Find the first match and total count. */
    if (!*count && match)
      *position = i, *count = 1;
    else if (match) /* Subsequent matches */
      ++*count;
    else if (*position) /* We moved past the last match */
      break;
  }
}

static void
vvm_eds_load_contact (VvmEds     *self,
                      EContact      *contact,
                      EContactField  field_id)
{
  g_autoptr(GList) attributes = NULL;

  g_assert (VVM_IS_EDS (self));
  g_assert (E_IS_CONTACT (contact));

  /* Only container should be freed, attribute is freed in VvmContact */
  attributes = e_contact_get_attributes (contact, field_id);

  for (GSList *l = (GSList *)attributes; l != NULL; l = l->next) {
    g_autofree char *value = NULL;

    value = e_vcard_attribute_get_value (l->data);

    if (value && *value)
      g_ptr_array_add (self->contacts_array,
                       vvm_contact_new (contact, l->data));

    }
}

static void
vvm_eds_remove_contact (VvmEds  *self,
                        const char *uid)
{
  guint position, count;

  g_assert (VVM_IS_EDS (self));

  eds_find_contact_index (self, uid, &position, &count);

  if (count)
    g_list_store_splice (self->contacts_list, position, count, NULL, 0);
}

static void
vvm_eds_objects_added_cb (VvmEds       *self,
                          const GSList    *objects,
                          EBookClientView *view)
{
  g_assert (VVM_IS_EDS (self));
  g_assert (E_IS_BOOK_CLIENT_VIEW (view));
  if (!self->contacts_array)
    self->contacts_array = g_ptr_array_new_full (100, g_object_unref);

  for (GSList *l = (GSList *)objects; l != NULL; l = l->next)
    {
        vvm_eds_load_contact (self, l->data, E_CONTACT_TEL);
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
}

static void
vvm_eds_objects_modified_cb (VvmEds       *self,
                             const GSList    *objects,
                             EBookClientView *view)
{
  g_assert (VVM_IS_EDS (self));
  g_assert (E_IS_BOOK_CLIENT_VIEW (view));
  for (GSList *l = (GSList *)objects; l != NULL; l = l->next)
    vvm_eds_remove_contact (self, e_contact_get_const (l->data, E_CONTACT_UID));

  vvm_eds_objects_added_cb (self, objects, view);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
}

static void
vvm_eds_objects_removed_cb (VvmEds       *self,
                            const GSList    *objects,
                            EBookClientView *view)
{
  g_assert (VVM_IS_EDS (self));
  g_assert (E_IS_BOOK_CLIENT_VIEW (view));
  for (GSList *node = (GSList *)objects; node; node = node->next)
    vvm_eds_remove_contact (self, node->data);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
}

static void
vvm_eds_load_complete_cb (VvmEds *self)
{
  g_autoptr(GPtrArray) array = NULL;
  if (!self->contacts_array || !self->contacts_array->len)
    return;

  array = g_steal_pointer (&self->contacts_array);
  g_list_store_splice (self->contacts_list, 0, 0, array->pdata, array->len);

  self->is_ready = TRUE;
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
}

static void
vvm_eds_get_view_cb (GObject      *object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
  g_autoptr(VvmEds) self = user_data;
  EBookClient *client = E_BOOK_CLIENT (object);
  EBookClientView *client_view;
  g_autoptr(GError) error = NULL;

  g_assert (VVM_IS_EDS (self));
  g_assert (E_IS_BOOK_CLIENT (client));

  e_book_client_get_view_finish (E_BOOK_CLIENT (client), result, &client_view, &error);

  if (error)
    {
      if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        g_warning ("Error: %s", error->message);

      return;
    }

  g_list_store_append (self->eds_view_list, client_view);
  g_object_unref (client_view);

  g_signal_connect_object (client_view, "objects-added",
                           G_CALLBACK (vvm_eds_objects_added_cb), self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (client_view, "objects-modified",
                           G_CALLBACK (vvm_eds_objects_modified_cb), self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (client_view, "objects-removed",
                           G_CALLBACK (vvm_eds_objects_removed_cb), self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (client_view, "complete",
                           G_CALLBACK (vvm_eds_load_complete_cb), self,
                           G_CONNECT_SWAPPED);

  e_book_client_view_start (client_view, &error);

  if (error)
    g_warning ("Error: %s", error->message);
}

static void
vvm_eds_client_connected_cb (GObject      *object,
                             GAsyncResult *result,
                             gpointer      user_data)
{
  g_autoptr(VvmEds) self = user_data;
  EClient *client;
  g_autoptr(GError) error = NULL;

  g_assert (VVM_IS_EDS (self));

  client = e_book_client_connect_finish (result, &error);

  if (!error)
    {
      ESourceOffline *extension;
      ESource *source;

      source = e_client_get_source (client);
      extension = e_source_get_extension (source, E_SOURCE_EXTENSION_OFFLINE);
      e_source_offline_set_stay_synchronized (extension, TRUE);
      e_source_registry_commit_source_sync (self->source_registry, source,
                                            self->cancellable, &error);
    }

  if (error)
    {
      if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        g_warning ("Error: %s", error->message);
      g_clear_object (&client);

      return;
    }

  e_book_client_get_view (E_BOOK_CLIENT (client),
                          PHONE_SEXP,
                          NULL,
                          vvm_eds_get_view_cb,
                          g_object_ref (self));
  g_clear_object (&client);
}

static void
vvm_eds_load_contacts (VvmEds *self)
{
  GList *sources;

  g_assert (VVM_IS_EDS (self));
  g_assert (E_IS_SOURCE_REGISTRY (self->source_registry));

  sources = e_source_registry_list_sources (self->source_registry,
                                            E_SOURCE_EXTENSION_ADDRESS_BOOK);

  for (GList *l = sources; l != NULL; l = l->next)
    {
      self->providers_to_load++;
      e_book_client_connect (l->data,
                             -1,    /* timeout seconds */
                             NULL,
                             vvm_eds_client_connected_cb,
                             g_object_ref (self));
    }

  g_list_free_full (sources, g_object_unref);
}

static void
vvm_eds_registry_new_finish_cb (GObject      *object,
                                GAsyncResult *result,
                                gpointer      user_data)
{
  g_autoptr(VvmEds) self = user_data;
  g_autoptr(GError) error = NULL;

  g_assert (VVM_IS_EDS (self));

  self->source_registry = e_source_registry_new_finish (result, &error);

  if (!error)
    vvm_eds_load_contacts (self);
  else if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    g_warning ("Error: %s", error->message);
}

static void
vvm_eds_load (VvmEds *self)
{
  g_assert (VVM_IS_EDS (self));
  g_assert (G_IS_CANCELLABLE (self->cancellable));

  e_source_registry_new (self->cancellable,
                         vvm_eds_registry_new_finish_cb,
                         g_object_ref (self));
}

static void
vvm_eds_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  VvmEds *self = (VvmEds *)object;

  switch (prop_id)
    {
    case PROP_IS_READY:
      g_value_set_boolean (value, self->is_ready);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

int
vvm_eds_get_is_ready (VvmEds *self)
{
  return self->is_ready;
}

static void
vvm_eds_constructed (GObject *object)
{
  VvmEds *self = (VvmEds *)object;

  G_OBJECT_CLASS (vvm_eds_parent_class)->constructed (object);

  vvm_eds_load (self);
}


static void
vvm_eds_finalize (GObject *object)
{
  VvmEds *self = (VvmEds *)object;

  g_cancellable_cancel (self->cancellable);
  g_clear_object (&self->cancellable);
  g_clear_object (&self->eds_view_list);
  g_clear_object (&self->contacts_list);
  if (self->contacts_array)
    g_ptr_array_free (self->contacts_array, TRUE);

  G_OBJECT_CLASS (vvm_eds_parent_class)->finalize (object);
}


static void
vvm_eds_class_init (VvmEdsClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);

  object_class->get_property = vvm_eds_get_property;
  object_class->constructed  = vvm_eds_constructed;
  object_class->finalize = vvm_eds_finalize;

  properties[PROP_IS_READY] =
    g_param_spec_boolean ("is-ready",
                          "Is Ready",
                          "The contact provider is ready and loaded",
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
}


static void
vvm_eds_init (VvmEds *self)
{
  self->eds_view_list = g_list_store_new (E_TYPE_BOOK_CLIENT_VIEW);
  self->contacts_list = g_list_store_new (VVM_TYPE_CONTACT);
  self->cancellable = g_cancellable_new ();
}


VvmEds *
vvm_eds_get_default (void)
{
  static VvmEds *self;

  if (!self) {
    self = g_object_new (VVM_TYPE_EDS, NULL);
    g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
  }

  return self;
}

gboolean
vvm_eds_is_ready (VvmEds  *self)
{
  g_return_val_if_fail (VVM_IS_EDS (self), FALSE);

  return self->is_ready;
}

/**
 * vvm_eds_find_by_number:
 * @self: A #VvmEds
 * @phone_number: A Valid Phone number to match
 *
 * Find the first #VvmContact matching @phone_number.
 * A match can be either exact or one excluding the
 * country prefix (Eg: +1987654321 and 987654321 matches)
 *
 * Returns: (transfer none) (nullable): A #VvmContact.
 */
VvmContact *
vvm_eds_find_by_number (VvmEds  *self,
                           const char *phone_number)
{
  g_return_val_if_fail (VVM_IS_EDS (self), NULL);

  GListModel *model;
  guint n_items;
  gboolean match;

  model = G_LIST_MODEL (self->contacts_list);
  n_items = g_list_model_get_n_items (model);

  for (guint i = 0; i < n_items; i++)
    {
      g_autoptr(GObject) item = NULL;

      item = g_list_model_get_item (model, i);
      match = vvm_contact_matches (VVM_CONTACT (item), phone_number);

      if (match)
        return VVM_CONTACT (item);
    }

  return NULL;
}

/**
 * vvm_eds_get_model:
 * @self: A #VvmEds
 *
 * Get A #GListModel that contains all the
 * #VvmContact loaded.
 *
 * Returns: (transfer none): A #GListModel.
 */
GListModel *
vvm_eds_get_model (VvmEds *self)
{
  g_return_val_if_fail (VVM_IS_EDS (self), NULL);

  return G_LIST_MODEL (self->contacts_list);
}
