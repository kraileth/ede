/*
 * $Id$
 *
 * Eiconman, desktop and icon manager
 * Part of Equinox Desktop Environment (EDE).
 * Copyright (c) 2000-2007 EDE Authors.
 *
 * This program is licensed under the terms of 
 * the GNU General Public License version 2 or later.
 * See COPYING for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eiconman.h"
#include "DesktopIcon.h"
#include "Utils.h"
#include "Wallpaper.h"
#include "NotifyBox.h"

#include <edelib/Debug.h>
#include <edelib/File.h>
#include <edelib/DesktopFile.h>
#include <edelib/Directory.h>
#include <edelib/DirWatch.h>
#include <edelib/MimeType.h>
#include <edelib/StrUtil.h>
#include <edelib/IconTheme.h>
#include <edelib/Run.h>
#include <edelib/Nls.h>

#include <FL/Fl.h>
#include <FL/x.h>
#include <FL/fl_draw.h>
#include <FL/Fl_Box.h>
#include <FL/Fl_Shared_Image.h>
#include <FL/Fl_Menu_Button.h>
#include <FL/fl_ask.h>

#include <unistd.h> // sleep
#include <signal.h>
#include <stdlib.h> // rand, srand
#include <time.h>   // time

#if 0
#include <X11/extensions/Xdamage.h>
#endif

#define EICONMAN_UID       0x10
#define CONFIG_NAME        "eiconman.conf"
#define ICONS_CONFIG_NAME  "eiconman-icons.conf"

#define SELECTION_SINGLE (Fl::event_button() == 1)
#define SELECTION_MULTI  (Fl::event_button() == 1 && (Fl::event_key(FL_Shift_L) || Fl::event_key(FL_Shift_R)))

#undef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))

/*
 * Which widgets Fl::belowmouse() should skip. This should be updated
 * when new non-icon child is added to Desktop window.
 */
#define NOT_SELECTABLE(widget) ((widget == this) || (widget == wallpaper) || (widget == dmenu))

void background_conf_cb(Fl_Widget*, void*);
void icons_conf_cb(Fl_Widget*, void*);

Fl_Menu_Item desktop_menu[] = {
	{_("    &New desktop item    "), 0, 0, 0, FL_SUBMENU},
		{_("    &Application...    "), 0, 0},
		{_("    &Directory...    "), 0, 0},
		{_("    &URL...    "), 0, 0},
		{_("    &Link...    "), 0, 0},
		{0},
	{_("    &Arrange    "), 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER},
		{_("    &Vertical line up    "), 0, 0},
		{_("    &Horizontal line up   "), 0, 0},
		{0},
	{_("    &Copy    "), 0, 0},
	{_("    &Paste    "), 0, 0, 0, FL_MENU_DIVIDER},
	{_("    &Icons settings...    "), 0, icons_conf_cb, 0},
	{_("    &Background...    "), 0, background_conf_cb, 0},
	{0}
};


#if 0
XserverRegion xregion;
XserverRegion xpart;
Damage        xdamage;
int           xevent_base, xerror_base;
#endif

Desktop* Desktop::pinstance = NULL;
bool running = false;

inline unsigned int random_pos(int max) { 
	return (rand() % max); 
}

inline bool intersects(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
	return (MAX(x1, x2) <= MIN(w1, w2) && 
			MAX(y1, y2) <= MIN(h1, h2));
}

// assume fl_open_display() is called before
inline void dpy_sizes(int& width, int& height) {
	width = DisplayWidth(fl_display, fl_screen);
	height = DisplayHeight(fl_display, fl_screen);
}

void exit_signal(int signum) {
    EDEBUG(ESTRLOC ": Exiting (got signal %d)\n", signum);
    running = false;
}

void restart_signal(int signum) {
	EDEBUG(ESTRLOC ": Restarting (got signal %d)\n", signum);
}

void dir_watch_cb(const char* dir, const char* changed, int flags, void* data) {
	Desktop::instance()->dir_watch(dir, changed, flags);
}

void settings_changed_cb(void* data) {
	Desktop::instance()->read_config();
	Desktop::instance()->redraw();
}

void background_conf_cb(Fl_Widget*, void*) {
	Desktop::instance()->execute("../edesktopconf/edesktopconf");
}

void icons_conf_cb(Fl_Widget*, void*) {
	Desktop::instance()->execute("../edesktopconf/edesktopconf --icons");
}

