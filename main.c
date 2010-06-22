#include <stdlib.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>

static void onDestroy(GtkWidget* widget, gpointer data);

int main(int argc, char* argv[]) {
   char* uri = NULL;
   if(argc < 2) {
      printf("ERROR: You must give a uri\n");
      exit(EXIT_FAILURE);
   }

   uri = argv[argc-1];

   gtk_init(&argc, &argv);

   if(!g_thread_supported()) {
      g_thread_init(NULL);
   }

   GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
   g_signal_connect(window, "destroy", G_CALLBACK(onDestroy), NULL);
   GdkScreen *screen = gtk_widget_get_screen(window);
   GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
   if (colormap && gdk_screen_is_composited(screen)) {
      gtk_widget_set_default_colormap(colormap);
      gtk_widget_set_colormap(GTK_WIDGET(window), colormap);
   }
   gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

   WebKitWebView* web_view = web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
   webkit_web_view_set_transparent(web_view, TRUE);
   gtk_container_add (GTK_CONTAINER(window), GTK_WIDGET(web_view));

   webkit_web_view_load_uri(web_view, uri);

   gtk_widget_grab_focus(GTK_WIDGET(web_view));
   gtk_widget_show_all(window);
   gtk_main();

   exit(EXIT_SUCCESS);
}

static void onDestroy(GtkWidget* widget, gpointer data) {
   gtk_main_quit();
}

