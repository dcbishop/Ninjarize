#include <stdlib.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>

#define PROGRAM_NAME "Ninjarize"
#define PROGRAM_VERSION_STRING "v0.2"

static void onDestroy(GtkWidget* widget, gpointer data);

typedef struct options {
      int width;
      int height;
      int x;
      int y;
      char borders;
      char fullscreen;
      char maximize;
      char sticky;
      char desktop;
      char center;
      char click_through;
      char* uri;
} options;

void print_help();

int main(int argc, char* argv[]) {

   /* Set the default options */
   options opts;
   opts.width = 800;
   opts.height = 600;
   opts.x = 0;
   opts.y = 0;
   opts.borders = FALSE;
   opts.fullscreen = FALSE;
   opts.maximize = TRUE;
   opts.sticky = FALSE;
   opts.desktop = FALSE;
   opts.center = FALSE;
   opts.click_through = FALSE;
   opts.uri = "file://html/index.xhtml";

   int c;

   /* Assume the last arg is the uri to open if it doesn't start with -. */
   if(argv[argc-1][0] != '-' && argc > 1) {
      opts.uri = argv[argc-1];
   }

   /* If no args are specified then enable 'demo' mode */
   if(argc < 2) {
      opts.desktop = TRUE;
      opts.sticky = TRUE;
      opts.click_through = TRUE;
   }

   gtk_init(&argc, &argv);

   while(1) {
      /* The possible command line args... */
      static struct option long_options[] = {
         {"width",      required_argument,   0, 'w'},
         {"top",        required_argument,   0, 't'},
         {"borders",    no_argument,         0, 'b'},
         {"sticky",     no_argument,         0, 's'},
         {"desktop",    no_argument,         0, 'd'},
         {"center",     no_argument,         0, 'c'},
         {"version",    no_argument,         0, 'v'},
         {"noclick",    no_argument,         0, 'n'},
         {"fullscreen", no_argument,         0, 'f'},
         {"help",       no_argument,         0, '?'},
         {0, 0, 0, 0}
      };

      int option_index = 0;
      c = getopt_long(argc, argv, "w:h:t:l:x:y:vbsdncf", long_options, &option_index);

      /* Process command line args... */
      switch(c) {
         case 'w':
            opts.width = atoi(optarg);
            opts.fullscreen = FALSE;
            opts.maximize = FALSE;
            break;
         case 'h':
            opts.height = atoi(optarg);
            opts.fullscreen = FALSE;
            opts.maximize = FALSE;
            break;
         case 'b':
            opts.borders = TRUE;
            opts.fullscreen = FALSE;
            opts.maximize = FALSE;
            break;
         case 'y':
            opts.y = atoi(optarg);
            opts.fullscreen = FALSE;
            opts.maximize = FALSE;
            break;
         case 'x':
            opts.x = atoi(optarg);
            opts.fullscreen = FALSE;
            opts.maximize = FALSE;
            break;
         case 's':
            opts.sticky = TRUE;
            break;
         case 'd':
            opts.desktop = TRUE;
            break;
         case 'c':
            opts.center = TRUE;
            opts.fullscreen = FALSE;
            opts.maximize = FALSE;
            break;
         case 'n':
            opts.click_through = TRUE;
            break;
         case 'v':
            printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION_STRING);
            exit(EXIT_SUCCESS);
            break;
         case 'f':
            opts.fullscreen = TRUE;
            opts.maximize = FALSE;
            break;
         case '?':
            print_help(argv[0]);
            exit(EXIT_SUCCESS);
            break;
      }

      if(c < 0) {
         break;
      }
   }

   if(!g_thread_supported()) {
      g_thread_init(NULL);
   }

   /* Setup a GTK window */
   GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   g_signal_connect(window, "destroy", G_CALLBACK(onDestroy), NULL);
   gtk_window_set_decorated(GTK_WINDOW(window), opts.borders);

   /* Fullscreen window */
   if(opts.fullscreen) {
      GdkScreen* screen = NULL;
      screen = gtk_window_get_screen(GTK_WINDOW(window));
      opts.width = gdk_screen_get_width(screen);
      opts.height = gdk_screen_get_height(screen);
      gtk_window_resize(GTK_WINDOW(window), opts.width, opts.height);
      gtk_window_fullscreen(GTK_WINDOW(window));
   } else if(opts.maximize) { /* Maximize window */
      gtk_window_maximize(GTK_WINDOW(window));
   } else {
      gtk_window_set_default_size(GTK_WINDOW(window), opts.width, opts.height);
      gtk_window_resize(GTK_WINDOW(window), opts.width, opts.height);
   }
   gtk_window_get_size(GTK_WINDOW(window), &opts.width, &opts.height);

   /* If it's pinned to the desktop */
   if(opts.desktop) {
      gtk_window_set_keep_below(GTK_WINDOW(window), TRUE);
      
      /* Stop showing up in taskbar and alt+tab */
      gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);

      /* Stop showing up in things like the Compiz Expo plugin */
      gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);

      /* If set, in compiz when the cube has tranparency ninjarize will fade 
       * with the background, but when the desktop is clicked on it will draw
       * over ninjarize. */
      //gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DESKTOP);
   }

   /* Make window sticky so it appears on all virtual desktops */
   if(opts.sticky) {
      gtk_window_stick(GTK_WINDOW(window));
   }

   /* Make the window RGBA to allow for transparancy. */
   GdkScreen *screen = gtk_widget_get_screen(window);
   GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
   if (colormap && gdk_screen_is_composited(screen)) {
      gtk_widget_set_default_colormap(colormap);
      gtk_widget_set_colormap(GTK_WIDGET(window), colormap);
   } else {
      fprintf(stderr, "ERROR: No compositing. Transparency won't work.\n");
   }

   /* Make the webkit widget */
   WebKitWebView* web_view = web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
   webkit_web_view_set_transparent(web_view, TRUE);
   gtk_container_add (GTK_CONTAINER(window), GTK_WIDGET(web_view));
   webkit_web_view_load_uri(web_view, opts.uri); /* Load the URI */
   gtk_widget_show_all(window);

   /* Center or position the window */
   if(opts.center) {
      gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
   } else {
      gtk_window_move(GTK_WINDOW(window), opts.x, opts.y);
   }

   /* Make it so the window can be clicked through. */
   static GdkBitmap* shape_pixmap = NULL;
   if(opts.click_through) {
      shape_pixmap = (GdkBitmap*)gdk_pixmap_new(NULL, 1, 1, 1);
      gtk_widget_input_shape_combine_mask (window, NULL, 0, 0);
      gtk_widget_input_shape_combine_mask (window, shape_pixmap, 0, 0);
      gtk_window_set_focus(GTK_WINDOW(window), FALSE);
      gtk_widget_set_can_focus(GTK_WIDGET(web_view), FALSE);
   } else {
      gtk_widget_grab_focus(GTK_WIDGET(web_view));
   }

   gtk_main();

   exit(EXIT_SUCCESS);
}

