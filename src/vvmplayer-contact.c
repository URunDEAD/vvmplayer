/* vvm-contact.c
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

#define G_LOG_DOMAIN "vvmplayer-contact"

#ifdef HAVE_CONFIG_H
# include "config.h"
# include "version.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libebook-contacts/libebook-contacts.h>

#include "vvmplayer-contact.h"
#include "vvmplayer-settings.h"

struct _VvmContact
{
  GObject       parent_instance;

  EContact        *e_contact;
  EVCardAttribute *attribute;

  char       *name;
  char       *value;
};

G_DEFINE_TYPE (VvmContact, vvm_contact, G_TYPE_OBJECT)


gboolean
vvm_contact_matches (VvmContact     *self,
                        const char     *needle)
{
  const char *value;
  VvmSettings *vvm_setting;
  const char *country;
  EPhoneNumberMatch match;
  g_assert (VVM_IS_CONTACT (self));

  value = vvm_contact_get_value (self);
  vvm_setting = vvm_settings_get_default ();
  country = vvm_settings_get_vvm_country_code (vvm_setting);
  match = e_phone_number_compare_strings_with_region (value, needle, country, NULL);

  if (match == E_PHONE_NUMBER_MATCH_EXACT ||
      match == E_PHONE_NUMBER_MATCH_NATIONAL)
    return TRUE;

  if (g_str_equal (value, needle))
    return TRUE;

  return FALSE;
}


const char *
vvm_contact_get_name (VvmContact *self)
{
  g_assert (VVM_IS_CONTACT (self));

  if (self->name)
    return self->name;

  if (self->e_contact) {
    const char *value;

    value = e_contact_get_const (self->e_contact, E_CONTACT_FULL_NAME);

    if (value)
      return value;
  }

  return "";
}

static void
vvm_contact_class_init (VvmContactClass *klass)
{
  //GObjectClass *object_class  = G_OBJECT_CLASS (klass);

}


static void
vvm_contact_init (VvmContact *self)
{
}


VvmContact *
vvm_contact_new (EContact        *contact,
                    EVCardAttribute *attr)
{
  VvmContact *self;

  self = g_object_new (VVM_TYPE_CONTACT, NULL);
  self->e_contact = g_object_ref (contact);
  self->attribute = attr;

  return self;
}

void
vvm_contact_set_name (VvmContact *self,
                         const char    *name)
{
  g_return_if_fail (VVM_IS_CONTACT (self));

  g_free (self->name);
  self->name = g_strdup (name);
}

/**
 * vvm_contact_get_value:
 * @self: A #VvmContact
 *
 * Get the value stored in @self. It can be a phone
 * number, an XMPP ID, etc.
 * Also see vvm_contact_get_value_type().
 *
 * Returns: (transfer none): The value of @self.
 * Or an empty string if no value.
 */
const char *
vvm_contact_get_value (VvmContact *self)
{
  g_return_val_if_fail (VVM_IS_CONTACT (self), NULL);

  if (!self->value && self->attribute)
    self->value = e_vcard_attribute_get_value (self->attribute);

  if (self->value)
    return self->value;

  return "";
}

void
vvm_contact_set_value (VvmContact *self,
                          const char    *value)
{
  g_return_if_fail (VVM_IS_CONTACT (self));

  g_free (self->value);
  self->value = g_strdup (value);
}

/**
 * vvm_contact_get_value:
 * @self: A #VvmContact
 *
 * Get the type of value stored in @self.
 * Eg: “Mobile”, “Work”, etc. translated to
 * the current locale.
 *
 * Returns: (transfer none): The value type of @self.
 */
const char *
vvm_contact_get_value_type (VvmContact *self)
{
  g_return_val_if_fail (VVM_IS_CONTACT (self), NULL);

  if (!self->attribute)
    return "";

  if (e_vcard_attribute_has_type (self->attribute, "cell"))
    return _("Mobile: ");
  if (e_vcard_attribute_has_type (self->attribute, "work"))
    return _("Work: ");
  if (e_vcard_attribute_has_type (self->attribute, "other"))
    return _("Other: ");

  return "";
}


/**
 * vvm_contact_get_uid:
 * @self: A #VvmContact
 *
 * A unique ID reperesenting the contact.  This
 * ID won’t change unless the contact is modified.
 *
 * Returns: (transfer none): A unique ID of @self.
 */
const char *
vvm_contact_get_uid (VvmContact *self)
{
  g_return_val_if_fail (VVM_IS_CONTACT (self), "");

  if (self->e_contact)
    return e_contact_get_const (self->e_contact, E_CONTACT_UID);

  return "";
}

gboolean
vvm_contact_is_dummy (VvmContact *self)
{
  g_return_val_if_fail (VVM_IS_CONTACT (self), TRUE);

  return !!g_object_get_data (G_OBJECT (self), "dummy");
}
