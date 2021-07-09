#include "vvmplayer-preferences-window.h"

struct _HdyVvmPreferencesWindow
{
  HdyPreferencesWindow parent_instance;

  GtkWidget *subpage1;
  GtkWidget *subpage2;
};

G_DEFINE_TYPE (HdyVvmPreferencesWindow, hdy_vvm_preferences_window, HDY_TYPE_PREFERENCES_WINDOW)

HdyVvmPreferencesWindow *
hdy_vvm_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_VVM_PREFERENCES_WINDOW, NULL);
}

static void
return_to_preferences_cb (HdyVvmPreferencesWindow *self)
{
  hdy_preferences_window_close_subpage (HDY_PREFERENCES_WINDOW (self));
}

static void
subpage1_activated_cb (HdyVvmPreferencesWindow *self)
{
  hdy_preferences_window_present_subpage (HDY_PREFERENCES_WINDOW (self), self->subpage1);
}

static void
subpage2_activated_cb (HdyVvmPreferencesWindow *self)
{
  hdy_preferences_window_present_subpage (HDY_PREFERENCES_WINDOW (self), self->subpage2);
}

static void
hdy_vvm_preferences_window_class_init (HdyVvmPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-preferences-window.ui");

  gtk_widget_class_bind_template_child (widget_class, HdyVvmPreferencesWindow, subpage1);
  gtk_widget_class_bind_template_child (widget_class, HdyVvmPreferencesWindow, subpage2);

  gtk_widget_class_bind_template_callback (widget_class, return_to_preferences_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpage1_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpage2_activated_cb);
}

static void
hdy_vvm_preferences_window_init (HdyVvmPreferencesWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
