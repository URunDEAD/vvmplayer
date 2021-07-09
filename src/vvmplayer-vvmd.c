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
      //TODO: Load vvmd dbus!
    }
  return self;
}
