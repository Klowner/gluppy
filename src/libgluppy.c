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
    
 
    // Run checkbox thing 
    GtkWidget *run_button = hildon_check_button_new (HILDON_SIZE_HALFSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT );
    gtk_button_set_label(GTK_BUTTON(run_button), "Perform location updates");
    gtk_container_add(GTK_CONTAINER(dialog->vbox), GTK_WIDGET(run_button));
    
    // Username entry
    GtkWidget *user_entry = hildon_entry_new( HILDON_SIZE_HALFSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT );
    gtk_container_add(GTK_CONTAINER(dialog->vbox), GTK_WIDGET(user_entry));
    
    // Password entry
    GtkWidget *pass_entry = hildon_entry_new( HILDON_SIZE_HALFSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT );
    gtk_entry_set_visibility(GTK_ENTRY(pass_entry), FALSE);
    gtk_container_add(GTK_CONTAINER(dialog->vbox), GTK_WIDGET(pass_entry));

    gtk_widget_show_all(GTK_WIDGET(dialog));

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_OK) {
        // Dump the input widget values to gconf
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return OSSO_OK;
}

osso_return_t
save_state(osso_context_t* osso, gpointer data)
{
    return OSSO_OK;
}

