/* -*- mode: C; c-basic-offset: 2 -*-
 *
 * Copyright © 2003,2010 James Henstridge, Steven Chaplin
 *
 * This file is part of pycairo.
 *
 * Pycairo is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Pycairo is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with pycairo. If not, see <http://www.gnu.org/licenses/>.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "config.h"
#include "private.h"


/* PycairoMatrix_FromMatrix
 * Create a new PycairoMatrix from a cairo_matrix_t
 * matrix - a cairo_matrix_t to 'wrap' into a Python object.
 *          the cairo_matrix_t values are copied.
 * Return value: New reference or NULL on failure
 */
PyObject *
PycairoMatrix_FromMatrix (const cairo_matrix_t *matrix) {
  PyObject *o;
  assert (matrix != NULL);
  o = PycairoMatrix_Type.tp_alloc (&PycairoMatrix_Type, 0);
  if (o != NULL)
    ((PycairoMatrix *)o)->matrix = *matrix; // copy struct
  return o;
}

static void
matrix_dealloc (PycairoMatrix *o) {
  //o->ob_type->tp_free((PyObject *)o);
  Py_TYPE(o)->tp_free(o);
}

static PyObject *
matrix_new (PyTypeObject *type, PyObject *args, PyObject *kwds) {
  cairo_matrix_t mx;
  static char *kwlist[] = { "xx", "yx", "xy", "yy", "x0", "y0", NULL };
  double xx = 1.0, yx = 0.0, xy = 0.0, yy = 1.0, x0 = 0.0, y0 = 0.0;

  if (!PyArg_ParseTupleAndKeywords(args, kwds,
				   "|dddddd:Matrix.__init__", kwlist,
				   &xx, &yx, &xy, &yy, &x0, &y0))
    return NULL;

  cairo_matrix_init (&mx, xx, yx, xy, yy, x0, y0);
  return PycairoMatrix_FromMatrix (&mx);
}

static PyObject *
matrix_init_rotate (PyTypeObject *type, PyObject *args) {
  cairo_matrix_t matrix;
  double radians;

  if (!PyArg_ParseTuple(args, "d:Matrix.init_rotate", &radians))
    return NULL;

  cairo_matrix_init_rotate (&matrix, radians);
  return PycairoMatrix_FromMatrix (&matrix);
}

static PyObject *
matrix_invert (PycairoMatrix *o) {
  if (Pycairo_Check_Status (cairo_matrix_invert (&o->matrix)))
    return NULL;
  Py_RETURN_NONE;
}

/* cairo_matrix_multiply */
static PyObject *
matrix_multiply (PycairoMatrix *o, PyObject *args) {
  PycairoMatrix *mx2;
  cairo_matrix_t result;

  if (!PyArg_ParseTuple(args, "O!:Matrix.multiply",
			&PycairoMatrix_Type, &mx2))
    return NULL;

  cairo_matrix_multiply (&result, &o->matrix, &mx2->matrix);
  return PycairoMatrix_FromMatrix (&result);
}

/* standard matrix multiply, for use by '*' operator */
static PyObject *
matrix_operator_multiply (PycairoMatrix *o, PycairoMatrix *o2) {
  cairo_matrix_t result;
  cairo_matrix_multiply (&result, &o->matrix, &o2->matrix);
  return PycairoMatrix_FromMatrix (&result);
}

static PyObject *
matrix_repr (PycairoMatrix *o) {
  char buf[256];

  PyOS_snprintf(buf, sizeof(buf), "cairo.Matrix(%g, %g, %g, %g, %g, %g)",
		o->matrix.xx, o->matrix.yx,
		o->matrix.xy, o->matrix.yy,
		o->matrix.x0, o->matrix.y0);
  return PyUnicode_FromString(buf);
}

