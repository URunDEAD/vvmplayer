#include "vvmplayer-vvmd.h"

struct _VvmVvmd
{
  GObject     parent_instance;
};

G_DEFINE_TYPE (VvmVvmd, vvm_vvmd, G_TYPE_OBJECT)

static void
vvm_vvmd_constructed (GObject *object)
{
  VvmVvmd *self = (VvmVvmd *)object;

  G_OBJECT_CLASS (vvm_vvmd_parent_class)->constructed (object);

}

static void
vvm_vvmd_finalize (GObject *object)
{
  VvmVvmd *self = (VvmVvmd *)object;

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
}

VvmVvmd *
vvm_vvmd_get_default (void)
{
  static VvmVvmd *self;

  if (!self)
    {
      self = g_object_new (VVM_TYPE_VVMD, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
    }
  return self;
}
