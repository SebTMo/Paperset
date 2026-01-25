#include <gtk/gtk.h>
#include <adwaita.h>

static GtkWidget *global_flowbox;

static void set_wallpaper(GtkButton *button, gpointer user_data) {
    char *path = (char *)user_data;
    GSettings *settings = g_settings_new("org.gnome.desktop.background");
    char *uri = g_strdup_printf("file://%s", path);
    g_settings_set_string(settings, "picture-uri", uri);
    g_settings_set_string(settings, "picture-uri-dark", uri);
    g_free(uri);
    g_object_unref(settings);
}

static void on_view_toggled(GtkToggleButton *source, gpointer user_data) {
    gboolean is_list = gtk_toggle_button_get_active(source);
    
    // Update icon visually
    gtk_button_set_icon_name(GTK_BUTTON(source), is_list ? "view-list-symbolic" : "view-grid-symbolic");

    if (is_list) {
        gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(global_flowbox), 1);
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(global_flowbox), 1);
    } else {
        gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(global_flowbox), 2);
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(global_flowbox), 12);
    }

    for (GtkWidget *child = gtk_widget_get_first_child(global_flowbox); 
         child != NULL; 
         child = gtk_widget_get_next_sibling(child)) {
        
        GtkWidget *button = gtk_flow_box_child_get_child(GTK_FLOW_BOX_CHILD(child));
        GtkWidget *vbox = gtk_button_get_child(GTK_BUTTON(button));
        GtkWidget *pic = gtk_widget_get_first_child(vbox);

        if (is_list) {
            gtk_widget_set_hexpand(child, TRUE);
            gtk_widget_set_size_request(child, -1, -1); 
            gtk_widget_set_size_request(pic, 850, 450); 
        } else {
            // Locked Grid Size: 192px width
            gtk_widget_set_hexpand(child, FALSE);
            gtk_widget_set_size_request(child, 192, 145); 
            gtk_widget_set_size_request(pic, 192, 101); // 16:9 ratio
        }
    }
    gtk_widget_queue_resize(global_flowbox);
}

static gboolean filter_func(GtkFlowBoxChild *child, gpointer user_data) {
    const char *search_text = (const char *)user_data;
    if (!search_text || strlen(search_text) == 0) return TRUE;
    GtkWidget *button = gtk_flow_box_child_get_child(child);
    GtkWidget *vbox = gtk_button_get_child(GTK_BUTTON(button));
    GtkWidget *label = gtk_widget_get_last_child(vbox);
    return g_str_match_string(search_text, gtk_label_get_text(GTK_LABEL(label)), TRUE);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    gtk_flow_box_set_filter_func(GTK_FLOW_BOX(global_flowbox), filter_func, (gpointer)gtk_editable_get_text(GTK_EDITABLE(entry)), NULL);
}

static void load_wallpapers(GtkWidget *flowbox, const char *path) {
    GFile *dir = g_file_new_for_path(path);
    GFileEnumerator *enumerator = g_file_enumerate_children(dir, "standard::name", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (enumerator) {
        GFileInfo *info;
        while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
            const char *name = g_file_info_get_name(info);
            if (g_str_has_suffix(name, ".jpg") || g_str_has_suffix(name, ".png") || g_str_has_suffix(name, ".jpeg")) {
                char *full_path = g_build_filename(path, name, NULL);
                GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
                
                GtkWidget *pic = gtk_picture_new_for_filename(full_path);
                gtk_picture_set_content_fit(GTK_PICTURE(pic), GTK_CONTENT_FIT_COVER);
                gtk_widget_set_size_request(pic, 180, 101); 

                GtkWidget *label = gtk_label_new(name);
                gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

                GtkWidget *btn = gtk_button_new();
                gtk_button_set_child(GTK_BUTTON(btn), vbox);
                gtk_box_append(GTK_BOX(vbox), pic);
                gtk_box_append(GTK_BOX(vbox), label);
                
                g_signal_connect_data(btn, "clicked", G_CALLBACK(set_wallpaper), g_strdup(full_path), (GClosureNotify)g_free, 0);
                gtk_flow_box_insert(GTK_FLOW_BOX(flowbox), btn, -1);
                
                // Start as fixed-size grid item
                GtkWidget *child = gtk_widget_get_last_child(flowbox);
                gtk_widget_set_hexpand(child, FALSE);
                gtk_widget_set_size_request(child, 192, 140);
            }
            g_object_unref(info);
        }
        g_object_unref(enumerator);
    }
    g_object_unref(dir);
}

static void activate(AdwApplication *app) {
    GtkWidget *window = adw_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "PaperSet");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 690);
    
    GtkWidget *root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), root_box);

    GtkWidget *header = adw_header_bar_new();
    gtk_box_append(GTK_BOX(root_box), header);
    
    GtkWidget *view_toggle = gtk_toggle_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(view_toggle), "view-grid-symbolic");
    adw_header_bar_pack_start(ADW_HEADER_BAR(header), view_toggle);

    const char *scaling_options[] = {"Fill", "Centered", "Scaled", "Stretched", "Zoom", "Spanned", NULL};
    GtkWidget *scaling_dropdown = gtk_drop_down_new_from_strings(scaling_options);
    
    GtkWidget *refresh_btn = gtk_button_new_from_icon_name("view-refresh-symbolic");
    adw_header_bar_pack_start(ADW_HEADER_BAR(header), refresh_btn);

    GtkWidget *menu_btn = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menu_btn), "emblem-system-symbolic");
    GtkWidget *popover = gtk_popover_new();
    GtkWidget *pop_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_widget_set_hexpand(search_entry, TRUE);
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(header), search_entry);

    // Scrolled Window setup (Always show scrollbar when needed)
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_append(GTK_BOX(root_box), scrolled);
  
    gtk_box_append(GTK_BOX(pop_box), gtk_label_new("Wallpaper Scaling:"));
    gtk_box_append(GTK_BOX(pop_box), scaling_dropdown);
    gtk_popover_set_child(GTK_POPOVER(popover), pop_box);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_btn), popover);
    adw_header_bar_pack_end(ADW_HEADER_BAR(header), menu_btn);

    global_flowbox = gtk_flow_box_new();
    gtk_widget_set_valign(global_flowbox, GTK_ALIGN_START);
    
    // Start in Grid Mode (Multi-column)
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(global_flowbox), 2);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(global_flowbox), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(global_flowbox), 12);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(global_flowbox), 18);
    
    gtk_widget_set_margin_start(global_flowbox, 20);
    gtk_widget_set_margin_end(global_flowbox, 20);
    gtk_widget_set_margin_top(global_flowbox, 20);
    gtk_widget_set_margin_bottom(global_flowbox, 20);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), global_flowbox);

    g_signal_connect(view_toggle, "toggled", G_CALLBACK(on_view_toggled), NULL);
    g_signal_connect(search_entry, "search-changed", G_CALLBACK(on_search_changed), NULL);
    
    load_wallpapers(global_flowbox, "/home/seb/Bilder/Wallpaper");
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    AdwApplication *app = adw_application_new("com.sebtech.paperset", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