static PyObject *
matrix_richcmp (PycairoMatrix *m1, PycairoMatrix *m2, int op) {
  int equal;
  PyObject *ret;
  cairo_matrix_t *mx1 = &m1->matrix;
  cairo_matrix_t *mx2 = &m2->matrix;

  if (!PyObject_TypeCheck(m2, &PycairoMatrix_Type) ||
      !(op == Py_EQ || op == Py_NE)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  equal = mx1->xx == mx2->xx && mx1->yx == mx2->yx &&
    mx1->xy == mx2->xy && mx1->yy == mx2->yy &&
    mx1->x0 == mx2->x0 && mx1->y0 == mx2->y0;

  if (op == Py_EQ)
    ret = equal ? Py_True : Py_False;
  else
    ret = equal ? Py_False : Py_True;
  Py_INCREF(ret);
  return ret;
}

static PyObject *
matrix_rotate (PycairoMatrix *o, PyObject *args) {
  double radians;

  if (!PyArg_ParseTuple(args, "d:Matrix.rotate", &radians))
    return NULL;

  cairo_matrix_rotate (&o->matrix, radians);
  Py_RETURN_NONE;
}

static PyObject *
matrix_scale (PycairoMatrix *o, PyObject *args) {
  double sx, sy;

  if (!PyArg_ParseTuple(args, "dd:Matrix.scale", &sx, &sy))
    return NULL;

  cairo_matrix_scale (&o->matrix, sx, sy);
  Py_RETURN_NONE;
}

static PyObject *
matrix_transform_distance (PycairoMatrix *o, PyObject *args) {
  double dx, dy;

  if (!PyArg_ParseTuple(args, "dd:Matrix.transform_distance", &dx, &dy))
    return NULL;

  cairo_matrix_transform_distance (&o->matrix, &dx, &dy);
  return Py_BuildValue("(dd)", dx, dy);
}

static PyObject *
matrix_transform_point (PycairoMatrix *o, PyObject *args) {
  double x, y;

  if (!PyArg_ParseTuple(args, "dd:Matrix.transform_point", &x, &y))
    return NULL;

  cairo_matrix_transform_point (&o->matrix, &x, &y);
  return Py_BuildValue("(dd)", x, y);
}

static PyObject *
matrix_translate (PycairoMatrix *o, PyObject *args) {
  double tx, ty;

  if (!PyArg_ParseTuple(args, "dd:Matrix.translate", &tx, &ty))
    return NULL;

  cairo_matrix_translate (&o->matrix, tx, ty);
  Py_RETURN_NONE;
}

static PyObject *
matrix_item (PycairoMatrix *o, Py_ssize_t i) {
  switch (i) {
  case 0:
    return Py_BuildValue("d", o->matrix.xx);
  case 1:
    return Py_BuildValue("d", o->matrix.yx);
  case 2:
    return Py_BuildValue("d", o->matrix.xy);
  case 3:
    return Py_BuildValue("d", o->matrix.yy);
  case 4:
    return Py_BuildValue("d", o->matrix.x0);
  case 5:
    return Py_BuildValue("d", o->matrix.y0);
  default:
    PyErr_SetString(PyExc_IndexError, "Matrix index out of range");
    return NULL;
  }
}

static PyNumberMethods matrix_as_number = {
  0,                                     /* nb_add*/
  0,                                     /* nb_subtract*/
  (binaryfunc)matrix_operator_multiply,  /* nb_multiply*/
  0,    	   			 /* nb_remainder*/
  0,    	   			 /* nb_divmod*/
  0,    	   			 /* nb_power*/
  0,    	   			 /* nb_negative*/
  0,    	   			 /* nb_positive*/
  0,    	   			 /* nb_absolute*/
  0,    	   			 /* nb_bool*/
  0,    	   			 /* nb_invert*/
  0,    	   			 /* nb_lshift*/
  0,    	   			 /* nb_rshift*/
  0,    	   			 /* nb_and*/
  0,    	   			 /* nb_xor*/
  0,    	   			 /* nb_or*/
  0,    	   			 /* nb_int*/
  0,    	   			 /* nb_reserved*/
  0,    	   			 /* nb_float*/
  0,		   			 /* nb_inplace_add*/
  0,		   			 /* nb_inplace_subtract*/
  0,		   			 /* nb_inplace_multiply*/
  0,		   			 /* nb_inplace_divide*/
  0,		   			 /* nb_inplace_remainder*/
  0,		   			 /* nb_inplace_power*/
  0,		   			 /* nb_inplace_lshift*/
  0,		   			 /* nb_inplace_rshift*/
  0,		   			 /* nb_inplace_and*/
  0,		   			 /* nb_inplace_xor*/
  0,		   			 /* nb_inplace_or*/
  0,               			 /* nb_floor_divide */
  0,	           			 /* nb_true_divide */
  0,		   			 /* nb_inplace_floor_divide */
  0,		   			 /* nb_inplace_true_divide */
  //  0,      	   			 /* nb_index */
};

static PySequenceMethods matrix_as_sequence = {
  0,                  		/* sq_length */
  0,                  		/* sq_concat */
  0,                  		/* sq_repeat */
  (ssizeargfunc)matrix_item,	/* sq_item */
  0,                     	/* sq_slice */
  0,				/* sq_ass_item */
  0,				/* sq_ass_slice */
  0,		                /* sq_contains */
};

static PyMethodDef matrix_methods[] = {
  /* Do not need to wrap all cairo_matrix_init_*() functions
   * C API Matrix constructors       Python equivalents
   * cairo_matrix_init()             cairo.Matrix(xx,yx,xy,yy,x0,y0)
   * cairo_matrix_init_rotate()      cairo.Matrix.init_rotate(radians)

   * cairo_matrix_init_identity()    cairo.Matrix()
   * cairo_matrix_init_translate()   cairo.Matrix(x0=x0,y0=y0)
   * cairo_matrix_init_scale()       cairo.Matrix(xx=xx,yy=yy)
   */
  {"init_rotate", (PyCFunction)matrix_init_rotate, METH_VARARGS | METH_CLASS },
  {"invert",      (PyCFunction)matrix_invert,                METH_NOARGS },
  {"multiply",    (PyCFunction)matrix_multiply,              METH_VARARGS },
  {"rotate",      (PyCFunction)matrix_rotate,                METH_VARARGS },
  {"scale",       (PyCFunction)matrix_scale,                 METH_VARARGS },
  {"transform_distance",(PyCFunction)matrix_transform_distance, METH_VARARGS },
  {"transform_point", (PyCFunction)matrix_transform_point,   METH_VARARGS },
  {"translate",   (PyCFunction)matrix_translate,             METH_VARARGS },
  {NULL, NULL, 0, NULL},
};

PyTypeObject PycairoMatrix_Type = {
  PyVarObject_HEAD_INIT((PyTypeObject *)0x7FFFFFFF, 0)
  //PyObject_HEAD_INIT(NULL)
  //0,                                  /* ob_size */
  "cairo.Matrix",                     /* tp_name */
  sizeof(PycairoMatrix),              /* tp_basicsize */
  0,                                  /* tp_itemsize */
  (destructor)matrix_dealloc,         /* tp_dealloc */
  0,                                  /* tp_print */
  0,                                  /* tp_getattr */
  0,                                  /* tp_setattr */
  0,                                  /* tp_compare */
  (reprfunc)matrix_repr,              /* tp_repr */
  &matrix_as_number,                  /* tp_as_number */
  &matrix_as_sequence,                /* tp_as_sequence */
  0,                                  /* tp_as_mapping */
  0,                                  /* tp_hash */
  0,                                  /* tp_call */
  0,                                  /* tp_str */
  0,                                  /* tp_getattro */
  0,                                  /* tp_setattro */
  0,                                  /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,                 /* tp_flags */
  NULL,                               /* tp_doc */
  0,                                  /* tp_traverse */
  0,                                  /* tp_clear */
  (richcmpfunc)matrix_richcmp,        /* tp_richcompare */
  0,                                  /* tp_weaklistoffset */
  0,                                  /* tp_iter */
  0,                                  /* tp_iternext */
  matrix_methods,                     /* tp_methods */
  0,                                  /* tp_members */
  0,                                  /* tp_getset */
  0,                                  /* tp_base */
  0,                                  /* tp_dict */
  0,                                  /* tp_descr_get */
  0,                                  /* tp_descr_set */
  0,                                  /* tp_dictoffset */
  0,                                  /* tp_init */
  0,                                  /* tp_alloc */
  (newfunc)matrix_new,                /* tp_new */
  0,                                  /* tp_free */
  0,                                  /* tp_is_gc */
  0,                                  /* tp_bases */
};