int desktop_xmessage_handler(int event) { 
#if 0
	if(fl_xevent->type == xevent_base + XDamageNotify) {
		XDamageNotifyEvent* xdev = (XDamageNotifyEvent*)fl_xevent;

		EDEBUG(ESTRLOC ": Damaged region %i %i %i %i on 0x%lx\n", xdev->area.x, xdev->area.y,
				xdev->area.width, xdev->area.height, xdev->drawable);

		//XDamageSubtract(fl_display, xdev->damage, None, None);
  		XFixesSetRegion(fl_display, xpart, &xdev->area, 1);
        XFixesUnionRegion(fl_display, xregion, xregion, xpart);
	}

		XDamageSubtract(fl_display, xdamage, xregion, None);
    	XFixesSetRegion(fl_display, xregion, 0, 0);
#endif

	if(fl_xevent->type == PropertyNotify) {
		if(fl_xevent->xproperty.atom == _XA_NET_CURRENT_DESKTOP) {
			Desktop::instance()->notify_desktop_changed();
			return 1;
		}

		if(fl_xevent->xproperty.atom == _XA_EDE_DESKTOP_NOTIFY) {
			char buff[256];
			if(ede_get_desktop_notify(buff, 256) && buff[0] != '\0')
				Desktop::instance()->notify_box(buff, true);
			return 1;
		}

		if(fl_xevent->xproperty.atom == _XA_EDE_DESKTOP_NOTIFY_COLOR) {
			Desktop::instance()->notify_box_color(ede_get_desktop_notify_color());
			return 1;
		}

		if(fl_xevent->xproperty.atom == _XA_NET_WORKAREA) {
			Desktop::instance()->update_workarea();
			return 1;
		}
	}

	return 0; 
}

Desktop::Desktop() : DESKTOP_WINDOW(0, 0, 100, 100, "") {
	selection_x = selection_y = 0;
	moving = false;
	do_dirwatch = true;

#ifdef USE_EDELIB_WINDOW
	DESKTOP_WINDOW::init();
	settings_uid(EICONMAN_UID);
	settings_callback(settings_changed_cb);
#endif

	selbox = new SelectionOverlay;
	selbox->x = selbox->y = selbox->w = selbox->h = 0;
	selbox->show = false;

	dsett = new DesktopSettings;
	dsett->color = FL_GRAY;
	dsett->wp_use = false;
}

Desktop::~Desktop() { 
	EDEBUG("Desktop::~Desktop()\n");

	save_icons();

	delete dsett;
	delete selbox;
	delete notify;

	edelib::DirWatch::shutdown();
}

void Desktop::init_internals(void) {
	init_atoms();
	update_workarea();

	/*
	 * NOTE: order how childs are added is important. First
	 * non iconable ones should be added (wallpaper, menu, ...)
	 * then icons, so they can be drawn at top of them.
	 */
	begin();
		dmenu = new Fl_Menu_Button(0, 0, 0, 0);
		dmenu->menu(desktop_menu);

		wallpaper = new Wallpaper(0, 0, w(), h());
	end();

	notify = new NotifyBox(w(), h());
	set_bg_color(dsett->color, false);

	read_config();

	/*
	 * Now try to load icons, first looking inside ~/Desktop directory
	 * FIXME: dir_exists() can't handle '~/desktop' ???
	 */
	edelib::String dd = edelib::dir_home();
	if(dd.empty()) {
		EWARNING(ESTRLOC ": can't read home directory; icons will not be loaded\n");
		return;
	}
	dd += "/Desktop";

	if(edelib::dir_exists(dd.c_str())) {
		load_icons(dd.c_str());
		install_watch(dd.c_str());
	}

	running = true;
}

void Desktop::init(void) {
	if(Desktop::pinstance != NULL)
		return;

	Desktop::pinstance = new Desktop();
	Desktop::pinstance->init_internals();
}

void Desktop::shutdown(void) {
	if(Desktop::pinstance == NULL)
		return;

	delete Desktop::pinstance;
	Desktop::pinstance = NULL;
}

Desktop* Desktop::instance(void) {
	EASSERT(Desktop::pinstance != NULL && "Desktop::init() should be run first");
	return Desktop::pinstance;
}

/*
 * This function must be overriden so window can inform
 * wm to see it as desktop. It will send data when window
 * is created, but before is shown.
 */