static void onDestroy(GtkWidget* widget, gpointer data) {
   gtk_main_quit();
}

void print_help(char* execname) {
   printf("Usage: %s [OPTION...] [URI]\n", execname);
   printf("Renders a URI using webkit on the desktop (allowing transparency)\n");
   printf("\n");
   printf("  -w,--width=WIDTH                Set the window width.\n");
   printf("  -h,--height=HEIGHT              Set the window height.\n");
   printf("  -x                              Set the window's X coordinate.\n");
   printf("  -y                              Set the window's Y coordinate.\n");
   printf("  -b, --borders                   Enable window decoration.\n");
   printf("  -s, --sticky                    Make window appear on all desktops.\n");
   printf("  -d, --desktop                   Pin window to the desktop.\n");
   printf("  -c, --center                    Center the window (overrides -x and -y).\n");
   printf("  -n, --noinput                   Makes window clickthrowable.\n");
   printf("  -f, --fullscreen                Fullscreens the window.");
   printf("  -v, --version                   Print program version.\n");
   printf("\n");
   printf("Notes:\n");
   printf("If you don't specify size, coordinates of center the default is to maximize the window.\n");
   printf("'Maximize' mode is differnt from 'fullscreen', maximize doesn't draw in the taskbar area like fullscreen does so if content is hidden under the top taskbar turn off fullscreen. (there is no arg to force maximize, just don't override it.)\n");
   printf("If you don't give any args then Ninjarize will enter a demo mode and --desktop, --noinput and --sticky will be turned on and a default uri 'file://html/index.xhtml' will be loaded.\n");
}
