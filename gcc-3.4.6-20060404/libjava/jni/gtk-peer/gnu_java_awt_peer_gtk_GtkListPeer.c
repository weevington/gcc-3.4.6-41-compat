/* GtkListPeer.c -- implements GtkListPeer's native methods
   Copyright (C) 1998, 1999, 2003, 2004 Free Software Foundation, Inc.

   This file is part of GNU Classpath.

   GNU Classpath is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Classpath is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Classpath; see the file COPYING.  If not, write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.

   Linking this library statically or dynamically with other modules is
   making a combined work based on this library.  Thus, the terms and
   conditions of the GNU General Public License cover the whole
   combination.

   As a special exception, the copyright holders of this library give you
   permission to link this library with independent modules to produce an
   executable, regardless of the license terms of these independent
   modules, and to copy and distribute the resulting executable under
   terms of your choice, provided that you also meet, for each linked
   independent module, the terms and conditions of the license of that
   module.  An independent module is a module which is not derived from
   or based on this library.  If you modify this library, you may extend
   this exception to your version of the library, but you are not
   obligated to do so.  If you do not wish to do so, delete this
   exception statement from your version. */

#include "gtkpeer.h"
#include "gnu_java_awt_peer_gtk_GtkListPeer.h"

enum
  {
    COLUMN_STRING,
    N_COLUMNS
  };

gboolean item_highlighted (GtkTreeSelection *selection,
                           GtkTreeModel *model,
                           GtkTreePath *path,
                           gboolean path_currently_selected,
                           jobject peer);


#define TREE_VIEW_FROM_SW(obj) \
           (GTK_TREE_VIEW (GTK_SCROLLED_WINDOW (obj)->container.child))

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_create
  (JNIEnv *env, jobject obj, jint rows)
{
  GtkWidget *sw;
  GtkWidget *list;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkRequisition req;
  gint i;

  /* Create global reference and save it for future use */
  NSA_SET_GLOBAL_REF (env, obj);

  gdk_threads_enter ();

  list_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);
  /* Add the number of rows so that we can calculate the tree view's
     size request. */
  for (i = 0; i < rows; i++)
    {
      gtk_list_store_append (list_store, &iter);
      gtk_list_store_set (list_store, &iter,
                          COLUMN_STRING, "",
                          -1);
    }
  list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
				     renderer,
				     "text",
				     COLUMN_STRING,
				     NULL);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);

  gtk_widget_size_request (GTK_WIDGET (list), &req);

  gtk_widget_set_size_request (GTK_WIDGET (list), req.width, req.height);

  gtk_container_add (GTK_CONTAINER (sw), list);

  /* Remove the blank rows. */
  gtk_list_store_clear (list_store);

  gtk_widget_show (list);
  gtk_widget_show (sw);

  gdk_threads_leave ();

  NSA_SET_PTR (env, obj, sw);
}