void Desktop::show(void) {
	if(!shown()) {
		Fl_X::make_xid(this);
		net_make_me_desktop(this);
#if 0
		XDamageQueryExtension(fl_display, &xevent_base, &xerror_base);

		xdamage = XDamageCreate(fl_display, fl_xid(this), XDamageReportBoundingBox);
		xregion = XFixesCreateRegionFromWindow(fl_display, fl_xid(this), 0);
		xpart   = XFixesCreateRegionFromWindow(fl_display, fl_xid(this), 0);
#endif
	}
}

/*
 * If someone intentionaly hide desktop
 * then quit from it.
 */
void Desktop::hide(void) {
	running = false;
}

void Desktop::update_workarea(void) {
	int X, Y, W, H;
	if(!net_get_workarea(X, Y, W, H)) {
		EWARNING(ESTRLOC ": wm does not support _NET_WM_WORKAREA; using screen sizes...\n");
		X = Y = 0;
		dpy_sizes(W, H);
	}

	resize(X, Y, W, H);
}

void Desktop::set_bg_color(int c, bool do_redraw) {
	if(color() == c)
		return;

	dsett->color = c;
	color(c);
	if(do_redraw)
		redraw();
}

void Desktop::read_config(void) {
	edelib::Config conf;
	if(!conf.load(CONFIG_NAME)) {
		EWARNING(ESTRLOC ": Can't load %s, using default...\n", CONFIG_NAME);
		return;
	}

	/*
	 * TODO:
	 * Add IconArea[X,Y,W,H] so icons can live
	 * inside that area only (aka margins).
	 */
	int  default_bg_color = FL_BLUE;
	int  default_wp_use   = false;
	char wpath[256];

	conf.get("Desktop", "Color", dsett->color, default_bg_color);

	if(conf.error() != edelib::CONF_ERR_SECTION) {
		conf.get("Desktop", "WallpaperUse", dsett->wp_use, default_wp_use);
		conf.get("Desktop", "Wallpaper", wpath, sizeof(wpath));

		// keep path but disable wallpaper if file does not exists
		if(!edelib::file_exists(wpath)) {
			EDEBUG(ESTRLOC ": %s as wallpaper does not exists\n", wpath);
			dsett->wp_use = false;
		}
	} else {
		// color is already filled
		dsett->wp_use = default_wp_use;
	}

	conf.get("Icons", "Label Background", gisett.label_background, FL_BLUE);
	conf.get("Icons", "Label Foreground", gisett.label_foreground, FL_WHITE);
	conf.get("Icons", "Label Fontsize",   gisett.label_fontsize, 12);
	conf.get("Icons", "Label Maxwidth",   gisett.label_maxwidth, 75);
	conf.get("Icons", "Label Transparent",gisett.label_transparent, false);
	conf.get("Icons", "Label Visible",    gisett.label_draw, true);
	conf.get("Icons", "OneClickExec",     gisett.one_click_exec, false);
	conf.get("Icons", "AutoArrange",      gisett.auto_arrange, true);

	if(dsett->wp_use)
		wallpaper->set(wpath);
}

void Desktop::install_watch(const char* path) {
	edelib::DirWatch::init();
	edelib::DirWatch::callback(dir_watch_cb);

	if(!edelib::DirWatch::add(path,
				edelib::DW_CREATE | edelib::DW_MODIFY | edelib::DW_RENAME | edelib::DW_DELETE)) {

		EWARNING(ESTRLOC ": Can't setup watch on %s\n", path);
		edelib::DirWatch::shutdown();
	}
}

void Desktop::load_icons(const char* path) {
	EASSERT(path != NULL);
	edelib::Config conf, *conf_ptr = NULL;

	if(!conf.load(ICONS_CONFIG_NAME))
		EWARNING(ESTRLOC ": Can't load icons positions from %s, randomizing them...\n", ICONS_CONFIG_NAME);
	else 
		conf_ptr = &conf;

	StringList lst;

	// list with full path; icon basename is extracted in add_icon_pathed()
	if(!dir_list(path, lst, true)) {
		EDEBUG(ESTRLOC ": Can't read %s\n", path);
		return;
	}

	StringListIter it, it_end;
	for(it = lst.begin(), it_end = lst.end(); it != it_end; ++it)
		add_icon_pathed((*it).c_str(), conf_ptr);
}

