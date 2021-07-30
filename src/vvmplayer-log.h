/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-log.h
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

#pragma once

#ifndef VVMPLAYER_LOG_LEVEL_TRACE
# define VVMPLAYER_LOG_LEVEL_TRACE ((GLogLevelFlags)(1 << G_LOG_LEVEL_USER_SHIFT))
#endif

/* XXX: Should we use the semi-private g_log_structured_standard() API? */
#define VVMPLAYER_TRACE_MSG(fmt, ...)                        \
  g_log_structured (G_LOG_DOMAIN, VVMPLAYER_LOG_LEVEL_TRACE, \
                    "MESSAGE", "%s():%d: " fmt,              \
                    G_STRFUNC, __LINE__, ## __VA_ARGS__)
#define VVMPLAYER_TRACE(fmt, ...)                            \
  g_log_structured (G_LOG_DOMAIN, VVMPLAYER_LOG_LEVEL_TRACE, \
                    "MESSAGE", fmt, ## __VA_ARGS__)
#define VVMPLAYER_DEBUG_MSG(fmt, ...)                \
  g_log_structured (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, \
                    "MESSAGE", "%s():%d: " fmt,      \
                    G_STRFUNC, __LINE__, ## __VA_ARGS__)
#define VVMPLAYER_TODO(_msg)                                 \
  g_log_structured (G_LOG_DOMAIN, VVMPLAYER_LOG_LEVEL_TRACE, \
                    "MESSAGE", "TODO: %s():%d: %s",          \
                    G_STRFUNC, __LINE__, _msg)

void vvmplayer_log_init               (void);
void vvmplayer_log_increase_verbosity (void);
int  vvmplayer_log_get_verbosity      (void);
