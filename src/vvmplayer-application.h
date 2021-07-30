#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VVM_TYPE_PLAYER (calls_application_get_type ())

G_DECLARE_FINAL_TYPE (VvmPlayer, vvm_player, VVM, PLAYER, GtkApplication)

VvmPlayer *vvm_player_new (void);

G_END_DECLS