bool Desktop::add_icon_pathed(const char* path, edelib::Config* conf) {
	EASSERT(path != NULL);

	IconSettings is;
	bool can_add = false;
	const char* base = get_basename(path);

	/*
	 * see is possible .desktop file; icon, name fields are filled from read_desktop_file()
	 *
	 * FIXME: maybe the good thing is to use MimeType to check .desktop; 
	 * extension does not have to be always present, or .desktop does not have to be
	 * Desktop File at all
	 */
	if(edelib::str_ends(path, ".desktop")) {
		if(read_desktop_file(path, is))
			can_add = true;
	} else {
		edelib::MimeType mt;

		// then try to figure out it's mime; if fails, ignore it
		if(mt.set(path)) {
			EDEBUG(ESTRLOC ": Loading icon as mime-type %s\n", mt.icon_name().c_str());
			is.icon = mt.icon_name();
			// icon label path's basename
			is.name = base;
			is.type = ICON_FILE;

			can_add = true;
		} else {
			EDEBUG(ESTRLOC ": Failed mime-type for %s, ignoring...\n", path);
			can_add = false;
		}
	}

	if(can_add) {
		/*
		 * key_name is section in config file with icon X/Y values
		 * FIXME: this should be named 'section_name'
		 */
		is.key_name = base;

		// random_pos() is used if X/Y keys are not found
		int icon_x = random_pos(w() - 10);
		int icon_y = random_pos(w() - 10);

		if(conf) {
			conf->get(base, "X", icon_x, icon_x);
			conf->get(base, "Y", icon_y, icon_y);
		}

		EDEBUG(ESTRLOC ": %s found with: %i %i\n", base, icon_x, icon_y);
		is.x = icon_x;
		is.y = icon_y;
		is.full_path = path;

		DesktopIcon* dic = new DesktopIcon(&gisett, &is, dsett->color);
		add_icon(dic);
	}

	return can_add;
}

void Desktop::save_icons(void) {
	edelib::Config conf;
	char* icon_base;
	DesktopIconListIter it = icons.begin(), it_end = icons.end();

	for(; it != it_end; ++it) {
		if((*it)->path().empty())
			continue;

		icon_base = get_basename((*it)->path().c_str());
		conf.set(icon_base, "X", (*it)->x());
		conf.set(icon_base, "Y", (*it)->y());
	}

	if(!conf.save(ICONS_CONFIG_NAME))
		EWARNING(ESTRLOC ": Unable to store icons positions\n");
}

DesktopIcon* Desktop::find_icon_pathed(const char* path) {
	EASSERT(path != NULL);

	if(icons.empty())
		return NULL;

	DesktopIconListIter it, it_end;
	for(it = icons.begin(), it_end = icons.end(); it != it_end; ++it) {
		if((*it)->path() == path)
			return (*it);
	}

	return NULL;
}

bool Desktop::remove_icon_pathed(const char* path) {
	EASSERT(path != NULL);

	if(icons.empty())
		return false;

	DesktopIconListIter it, it_end;
	bool found = false;

	for(it = icons.begin(), it_end = icons.end(); it != it_end; ++it) {
		if((*it)->path() == path) {
			found = true;
			break;
		}
	}

	if(found) {
		DesktopIcon* ic = (*it);
		icons.erase(it);
		// Fl_Group::remove() does not delete child, just pops it out
		remove(ic);
		delete ic;
	}

	return found;
}

// read .desktop files
bool Desktop::read_desktop_file(const char* path, IconSettings& is) {
	EASSERT(path != NULL);

	if(!edelib::file_exists(path)) {
		EDEBUG(ESTRLOC ": %s don't exists\n");
		return false;
	}

	edelib::DesktopFile dconf;
	if(!dconf.load(path)) {
		EWARNING(ESTRLOC ": Can't read %s (%s)\n", path, dconf.strerror());
		return false;
	}

	char buff[128];
	int buffsz = sizeof(buff);

	/*
	 * Check for 'EmptyIcon' key since via it is determined
	 * is icon trash type or not (in case of trash, 'Icon' key is used for full trash).
	 * FIXME: any other way to check for trash icons ???
	 */
	if(dconf.get("Desktop Entry", "EmptyIcon", buff, buffsz)) {
		is.type = ICON_TRASH;
		is.icon = buff;
	} else
		is.type = ICON_NORMAL;
	
	if(!dconf.icon(buff, buffsz)) {
		EWARNING(ESTRLOC ": No Icon key, balling out\n");
		return false;
	}

	if(is.type == ICON_TRASH)
		is.icon2 = buff;
	else
		is.icon = buff;

	edelib::DesktopFileType dtype = dconf.type();
	if(dtype == edelib::DESK_FILE_TYPE_LINK) {
		is.cmd_is_url = true;
		dconf.url(buff, buffsz);
	}
	else {
		is.cmd_is_url = false;
		dconf.exec(buff, buffsz);
	}

	is.cmd = buff;

	if(!dconf.name(buff, buffsz)) {
		EDEBUG(ESTRLOC ": No Name key\n");
		is.name = "(none)";
	} else
		is.name = buff;

	return true;
}

