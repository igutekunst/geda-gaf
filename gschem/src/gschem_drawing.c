/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 2010 gEDA Contributors (see ChangeLog for details)
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 */
#include <config.h>

#include "gschem.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

enum {
  PROP_PAGE = 1,
  PROP_DRAW_SELECTED = 2,
};

static GObjectClass *drawing_parent_class = NULL;

#ifndef HAS_RINT
  #define rint(x) (x)
#endif

static inline void
scr_to_wld_xy (GschemDrawing *drawing, int sx, int sy, int *wx, int *wy)
{
  h = (GTK_WIDGET (drawing))->allocation.height;
  *wx = rint (drawing->_mil_per_px * sx + drawing->_x_ofs_mil);
  *wy = rint (drawing->_mil_per_px * (h - sy) + drawing->_y_ofs_mil);
}

static inline void
wld_to_scr_xy (GschemDrawing *drawing, int wx, int wy, int *sx, int *sy)
{
  h = (GTK_WIDGET (drawing))->allocation.height;
  *sx = rint ((wx - drawing->_x_ofs_mil) / drawing->_mil_per_px);
  *sy = h - rint ((wy - drawing->_y_ofs_mil) / drawing->_mil_per_px);
}

static inline double
scr_to_wld_dist (GschemDrawing *drawing, double w)
{
  return rint (w * drawing->_mil_per_px);
}

static inline int
wld_to_scr_dist (GschemDrawing *drawing, int wx, int wy, int *sx, int *sy)
{
  return rint (w / drawing->_mil_per_px);
}

static int
gschem_drawing_grip_radius (GschemDrawing *drawing)
{
  int size;
  if (drawing->_mil_per_px > SMALL_ZOOMFACTOR1) {
    size = GRIP_SIZE1;
  } else if (drawing->_mil_per_px > SMALL_ZOOMFACTOR2) {
    size = GRIP_SIZE2;
  } else {
    size = GRIP_SIZE3;
  }
  return min (size, MAXIMUM_GRIP_PIXELS);
}

static int
gschem_drawing_cue_radius (GschemDrawing *drawing)
{
  return CUE_BOX_SIZE;
}

static void
gschem_drawing_handle_expose_object (GschemDrawing *drawing,
                                     OBJECT *object)
{
  switch (object->type) {
  case OBJ_LINE:
    
    break;
  case OBJ_PATH:
    break;
  case OBJ_BOX:
    break;
  case OBJ_PICTURE:
    break;
  case OBJ_CIRCLE:
    break;
  case OBJ_NET:
    break;
  case OBJ_BUS:
    break;
  case OBJ_TEXT:
    break;
  case OBJ_PIN:
    break;
  case OBJ_ARC:
    break;
  case OBJ_PLACEHOLDER:
  case OBJ_COMPLEX:
    break;
  default:
    /* Do nothing */
    break;
  }
}

static void
gschem_drawing_handle_expose (GtkWidget *widget, GdkEventExpose *event,
                              gpointer data)
{
  GschemDrawing *drawing;
  GdkRectangle *g_rectangles =
    gdk_region_get_rectangles (event->region, &rectangles, &n_rectangles);
  int n_rectangles;
  int i;
  cairo_t *cr;
  PangoLayout *pl;

  PAGE *page;
  TOPLEVEL *toplevel;
  BOX *w_rectangles;
  int w_rectangle_bloat;

  GList *objs_to_redraw;
  GLst *iter;

  rectangles = gdk_region_get_rectangles (event->region,
                                          &g_rectangles, &n_rectangles);
  drawing = (GschemDrawing *) widget;
  cr = gdk_cairo_create (widget->window);

  /* If we have no page to draw, just blank with background colour */
  if (drawing->_page == NULL) {
    gschem_cairo_set_source_color (NULL/*FIXME*/, BACKGROUND_COLOR);
    cairo_paint ();
    return;
  }

  /* Create array of rectangles in world coordinates. We include some
   * bloat to make sure adding/removing cues/grips gets redrawn
   * correctly. */
  page = drawing->_page;
  toplevel = page->toplevel;
  w_rectangles = g_new (BOX, n_rectangles);
  bloat = max (gschem_drawing_cue_radius (drawing),
               gschem_drawing_grip_radius (drawing));

  for (i = 0; i < n_rectangles; i++) {
    int x, y, width, height;
    x = g_rectangles[i].x;
    y = g_rectangles[i].y;
    width = g_rectangles[i].width;
    height = g_rectangles[i].height;

    scr_to_wld_xy (drawing, x - bloat, y + height + bloat,
                   &w_rectangles[i].lower_x, &w_rectangles[i].lower_y);
    scr_to_wld_xy (drawing, x + width + bloat, y - bloat,
                   &w_rectangles[i].upper_x, &w_rectangles[i].upper_y);
  }

  /* We then use the world coordinate rectangles to build a list of
   * objects to redraw. */
  objs_to_redraw =
    s_page_objects_in_regions (toplevel, page, w_rectangles, n_rectangles);
  g_free (w_rectangles);

  /* First pass: draw the objects themselves. */
  for (iter = objs_to_redraw; iter != NULL; iter = g_list_next (iter)) {
    gschem_drawing_handle_expose_object (drawing, object);
  }
}

