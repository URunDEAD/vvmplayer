/* See https://gitlab.gnome.org/GNOME/evolution-data-server/-/issues/332#note_1107764 */
#define EDS_DISABLE_DEPRECATED

#include <glib-object.h>
#include <libebook-contacts/libebook-contacts.h>

G_BEGIN_DECLS

#define VVM_TYPE_CONTACT (vvm_contact_get_type ())

G_DECLARE_FINAL_TYPE (VvmContact, vvm_contact, VVM, CONTACT, GObject)

VvmContact        *vvm_contact_new                   (EContact          *contact,
                                                         EVCardAttribute   *attr);
void               vvm_contact_set_name              (VvmContact     *self,
                                                         const char        *name);
const char        *vvm_contact_get_value             (VvmContact     *self);
void               vvm_contact_set_value             (VvmContact     *self,
                                                         const char        *value);
const char        *vvm_contact_get_value_type        (VvmContact     *self);
const char        *vvm_contact_get_uid               (VvmContact     *self);
gboolean           vvm_contact_is_dummy              (VvmContact     *self);
const char        *vvm_contact_get_name              (VvmContact *self);
gboolean           vvm_contact_matches               (VvmContact     *self,
                                                      const char     *needle);

G_END_DECLS
