#include "vvmplayer-settings.h"

struct _VvmSettings
{
  GObject     parent_instance;

  GtkSettings *gtk_settings;
};

G_DEFINE_TYPE (VvmSettings, vvm_settings, G_TYPE_OBJECT)

static void
vvm_settings_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  VvmSettings *self = (VvmSettings *)object;

}

static void
vvm_settings_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  VvmSettings *self = (VvmSettings *)object;
}

static void
vvm_settings_constructed (GObject *object)
{
  VvmSettings *self = (VvmSettings *)object;

  G_OBJECT_CLASS (vvm_settings_parent_class)->constructed (object);

}

static void
vvm_settings_finalize (GObject *object)
{
  VvmSettings *self = (VvmSettings *)object;

  g_object_unref (self->gtk_settings);

  G_OBJECT_CLASS (vvm_settings_parent_class)->finalize (object);
}

static void
vvm_settings_class_init (VvmSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = vvm_settings_get_property;
  object_class->set_property = vvm_settings_set_property;
  object_class->constructed = vvm_settings_constructed;
  object_class->finalize = vvm_settings_finalize;
}

static void
vvm_settings_init (VvmSettings *self)
{
  self->gtk_settings = gtk_settings_get_default ();
}

VvmSettings *
vvm_settings_get_default (void)
{
  static VvmSettings *self;

  if (!self)
    {
      self = g_object_new (VVM_TYPE_SETTINGS, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
    }
  g_debug("Hello!");
  return self;
}
