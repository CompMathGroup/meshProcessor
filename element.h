#ifndef __MESH3D__ELEMENT_H__
#define __MESH3D__ELEMENT_H__

#include "common.h"

/** Represents mesh element. Every mesh element has index and color */
class element {
	index _color;
	index _idx;
public:
	/** Construct element with invalid color and index */
	element() : _color(BAD_INDEX), _idx(BAD_INDEX) { }
	/** Get element index */
	index idx() const {
		return _idx;
	}
	/** Get element color */
	index color() const {
		return _color;
	}
	/** Set element index */
	void set_idx(index idx) {
		_idx = idx;
	}
	/** Set element color */
	void set_color(index color) {
		_color = color;
	}
};

#endif
