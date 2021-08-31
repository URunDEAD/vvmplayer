/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/* vvmplayer-voicemail-window.c
 *
 * Copyright 2021 Chris Talbot
 *           2021 GStreamer developers
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

#define G_LOG_DOMAIN "vvmplayer-voicemail-window"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "vvmplayer-voicemail-window.h"

#define ICON_PLAYBACK_STOP "media-playback-stop"
#define ICON_PLAYBACK_START "media-playback-start"
#define ICON_PLAYBACK_PAUSE "media-playback-pause"
#define ICON_DELETE "edit-delete"

struct _VoicemailWindow
{
  HdyActionRow parent_instance;

  GstElement *playbin;
  GstState state;

  GtkImage *play_button_icon;
  GtkImage *delete_button_icon;

  HdyExpanderRow *action_expander_row;
  HdyActionRow *second_row;
  HdyActionRow *call_row;
  HdyActionRow *sms_row;
  gint64 duration;
  gint64 minutes;
  gint64 seconds;
  char *duration_formatted;
  char *objectpath;
  char *contact_name;
  char *number;
  int lifetime_status;
  GDBusProxy *message_proxy;
  GDateTime *time_local;

};

G_DEFINE_TYPE (VoicemailWindow, voicemail_window, HDY_TYPE_ACTION_ROW)

static void
vvm_preferences_set_hdyexpanderrow_text (HdyExpanderRow *hdy_row, const char *text) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, text);
  g_object_set_property (G_OBJECT (hdy_row), "title", &a);
}

static void
voicemail_window_set_title (VoicemailWindow *self)
{
  g_autofree char *title = NULL;

  if (self->contact_name) {
    title = g_strdup(self->contact_name);
  } else {
    title = g_strdup(self->number);
  }

  if (self->lifetime_status == VVM_LIFETIME_STATUS_NOT_READ) {
    title = g_strdup_printf ("%s (Unread)", title);
  }
  vvm_preferences_set_hdyexpanderrow_text(self->action_expander_row, title);
}

char
*voicemail_window_get_number (VoicemailWindow *self)
{
  return g_strdup(self->number);
}

GDateTime
*voicemail_window_get_datetime (VoicemailWindow *self)
{
  return self->time_local;
}

char
*voicemail_window_get_objectpath (VoicemailWindow *self)
{
  return g_strdup(self->objectpath);
}

void
voicemail_window_set_contact_name (VoicemailWindow *self,
                                   const char *contact_name)
{
  g_free(self->contact_name);
  self->contact_name = g_strdup(contact_name);
  voicemail_window_set_title(self);
}

static void
vvm_preferences_set_hdyactionrow_text (HdyActionRow *hdy_row, const char *text) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, text);
  g_object_set_property (G_OBJECT (hdy_row), "title", &a);
}

static gboolean refresh_ui (VoicemailWindow *self) {

  if (self->state == GST_STATE_READY || self->state == GST_STATE_PAUSED) {
    /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
    return FALSE;
  }

  g_debug("Refreshing UI!");
  g_autofree char *duration = NULL;
  self->seconds = self->seconds + 1;
  if (self->seconds == 60) {
    self->seconds = 0;
    self->minutes = self->minutes + 1;
  }
  duration = g_strdup_printf("%02lu:%02lu/%s", self->minutes, self->seconds, self->duration_formatted);
  vvm_preferences_set_hdyactionrow_text(self->second_row, duration);

  return TRUE;

}

static void
vvm_voicemail_set_gtkimage_icon (GtkImage *image_to_set, const char *icon) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, icon);
  g_object_set_property (G_OBJECT (image_to_set), "icon-name", &a);
}

static void
play_button_clicked_cb (GtkButton     *btn,
                        VoicemailWindow *self)
{
  g_debug("Play button clicked!");
  if (!self->playbin) {
    g_debug("Playbin is NULL!");
    return;
  }
  VvmVvmd *vvmd_object = vvm_vvmd_get_default ();

  switch(self->lifetime_status) {
    case VVM_TYPE_UNKNOWN:
    case VVM_LIFETIME_STATUS_NOT_READ:
      g_debug("Marking as read....");
      vvmplayer_vvmd_decrement_unread (vvmd_object);
      self->lifetime_status = VVM_LIFETIME_STATUS_READ;
      voicemail_window_set_title (self);
      vvmplayer_vvmd_mark_message_read (self->message_proxy, self->objectpath, self);
      break;

    case VVM_LIFETIME_STATUS_READ:
      break;

    default:
      g_warning("Something bad happened!");
  }


  // We are not playing anything
  // Change it to Playing
  if (self->state != GST_STATE_PLAYING) {
    GstStateChangeReturn ret;
    vvm_voicemail_set_gtkimage_icon (self->delete_button_icon, ICON_PLAYBACK_STOP);
    ret = gst_element_set_state (self->playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr ("Unable to set the pipeline to the playing state.\n");
    } else {
      vvm_voicemail_set_gtkimage_icon (self->play_button_icon, ICON_PLAYBACK_PAUSE);
      /* Register a function that GLib will call every second */
      g_timeout_add_seconds (1, (GSourceFunc)refresh_ui, self);
      self->state = GST_STATE_PLAYING;
    }
  // We are playing
  // Change it to paused
  } else {
    GstStateChangeReturn ret;
    vvm_voicemail_set_gtkimage_icon (self->delete_button_icon, ICON_DELETE);
    ret = gst_element_set_state (self->playbin, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr ("Unable to set the pipeline to the playing state.\n");
    } else {
      vvm_voicemail_set_gtkimage_icon (self->play_button_icon, ICON_PLAYBACK_START);
      self->state = GST_STATE_PAUSED;
    }
  }
}

