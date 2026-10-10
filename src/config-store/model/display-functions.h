
#ifndef DISPLAY_FUNCTIONS_H
#define DISPLAY_FUNCTIONS_H

#include "model-node-creator.h"
#include "model-typeid-creator.h"

#include <gtk/gtk.h>

namespace ns3 {
void cell_data_function_col_1(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                              GtkTreeModel *model, GtkTreeIter *iter,
                              gpointer user_data);
void cell_data_function_col_0(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                              GtkTreeModel *model, GtkTreeIter *iter,
                              gpointer user_data);
void cell_edited_callback(GtkCellRendererText *cell, gchar *path_string,
                          gchar *new_text, gpointer user_data);
int get_col_number_from_tree_view_column(GtkTreeViewColumn *col);
gboolean cell_tooltip_callback(GtkWidget *widget, gint x, gint y,
                               gboolean keyboard_tip, GtkTooltip *tooltip,
                               gpointer user_data);
GtkWidget *create_view(GtkTreeStore *model);
void exit_clicked_callback(GtkButton *button, gpointer user_data);
gboolean delete_event_callback(GtkWidget *widget, GdkEvent *event,
                               gpointer user_data);
gboolean clean_model_callback(GtkTreeModel *model, GtkTreePath *path,
                              GtkTreeIter *iter, gpointer data);

void cell_data_function_col_1_config_default(GtkTreeViewColumn *col,
                                             GtkCellRenderer *renderer,
                                             GtkTreeModel *model,
                                             GtkTreeIter *iter,
                                             gpointer user_data);
void cell_data_function_col_0_config_default(GtkTreeViewColumn *col,
                                             GtkCellRenderer *renderer,
                                             GtkTreeModel *model,
                                             GtkTreeIter *iter,
                                             gpointer user_data);
void save_clicked_default(GtkButton *button, gpointer user_data);
void load_clicked_default(GtkButton *button, gpointer user_data);
void save_clicked_attribute(GtkButton *button, gpointer user_data);
void load_clicked_attribute(GtkButton *button, gpointer user_data);
void cell_edited_callback_config_default(GtkCellRendererText *cell,
                                         gchar *path_string, gchar *new_text,
                                         gpointer user_data);
gboolean cell_tooltip_callback_config_default(GtkWidget *widget, gint x, gint y,
                                              gboolean keyboard_tip,
                                              GtkTooltip *tooltip,
                                              gpointer user_data);
GtkWidget *create_view_config_default(GtkTreeStore *model);
gboolean clean_model_callback_config_default(GtkTreeModel *model,
                                             GtkTreePath *path,
                                             GtkTreeIter *iter, gpointer data);
} // namespace ns3

#endif