void Desktop::add_icon(DesktopIcon* ic) {
	EASSERT(ic != NULL);

	icons.push_back(ic);
	add((Fl_Widget*)ic);
}

void Desktop::unfocus_all(void) { 
	DesktopIcon* ic;
	DesktopIconListIter it, it_end;

	for(it = icons.begin(), it_end = icons.end(); it != it_end; ++it) {
		ic = (*it);

		if(ic->is_focused()) {
			ic->do_unfocus();
			ic->fast_redraw();
		}
	}
}

void Desktop::select(DesktopIcon* ic, bool do_redraw) { 
	EASSERT(ic != NULL);

	if(in_selection(ic))
		return;

	selectionbuff.push_back(ic);

	if(!ic->is_focused()) {
		ic->do_focus();

		if(do_redraw)
			ic->fast_redraw();
	}
}

void Desktop::select_only(DesktopIcon* ic) { 
	EASSERT(ic != NULL);

	unfocus_all();
	selectionbuff.clear();
	selectionbuff.push_back(ic);

	ic->do_focus();
	ic->fast_redraw();
}

bool Desktop::in_selection(const DesktopIcon* ic) { 
	EASSERT(ic != NULL);

	if(selectionbuff.empty())
		return false;

	DesktopIconListIter it, it_end;

	for(it = selectionbuff.begin(), it_end = selectionbuff.end(); it != it_end; ++it) {
		if((*it) == ic)
			return true;
	}

	return false;
}

void Desktop::move_selection(int x, int y, bool apply) { 
	if(selectionbuff.empty())
		return;

	int prev_x, prev_y, tmp_x, tmp_y;
	DesktopIcon* ic;

	DesktopIconListIter it, it_end;

	for(it = selectionbuff.begin(), it_end = selectionbuff.end(); it != it_end; ++it) {
		ic = (*it);

		prev_x = ic->drag_icon_x();
		prev_y = ic->drag_icon_y();

		tmp_x = x - selection_x;
		tmp_y = y - selection_y;

		ic->drag(prev_x + tmp_x, prev_y + tmp_y, apply);

		// very slow if not checked
		//if(apply == true)
		//	ic->fast_redraw();
	}

	selection_x = x;
	selection_y = y;

	/*
	 * Redraw whole screen so it reflects
	 * new icon position
	 */
	if(apply)
		redraw();
}

/*
 * Tries to figure out icon below mouse. It is alternative to
 * Fl::belowmouse() since with this we hunt only icons, not other
 * childs (wallpaper, menu), which can be returned by Fl::belowmouse()
 * and bad things be hapened.
 */
DesktopIcon* Desktop::below_mouse(int px, int py) {
	DesktopIcon* ic = NULL;
	DesktopIconListIter it, it_end;
	
	for(it = icons.begin(), it_end = icons.end(); it != it_end; ++it) {
		ic = (*it);
		if(ic->x() < px && ic->y() < py && px < (ic->x() + ic->h()) && py < (ic->y() + ic->h()))
			return ic;
	}

	return NULL;
}

void Desktop::select_in_area(void) {
	if(!selbox->show)
		return;

	int ax = selbox->x;
	int ay = selbox->y;
	int aw = selbox->w;
	int ah = selbox->h;

	if(aw < 0) {
		ax += aw;
		aw = -aw;
	} else if(!aw)
		aw = 1;

	if(ah < 0) {
		ay += ah;
		ah = -ah;
	} else if(!ah)
		ah = 1;

	/*
	 * XXX: This function can fail since icon coordinates are absolute (event_x_root)
	 * but selbox use relative (event_root). It will work as expected if desktop is at x=0 y=0.
	 * This should be checked further.
	 */
	DesktopIcon* ic = NULL;
	DesktopIconListIter it, it_end;

	for(it = icons.begin(), it_end = icons.end(); it != it_end; ++it) {
		ic = (*it);
		EASSERT(ic != NULL && "Impossible to happen");

		if(intersects(ax, ay, ax+aw, ay+ah, ic->x(), ic->y(), ic->w()+ic->x(), ic->h()+ic->y())) {
			if(!ic->is_focused()) {
				ic->do_focus();
				// updated from Desktop::draw()
				ic->damage(EDAMAGE_CHILD_LABEL);
			}
		} else {
			if(ic->is_focused()) {
				ic->do_unfocus();
				// updated from Desktop::draw()
				ic->fast_redraw();
				//ic->damage(EDAMAGE_CHILD_LABEL);
				//ic->redraw();
				//ic->damage(EDAMAGE_CHILD_LABEL);
			}
		}
	}
}