static void
call_button_clicked_cb (GtkButton     *btn,
                        VoicemailWindow *self)
{
  g_autoptr(GError) error = NULL;
  g_autofree char *uri = NULL;

  uri = g_strconcat ("tel://",
                     self->number,
                     NULL);

  if (!gtk_show_uri_on_window (NULL, uri, GDK_CURRENT_TIME, &error))
    g_warning ("Failed to launch call: %s", error->message);

}

static void
sms_button_clicked_cb (GtkButton     *btn,
                        VoicemailWindow *self)
{
  g_autoptr(GError) error = NULL;
  g_autofree char *uri = NULL;

  uri = g_strconcat ("sms://",
                     self->number,
                     NULL);

  if (!gtk_show_uri_on_window (NULL, uri, GDK_CURRENT_TIME, &error))
    g_warning ("Failed to launch call: %s", error->message);

}


static void
delete_button_clicked_cb (GtkButton     *btn,
                        VoicemailWindow *self)
{
  if (!self->playbin) {
    g_debug("Playbin is NULL!");
    return;
  }

  if (self->state != GST_STATE_PLAYING) { //We are stopped
      VvmVvmd *vvmd_object = vvm_vvmd_get_default ();
      g_debug("Deleting: %s", self->objectpath);
      vvmplayer_vvmd_delete_vvm (vvmd_object, self->message_proxy, self->objectpath, self);
      gtk_widget_destroy (GTK_WIDGET (self));
      return;
  } else { //We are playing or paused
    GstStateChangeReturn ret;
    ret = gst_element_set_state (self->playbin, GST_STATE_READY);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr ("Unable to set the pipeline to the stopped state.\n");
    } else {
      g_autofree char *duration = NULL;
      g_debug ("Stopping playback\n");
      gst_element_set_state (self->playbin, GST_STATE_READY);
      vvm_voicemail_set_gtkimage_icon (self->delete_button_icon, ICON_DELETE);
      vvm_voicemail_set_gtkimage_icon (self->play_button_icon, ICON_PLAYBACK_START);
      self->state = GST_STATE_PAUSED;
      self->minutes = 0;
      self->seconds = 0;
      duration = g_strdup_printf("%02lu:%02lu/%s", self->minutes, self->seconds, self->duration_formatted);
      vvm_preferences_set_hdyactionrow_text(self->second_row, duration);
    }
  }
}

/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg, VoicemailWindow *self) {
  GError *err;
  gchar *debug_info;

  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
  g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
  g_clear_error (&err);
  g_free (debug_info);

  /* Set the pipeline to READY (which stops playback) */
  gst_element_set_state (self->playbin, GST_STATE_READY);

  vvm_voicemail_set_gtkimage_icon (self->play_button_icon, ICON_PLAYBACK_START);
  self->state = GST_STATE_READY;
  vvm_preferences_set_hdyactionrow_text(self->second_row, "Playback Error!");
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb (GstBus *bus, GstMessage *msg, VoicemailWindow *self) {
  g_autofree char *duration = NULL;
  g_debug ("End-Of-Stream reached.");
  gst_element_set_state (self->playbin, GST_STATE_READY);
  self->state = GST_STATE_READY;

  vvm_voicemail_set_gtkimage_icon (self->play_button_icon, ICON_PLAYBACK_START);


  self->minutes = 0;
  self->seconds = 0;
  duration = g_strdup_printf("%02lu:%02lu/%s", self->minutes, self->seconds, self->duration_formatted);
  vvm_preferences_set_hdyactionrow_text(self->second_row, duration);
  vvm_voicemail_set_gtkimage_icon (self->delete_button_icon, ICON_DELETE);
}

/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
static void state_changed_cb (GstBus *bus, GstMessage *msg, VoicemailWindow *self) {
  GstState old_state, new_state, pending_state;
  gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (self->playbin)) {
    self->state = new_state;
    g_print ("State set to %s\n", gst_element_state_get_name (new_state));
    if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED) {
      /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
      refresh_ui (self);
    }
  }
}

