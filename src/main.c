#include <gtk/gtk.h>
#include <adwaita.h>

static GtkWidget *global_flowbox;
static GtkWidget *global_scrolled;
static GtkWidget *global_status_page;
static char *global_wallpaper_path = NULL;

static void set_wallpaper(GtkButton *G_GNUC_UNUSED button, gpointer user_data) {
    char *path = (char *)user_data;
    GSettings *settings = g_settings_new("org.gnome.desktop.background");
    char *uri = g_strdup_printf("file://%s", path);
    g_settings_set_string(settings, "picture-uri", uri);
    g_settings_set_string(settings, "picture-uri-dark", uri);
    g_free(uri);
    g_object_unref(settings);
}

static void update_empty_state(void) {
    gboolean has_content = gtk_widget_get_first_child(global_flowbox) != NULL;
    gtk_widget_set_visible(global_scrolled, has_content);
    gtk_widget_set_visible(global_status_page, !has_content);
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
                
                GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
                gtk_widget_set_size_request(vbox, 192, 140);

                GtkWidget *pic = gtk_picture_new_for_filename(full_path);
                gtk_picture_set_content_fit(GTK_PICTURE(pic), GTK_CONTENT_FIT_COVER);
                gtk_picture_set_can_shrink(GTK_PICTURE(pic), TRUE);
                gtk_widget_set_size_request(pic, 192, 108);
                gtk_widget_set_overflow(pic, GTK_OVERFLOW_HIDDEN);

                GtkWidget *label = gtk_label_new(name);
                gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
                gtk_widget_set_margin_top(label, 4);

                GtkWidget *btn = gtk_button_new();
                gtk_button_set_child(GTK_BUTTON(btn), vbox);
                gtk_box_append(GTK_BOX(vbox), pic);
                gtk_box_append(GTK_BOX(vbox), label);
                
                // Fixed the cast warning here:
                g_signal_connect_data(btn, "clicked", G_CALLBACK(set_wallpaper), g_strdup(full_path), (GClosureNotify)(void(*)(void))g_free, 0);
                gtk_flow_box_insert(GTK_FLOW_BOX(flowbox), btn, -1);
                
                GtkWidget *child = gtk_widget_get_last_child(flowbox);
                gtk_widget_set_valign(child, GTK_ALIGN_START);
                gtk_widget_set_size_request(child, 192, 140);
            }
            g_object_unref(info);
        }
        g_object_unref(enumerator);
        g_object_unref(dir);
    }
    update_empty_state();
}

static void on_refresh_clicked(GtkButton *G_GNUC_UNUSED btn, gpointer G_GNUC_UNUSED user_data) {
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(global_flowbox)) != NULL) {
        gtk_flow_box_remove(GTK_FLOW_BOX(global_flowbox), child);
    }
    load_wallpapers(global_flowbox, global_wallpaper_path);
}

static void on_scaling_changed(GtkDropDown *dropdown, gpointer G_GNUC_UNUSED user_data) {
    const char *options[] = {"none", "centered", "scaled", "stretched", "zoom", "spanned"};
    guint selected = gtk_drop_down_get_selected(dropdown);
    GSettings *settings = g_settings_new("org.gnome.desktop.background");
    g_settings_set_string(settings, "picture-options", options[selected]);
    g_object_unref(settings);
}

static void on_dir_picked(GObject *source, GAsyncResult *res, gpointer G_GNUC_UNUSED user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GFile *file = gtk_file_dialog_select_folder_finish(dialog, res, NULL);
    if (file) {
        g_free(global_wallpaper_path);
        global_wallpaper_path = g_file_get_path(file);
        on_refresh_clicked(NULL, NULL);
        g_object_unref(file);
    }
}

static void on_open_folder_clicked(GtkButton *G_GNUC_UNUSED btn, gpointer user_data) {
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Wallpaper Folder");
    gtk_file_dialog_select_folder(dialog, GTK_WINDOW(user_data), NULL, on_dir_picked, NULL);
}

static void on_view_toggled(GtkToggleButton *source, gpointer G_GNUC_UNUSED user_data) {
    gboolean is_list = gtk_toggle_button_get_active(source);
    gtk_button_set_icon_name(GTK_BUTTON(source), is_list ? "view-list-symbolic" : "view-grid-symbolic");

    if (is_list) {
        gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(global_flowbox), 1);
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(global_flowbox), 1);
    } else {
        gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(global_flowbox), 2);
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(global_flowbox), 2);
    }

    for (GtkWidget *child = gtk_widget_get_first_child(global_flowbox); 
         child != NULL; 
         child = gtk_widget_get_next_sibling(child)) {
        
        GtkWidget *button = gtk_flow_box_child_get_child(GTK_FLOW_BOX_CHILD(child));
        GtkWidget *vbox = gtk_button_get_child(GTK_BUTTON(button));
        GtkWidget *pic = gtk_widget_get_first_child(vbox);

        if (is_list) {
            gtk_widget_set_size_request(child, -1, -1); 
            gtk_widget_set_size_request(pic, 384, 216); 
            gtk_widget_set_valign(child, GTK_ALIGN_FILL);
        } else {
            gtk_widget_set_size_request(child, 192, 140); 
            gtk_widget_set_size_request(pic, 192, 108); 
            gtk_widget_set_valign(child, GTK_ALIGN_START);
        }
    }
}

