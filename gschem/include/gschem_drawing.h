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

#ifndef __GSCHEM_DRAWING_H__
#define __GSCHEM_DRAWING_H__

#define GSCHEM_TYPE_DRAWING (gschem_drawing_get_type())
#define GSCHEM_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSCHEM_TYPE_DRAWING, GschemDrawing))
#define GSCHEM_DRAWING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GSCHEM_TYPE_DRAWING, GschemDrawingClass))
#define GSCHEM_IS_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_TYPE (

typedef struct _GschemDrawingClass GschemDrawingClass;
typedef struct _GschemDrawing GschemDrawing;

struct _GschemDrawingClass {
  GtkDrawingAreaClass *parent_class;
};

struct _GschemDrawing {
  GtkDrawingArea *parent_instance;
  PAGE *_page;
  int _draw_selected;
  double _mil_per_px; /* Zoom factor */
  int _x_ofs_mil, _y_ofs_mil;
};

GType gschem_drawing_get_type (void);

GtkWidget *gschem_drawing_new (PAGE *page);

#endif /* __GSCHEM_DRAWING_H__ */
