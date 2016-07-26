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

#include <assert.h>

#include <ObjectivelyMVC/Checkbox.h>

#include "CvarCheckboxInput.h"

#define _Class _CvarCheckboxInput

#pragma mark - CvarCheckboxInput

/**
 * @brief ActionFunction for the Checkbox.
 */
static void action(Control *control, const SDL_Event *event, ident sender, ident data) {

	const CvarCheckboxInput *this = (CvarCheckboxInput *) sender;

	Cvar_SetValue(this->var->name, control->state & ControlStateSelected);
}

/**
 * @fn CvarCheckboxInput *CvarCheckboxInput::initWithVariable(CvarCheckboxInput *self, cvar_t *var, const char *name)
 *
 *
 * @memberof CvarCheckboxInput
 */
static CvarCheckboxInput *initWithVariable(CvarCheckboxInput *self, cvar_t *var, const char *name) {
	
	Control *control = (Control *) $(alloc(Checkbox), initWithFrame, NULL, ControlStyleDefault);

	Label *label = $(alloc(Label), initWithText, name, NULL);
	label->view.frame.w = CVAR_CHECKBOX_INPUT_LABEL_WIDTH;

	self = (CvarCheckboxInput *) super(Input, self, initWithOrientation, InputOrientationLeft, control, label);
	if (self) {

		self->var = var;
		assert(self->var);
	}

	control->state = var->integer ? ControlStateSelected : ControlStateDefault;

	$(control, addActionForEventType, SDL_MOUSEBUTTONUP, action, self, NULL);
	
	return self;
}

/**
 * @fn void CvarCheckboxInput::input(View *view, cvar_t *var, const char *name)
 *
 * @memberof CvarCheckboxInput
 */
static void input(View *view, cvar_t *var, const char *name) {

	assert(view);
	
	CvarCheckboxInput *input = $(alloc(CvarCheckboxInput), initWithVariable, var, name);
	assert(input);

	$(view, addSubview, (View *) input);
	release(input);
}

#pragma mark - Class lifecycle

/**
 * @see Class::initialize(Class *)
 */
static void initialize(Class *clazz) {

	((CvarCheckboxInputInterface *) clazz->interface)->initWithVariable = initWithVariable;

	((CvarCheckboxInputInterface *) clazz->interface)->input = input;
}

Class _CvarCheckboxInput = {
	.name = "CvarCheckboxInput",
	.superclass = &_Input,
	.instanceSize = sizeof(CvarCheckboxInput),
	.interfaceOffset = offsetof(CvarCheckboxInput, interface),
	.interfaceSize = sizeof(CvarCheckboxInputInterface),
	.initialize = initialize,
};

#undef _Class