JNIEXPORT void JNICALL 
Java_gnu_java_awt_peer_gtk_GtkListPeer_connectJObject
  (JNIEnv *env, jobject obj)
{
  void *ptr;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  gtk_widget_realize (GTK_WIDGET (ptr));
  connect_awt_hook (env, obj, 1, GTK_WIDGET (ptr)->window);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL 
Java_gnu_java_awt_peer_gtk_GtkListPeer_connectSignals
  (JNIEnv *env, jobject obj)
{
  void *ptr;
  jobject *gref;
  GtkTreeView *list;
  GtkTreeSelection *selection;

  ptr = NSA_GET_PTR (env, obj);
  gref = NSA_GET_GLOBAL_REF (env, obj);

  gdk_threads_enter ();

  g_assert (gref);

  gtk_widget_realize (GTK_WIDGET (ptr));

  list = TREE_VIEW_FROM_SW (ptr);

  g_signal_connect (G_OBJECT (list), "event",
                    G_CALLBACK (pre_event_handler), *gref);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list));
  gtk_tree_selection_set_select_function (selection, item_highlighted,
                                          *gref, NULL);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_gtkSetFont
  (JNIEnv *env, jobject obj, jstring name, jint style, jint size)
{
  const char *font_name;
  void *ptr;
  GtkWidget *list;
  PangoFontDescription *font_desc;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter();

  list = GTK_WIDGET (TREE_VIEW_FROM_SW (ptr));

  font_name = (*env)->GetStringUTFChars (env, name, NULL);

  font_desc = pango_font_description_from_string (font_name);
  pango_font_description_set_size (font_desc, size * dpi_conversion_factor);

  if (style & AWT_STYLE_BOLD)
    pango_font_description_set_weight (font_desc, PANGO_WEIGHT_BOLD);

  if (style & AWT_STYLE_ITALIC)
    pango_font_description_set_style (font_desc, PANGO_STYLE_OBLIQUE);

  gtk_widget_modify_font (GTK_WIDGET (list), font_desc);

  pango_font_description_free (font_desc);

  (*env)->ReleaseStringUTFChars (env, name, font_name);

  gdk_threads_leave();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_gtkWidgetRequestFocus
  (JNIEnv *env, jobject obj)
{
  void *ptr;
  GtkWidget *list;

  ptr = NSA_GET_PTR (env, obj);
  
  gdk_threads_enter ();

  list = GTK_WIDGET (TREE_VIEW_FROM_SW (ptr));
  gtk_widget_grab_focus (list);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_append
  (JNIEnv *env, jobject obj, jobjectArray items)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreeIter iter;
  GtkTreeModel *list_store;
  jint count;
  jint i;

  ptr = NSA_GET_PTR (env, obj);

  count = (*env)->GetArrayLength (env, items);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  list_store = gtk_tree_view_get_model (list);

  for (i = 0; i < count; i++)
    {
      const char *text;
      jobject item;

      item = (*env)->GetObjectArrayElement (env, items, i);

      text = (*env)->GetStringUTFChars (env, item, NULL);
      gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                          COLUMN_STRING, text,
                          -1);
      (*env)->ReleaseStringUTFChars (env, item, text);
    }

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_add
  (JNIEnv *env, jobject obj, jstring text, jint index)
{
  void *ptr;
  const char *str;
  GtkTreeView *list;
  GtkTreeIter iter;
  GtkTreeModel *list_store;

  ptr = NSA_GET_PTR (env, obj);
  str = (*env)->GetStringUTFChars (env, text, NULL);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  list_store = gtk_tree_view_get_model (list);

  if (index == -1)
    gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
  else
    gtk_list_store_insert (GTK_LIST_STORE (list_store), &iter, index);

  gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                      COLUMN_STRING, str, -1);

  gdk_threads_leave ();

  (*env)->ReleaseStringUTFChars (env, text, str);
}


JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_delItems
  (JNIEnv *env, jobject obj, jint start, jint end)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreeIter iter;
  GtkTreeModel *list_store;
  jint i;
  jint num_items;
    
  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  list_store = gtk_tree_view_get_model (list);

  /* Special case: remove all rows. */
  if (end == -1)
    gtk_list_store_clear (GTK_LIST_STORE (list_store));
  else
    {
      i = 0;
      num_items = end - start + 1;
      gtk_tree_model_iter_nth_child (list_store, &iter, NULL, start);
      while (i < num_items)
	{
	  gtk_list_store_remove (GTK_LIST_STORE (list_store), &iter);
	  i++;
	}
    }

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_select
  (JNIEnv *env, jobject obj, jint index)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreePath *path;
    
  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  path = gtk_tree_path_new_from_indices (index, -1);
  gtk_tree_view_set_cursor (list, path, NULL, FALSE);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_deselect
  (JNIEnv *env, jobject obj, jint index)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreeSelection *selection;
  GtkTreePath *path;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  selection = gtk_tree_view_get_selection (list);
  path = gtk_tree_path_new_from_indices (index, -1);
  gtk_tree_selection_unselect_path (selection, path);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_getSize
  (JNIEnv *env, jobject obj, jint rows, jint visible_rows, jintArray jdims)
{
  void *ptr;
  jint *dims;
  GtkRequisition current_req;
  GtkRequisition natural_req;

  dims = (*env)->GetIntArrayElements (env, jdims, NULL);
  dims[0] = dims[1] = 0;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  /* Save the widget's current size request. */
  gtk_widget_size_request (GTK_WIDGET (ptr), &current_req);
      
  /* Get the widget's "natural" size request. */
  gtk_widget_set_size_request (GTK_WIDGET (ptr), -1, -1);
  gtk_widget_size_request (GTK_WIDGET (ptr), &natural_req);

  /* Reset the widget's size request. */
  gtk_widget_set_size_request (GTK_WIDGET (ptr),
                               current_req.width, current_req.height);

  dims[0] = natural_req.width;

  /* Calculate the final height, by comparing the number of rows
     in the list to the number of rows requested by the caller.
     FIXME: Is there a GTK method that counts the number of rows
     in the list? If so, we don't need to bring visible_rows from
     the Java peer. */
  if (rows == visible_rows)
    dims[1] = natural_req.height;
  else
    dims[1] = natural_req.height / visible_rows * rows;

  gdk_threads_leave ();

  (*env)->ReleaseIntArrayElements (env, jdims, dims, 0);
}


