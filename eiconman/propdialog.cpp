// generated by Fast Light User Interface Designer (fluid) version 2.1000

#include "propdialog.h"
/*
 * $Id$
 *
 * Desktop icons manager
 * Part of Equinox Desktop Environment (EDE).
 * Copyright (c) 2000-2006 EDE Authors.
 *
 * This program is licenced under terms of the
 * GNU General Public Licence version 2 or newer.
 * See COPYING for details.
 */
#include <fltk/file_chooser.h>
#include "edeskicon.h"
static Icon *ds_icon;

fltk::Window *i_propertieswindow=(fltk::Window *)0;

fltk::Group *i_icontab=(fltk::Group *)0;

fltk::InvisibleBox *pr_icon=(fltk::InvisibleBox *)0;

fltk::InvisibleBox *pr_name=(fltk::InvisibleBox *)0;

fltk::InvisibleBox *pr_size=(fltk::InvisibleBox *)0;

fltk::InvisibleBox *pr_exec=(fltk::InvisibleBox *)0;

fltk::Group *i_settingstab=(fltk::Group *)0;

fltk::Output *i_link=(fltk::Output *)0;

fltk::Input *i_name=(fltk::Input *)0;

fltk::Input *i_location=(fltk::Input *)0;

static void cb_(fltk::Button*, void*) {
  char *file_types = _("Executables (*.*), *, All files (*.*), *");
  const char *f = fltk::file_chooser(_("Open location..."), file_types, i_location->value());
  if (f) i_location->value(f);
}

fltk::Input *i_filename=(fltk::Input *)0;

static void cb_1(fltk::Button*, void*) {
  char *file_types = _("Png images (*.png), *.png, Jpeg Images (*.jpeg), *.{jpeg|jpg}, Bmp Files (*.bmp), *.bmp, Gif Files (*.gif), *.gif, Xpm Files (*.xpm), *.xpm, All files (*.*), *");
  const char *f = fltk::file_chooser(_("Icon file selection"), file_types, PREFIX"/share/ede/icons/48x48/");
  if (f) i_filename->value(f);
}

static void cb_Apply(fltk::Button*, void*) {
  save_icon(ds_icon);
}

static void cb_Close(fltk::Button*, void*) {
  i_propertieswindow->hide();
}

void property_dialog(fltk::Widget *, Icon *icon, bool show_settings) {
  fltk::Window* w;
  if(i_propertieswindow) {
  	update_property_dialog(icon);
  	update_iconeditdialog(icon);
  	ds_icon=icon;
  	if(show_settings) { i_icontab->hide(); i_settingstab->show(); }
  	i_propertieswindow->exec();
  	return;
  }
   {fltk::Window* o = i_propertieswindow = new fltk::Window(295, 310, "Icon properties");
    w = o;
    o->set_vertical();
    o->begin();
     {fltk::TabGroup* o = new fltk::TabGroup(5, 5, 285, 270);
      o->begin();
       {fltk::Group* o = i_icontab = new fltk::Group(0, 25, 285, 240, "Icon");
        o->hide();
        o->begin();
         {fltk::InvisibleBox* o = pr_icon = new fltk::InvisibleBox(10, 10, 60, 55);
          o->box(fltk::THIN_DOWN_BOX);
        }
         {fltk::InvisibleBox* o = new fltk::InvisibleBox(10, 80, 65, 38, "Name:");
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_CENTER|fltk::ALIGN_WRAP);
        }
         {fltk::InvisibleBox* o = new fltk::InvisibleBox(10, 125, 65, 33, "Size:");
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_CENTER|fltk::ALIGN_WRAP);
        }
         {fltk::InvisibleBox* o = new fltk::InvisibleBox(10, 175, 65, 65, "Command:");
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_CENTER|fltk::ALIGN_WRAP);
        }
         {fltk::InvisibleBox* o = pr_name = new fltk::InvisibleBox(75, 78, 200, 40);
          o->labelsize(20);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_CENTER|fltk::ALIGN_CLIP|fltk::ALIGN_WRAP);
        }
         {fltk::InvisibleBox* o = pr_size = new fltk::InvisibleBox(80, 123, 195, 35);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_CENTER|fltk::ALIGN_CLIP|fltk::ALIGN_WRAP);
        }
         {fltk::InvisibleBox* o = pr_exec = new fltk::InvisibleBox(80, 175, 195, 63);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_CENTER|fltk::ALIGN_CLIP|fltk::ALIGN_WRAP);
        }
        o->end();
      }
       {fltk::Group* o = i_settingstab = new fltk::Group(0, 25, 285, 240, "Settings");
        o->begin();
         {fltk::Output* o = i_link = new fltk::Output(10, 20, 265, 22, "Link file:");
          o->color((fltk::Color)0xffffff00);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT);
        }
         {fltk::Input* o = i_name = new fltk::Input(10, 77, 265, 22, "Icon name:");
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_WRAP);
        }
         {fltk::Input* o = i_location = new fltk::Input(10, 117, 215, 22, "Location to open:");
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_WRAP);
        }
         {fltk::Button* o = new fltk::Button(230, 117, 45, 22, "...");
          o->labeltype(fltk::ENGRAVED_LABEL);
          o->callback((fltk::Callback*)cb_);
        }
         {fltk::Input* o = i_filename = new fltk::Input(10, 162, 215, 22, "Icon filename:");
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_WRAP);
        }
         {fltk::Button* o = new fltk::Button(230, 162, 45, 22, "...");
          o->labeltype(fltk::ENGRAVED_LABEL);
          o->callback((fltk::Callback*)cb_1);
        }
         {fltk::Button* o = new fltk::Button(210, 215, 65, 25, "&Apply");
          o->callback((fltk::Callback*)cb_Apply);
        }
        o->end();
      }
      o->end();
    }
     {fltk::Button* o = new fltk::Button(200, 280, 90, 25, "&Close");
      o->callback((fltk::Callback*)cb_Close);
    }
    o->end();
    o->resizable(o);
  }
  update_property_dialog(icon);
  update_iconeditdialog(icon);
  ds_icon=icon;
  i_propertieswindow->end();
  if(show_settings) { i_icontab->hide(); i_settingstab->show(); }
  i_propertieswindow->exec();
}