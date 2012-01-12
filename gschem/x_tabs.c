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

void
x_tabs_create (GSCHEM_TOPLEVEL *w_current) {
  GtkNotebook *tabs =  gtk_notebook_new ();

  gtk_notebook_set_tab_pos (tabs, GTK_POS_TOP);
  gtk_notebook_set_show_tabs (tabs, FALSE);
  gtk_notebook_set_scrollable (tabs, TRUE);
  gtk_notebook_set_show_border (tabs, FALSE);
  gtk_notebook_popup_disable (tabs);

  w_current->notebook = (GtkWidget *) tabs;
}
