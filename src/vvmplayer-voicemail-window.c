/* vvmplayer-voicemail-window.c
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

#include "vvmplayer-voicemail-window.h"
#include <gst/pbutils/pbutils.h>
#include <time.h>

#define _XOPEN_SOURCE 700

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
  gint64 duration;
  gint64 minutes;
  gint64 seconds;
  char *duration_formatted;

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
  g_autofree char *duration;
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

  vvm_voicemail_set_gtkimage_icon (self->delete_button_icon, ICON_PLAYBACK_STOP);

  if (self->state != GST_STATE_PLAYING) { //We are not playing anything
    GstStateChangeReturn ret;
    ret = gst_element_set_state (self->playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr ("Unable to set the pipeline to the playing state.\n");
    } else {
      vvm_voicemail_set_gtkimage_icon (self->play_button_icon, ICON_PLAYBACK_PAUSE);
      /* Register a function that GLib will call every second */
      g_timeout_add_seconds (1, (GSourceFunc)refresh_ui, self);
      self->state = GST_STATE_PLAYING;
    }
  } else { //We are playing
    GstStateChangeReturn ret;
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
delete_button_clicked_cb (GtkButton     *btn,
                        VoicemailWindow *self)
{
  g_debug("Delete button clicked!");
  if (!self->playbin) {
    g_debug("Playbin is NULL!");
    return;
  }

  if (self->state == GST_STATE_READY) { //We are stopped
    g_debug("Need to delete!");
  } else { //We are playing or paused
    GstStateChangeReturn ret;
    ret = gst_element_set_state (self->playbin, GST_STATE_READY);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr ("Unable to set the pipeline to the stopped state.\n");
    } else {
      g_autofree char *duration = NULL;
      g_printerr ("Stopping playback\n");
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
  g_autofree char *duration;
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
voicemail_window_new (GVariant *message_t)
{
  VoicemailWindow      *self;

  g_autofree char      *objectpath = NULL;
  g_autofree char      *Date = NULL;
  g_autofree char      *Sender = NULL;
  g_autofree char      *To = NULL;
  g_autofree char      *MessageContext = NULL;
  g_autofree char      *Attachments = NULL;
  g_autofree char     **Attachment_list = NULL;

  GVariant             *properties;
  GVariantDict          dict;

  GstBus *bus;

  int number_of_attachments;

  self = g_object_new (GTK_TYPE_VOICEMAIL_WINDOW, NULL);
  self->minutes = 0;
  self->seconds = 0;

  g_variant_get (message_t, "(o@a{?*})", &objectpath, &properties);

  g_debug ("%s: New Voicemail at %s", __func__, objectpath);
  g_debug ("%s", g_variant_print (properties, FALSE));
  g_variant_dict_init (&dict, properties);
  if (!g_variant_dict_lookup (&dict, "Date", "s", &Date))
    Date = NULL;
  if (!g_variant_dict_lookup (&dict, "Sender", "s", &Sender))
    Sender = g_strdup("Invalid Sender");
  if (!g_variant_dict_lookup (&dict, "To", "s", &To))
    To = g_strdup("Invalid To");
  if (!g_variant_dict_lookup (&dict, "MessageContext", "s", &MessageContext))
    MessageContext = g_strdup("Invalid Message Context");
  if (!g_variant_dict_lookup (&dict, "Attachments", "s", &Attachments))
    Attachments = NULL;

  if (Attachments)
    Attachment_list = g_strsplit(Attachments, ";", -1);

  number_of_attachments = g_strv_length (Attachment_list);

  for (int i = 0; i < (number_of_attachments); i++) {
    GFile *vvm_file;
    GFileInfo *vvm_file_info;
    g_autoptr (GError) error = NULL;
    g_autofree char *mime_type = NULL;
    g_autofree char *attachment_uri = NULL;
    g_autofree char *attachment_uri_playbin = NULL;
    g_autofree char *duration;
    GstDiscoverer *discoverer;
    GstDiscovererInfo *info;

    g_debug("On Attachment: %s", Attachment_list[i]);
    vvm_file = g_file_new_for_path (Attachment_list[i]);
    vvm_file_info = g_file_query_info (vvm_file,
                                       "*",
                                       G_FILE_QUERY_INFO_NONE,
                                       NULL,
                                       &error);

    if (error != NULL) {
      g_warning ("Error getting file info: %s", error->message);
      return NULL;
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
    }
    self->duration = GST_CLOCK_TIME_NONE;



    attachment_uri = g_strdup_printf("file://%s", Attachment_list[i]);
    g_debug("file URI: %s", attachment_uri);
    // amr files have issues playing on Mobian, so I need to add `audio-sink='capsfilter caps=audio/x-raw,rate=48000 ! pulsesink'`
    // gst-launch-1.0 playbin uri=file:///path/to/foo audio-sink='capsfilter caps=audio/x-raw,rate=48000 ! pulsesink'
    attachment_uri_playbin = g_strdup_printf("playbin uri=%s audio-sink='capsfilter caps=audio/x-raw,rate=48000 ! pulsesink'", attachment_uri);

    discoverer = gst_discoverer_new (GST_SECOND, &error);
    if (!discoverer) {
      g_print ("Error creating discoverer instance: %s\n", error->message);
      error = NULL;
      return NULL;
    }

    info = gst_discoverer_discover_uri(discoverer, attachment_uri, &error);
    if (error != NULL) {
      g_warning ("Error discovering file: %s", error->message);
      self->duration_formatted = g_strdup ("??:??");
    } else {
      self->duration = gst_discoverer_info_get_duration (info);
      self->minutes = (self->duration / (GST_SECOND * 60)) % 60;
      self->seconds = GST_TIME_AS_SECONDS (self->duration) - self->minutes * 60;
      self->duration_formatted = g_strdup_printf ("%02lu:%02lu", self->minutes, self->seconds);
      g_print ("\nDuration: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_discoverer_info_get_duration (info)));
    }

    self->minutes = 0;
    self->seconds = 0;
    duration = g_strdup_printf("%02lu:%02lu/%s", self->minutes, self->seconds, self->duration_formatted);
    vvm_preferences_set_hdyactionrow_text(self->second_row, duration);

    g_object_unref (discoverer);

    self->playbin = gst_parse_launch (attachment_uri_playbin, NULL);

    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    bus = gst_element_get_bus (self->playbin);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, self);
    gst_object_unref (bus);

  }

  //TODO: If the Sender is a contact, substitute that
  vvm_preferences_set_hdyexpanderrow_text(self->action_expander_row, Sender);

  g_debug("Old Date: %s", Date);

  if (Date) {
    //TODO: Standardize date
    //TODO: THis may be easier in vvmd: https://curl.se/libcurl/c/curl_getdate.html
    // https://developer.gnome.org/glib/stable/glib-GDateTime.html#g-date-time-new-from-unix-utc
/*
    struct tm tm;
    struct tm tm_new;
    time_t time_to_display;
    static char buf[128];
    //Tue, 06 Jul 2021 13:25:39 +0000
    strptime (Date, "%a, %d %b %Y %H:%M:%S %z", &tm);
    time_to_display = mktime(&tm);
    strftime (buf, 127, "%a, %d %b %Y %I:%M:%S %p", localtime_r (&time_to_display, &tm_new));
    buf[127] = '\0';
    g_free(Date);
    Date = g_strdup(buf);
*/
  } else {
    Date = g_strdup("Invalid Date");
  }
  g_debug("New Date: %s", Date);
  hdy_expander_row_set_subtitle(self->action_expander_row, Date);

  return GTK_WIDGET (self);
}

static void
voicemail_window_finalize (GObject *object)
{
  VoicemailWindow *self = (VoicemailWindow *)object;

  g_free (self->duration_formatted);
  G_OBJECT_CLASS (voicemail_window_parent_class)->finalize (object);

}


static void
voicemail_window_class_init (VoicemailWindowClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/kop316/vvmplayer/vvmplayer-voicemail-window.ui");

  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, action_expander_row);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, play_button_icon);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, delete_button_icon);
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, second_row);

  gtk_widget_class_bind_template_callback (widget_class, play_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, delete_button_clicked_cb);

  object_class->finalize = voicemail_window_finalize;

}

static void
voicemail_window_init (VoicemailWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