void Desktop::notify_box(const char* msg, bool copy) {
	if(!msg)
		return;

	if(copy)
		notify->copy_label(msg);
	else	
		notify->label(msg);

	if(!notify->shown())
		notify->show();
}

void Desktop::notify_box_color(Fl_Color col) {
	notify->color(col);
}

void Desktop::notify_desktop_changed(void) {
	int num = net_get_current_desktop();
	if(num == -1)
		return;

	char** names;
	int ret = net_get_workspace_names(names);
	if(!ret)
		return;

	if(num >= ret) {
		XFreeStringList(names);
		return;
	}

	notify_box(names[num], true);
	XFreeStringList(names);
}

void Desktop::drop_source(const char* src, int src_len, int x, int y) {
	if(!src)
		return;

	if(src_len < 1)
		return;

	char* src_copy = new char[src_len + 1];
	int real_len = 0;

	// mozilla sends UTF-16 form; for now use this hack untill Utf8.cpp code is ready
	for(int i = 0, j = 0; i < src_len; i++) {
		if(src[i] != 0) {
			src_copy[j++] = src[i];
			real_len++;
		}
	}
	src_copy[real_len] = '\0';

	if(real_len < 1) {
		delete [] src_copy;
		return;
	}

	EDEBUG(ESTRLOC ": DND on Desktop, got: %s\n", src_copy);

	// valid url's (if got) ends with \r\n, clean that
	char* pp = strstr(src_copy, "\r\n");
	if(pp)
		*pp = '\0';

	char* sptr = 0;
	if((real_len > 7) && (strncmp(src_copy, "file://", 7) == 0))
		sptr = src_copy + 7;
	else
		sptr = src_copy;

	if(!edelib::file_exists(sptr) && !edelib::dir_exists(sptr)) {
		fl_message("Droping file content is not implemented yet. Soon, soon... :)");
		delete [] src_copy;
		return;
	}

	IconSettings is;
	is.x = x;
	is.y = y;

	bool is_read = false;

	if(edelib::str_ends(src_copy, ".desktop")) {
		edelib::DesktopFile dconf;

		edelib::String path = sptr;
		edelib::DesktopFile dfile;

		if(dfile.load(path.c_str())) {
			char buff[256];
			if(dfile.type() == edelib::DESK_FILE_TYPE_LINK) {
				dfile.url(buff, 256);
				is.cmd_is_url = true;
			}
			else {
				dfile.exec(buff, 256);
				is.cmd_is_url = false;
			}
			is.cmd = buff;
			is.type = ICON_NORMAL;

			dfile.name(buff, 256);
			is.name = buff;
			dfile.icon(buff, 256);
			is.icon = buff;

			is_read = true;
		}
	}

	if(!is_read) {
		// absolute path is (for now) seen as non-url
		if(sptr[0] == '/')
			is.cmd_is_url = false;
		else
			is.cmd_is_url = true;

		is.cmd = "(none)";
		is.type = ICON_NORMAL;

		edelib::MimeType mt;
		if(!mt.set(sptr)) {
			EWARNING(ESTRLOC ": MimeType for %s (%s) failed, not dropping icon\n", sptr, src_copy);
			delete [] src_copy;
			return;
		}

		is.name = get_basename(sptr);
		is.icon = mt.icon_name();
	}

	DesktopIcon* dic = new DesktopIcon(&gisett, &is, color());
	add_icon(dic);

	delete [] src_copy;

	redraw();
}