static void
gschem_drawing_handle_object_change (void *user_data, OBJECT *object)
{
}

static void
gschem_drawing_handle_pre_object_change (void *user_data, OBJECT *object)
{
}

static void
gschem_drawing_change_page (GschemDrawing *drawing, PAGE *new_page)
{
  TOPLEVEL *toplevel;

  /* If we already know about a page, get rid of it. */
  if (drawing->_page != NULL) {
    /* Unregister change notification callbacks */
    toplevel = drawing->_page->toplevel;
    o_remove_change_notify (handle_object_change, handle_pre_object_change,
                            drawing);

    /* Unregister weak pointer */
    s_page_remove_weak_ptr (drawing->_page, &drawing->_page);
    drawing->_page = NULL;
    toplevel = NULL;
  }

  /* Set up new page for rendering */
  if (new_page != NULL) {
    /* Register weak pointer */
    drawing->_page = new_page;
    s_page_add_weak_ptr (drawing->_page, &drawing->_page);

    /* Register change notification callbacks */
    toplevel = drawing->_page->toplevel;
    o_add_change_notify (handle_object_change, handle_pre_object_change,
                         drawing);
  }

  /* Mark widget as needing a redraw */
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing)),
                              NULL, TRUE);

  /* FIXME Need to update adjustments too */
}

static void
gschem_drawing_drawing_finalize (GObject *object)
{
  GschemDrawing *drawing = GSCHEM_DRAWING (object);

  if (drawing->_page != NULL) {
    s_page_remove_weak_ptr (drawing->_page, &drawing->_page);
    drawing->_page = NULL;
  }
}

static void
gschem_drawing_set_property (GObject *object, guint property_id,
                      const GValue *value, GParamSpec *pspec)
{
  GschemDrawing *drawing = GSCHEM_DRAWING (object);

  switch (property_id) {
  case PROP_PAGE:
    change_page (g_value_get_pointer (value));
    break;
  case PROP_DRAW_SELECTED:
    drawing->_draw_selected = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
gschem_drawing_get_property (GObject *object, guint property_id, GValue *value,
                             GParamSpec *pspec)
{
  GschemDrawing *drawing = GSCHEM_DRAWING (object);

  switch (property_id) {
  case PROP_PAGE:
    g_value_set_pointer (value, drawing->_page);
    break;
  case PROP_DRAW_SELECTED:
    g_value_set_boolean (value, drawing->_draw_selected);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
gschem_drawing_class_init (GschemDrawingClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkDrawingAreaClass *gobject_parent_class =
    GTK_DRAWING_AREA_CLASS (klass);

  drawing_parent_class = g_type_class_peek_parent (klass);
  gobject_class->finalize = gschem_drawing_finalize;
  gobject_class->set_property = gschem_drawing_set_property;
  gobject_class->get_property = gschem_drawing_get_property;
}

GType
gschem_drawing_get_type ()
{
  static GType gschem_drawing_type = 0;

  if (!gschem_drawing_type) {
    static const GTypeInfo gschem_drawing_info = {
      sizeof (GschemDrawingClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) gschem_drawing_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (GschemDrawing),
      0,    /* n_preallocs */
      NULL, /* instance_init */
    };
    gschem_drawing_type = g_type_register_static (GTK_TYPE_DRAWING_AREA,
                                                  "GschemWindow",
                                                  &gschem_drawing_type, 0);
  }
  return gschem_drawing_type;
}

GschemDrawing *
gschem_drawing_new (PAGE *page)
{
  g_return_val_if_fail (page != NULL, NULL);

  return g_object_new (GSCHEM_TYPE_DRAWING,
                       "page", page,
                       NULL);
}
