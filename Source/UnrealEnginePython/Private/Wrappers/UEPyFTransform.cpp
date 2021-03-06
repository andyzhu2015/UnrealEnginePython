#include "UnrealEnginePythonPrivatePCH.h"


static PyMethodDef ue_PyFTransform_methods[] = {

	{ NULL }  /* Sentinel */
};

static PyObject *py_ue_ftransform_get_translation(ue_PyFTransform *self, void *closure) {
	return py_ue_new_fvector(self->transform.GetTranslation());
}

static PyObject *py_ue_ftransform_get_scale(ue_PyFTransform *self, void *closure) {
	return py_ue_new_fvector(self->transform.GetScale3D());
}

static PyObject *py_ue_ftransform_get_rotation(ue_PyFTransform *self, void *closure) {
	return py_ue_new_frotator(self->transform.GetRotation().Rotator());
}

static int py_ue_ftransform_set_translation(ue_PyFTransform *self, PyObject *value, void *closure) {
	if (ue_PyFVector *py_vec = py_ue_is_fvector(value)) {
		self->transform.SetLocation(py_vec->vec);
		return 0;
	}
	PyErr_SetString(PyExc_TypeError, "value is not a vector");
	return -1;
}

static int py_ue_ftransform_set_rotation(ue_PyFTransform *self, PyObject *value, void *closure) {
	if (ue_PyFRotator *py_rot = py_ue_is_frotator(value)) {
		self->transform.SetRotation(py_rot->rot.Quaternion());
		return 0;
	}
	PyErr_SetString(PyExc_TypeError, "value is not a rotator");
	return -1;
}

static int py_ue_ftransform_set_scale(ue_PyFTransform *self, PyObject *value, void *closure) {
	if (ue_PyFVector *py_vec = py_ue_is_fvector(value)) {
		self->transform.SetScale3D(py_vec->vec);
		return 0;
	}
	PyErr_SetString(PyExc_TypeError, "value is not a vector");
	return -1;
}



static PyGetSetDef ue_PyFTransform_getseters[] = {
	{(char *) "translation", (getter)py_ue_ftransform_get_translation, (setter)py_ue_ftransform_set_translation, (char *)"", NULL },
	{(char *) "scale", (getter)py_ue_ftransform_get_scale, (setter)py_ue_ftransform_set_scale, (char *)"", NULL },
	{(char *) "rotation", (getter)py_ue_ftransform_get_rotation, (setter)py_ue_ftransform_set_rotation, (char *)"", NULL },
	{ NULL }  /* Sentinel */
};

static PyObject *ue_PyFTransform_str(ue_PyFTransform *self)
{
	FVector vec = self->transform.GetTranslation();
	FRotator rot = self->transform.Rotator();
	FVector scale = self->transform.GetScale3D();

	return PyUnicode_FromFormat("<unreal_engine.FTransform {'translation': (%S, %S, %S), 'rotation': (%S, %S, %S), 'scale': (%S, %S, %S)}>",
		PyFloat_FromDouble(vec.X),
		PyFloat_FromDouble(vec.Y),
		PyFloat_FromDouble(vec.Z),
		PyFloat_FromDouble(rot.Roll),
		PyFloat_FromDouble(rot.Pitch),
		PyFloat_FromDouble(rot.Yaw),
		PyFloat_FromDouble(scale.X),
		PyFloat_FromDouble(scale.Y),
		PyFloat_FromDouble(scale.Z)
		);
}


static PyTypeObject ue_PyFTransformType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"unreal_engine.FTransform", /* tp_name */
	sizeof(ue_PyFTransform), /* tp_basicsize */
	0,                         /* tp_itemsize */
	0,       /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	(reprfunc)ue_PyFTransform_str,                         /* tp_str */
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"Unreal Engine FTransform",           /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	ue_PyFTransform_methods,             /* tp_methods */
	0,
	ue_PyFTransform_getseters,
};

