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

#pragma once

#include <ObjectivelyMVC/NavigationViewController.h>

#include "DialogView.h"
#include "PrimaryButton.h"

/**
 * @file
 *
 * @brief The MainViewController.
 */

typedef struct MainViewController MainViewController;
typedef struct MainViewControllerInterface MainViewControllerInterface;

/**
 * @brief The MainViewController type.
 * @extends MenuViewController
 * @ingroup ViewControllers
 */
struct MainViewController {

	/**
	 * @brief The superclass.
	 * @private
	 */
	NavigationViewController navigationViewController;

	/**
	 * @brief The interface.
	 * @private
	 */
	MainViewControllerInterface *interface;

	/*
	 * @brief The menu background images; logo and wallpaper
	 */
	View *decorationView;

	/*
	 * @brief The Quetoo logo.
	 */
	ImageView *logoImage;

	/*
	 * @brief The notification dialog.
	 */
	DialogView *dialog;

	/*
	 * @brief The bottom button bar.
	 */
	View *bottomBar;

	/*
	 * @brief The client's state.
	 */
	cl_state_t state;
};

/**
 * @brief The MainViewController interface.
 */
struct MainViewControllerInterface {

	/**
	 * @brief The superclass interface.
	 */
	NavigationViewControllerInterface navigationViewControllerInterface;

	/**
	 * @fn MainViewController *MainViewController::init(MainViewController *self)
	 * @brief Initializes this ViewController.
	 * @return The initialized MainViewController, or `NULL` on error.
	 * @memberof MainViewController
	 */
	MainViewController *(*init)(MainViewController *self);
};

/**
 * @fn Class *MainViewController::_MainViewController(void)
 * @brief The MainViewController archetype.
 * @return The MainViewController Class.
 * @memberof MainViewController
 */
extern Class *_MainViewController(void);