JNIEXPORT jintArray JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_getSelectedIndexes
  (JNIEnv *env, jobject obj)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreeSelection *selection;
  jintArray result_array;
  jint *result_array_iter;
  GList *current_row;
  GList *rows;
  gint *indices;
  jint count;
  jint i;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  selection = gtk_tree_view_get_selection (list);
  count = gtk_tree_selection_count_selected_rows (selection);
  if (count > 0)
    {
      current_row = rows = gtk_tree_selection_get_selected_rows (selection, NULL);

      result_array = (*env)->NewIntArray (env, count);
      result_array_iter = (*env)->GetIntArrayElements (env, result_array, NULL);

      for (i = 0; i < count; i++)
        {
          indices = gtk_tree_path_get_indices (current_row->data);
          result_array_iter[i] = indices ? indices[0] : -1;
          current_row = g_list_next (current_row);
        }

      if (rows)
        {
          g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
          g_list_free (rows);
        }

      gdk_threads_leave ();

      (*env)->ReleaseIntArrayElements (env, result_array, result_array_iter, 0);

      return result_array;
    }
  else
    gdk_threads_leave ();

  return (jintArray) NULL;
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_makeVisible
  (JNIEnv *env, jobject obj, jint index)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreePath *path;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  path = gtk_tree_path_new_from_indices (index, -1);
  gtk_tree_view_scroll_to_cell (list, path, NULL, FALSE, 0.0, 0.0);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkListPeer_setMultipleMode
  (JNIEnv *env, jobject obj, jboolean mode)
{
  void *ptr;
  GtkTreeView *list;
  GtkTreeSelection *selection;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  list = TREE_VIEW_FROM_SW (ptr);
  selection = gtk_tree_view_get_selection (list);
  gtk_tree_selection_set_mode (selection,
                               mode ? GTK_SELECTION_MULTIPLE
                               : GTK_SELECTION_SINGLE);

  gdk_threads_leave ();
}

gboolean
item_highlighted (GtkTreeSelection *selection __attribute__((unused)),
                  GtkTreeModel *model,
                  GtkTreePath *path,
                  gboolean path_currently_selected,
                  jobject peer)
{
  GtkTreeIter iter;
  jint row;
  gint *indices;

  if (gtk_tree_model_get_iter (model, &iter, path))
    {
      indices = gtk_tree_path_get_indices (path);
      row = indices ? indices[0] : -1;

      if (!path_currently_selected)
        (*gdk_env)->CallVoidMethod (gdk_env, peer,
                                    postListItemEventID,
                                    row,
                                    (jint) AWT_ITEM_SELECTED);
      else
        (*gdk_env)->CallVoidMethod (gdk_env, peer,
                                    postListItemEventID,
                                    row,
                                    (jint) AWT_ITEM_DESELECTED);
    }

  return TRUE;
}