void Desktop::draw(void) {
	if(!damage())
		return;

	if(damage() & (FL_DAMAGE_ALL | FL_DAMAGE_EXPOSE)) {
		/*
		 * If any overlay was previously visible during full
		 * redraw, it will not be cleared because of fast flip.
		 * This will assure that does not happened.
		 */
		clear_xoverlay();

		DESKTOP_WINDOW::draw();
		EDEBUG(ESTRLOC ": REDRAW ALL\n");
	}

	if(damage() & EDAMAGE_OVERLAY) {
		if(selbox->show)
			draw_xoverlay(selbox->x, selbox->y, selbox->w, selbox->h);
		else
			clear_xoverlay();

		/*
		 * now scan all icons and see if they needs redraw, and if do
		 * just update their label since it is indicator of selection
		 */
		for(int i = 0; i < children(); i++) {
			if(child(i)->damage() == EDAMAGE_CHILD_LABEL) {
				update_child(*child(i));
				child(i)->clear_damage();
				EDEBUG(ESTRLOC ": ICON REDRAW \n");
			}
		}
	}

	clear_damage();
}

void Desktop::dir_watch(const char* dir, const char* changed, int flags) {
	if(!do_dirwatch || !changed || flags == edelib::DW_REPORT_NONE)
		return;

	/*
	 * Check first we don't get any temporary files (starting with '.'
	 * or ending with '~', like vim does when editing file). For now these
	 * are only conditions, but I will probably add them more when issues occured.
	 *
	 * FIXME: use strcmp() family ?
	 */
	edelib::String tmp(changed);
	if(tmp.empty() || tmp[0] == '.' || tmp[tmp.length()-1] == '~')
		return;

	sleep(1);

	if(flags == edelib::DW_REPORT_CREATE) {
		EDEBUG(ESTRLOC ": adding %s\n", changed);

		if(find_icon_pathed(changed)) {
			EDEBUG(ESTRLOC ": %s already registered; skipping...\n", changed);
			return;
		}

		/*
		 * Uh; looks like kernel report event faster than file is created
		 * (in some cases). This can be bad for .desktop files when are copied
		 * (eg. on my machine after 'cp Home.desktop foo.desktop', will fail
		 * to load foo.desktop since it's content is not fully copied). 
		 * Due that we stop for one sec (use usleep() ???)
		 */
		//sleep(1);

		if(add_icon_pathed(changed, 0))
			redraw();

	} else if(flags == edelib::DW_REPORT_MODIFY) {
		EDEBUG(ESTRLOC ": modified %s\n", changed);
	} else if(flags == edelib::DW_REPORT_DELETE) {
		EDEBUG(ESTRLOC ": deleted %s\n", changed);
		if(remove_icon_pathed(changed))
			redraw();
	} else
		EDEBUG(ESTRLOC ": %s changed with %i\n", changed, flags);
}

void Desktop::execute(const char* cmd) {
	EASSERT(cmd != NULL);
	edelib::run_program(cmd, false);
}