static int ue_py_ftransform_init(ue_PyFTransform *self, PyObject *args, PyObject *kwargs) {
	PyObject *py_translation = nullptr;
	PyObject *py_rotation = nullptr;
	PyObject *py_scale = nullptr;
	if (!PyArg_ParseTuple(args, "|OOO:__init__", &py_translation, &py_rotation, &py_scale)) {
		return -1;
	}

	if (py_translation) {
		if (ue_PyFVector *py_vec = py_ue_is_fvector(py_translation)) {
			self->transform.SetTranslation(py_vec->vec);
		}
		else {
			PyErr_SetString(PyExc_Exception, "argument is not a FVector");
			return -1;
		}
	}

	if (py_rotation) {
		if (ue_PyFRotator *py_rot = py_ue_is_frotator(py_rotation)) {
			self->transform.SetRotation(py_rot->rot.Quaternion());
		}
		else if (ue_PyFQuat *py_quat = py_ue_is_fquat(py_rotation)) {
			self->transform.SetRotation(py_quat->quat);
		}
		else {
			PyErr_SetString(PyExc_Exception, "argument is not a FRotator or a FQuat");
			return -1;
		}
	}
	else {
		self->transform.SetRotation(FQuat::Identity);
	}

	// ensure scaling is set to 1,1,1
	FVector scale(1, 1, 1);

	if (py_scale) {
		if (ue_PyFVector *py_vec = py_ue_is_fvector(py_scale)) {
			scale = py_vec->vec;
		}
		else {
			PyErr_SetString(PyExc_Exception, "argument is not a FVector");
			return -1;
		}
	}
	self->transform.SetScale3D(scale);
	return 0;
}

static PyObject *ue_py_ftransform_mul(ue_PyFTransform *self, PyObject *value) {
	FTransform t = self->transform;
	if (ue_PyFQuat *py_quat = py_ue_is_fquat(value)) {
		t *= py_quat->quat;
	}
	else if (ue_PyFTransform *py_transform = py_ue_is_ftransform(value)) {
		t *= py_transform->transform;
	}
	else {
		return PyErr_Format(PyExc_TypeError, "FTransform can be multiplied only for an FQuat or an FTransform");
	}
	return py_ue_new_ftransform(t);
}

PyNumberMethods ue_PyFTransform_number_methods;

void ue_python_init_ftransform(PyObject *ue_module) {
	ue_PyFTransformType.tp_new = PyType_GenericNew;

	ue_PyFTransformType.tp_init = (initproc)ue_py_ftransform_init;

	memset(&ue_PyFTransform_number_methods, 0, sizeof(PyNumberMethods));
	ue_PyFTransformType.tp_as_number = &ue_PyFTransform_number_methods;
	ue_PyFTransform_number_methods.nb_multiply = (binaryfunc)ue_py_ftransform_mul;

	if (PyType_Ready(&ue_PyFTransformType) < 0)
		return;

	Py_INCREF(&ue_PyFTransformType);
	PyModule_AddObject(ue_module, "FTransform", (PyObject *)&ue_PyFTransformType);
}

PyObject *py_ue_new_ftransform(FTransform transform) {
	ue_PyFTransform *ret = (ue_PyFTransform *)PyObject_New(ue_PyFTransform, &ue_PyFTransformType);
	ret->transform = transform;
	return (PyObject *)ret;
}

ue_PyFTransform *py_ue_is_ftransform(PyObject *obj) {
	if (!PyObject_IsInstance(obj, (PyObject *)&ue_PyFTransformType))
		return nullptr;
	return (ue_PyFTransform *)obj;
}

bool py_ue_transform_arg(PyObject *args, FTransform &t) {

	if (PyTuple_Size(args) == 1) {
		PyObject *arg = PyTuple_GetItem(args, 0);
		ue_PyFTransform *py_t = py_ue_is_ftransform(arg);
		if (!py_t) {
			PyErr_Format(PyExc_TypeError, "argument is not a FTransform");
			return false;
		}
		t = py_t->transform;
		return true;
	}

	float x, y, z;
	float roll, pitch, yaw;
	float sx, sy, sz;
	if (!PyArg_ParseTuple(args, "fffffffff", &x, &y, &z, &roll, &pitch, &yaw, &sx, &sy, &sz))
		return false;

	t.SetLocation(FVector(x, y, z));
	t.SetRotation(FRotator(pitch, yaw, roll).Quaternion());
	t.SetScale3D(FVector(sx, sy, sz));
	return true;
}