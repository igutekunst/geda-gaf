/* System headers which gschem headers rely on */
#include <glib.h>
#include <gtk/gtk.h>
#include <libguile.h>
#include <libgeda/libgeda.h>
#include <libgeda/libgedaguile.h>
#include <libgedacairo/libgedacairo.h>

/* gschem headers */
#include "gschem_defines.h"
#include "gschem_struct.h"
#include "gschem_accel_label.h"
#include "gschem_action.h"
#include "gschem_dialog.h"
#include "i_vars.h"
#include "x_preview.h"
#include "x_compselect.h"
#include "x_dialog.h"
#include "x_log.h"
#include "x_multiattrib.h"
#include "x_pagesel.h"
#include "x_states.h"
#include "globals.h"
#include "prototype.h"

/* Gettext translation */
#include "gettext.h"
