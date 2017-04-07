/*
 * Copyright(c) 1997-2001 id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quetoo.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "cg_local.h"

#include "InterfaceViewController.h"

#include "CrosshairView.h"
#include "CvarSelect.h"
#include "CvarSlider.h"
#include "VideoModeSelect.h"

#define _Class _InterfaceViewController

#pragma mark - Crosshair selection

/**
 * @brief Fs_EnumerateFunc for crosshair selection.
 */
static void enumerateCrosshairs(const char *path, void *data) {
	char name[MAX_QPATH];

	StripExtension(Basename(path), name);

	intptr_t value = strtol(name + strlen("ch"), NULL, 10);
	assert(value);

	$((Select *) data, addOption, name, (ident) value);
}

/**
 * @brief ActionFunction for crosshair modification.
 */
static void modifyCrosshair(Control *control, const SDL_Event *event, ident sender, ident data) {

	InterfaceViewController *this = (InterfaceViewController *) sender;

	$((View *) this->crosshairView, updateBindings);
}

#pragma mark - ViewController

/**
 * @see ViewController::loadView(ViewController *)
 */
static void loadView(ViewController *self) {

	super(ViewController, self, loadView);

	TabViewController *this = (TabViewController *) self;

	StackView *columns = $(alloc(StackView), initWithFrame, NULL);

	columns->spacing = DEFAULT_PANEL_SPACING;

	columns->axis = StackViewAxisHorizontal;
	columns->distribution = StackViewDistributionFillEqually;

	columns->view.autoresizingMask = ViewAutoresizingFill;

	{
		StackView *column = $(alloc(StackView), initWithFrame, NULL);

		column->spacing = DEFAULT_PANEL_SPACING;

		{
			Box *box = $(alloc(Box), initWithFrame, NULL);
			$(box->label, setText, "Crosshair");

			box->view.autoresizingMask |= ViewAutoresizingWidth;

			StackView *stackView = $(alloc(StackView), initWithFrame, NULL);

			CvarSelect *crosshairSelect = $(alloc(CvarSelect), initWithVariable, cg_draw_crosshair);
			$((Select *) crosshairSelect, addOption, "", NULL);
			cgi.EnumerateFiles("pics/ch*", enumerateCrosshairs, crosshairSelect);

			$((Control *) crosshairSelect, addActionForEventType, SDL_MOUSEBUTTONUP, modifyCrosshair, self, NULL);
			Cg_Input((View *) stackView, "Crosshair", (Control *) crosshairSelect);

			CvarSelect *colorSelect = (CvarSelect *) $(alloc(CvarSelect), initWithVariable, cg_draw_crosshair_color);
			colorSelect->expectsStringValue = true;

			$((Select *) colorSelect, addOption, "default", (ident) 0);
			$((Select *) colorSelect, addOption, "red", (ident) 1);
			$((Select *) colorSelect, addOption, "green", (ident) 2);
			$((Select *) colorSelect, addOption, "yellow", (ident) 3);
			$((Select *) colorSelect, addOption, "orange", (ident) 4);

			$((Control *) colorSelect, addActionForEventType, SDL_MOUSEBUTTONUP, modifyCrosshair, self, NULL);
			Cg_Input((View *) stackView, "Crosshair color", (Control *) colorSelect);

			CvarSlider *scaleSlider = $(alloc(CvarSlider), initWithVariable, cg_draw_crosshair_scale, 0.1, 2.0, 0.1);

			$((Control *) scaleSlider, addActionForEventType, SDL_MOUSEMOTION, modifyCrosshair, self, NULL);
			Cg_Input((View *) stackView, "Crosshair scale", (Control *) scaleSlider);

			Cg_CvarCheckboxInput((View *) stackView, "Pulse on pickup", cg_draw_crosshair_pulse->name);

			const SDL_Rect frame = MakeRect(0, 0, 72, 72);

			((InterfaceViewController *) this)->crosshairView = $(alloc(CrosshairView), initWithFrame, &frame);
			((InterfaceViewController *) this)->crosshairView->view.alignment = ViewAlignmentMiddleCenter;

			$((View *) stackView, addSubview, (View *) ((InterfaceViewController *) this)->crosshairView);
			release(((InterfaceViewController *) this)->crosshairView);

			$((View *) box, addSubview, (View *) stackView);
			release(stackView);

			$((View *) column, addSubview, (View *) box);
			release(box);
		}

		$((View *) columns, addSubview, (View *) column);
		release(column);
	}

	{
		StackView *column = $(alloc(StackView), initWithFrame, NULL);

		column->spacing = DEFAULT_PANEL_SPACING;

		{
			Box *box = $(alloc(Box), initWithFrame, NULL);
			$(box->label, setText, "View");

			box->view.autoresizingMask |= ViewAutoresizingWidth;

			StackView *stackView = $(alloc(StackView), initWithFrame, NULL);

			// Field of view

			Cg_CvarSliderInput((View *) stackView, "FOV", cg_fov->name, 80.0, 130.0, 5.0);

			// Zoomed field of view

			Cg_CvarSliderInput((View *) stackView, "Zoom FOV", cg_fov_zoom->name, 20.0, 70.0, 5.0);

			// Zoom easing speed

			Cg_CvarSliderInput((View *) stackView, "Zoom interpolate", cg_fov_interpolate->name, 0.0, 2.0, 0.1);

			$((View *) box, addSubview, (View *) stackView);
			release(stackView);

			$((View *) column, addSubview, (View *) box);
			release(box);
		}

		{
			Box *box = $(alloc(Box), initWithFrame, NULL);
			$(box->label, setText, "Screen");

			box->view.autoresizingMask |= ViewAutoresizingWidth;

			StackView *stackView = $(alloc(StackView), initWithFrame, NULL);

			// Blending

			Cg_CvarCheckboxInput((View *) stackView, "Screen blending", cg_draw_blend->name);
			Cg_CvarCheckboxInput((View *) stackView, "Liquid blend", cg_draw_blend_liquid->name);
			Cg_CvarCheckboxInput((View *) stackView, "Pickup blend", cg_draw_blend_pickup->name);

			$((View *) box, addSubview, (View *) stackView);
			release(stackView);

			$((View *) column, addSubview, (View *) box);
			release(box);
		}

		{
			Box *box = $(alloc(Box), initWithFrame, NULL);
			$(box->label, setText, "Weapon");

			box->view.autoresizingMask |= ViewAutoresizingWidth;

			StackView *stackView = $(alloc(StackView), initWithFrame, NULL);

			// Weapon options

			Cg_CvarCheckboxInput((View *) stackView, "Draw weapon", cg_draw_weapon->name);
			Cg_CvarSliderInput((View *) stackView, "Weapon alpha", cg_draw_weapon_alpha->name, 0.1, 1.0, 0.1);

			$((View *) box, addSubview, (View *) stackView);
			release(stackView);

			$((View *) column, addSubview, (View *) box);
			release(box);
		}

		$((View *) columns, addSubview, (View *) column);
		release(column);
	}

	$((View *) this->panel->contentView, addSubview, (View *) columns);
	release(columns);
}

#pragma mark - Class lifecycle

/**
 * @see Class::initialize(Class *)
 */
static void initialize(Class *clazz) {

	((ViewControllerInterface *) clazz->def->interface)->loadView = loadView;
}

/**
 * @fn Class *InterfaceViewController::_InterfaceViewController(void)
 * @memberof InterfaceViewController
 */
Class *_InterfaceViewController(void) {
	static Class clazz;
	static Once once;

	do_once(&once, {
		clazz.name = "InterfaceViewController";
		clazz.superclass = _TabViewController();
		clazz.instanceSize = sizeof(InterfaceViewController);
		clazz.interfaceOffset = offsetof(InterfaceViewController, interface);
		clazz.interfaceSize = sizeof(InterfaceViewControllerInterface);
		clazz.initialize = initialize;
	});

	return &clazz;
}

#undef _Class
