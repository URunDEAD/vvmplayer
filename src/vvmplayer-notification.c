/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-notification.c
 *
 * Copyright 2021 Chris Talbot <chris@talbothome.com>
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
 *
 * Author(s):
 *   Chris Talbot <chris@talbothome.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "vvmplayer-notification"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "vvmplayer-notification.h"

/**
 * SECTION: vvmplayer-notification
 * @title: vvmplayer-notification
 * @short_description: Generic functions
 * @include: "vvmplayer-notification.h"
 *
 * A grouping of all notifications.
 */

void
vvmplayer_notification_send_unread_vvm (int unread_vvms)
{
  GNotification *notification;
  g_autofree char *title = NULL;
  g_autofree char *body = NULL;
  GApplication *vvm_player = g_application_get_default ();

  /* If the notification is already there, withdraw it */
  g_application_withdraw_notification (vvm_player, "unread-message");

  title = g_strdup ("New Voicemail");
  if (unread_vvms == 1)
    body = g_strdup_printf ("You have %d new voicemail", unread_vvms);
  else
    body = g_strdup_printf ("You have %d new voicemails", unread_vvms);
  notification = g_notification_new (title);
  g_notification_set_body (notification, body);
  g_notification_set_priority (notification, G_NOTIFICATION_PRIORITY_HIGH);

  g_application_send_notification (vvm_player, "unread-message", notification);

  g_object_unref (notification);
}

void
vvmplayer_notification_withdraw_unread_vvm (void)
{
  GApplication *vvm_player = g_application_get_default ();

  g_application_withdraw_notification (vvm_player, "unread-message");
}