int Desktop::handle(int event) {
	switch(event) {
		case FL_FOCUS:
		case FL_UNFOCUS:
		case FL_SHORTCUT:
			return 1;

		case FL_PUSH: {
			/*
			 * First check where we clicked. If we do it on desktop
			 * unfocus any possible focused childs, and handle
			 * specific clicks. Otherwise, do rest for childs.
			 */
			Fl_Widget* clicked = Fl::belowmouse();
			
			if(NOT_SELECTABLE(clicked)) {
				EDEBUG(ESTRLOC ": DESKTOP CLICK !!!\n");
				if(!selectionbuff.empty()) {
					/*
					 * Only focused are in selectionbuff, so this is 
					 * fine to do; also will prevent full redraw when is clicked on desktop
					 */
					unfocus_all();
					selectionbuff.clear();
				}

				// track position so moving can be deduced
				if(Fl::event_button() == 1) {
					selbox->x = Fl::event_x();
					selbox->y = Fl::event_y();
				} else if(Fl::event_button() == 3) {
					const Fl_Menu_Item* item = dmenu->menu()->popup(Fl::event_x(), Fl::event_y());
					dmenu->picked(item);
				}

				return 1;
			}

			// from here, all events are managed for icons
			DesktopIcon* tmp_icon = (DesktopIcon*)clicked;

			/*
			 * do no use assertion on this, since
			 * fltk::belowmouse() can miss our icon
			 */
			if(!tmp_icon)
				return 1;

			if(SELECTION_MULTI) {
				Fl::event_is_click(0);
				select(tmp_icon);
				return 1;
			} else if(SELECTION_SINGLE) {
				if(!in_selection(tmp_icon)) {
					/*
					 * for testing:
					 * notify_box(tmp_icon->label());
					 */
					select_only(tmp_icon);
				}
			} else if(Fl::event_button() == 3) {
				select_only(tmp_icon);
				/*
				 * for testing:
				 * notify_box(tmp_icon->label());
				 */
			}

			/* 
			 * Let child handle the rest.
			 * Also prevent click on other mouse buttons during move.
			 */
			if(!moving)
				tmp_icon->handle(FL_PUSH);

			EDEBUG(ESTRLOC ": FL_PUSH from desktop\n");
			selection_x = Fl::event_x_root();
			selection_y = Fl::event_y_root();
	
			return 1;
		 }

		case FL_DRAG:
			moving = true;
			if(!selectionbuff.empty()) {
				EDEBUG(ESTRLOC ": DRAG icon from desktop\n");
				move_selection(Fl::event_x_root(), Fl::event_y_root(), false);
			} else {
				EDEBUG(ESTRLOC ": DRAG from desktop\n");
				/*
				 * Moving is started with pushed button.
				 * From this point selection box is created and is rolled until release
				 */
				if(selbox->x != 0 || selbox->y != 0) {
					selbox->w = Fl::event_x() - selbox->x;
					selbox->h = Fl::event_y() - selbox->y;

					selbox->show = true;

					// see if there some icons inside selection area
					select_in_area();

					// redraw selection box
					damage(EDAMAGE_OVERLAY);
				}
			}

			return 1;

		case FL_RELEASE:
			EDEBUG(ESTRLOC ": RELEASE from desktop\n");
			EDEBUG(ESTRLOC ": clicks: %i\n", Fl::event_is_click());

			if(selbox->show) {
				selbox->x = selbox->y = selbox->w = selbox->h = 0;
				selbox->show = false;
				damage(EDAMAGE_OVERLAY);

				/*
				 * Now pickup those who are in is_focused() state.
				 *
				 * Possible flickers due overlay will be later removed when is
				 * called move_selection(), which will in turn redraw icons again
				 * after position them.
				 */
				if(!selectionbuff.empty())
					selectionbuff.clear();

				DesktopIconListIter it, it_end;
				for(it = icons.begin(), it_end = icons.end(); it != it_end; ++it) {
					if((*it)->is_focused())
						select((*it), false);
				}

				return 1;
			}

			if(!selectionbuff.empty() && moving)
				move_selection(Fl::event_x_root(), Fl::event_y_root(), true);

			/* 
			 * Do not send FL_RELEASE during move
			 *
			 * TODO: should be alowed FL_RELEASE to multiple icons? (aka. run 
			 * command for all selected icons ?
			 */
			if(selectionbuff.size() == 1 && !moving)
				(*selectionbuff.begin())->handle(FL_RELEASE);

			moving = false;
			return 1;

		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_LEAVE:
			return 1;

		case FL_DND_RELEASE: {
			DesktopIcon* di = below_mouse(Fl::event_x(), Fl::event_y());
			if(di)
				return di->handle(event);
			return 1;
		}

		case FL_PASTE: {
			DesktopIcon* di = below_mouse(Fl::event_x(), Fl::event_y());
			if(di)
				return di->handle(event);

			drop_source(Fl::event_text(), Fl::event_length(), Fl::event_x(), Fl::event_y());
			return 1;
		}

		case FL_ENTER:
		case FL_LEAVE:
		case FL_MOVE:
			return DESKTOP_WINDOW::handle(event);

		default:
			break;
	}

	return 0;
}

int main() {
	signal(SIGTERM, exit_signal);
	signal(SIGKILL, exit_signal);
	signal(SIGINT,  exit_signal);
	signal(SIGHUP,  restart_signal);

	srand(time(NULL));

	// a lot of preparing code depends on this
	fl_open_display();

#ifndef USE_EDELIB_WINDOW
	fl_register_images();
	edelib::IconTheme::init("edeneu");
#endif

	Desktop::init();
	Desktop::instance()->show();

	/*
	 * XSelectInput will redirect PropertyNotify messages, which
	 * are listened for
	 */
	XSelectInput(fl_display, RootWindow(fl_display, fl_screen), PropertyChangeMask | StructureNotifyMask | ClientMessage);

	Fl::add_handler(desktop_xmessage_handler);

	while(running) 
		Fl::wait();

	Desktop::shutdown();

#ifndef USE_EDELIB_WINDOW
	edelib::IconTheme::shutdown();
#endif
	return 0;
}
