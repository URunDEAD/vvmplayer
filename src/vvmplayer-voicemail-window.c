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

struct _VoicemailWindow
{
  HdyActionRow parent_instance;

  //GStreamer elements
  GstElement *playbin;           /* Our one and only pipeline */

  GstState state; //state of the pipeline

  GtkImage *play_button_icon;
  //Slider
  GtkWidget *play_scale;
  gulong slider_update_signal_id;

  gint64 duration;                /* Duration of the clip, in nanoseconds */

  HdyExpanderRow *action_expander_row;

};

G_DEFINE_TYPE (VoicemailWindow, voicemail_window, HDY_TYPE_ACTION_ROW)

static gboolean refresh_ui (VoicemailWindow *self) {
  gint64 current = -1;

  if (self->state == GST_STATE_READY || self->state == GST_STATE_PAUSED) {
    /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
    return FALSE;
  }

  g_debug("Refreshing UI!");

  /* If we didn't know it yet, query the stream duration */
  if (!GST_CLOCK_TIME_IS_VALID (self->duration)) {
    if (!gst_element_query_duration (self->playbin, GST_FORMAT_TIME, &self->duration)) {
      g_printerr ("Could not query current duration.\n");
    } else {
      /* Set the range of the slider to the clip duration, in SECONDS */
      gtk_range_set_range (GTK_RANGE (self->play_scale), 0, (gdouble)self->duration / GST_SECOND);
    }
  }

  if (gst_element_query_position (self->playbin, GST_FORMAT_TIME, &current)) {
    /* Block the "value-changed" signal, so the slider_cb function is not called
     * (which would trigger a seek the user has not requested) */
    g_signal_handler_block (self->play_scale, self->slider_update_signal_id);
    /* Set the position of the slider to the current pipeline positoin, in SECONDS */
    gtk_range_set_value (GTK_RANGE (self->play_scale), (gdouble)current / GST_SECOND);
    /* Re-enable the signal */
    g_signal_handler_unblock (self->play_scale, self->slider_update_signal_id);
  }
  return TRUE;

}

static void
vvm_preferences_set_hdyactionrow_text (HdyExpanderRow *hdy_row, const char *text) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, text);
  g_object_set_property (G_OBJECT (hdy_row), "title", &a);
}

static void
vvm_voicemail_set_gtkimage_icon (GtkImage *image_to_set, const char *icon) {
  GValue a = G_VALUE_INIT;

  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));
  g_value_set_static_string (&a, icon);
  g_object_set_property (G_OBJECT (image_to_set), "icon-name", &a);
}

/* This function is called when the slider changes its position. We perform a seek to the
 * new position here. */
static void slider_cb (GtkRange *range, VoicemailWindow *self) {
  gdouble value = gtk_range_get_value (GTK_RANGE (self->play_scale));
  g_debug("Slider is at %f", value);
  gst_element_seek_simple (self->playbin, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
      (gint64)(value * GST_SECOND));
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

  if (self->state != GST_STATE_PLAYING) { //We are not playing anything
    GstStateChangeReturn ret;
    ret = gst_element_set_state (self->playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr ("Unable to set the pipeline to the playing state.\n");
    } else {
      vvm_voicemail_set_gtkimage_icon (self->play_button_icon, "media-playback-pause");
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
      vvm_voicemail_set_gtkimage_icon (self->play_button_icon, "media-playback-start");
      self->state = GST_STATE_PAUSED;
    }
  }
}

/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg, VoicemailWindow *self) {
  g_print ("Errorreached.\n");
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb (GstBus *bus, GstMessage *msg, VoicemailWindow *self) {
  g_print ("End-Of-Stream reached.\n");
  gst_element_set_state (self->playbin, GST_STATE_READY);
  vvm_voicemail_set_gtkimage_icon (self->play_button_icon, "media-playback-start");
  self->state = GST_STATE_READY;
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

/* This function is called when an "application" message is posted on the bus.
 * Here we retrieve the message posted by the tags_cb callback */
static void application_cb (GstBus *bus, GstMessage *msg, VoicemailWindow *self) {
  g_debug("Application Callback!");
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

  g_variant_get (message_t, "(o@a{?*})", &objectpath, &properties);

  g_debug ("%s: New Voicemail at %s", __func__, objectpath);
  g_debug ("%s", g_variant_print (properties, FALSE));
  g_variant_dict_init (&dict, properties);
  if (!g_variant_dict_lookup (&dict, "Date", "s", &Date))
    Date = g_strdup("Invalid Date");
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
    /* Set the URI to play */
    //TODO FIXME: amr is not working with gstreamer, need to fix
//    attachment_uri = g_strdup_printf("playbin file://%s", Attachment_list[i]);
    attachment_uri = g_strdup("file:///home/mobian/test.mp3");
    g_debug("file URI: %s", attachment_uri);
    attachment_uri_playbin = g_strdup_printf("playbin uri=%s", attachment_uri);
    g_debug("file URI: %s", attachment_uri_playbin);

  discoverer = gst_discoverer_new (5 * GST_SECOND, &error);
  if (!discoverer) {
    g_print ("Error creating discoverer instance: %s\n", error->message);
    error = NULL;
    return NULL;
  }

    gst_discoverer_start (discoverer);
    //TODO FIXME this doesn't work...why?
    info = gst_discoverer_discover_uri(discoverer, attachment_uri, &error);
    if (error != NULL) {
      g_warning ("Error discovering file: %s", error->message);
    } else {
      g_print ("\nDuration: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_discoverer_info_get_duration (info)));
    }

    /* Stop the discoverer process */
    gst_discoverer_stop (discoverer);

    /* Free resources */
    g_object_unref (discoverer);

    /* Build the pipeline */
    self->playbin = gst_parse_launch (attachment_uri_playbin, NULL);

    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    bus = gst_element_get_bus (self->playbin);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, self);
    g_signal_connect (G_OBJECT (bus), "message::application", (GCallback)application_cb, self);
    gst_object_unref (bus);

  }

  //TODO: If the Sender is a contact, substitute that
  vvm_preferences_set_hdyactionrow_text(self->action_expander_row, Sender);
  hdy_expander_row_set_subtitle(self->action_expander_row, Date);

  self->play_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_draw_value (GTK_SCALE (self->play_scale), 0);
  self->slider_update_signal_id = g_signal_connect (G_OBJECT (self->play_scale), "value-changed", G_CALLBACK (slider_cb), self);
  gtk_scale_add_mark (GTK_SCALE (self->play_scale), 0, GTK_POS_TOP, NULL);

  //TODO FIXME This doesn't work the first couple of times....why?
  if (!GST_CLOCK_TIME_IS_VALID (self->duration)) {
      if (!gst_element_query_duration (self->playbin, GST_FORMAT_TIME, &self->duration)) {
        g_printerr ("Could not query current duration.\n");
    } else {
      /* Set the range of the slider to the clip duration, in SECONDS */
      gtk_range_set_range (GTK_RANGE (self->play_scale), 0, (gdouble)self->duration / GST_SECOND);
    }
  }
  g_debug("Duration is: %ld", self->duration);

  return GTK_WIDGET (self);
}

static void
voicemail_window_finalize (GObject *object)
{
//  VoicemailWindow *self = (VoicemailWindow *)object;

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
  gtk_widget_class_bind_template_child (widget_class, VoicemailWindow, play_scale);

  gtk_widget_class_bind_template_callback (widget_class, play_button_clicked_cb);

  object_class->finalize = voicemail_window_finalize;

}

static void
voicemail_window_init (VoicemailWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