GtkWidget *
voicemail_window_new (GVariant *message_t,
                      GDBusProxy *message_proxy,
                      GstDiscoverer    *discoverer)
{
  VoicemailWindow      *self;

  g_autofree char      *objectpath = NULL;
  g_autofree char      *Date = NULL;
  g_autofree char      *Sender = NULL;
  g_autofree char      *To = NULL;
  g_autofree char      *MessageContext = NULL;
  g_autofree char      *Attachments = NULL;
  g_autofree char     **Attachment_list = NULL;
  g_autoptr (GError) error = NULL;
  int lifetime_status;
  int number_of_attachments;
  GVariant             *properties;
  GVariantDict          dict;
  g_autoptr(GAppInfo) app_info_tel = NULL;
  g_autoptr(GAppInfo) app_info_sms = NULL;

  self = g_object_new (GTK_TYPE_VOICEMAIL_WINDOW, NULL);
  self->minutes = 0;
  self->seconds = 0;
  self->message_proxy = message_proxy;
  self->contact_name = NULL;

  app_info_tel = g_app_info_get_default_for_uri_scheme ("tel");
  if (app_info_tel) {
    gtk_widget_show (GTK_WIDGET (self->call_row));
  }

  app_info_sms = g_app_info_get_default_for_uri_scheme ("sms");
  if (app_info_sms) {
    gtk_widget_show (GTK_WIDGET (self->sms_row));
  }

  if (!discoverer) {
    self->duration_formatted = g_strdup ("??:??");
  }

  g_variant_get (message_t, "(o@a{?*})", &objectpath, &properties);

  self->objectpath = g_strdup(objectpath);
  g_debug ("%s: Making row for voicemail", __func__);
  g_variant_dict_init (&dict, properties);
  if (!g_variant_dict_lookup (&dict, "Date", "s", &Date)) {
    Date = g_strdup("Invalid Date");
  } else {
    GDateTime *time_utc;
    GTimeZone *here = g_time_zone_new_local ();

    time_utc = g_date_time_new_from_iso8601(Date, here);
    self->time_local = g_date_time_to_timezone (time_utc, here);
    g_date_time_unref (time_utc);

    Date = g_date_time_format (self->time_local, "%a, %d %b %Y %I:%M:%S %p");
    g_time_zone_unref (here);
  }

  if (!g_variant_dict_lookup (&dict, "Sender", "s", &Sender))
    Sender = g_strdup("Invalid Sender");
  if (!g_variant_dict_lookup (&dict, "To", "s", &To))
    To = g_strdup("Invalid To");
  if (!g_variant_dict_lookup (&dict, "MessageContext", "s", &MessageContext))
    MessageContext = g_strdup("Invalid Message Context");
  if (!g_variant_dict_lookup (&dict, "Attachments", "s", &Attachments))
    Attachments = NULL;
  if (!g_variant_dict_lookup (&dict, "LifetimeStatus", "i", &lifetime_status))
    lifetime_status = VVM_LIFETIME_STATUS_UNKNOWN;

  self->lifetime_status = lifetime_status;

  if (Attachments)
    Attachment_list = g_strsplit(Attachments, ";", -1);

  number_of_attachments = g_strv_length (Attachment_list);

  for (int attachment_counter = 0; attachment_counter < (number_of_attachments); attachment_counter++) {
    GFile *vvm_file;
    GFileInfo *vvm_file_info;
    g_autofree char *mime_type = NULL;
    g_autofree char *attachment_uri = NULL;
    g_autofree char *attachment_uri_playbin = NULL;
    g_autofree char *duration = NULL;
    GstDiscovererInfo *info;
    GstBus *bus;

    g_debug("On Attachment: %s", Attachment_list[attachment_counter]);
    vvm_file = g_file_new_for_path (Attachment_list[attachment_counter]);
    vvm_file_info = g_file_query_info (vvm_file,
                                       "*",
                                       G_FILE_QUERY_INFO_NONE,
                                       NULL,
                                       &error);

    if (error != NULL) {
      g_warning ("Error getting file info: %s", error->message);
      error = NULL;
      continue;
    }
    mime_type = g_content_type_get_mime_type (g_file_info_get_content_type (vvm_file_info));
    if (mime_type == NULL) {
      g_debug ("Could not get MIME type! Trying Content Type instead");
      if (g_file_info_get_content_type (vvm_file_info) != NULL) {
        mime_type = g_strdup (g_file_info_get_content_type (vvm_file_info));
      } else {
        g_debug ("Could not figure out Content Type! Using a Generic one");
        continue;
      }
    }
    g_debug("MIME Type: %s", mime_type);
    if (strstr (mime_type, "audio") == NULL) {
      g_debug("MIME Type is not audio! skipping....");
      /* Will I get anything else here? */
      continue;
    }
    self->duration = GST_CLOCK_TIME_NONE;



    attachment_uri = g_strdup_printf("file://%s", Attachment_list[attachment_counter]);
    // amr files have issues playing on Mobian, so I need to add `audio-sink='capsfilter caps=audio/x-raw,rate=48000 ! pulsesink'`
    // gst-launch-1.0 playbin uri=file:///path/to/foo audio-sink='capsfilter caps=audio/x-raw,rate=48000 ! pulsesink'
    attachment_uri_playbin = g_strdup_printf("playbin uri=%s audio-sink='capsfilter caps=audio/x-raw,rate=48000 ! pulsesink'", attachment_uri);
    if (discoverer) {
      info = gst_discoverer_discover_uri(discoverer, attachment_uri, &error);
      if (error != NULL) {
        g_warning ("Error discovering file: %s", error->message);
        self->duration_formatted = g_strdup ("??:??");
      } else {
        self->duration = gst_discoverer_info_get_duration (info);
        if (self->duration == 0) {
          /* There is a problem in gst-discoverer:
           * https://gitlab.freedesktop.org/gstreamer/gstreamer/-/issues/718
           * This code works around it.
           */
          int retry_counter = 0;
          g_warning ("Error in gst_discoverer!");
          while (self->duration == 0) {
             g_debug ("Retrying....");
             info = gst_discoverer_discover_uri(discoverer, attachment_uri, &error);
             self->duration = gst_discoverer_info_get_duration (info);
             retry_counter = retry_counter + 1;
             if (retry_counter == 10) {
               break;
             }
          }
         if (self->duration == 0) {
          g_warning ("Could not fix!");
         } else {
           g_debug ("Fixed!");
         }
        }
        if (self->duration == 0) {
          self->duration_formatted = g_strdup ("??:??");
        } else {
          self->minutes = (self->duration / (GST_SECOND * 60)) % 60;
          self->seconds = GST_TIME_AS_SECONDS (self->duration) - self->minutes * 60;
          self->duration_formatted = g_strdup_printf ("%02lu:%02lu", self->minutes, self->seconds);
        }
      }
    }
    self->minutes = 0;
    self->seconds = 0;
    duration = g_strdup_printf("%02lu:%02lu/%s", self->minutes, self->seconds, self->duration_formatted);
    vvm_preferences_set_hdyactionrow_text(self->second_row, duration);
    self->state = GST_STATE_READY;

    self->playbin = gst_parse_launch (attachment_uri_playbin, NULL);

    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    bus = gst_element_get_bus (self->playbin);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, self);
    gst_object_unref (bus);

  }

  self->number = g_strdup(Sender);

  if (self->lifetime_status == VVM_LIFETIME_STATUS_NOT_READ) {
    VvmVvmd *vvmd_object = vvm_vvmd_get_default ();
    vvmplayer_vvmd_increment_unread (vvmd_object);
  }

  voicemail_window_set_title (self);

  hdy_expander_row_set_subtitle(self->action_expander_row, Date);

  return GTK_WIDGET (self);
}

static void
voicemail_window_finalize (GObject *object)
{
  VoicemailWindow *self = (VoicemailWindow *)object;

  g_free (self->duration_formatted);
  g_free (self->objectpath);
  g_free (self->number);
  g_free (self->contact_name);
  g_date_time_unref (self->time_local);
  G_OBJECT_CLASS (voicemail_window_parent_class)->finalize (object);

}


static void
voicemail_window_class_init (VoicemailWindowClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/kop316/vvmplayer/"
                                               "ui/vvmplayer-voicemail-window.ui");

  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, action_expander_row);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, play_button_icon);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, delete_button_icon);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, second_row);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, call_row);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, sms_row);

  gtk_widget_class_bind_template_callback (widget_class, play_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, delete_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, call_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, sms_button_clicked_cb);

  object_class->finalize = voicemail_window_finalize;

}

static void
voicemail_window_init (VoicemailWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
