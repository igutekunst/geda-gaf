/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2010 gEDA Contributors (see ChangeLog for details)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <config.h>

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <math.h>

#include "gschem.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

static void
x_print_draw_page (GtkPrintOperation *print,
                   GtkPrintContext *context,
                   gint page_nr,
                   gpointer user_data)
{
  GtkPrintSettings *settings = gtk_print_operation_get_print_settings (print);
  GSCHEM_TOPLEVEL *w_current = (GSCHEM_TOPLEVEL *) user_data;
  PAGE *page;
  cairo_t *cr;
  cairo_matrix_t render_mtx;
  PangoContext *pc;
  EdaRenderer *renderer;
  GArray *color_map;
  int wx_min, wy_min, wx_max, wy_max;
  double w_width, w_height, p_width, p_height;
  double scale;
  int status;
  GList *iter;

  /* Find the page data */
  g_return_if_fail (page_nr != 1);
  page = w_current->toplevel->page_current;
  g_return_if_fail (page != NULL);

  /* Get extents of drawable objects. */
  status = world_get_object_glist_bounds (w_current->toplevel,
                                          s_page_objects (page),
                                          &wx_min, &wy_min, &wx_max, &wy_max);
  if (!status) {
    /* There are no printable objects. Leave page blank. */
    return;
  }

  /* Set up cairo context */
  cr = gtk_print_context_get_cairo_context (context);

  /* Set up a transformation matrix to fit the extents of the page
   * into the available page area. */
  w_width = wx_max - wx_min;
  w_height = wy_max - wy_min;
  p_width = gtk_print_context_get_width (context);
  p_height = gtk_print_context_get_height (context);
  scale = fmin (p_width / w_width, p_height / w_height);

  cairo_matrix_init (&render_mtx,
                     scale, 0,
                     0, -scale,
                     -wx_min * scale,
                     (wy_min + w_height) * scale);

  /* Set up renderer. */

  pc = gtk_print_context_create_pango_context (context);
  renderer = g_object_new (EDA_TYPE_RENDERER,
                           "cairo-context", cr,
                           "pango-context", pc,
                           "render-flags", 0,
                           NULL);

  /* Set up color map */
  color_map = g_array_sized_new (FALSE, FALSE, sizeof(COLOR), MAX_COLORS);
  color_map = g_array_append_vals (color_map, print_colors, MAX_COLORS);

  /* Handle black & white output mode */
  if (!gtk_print_settings_get_use_color (settings)) {
    /* If color printing is disabled, turn the print color map into a
     * black & white color map.  We do this by disabling the
     * background color, and turning every other enabled color solid
     * black. */
    int i;
    for (i = 0; i < MAX_COLORS; i++) {
      COLOR *c = &g_array_index (color_map, COLOR, i);
      if (!c->enabled) continue;

      /* Disable fully-transparent colors */
      if (c->a == 0) {
        c->enabled = FALSE;
        continue;
      }

      /* Set any remaining colors solid black */
      c->r = 0;
      c->g = 0;
      c->b = 0;
      c->a = ~0;
    }
    /* Disable the background color */
    (&g_array_index (color_map, COLOR, BACKGROUND_COLOR))->enabled = FALSE;
  }

  eda_renderer_set_color_map (renderer, color_map);

  cairo_save (cr);
  cairo_transform (cr, &render_mtx);

  /* Draw background */
  eda_cairo_set_source_color (cr, BACKGROUND_COLOR, color_map);
  cairo_paint (cr);

  /* Draw all objects and cues */
  for (iter = (GList *) s_page_objects (page);
       iter != NULL;
       iter = g_list_next (iter)) {
    eda_renderer_draw (renderer, (OBJECT *) iter->data);
  }
  for (iter = (GList *) s_page_objects (page);
       iter != NULL;
       iter = g_list_next (iter)) {
    eda_renderer_draw_cues (renderer, (OBJECT *) iter->data);
  }

  cairo_restore (cr);

  /* Clean up */
  g_object_unref (pc);
  g_object_unref (renderer);
}

void
x_print_page_setup (GSCHEM_TOPLEVEL *w_current)
{
  GtkPageSetup *new_setup;
  new_setup = gtk_print_run_page_setup_dialog (GTK_WINDOW (w_current->main_window),
                                               w_current->page_setup,
                                               w_current->print_settings);
  g_object_unref (w_current->page_setup);
  w_current->page_setup = new_setup;
}

void
x_print (GSCHEM_TOPLEVEL *w_current)
{
  GtkPrintOperation *print;
  GtkPrintOperationResult res;
  GError *err = NULL;
  int num_pages = 1;

  /* Create the print operation and set it up */
  print = g_object_new (GTK_TYPE_PRINT_OPERATION,
                        "n-pages", num_pages,
                        "use-full-page", FALSE,
                        "unit", GTK_UNIT_POINTS,
                        "default-page-setup", w_current->page_setup,
                        NULL);

  gtk_print_operation_set_print_settings (print, w_current->print_settings);

  g_signal_connect (print, "draw_page", G_CALLBACK (x_print_draw_page),
                    w_current);

  res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                 GTK_WINDOW (w_current->main_window), &err);

  if (res == GTK_PRINT_OPERATION_RESULT_ERROR) {
    /* If printing failed due to an error, show an error dialog */
    GtkWidget *error_dialog =
      gtk_message_dialog_new (GTK_WINDOW (w_current->main_window),
                              GTK_DIALOG_DESTROY_WITH_PARENT,
                              GTK_MESSAGE_ERROR,
                              GTK_BUTTONS_CLOSE,
                              _("Error printing file:\n%s"),
                              err->message);
    g_signal_connect (error_dialog, "response",
                      G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_widget_show (error_dialog);
    g_error_free (err);

  } else if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
    /* We're supposed to store the print settings, so do that */
    g_object_unref (w_current->print_settings);
    w_current->print_settings =
      g_object_ref (gtk_print_operation_get_print_settings (print));
  }

  /* Clean up */
  g_object_unref (print);
}
