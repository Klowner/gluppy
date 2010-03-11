#include <hildon-cp-plugin/hildon-cp-plugin-interface.h>
#include <hildon/hildon.h>
#include <gtk/gtk.h>
osso_return_t
execute(osso_context_t* osso, gpointer data, gboolean user_activated)
{
    GtkDialog *dialog;
    gint response;

    // Load the crap from gconf
    
    dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
                "Gluppy Configuration",
                GTK_WINDOW(data),
                GTK_DIALOG_MODAL,
                GTK_STOCK_OK,
                GTK_RESPONSE_OK,
                GTK_STOCK_CANCEL,
                GTK_RESPONSE_CANCEL,
                NULL));
    
    // This sucks, I need text input things.
    /*
    GtkWidget *interval_selector = hildon_touch_selector_new_text();
    hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(interval_selector), "1 minute");    
    hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(interval_selector), "5 minutes");    
    hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(interval_selector), "10 minutes");    
    hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(interval_selector), "15 minutes");    
    hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(interval_selector), "30 minutes");
    GtkWidget *interval_picker = hildon_picker_button_new(
                                        HILDON_SIZE_THUMB_HEIGHT | HILDON_SIZE_AUTO_WIDTH,
                                        HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
    hildon_button_set_title(HILDON_BUTTON(syntax_picker
    
    gtk_container_add(GTK_CONTAINER(dialog->vbox), interval_selector); 
    */
    
    gtk_widget_show_all(GTK_WIDGET(dialog));

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_OK) {
        // dump the values to gconf
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return OSSO_OK;
}

osso_return_t
save_state(osso_context_t* osso, gpointer data)
{
    return OSSO_OK;
}