static gboolean filter_func(GtkFlowBoxChild *child, gpointer user_data) {
    const char *search_text = (const char *)user_data;
    if (!search_text || strlen(search_text) == 0) return TRUE;
    GtkWidget *button = gtk_flow_box_child_get_child(child);
    GtkWidget *vbox = gtk_button_get_child(GTK_BUTTON(button));
    GtkWidget *label = gtk_widget_get_last_child(vbox);
    return g_str_match_string(search_text, gtk_label_get_text(GTK_LABEL(label)), TRUE);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer G_GNUC_UNUSED user_data) {
    gtk_flow_box_set_filter_func(GTK_FLOW_BOX(global_flowbox), filter_func, (gpointer)gtk_editable_get_text(GTK_EDITABLE(entry)), NULL);
}

static void activate(AdwApplication *app) {
    const char *pictures_dir = g_get_user_special_dir(G_USER_DIRECTORY_PICTURES);
    if (!pictures_dir) pictures_dir = g_get_home_dir();
    global_wallpaper_path = g_build_filename(pictures_dir, "Wallpaper", NULL);

    GtkWidget *window = adw_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "PaperSet");
    gtk_window_set_default_size(GTK_WINDOW(window), 460, 690);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE); 

    GtkWidget *root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), root_box);

    GtkWidget *header = adw_header_bar_new();
    gtk_box_append(GTK_BOX(root_box), header);
    
    GtkWidget *view_toggle = gtk_toggle_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(view_toggle), "view-grid-symbolic");
    adw_header_bar_pack_start(ADW_HEADER_BAR(header), view_toggle);

    GtkWidget *refresh_btn = gtk_button_new_from_icon_name("view-refresh-symbolic");
    adw_header_bar_pack_start(ADW_HEADER_BAR(header), refresh_btn);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(on_refresh_clicked), NULL);

    GtkWidget *folder_btn = gtk_button_new_from_icon_name("folder-open-symbolic");
    adw_header_bar_pack_start(ADW_HEADER_BAR(header), folder_btn);
    g_signal_connect(folder_btn, "clicked", G_CALLBACK(on_open_folder_clicked), window);

    GtkWidget *menu_btn = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menu_btn), "emblem-system-symbolic");
    GtkWidget *popover = gtk_popover_new();
    GtkWidget *pop_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(pop_box, 10);
    gtk_widget_set_margin_end(pop_box, 10);
    gtk_widget_set_margin_top(pop_box, 10);
    gtk_widget_set_margin_bottom(pop_box, 10);

    const char *scaling_options[] = {"None", "Centered", "Scaled", "Stretched", "Zoom", "Spanned", NULL};
    GtkWidget *scaling_dropdown = gtk_drop_down_new_from_strings(scaling_options);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(scaling_dropdown), 4);
    g_signal_connect(scaling_dropdown, "notify::selected", G_CALLBACK(on_scaling_changed), NULL);

    gtk_box_append(GTK_BOX(pop_box), gtk_label_new("Wallpaper Scaling:"));
    gtk_box_append(GTK_BOX(pop_box), scaling_dropdown);
    gtk_popover_set_child(GTK_POPOVER(popover), pop_box);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_btn), popover);
    adw_header_bar_pack_end(ADW_HEADER_BAR(header), menu_btn);

    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_widget_set_hexpand(search_entry, TRUE);
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(header), search_entry);

    global_status_page = adw_status_page_new();
    adw_status_page_set_icon_name(ADW_STATUS_PAGE(global_status_page), "folder-pictures-symbolic");
    adw_status_page_set_title(ADW_STATUS_PAGE(global_status_page), "No Wallpapers Found");
    adw_status_page_set_description(ADW_STATUS_PAGE(global_status_page), "Press the folder icon to set the wallpaper directory");
    gtk_box_append(GTK_BOX(root_box), global_status_page);

    global_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(global_scrolled, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(global_scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_append(GTK_BOX(root_box), global_scrolled);

    global_flowbox = gtk_flow_box_new();
    gtk_widget_set_valign(global_flowbox, GTK_ALIGN_START);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(global_flowbox), 2);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(global_flowbox), 2); 
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(global_flowbox), 12);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(global_flowbox), 18);
    gtk_widget_set_margin_start(global_flowbox, 20);
    gtk_widget_set_margin_end(global_flowbox, 20);
    gtk_widget_set_margin_top(global_flowbox, 20);
    gtk_widget_set_margin_bottom(global_flowbox, 20);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(global_scrolled), global_flowbox);

    g_signal_connect(view_toggle, "toggled", G_CALLBACK(on_view_toggled), NULL);
    g_signal_connect(search_entry, "search-changed", G_CALLBACK(on_search_changed), NULL);
    
    load_wallpapers(global_flowbox, global_wallpaper_path);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    AdwApplication *app = adw_application_new("com.sebtech.paperset", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_free(global_wallpaper_path);
    return status;
}
