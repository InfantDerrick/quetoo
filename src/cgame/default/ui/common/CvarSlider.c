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

#include "CvarSlider.h"

#define _Class _CvarSlider

#pragma mark - View

/**
 * @see View::awakeWithDictionary(View *, const Dictionary *)
 */
static void awakeWithDictionary(View *self, const Dictionary *dictionary) {

	super(View, self, awakeWithDictionary, dictionary);

	CvarSlider *this = (CvarSlider *) self;

	const Inlet inlets[] = MakeInlets(
		MakeInlet("var", InletTypeApplicationDefined, &this->var, Cg_BindCvar)
	);

	$(self, bind, inlets, dictionary);
}

/**
 * @see View::init(View *)
 */
static View *init(View *self) {
	return (View *) $((CvarSlider *) self, initWithVariable, NULL, 0.0, 0.0, 0.0);
}

/**
 * @see View::updateBindings(View *)
 */
static void updateBindings(View *self) {

	super(View, self, updateBindings);

	CvarSlider *this = (CvarSlider *) self;

	if (this->slider.step >= 1.0) {
		$((Slider *) this, setLabelFormat, "%g");
	}

	if (this->var) {
		$((Slider *) this, setValue, this->var->value);
	}
}

#pragma mark - Slider

/**
 * @see Slider::setValue(Slider *, double)
 */
static void setValue(Slider *self, double value) {

	super(Slider, self, setValue, value);

	const CvarSlider *this = (CvarSlider *) self;
	if (this->var) {
		cgi.SetCvarValue(this->var->name, value);
	}
}

#pragma mark - CvarSlider

/**
 * @fn CvarSlider *CvarSlider::initWithVariable(CvarSlider *self, cvar_t *var, double min, double max, double step)
 *
 * @memberof CvarSlider
 */
static CvarSlider *initWithVariable(CvarSlider *self, cvar_t *var, double min, double max, double step) {

	self = (CvarSlider *) super(Slider, self, initWithFrame, NULL);
	if (self) {

		self->var = var;

		Slider *this = (Slider *) self;

		this->min = min;
		this->max = max;
		this->step = step;
		this->snapToStep = true;
	}

	return self;
}

#pragma mark - Class lifecycle

/**
 * @see Class::initialize(Class *)
 */
static void initialize(Class *clazz) {

	((ViewInterface *) clazz->interface)->awakeWithDictionary = awakeWithDictionary;
	((ViewInterface *) clazz->interface)->init = init;
	((ViewInterface *) clazz->interface)->updateBindings = updateBindings;

	((SliderInterface *) clazz->interface)->setValue = setValue;

	((CvarSliderInterface *) clazz->interface)->initWithVariable = initWithVariable;
}

/**
 * @fn Class *CvarSlider::_CvarSlider(void)
 * @memberof CvarSlider
 */
Class *_CvarSlider(void) {
	static Class *clazz;
	static Once once;

	do_once(&once, {
		clazz = _initialize(&(const ClassDef) {
			.name = "CvarSlider",
			.superclass = _Slider(),
			.instanceSize = sizeof(CvarSlider),
			.interfaceOffset = offsetof(CvarSlider, interface),
			.interfaceSize = sizeof(CvarSliderInterface),
			.initialize = initialize,
		});
	});

	return clazz;
}

#undef _Class
