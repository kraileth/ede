// generated by Fast Light User Interface Designer (fluid) version 2.0100

#include "edisplayconf.h"
/*
 * $Id$
 *
 * X server properties
 * Part of Equinox Desktop Environment (EDE).
 * Copyright (c) 2000-2006 EDE Authors.
 *
 * This program is licenced under terms of the
 * GNU General Public Licence version 2 or newer.
 * See COPYING for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include "conf.h"
#include "../edelib2/NLS.h"
//#include <efltk/Fl_Util.h>
#include <fltk/run.h>

inline void edisplayconf::cb_win_i(fltk::Window*, void*) {
  _finish = true;
}
void edisplayconf::cb_win(fltk::Window* o, void* v) {
  ((edisplayconf*)(o->user_data()))->cb_win_i(o,v);
}

inline void edisplayconf::cb_Test_i(fltk::Button*, void*) {
  testbellCB();
}
void edisplayconf::cb_Test(fltk::Button* o, void* v) {
  ((edisplayconf*)(o->parent()->parent()->parent()->user_data()))->cb_Test_i(o,v);
}

inline void edisplayconf::cb_but_activate_i(fltk::Button*, void*) {
  TestBlankCB();
}
void edisplayconf::cb_but_activate(fltk::Button* o, void* v) {
  ((edisplayconf*)(o->parent()->parent()->parent()->user_data()))->cb_but_activate_i(o,v);
}

inline void edisplayconf::cb_radio_blank_i(fltk::RadioButton*, void*) {
  radioCB();
}
void edisplayconf::cb_radio_blank(fltk::RadioButton* o, void* v) {
  ((edisplayconf*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_radio_blank_i(o,v);
}

inline void edisplayconf::cb_radio_pattern_i(fltk::RadioButton*, void*) {
  radioCB();
}
void edisplayconf::cb_radio_pattern(fltk::RadioButton* o, void* v) {
  ((edisplayconf*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_radio_pattern_i(o,v);
}

inline void edisplayconf::cb_Cancel_i(fltk::Button*, void*) {
  cancelCB();
}
void edisplayconf::cb_Cancel(fltk::Button* o, void* v) {
  ((edisplayconf*)(o->parent()->user_data()))->cb_Cancel_i(o,v);
}

inline void edisplayconf::cb_but_kbd_apply_i(fltk::Button*, void*) {
  applyCB();
}
void edisplayconf::cb_but_kbd_apply(fltk::Button* o, void* v) {
  ((edisplayconf*)(o->parent()->user_data()))->cb_but_kbd_apply_i(o,v);
}

inline void edisplayconf::cb_OK_i(fltk::ReturnButton*, void*) {
  okCB();
}
void edisplayconf::cb_OK(fltk::ReturnButton* o, void* v) {
  ((edisplayconf*)(o->parent()->user_data()))->cb_OK_i(o,v);
}

edisplayconf::edisplayconf() {
  fltk::Window* w;
   {fltk::Window* o = win = new fltk::Window(265, 335, "Display configuration");
    w = o;
    o->set_vertical();
    o->callback((fltk::Callback*)cb_win, (void*)(this));
    o->begin();
     {fltk::TabGroup* o = new fltk::TabGroup(5, 5, 260, 285);
      o->set_vertical();
      o->begin();
       {fltk::Group* o = group_mouse = new fltk::Group(0, 24, 255, 260, "Mouse");
        o->set_vertical();
        o->align(fltk::ALIGN_LEFT);
        o->begin();
         {fltk::ValueSlider* o = slider_accel = new fltk::ValueSlider(20, 30, 220, 18, "Acceleration");
          o->maximum(10);
          o->step(1);
          o->value(2);
          o->align(fltk::ALIGN_TOP);
        }
         {fltk::ValueSlider* o = slider_thresh = new fltk::ValueSlider(20, 75, 220, 18, "Threshold (pixels)");
          o->maximum(20);
          o->step(1);
          o->value(4);
          o->align(fltk::ALIGN_TOP);
        }
        o->end();
      }
       {fltk::Group* o = group_bell = new fltk::Group(0, 25, 255, 259, "Bell");
        o->set_vertical();
        o->hide();
        o->begin();
         {fltk::ValueSlider* o = slider_volume = new fltk::ValueSlider(20, 30, 220, 18, "Volume in %");
          o->maximum(100);
          o->step(1);
          o->value(50);
          o->align(fltk::ALIGN_TOP);
        }
         {fltk::ValueSlider* o = slider_pitch = new fltk::ValueSlider(20, 75, 220, 18, "Pitch in Hz");
          o->minimum(100);
          o->maximum(1000);
          o->step(1);
          o->value(440);
          o->align(fltk::ALIGN_TOP);
        }
         {fltk::ValueSlider* o = slider_duration = new fltk::ValueSlider(20, 120, 220, 18, "Duration in ms");
          o->maximum(1000);
          o->step(1);
          o->value(200);
          o->align(fltk::ALIGN_TOP);
        }
         {fltk::Button* o = new fltk::Button(175, 170, 60, 25, "Test");
          o->callback((fltk::Callback*)cb_Test);
        }
        o->end();
      }
       {fltk::Group* o = group_keyboard = new fltk::Group(0, 25, 255, 259, "Keyboard");
        o->set_vertical();
        o->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT);
        o->hide();
        o->begin();
         {fltk::CheckButton* o = check_autorepeat = new fltk::CheckButton(24, 20, 221, 20, "  Repeat key activated");
          o->selection_color((fltk::Color)2);
          //o->set_flag(fltk::VALUE);
        }
         {fltk::ValueSlider* o = slider_click = new fltk::ValueSlider(25, 67, 220, 18, "Click volume %");
          o->maximum(100);
          o->step(1);
          o->value(50);
          o->align(fltk::ALIGN_TOP);
        }
        o->end();
      }
       {fltk::Group* o = group_screen = new fltk::Group(0, 25, 255, 259, "Screen");
        o->set_vertical();
        o->hide();
        o->begin();
         {fltk::Button* o = but_activate = new fltk::Button(167, 190, 80, 25, "&Test");
          o->callback((fltk::Callback*)cb_but_activate);
        }
        new fltk::InvisibleBox(10, 0, 234, 55);
         {fltk::ValueSlider* o = slider_delay = new fltk::ValueSlider(27, 71, 210, 18, "Activation delay (min)");
          o->minimum(5);
          o->maximum(120);
          o->step(1);
          o->value(15);
          o->align(fltk::ALIGN_TOP);
        }
         {fltk::CheckButton* o = check_blanking = new fltk::CheckButton(15, 20, 222, 20, " Screen blanker activated");
          o->selection_color((fltk::Color)2);
          //o->set_flag(fltk::VALUE);
        }
        new fltk::InvisibleBox(10, 99, 234, 37);
         {fltk::ValueSlider* o = slider_pattern = new fltk::ValueSlider(27, 155, 210, 18, "Pattern change delay (min)");
          o->minimum(1);
          o->maximum(5);
          o->step(0);
          o->value(2);
          o->align(fltk::ALIGN_TOP);
          o->deactivate();
        }
         {fltk::Group* o = new fltk::Group(35, 108, 210, 27);
          o->begin();
           {fltk::RadioButton* o = radio_blank = new fltk::RadioButton(0, 0, 105, 20, "Blank");
            o->selection_color((fltk::Color)1);
            //o->set_flag(fltk::VALUE);
            o->callback((fltk::Callback*)cb_radio_blank);
          }
           {fltk::RadioButton* o = radio_pattern = new fltk::RadioButton(107, 5, 95, 20, "Pattern");
            o->selection_color((fltk::Color)1);
            o->callback((fltk::Callback*)cb_radio_pattern);
          }
          o->end();
        }
        o->end();
      }
      o->end();
    }
     {fltk::Button* o = new fltk::Button(190, 300, 70, 25, "&Cancel");
      o->callback((fltk::Callback*)cb_Cancel);
      o->align(fltk::ALIGN_CENTER);
    }
     {fltk::Button* o = but_kbd_apply = new fltk::Button(115, 300, 70, 25, "&Apply");
      o->callback((fltk::Callback*)cb_but_kbd_apply);
    }
     {fltk::ReturnButton* o = new fltk::ReturnButton(40, 300, 70, 25, "OK");
      o->shortcut(0xd);
      o->callback((fltk::Callback*)cb_OK);
    }
    o->end();
    o->resizable(o);
  }
}

edisplayconf::~edisplayconf() {
}

void edisplayconf::Run() {
  _finish = false;
  	win->show();
  	while (!_finish) fltk::wait();
  	win->hide();
}

void edisplayconf::make_window() {
}

void edisplayconf::radioCB() {
  int l_blank;
  
    l_blank = (int)radio_blank->value();
    if (l_blank)
      slider_pattern->deactivate();
    else
      slider_pattern->activate();
}

int main(int ac,char **av) {
  //fl_init_locale_support("edisplayconf", PREFIX"/share/locale");
  app = new edisplayconf();
  read_disp_configuration();
  app->Run();
  delete app;
  return 0;
}
edisplayconf *app;
