#include <Python.h>
#include <gpiod.h>
#include <gpiod_frequency_counter.h>

typedef struct {
	PyObject_HEAD
	struct gpiod_line *line;
	PyObject *owner;
} gpiod_LineObject;

typedef struct {
	PyObject_HEAD
	struct gpiod_frequency_counter counter;
	PyObject *line;
} gpiod_frequency_counter_FrequencyCounterObject;

static struct gpiod_line* get_line_from_object(PyObject *object) {
	gpiod_LineObject *line_object = (void*)object; ///
	return line_object->line;
}

static PyObject* get_gpiod_line_type() {
	PyObject *gpiod = PyImport_ImportModule("gpiod");
	if (!gpiod) {
		return NULL;
	}
	PyObject *gpiod_Line = PyObject_GetAttrString(gpiod, "Line");
	Py_DECREF(gpiod);
	return gpiod_Line;
}

static int gpiod_frequency_counter_FrequencyCounter_init(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *args,
	PyObject *Py_UNUSED(kwargs)
) {
	PyObject *line_object;
	PyObject *line_type = get_gpiod_line_type();
	if (!line_type) {
		return -1;
	}
	unsigned buf_size;
	int rc = PyArg_ParseTuple(args, "O!I", line_type, &line_object, &buf_size);
	Py_DECREF(line_type);
	if (!rc) {
		return -1;
	}
	if (buf_size < 1) {
		PyErr_SetString(PyExc_ValueError, "Buffer size must be greater than 0");
		return -1;
	}
	struct gpiod_line *line = get_line_from_object(line_object);
	if (!line) {
		return -1;
	}
	rc = gpiod_frequency_counter_init(&self->counter, line, buf_size);
	if (rc) {
		PyErr_SetFromErrno(PyExc_OSError);
		return -1;
	}
	Py_INCREF(line_object);
	self->line = line_object;
	return 0;
}

static void gpiod_frequency_counter_FrequencyCounter_dealloc(
	gpiod_frequency_counter_FrequencyCounterObject *self
) {
	gpiod_frequency_counter_destroy(&self->counter);
	if (self->line) {
		Py_DECREF(self->line);
	}
	PyObject_Del(self);
}

static PyObject *gpiod_frequency_counter_FrequencyCounter_repr(
	gpiod_frequency_counter_FrequencyCounterObject *self
) {
	PyObject *line_repr, *ret;
	line_repr = PyObject_CallMethod((PyObject*)self->line, "__repr__", "");
	if (!line_repr) {
		return NULL;
	}
	ret = PyUnicode_FromFormat(
		"'FrequencyCounter(line = %S, buf_size = %zu)'",
		line_repr,
		self->counter.period_buf_size
	);
	Py_DECREF(line_repr);
	return ret;
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_count_doc,
"count([waves, [sec, [nsec]]]) -> None\n"
"\n"
"Count GPIO input frequency.\n"
"\n"
"  waves\n"
"    Number of waves to count (default: buf_size).\n"
"  sec\n"
"    Number of seconds before timeout (default: no timeout).\n"
"  nsec\n"
"    Number of nanoseconds before timeout (default: no timeout).\n"
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_count(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *args,
	PyObject *kwargs
) {
	static char *kwlist[] = { "waves", "sec", "nsec", NULL };

	int waves = 0;
	long sec = 0;
	long nsec = 0;
	struct timespec ts;
	struct timespec *pts = NULL;

	int rc = PyArg_ParseTupleAndKeywords(
		args, kwargs,
		"|ill", kwlist,
		&waves, &sec, &nsec
	);
	if (!rc) {
		return NULL;
	}
	if (sec > 0 || (sec == 0 && nsec > 0)) {
		ts.tv_sec = sec;
		ts.tv_nsec = nsec;
		pts = &ts;
	}

	Py_BEGIN_ALLOW_THREADS;
	rc = gpiod_frequency_counter_count(&self->counter, waves, pts);
	Py_END_ALLOW_THREADS;
	if (rc) {
		return PyErr_SetFromErrno(PyExc_OSError);
	}

	Py_RETURN_NONE;
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_get_buf_size_doc,
"Wave period buffer size (integer)."
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_get_buf_size(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *Py_UNUSED(args)
) {
	size_t res = self->counter.period_buf_size;
	return PyLong_FromSize_t(res);
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_get_frequency_doc,
"Frequency in hertz (float)."
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_get_frequency(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *Py_UNUSED(args)
) {
	double res = gpiod_frequency_counter_get_frequency(&self->counter);
	return PyFloat_FromDouble(res);
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_get_period_doc,
"Period in seconds (float)."
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_get_period(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *Py_UNUSED(args)
) {
	double res = gpiod_frequency_counter_get_period(&self->counter);
	return PyFloat_FromDouble(res);
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_get_high_period_doc,
"High period in seconds (float)."
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_get_high_period(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *Py_UNUSED(args)
) {
	double res = gpiod_frequency_counter_get_high_period(&self->counter);
	return PyFloat_FromDouble(res);
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_get_low_period_doc,
"Low period in seconds (float)."
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_get_low_period(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *Py_UNUSED(args)
) {
	double res = gpiod_frequency_counter_get_low_period(&self->counter);
	return PyFloat_FromDouble(res);
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounter_get_duty_cycle_doc,
"Duty cycle (ratio) (float)."
);

static PyObject* gpiod_frequency_counter_FrequencyCounter_get_duty_cycle(
	gpiod_frequency_counter_FrequencyCounterObject *self,
	PyObject *Py_UNUSED(args)
) {
	double res = gpiod_frequency_counter_get_duty_cycle(&self->counter);
	return PyFloat_FromDouble(res);
}

PyDoc_STRVAR(gpiod_frequency_counter_FrequencyCounterType_doc,
"Represents a GPIO input frequency counter.\n"
"\n"
"Example:\n"
"\n"
"    chip = gpiod.Chip('gpiochip0', gpiod.Chip.OPEN_BY_NAME)\n"
"    line = chip.get_line(0)\n"
"    buf_size = 64\n"
"    timeout_sec = 1\n"
"    counter = gpiod_frequency_counter.FrequencyCounter(line, buf_size)\n"
"    counter.count(sec=timeout_sec)\n"
"    print(counter.period, counter.frequency)\n"
);

static PyMethodDef gpiod_frequency_counter_FrequencyCounter_methods[] = {
	{
		.ml_name = "count",
		.ml_meth = (PyCFunction)gpiod_frequency_counter_FrequencyCounter_count,
		.ml_flags = METH_VARARGS | METH_KEYWORDS,
		.ml_doc = gpiod_frequency_counter_FrequencyCounter_count_doc,
	},
	{}
};

static PyGetSetDef gpiod_frequency_counter_FrequencyCounter_getset[] = {
	{
		.name = "buf_size",
		.get = (getter)gpiod_frequency_counter_FrequencyCounter_get_buf_size,
		.doc = gpiod_frequency_counter_FrequencyCounter_get_buf_size_doc,
	},
	{
		.name = "frequency",
		.get = (getter)gpiod_frequency_counter_FrequencyCounter_get_frequency,
		.doc = gpiod_frequency_counter_FrequencyCounter_get_frequency_doc,
	},
	{
		.name = "period",
		.get = (getter)gpiod_frequency_counter_FrequencyCounter_get_period,
		.doc = gpiod_frequency_counter_FrequencyCounter_get_period_doc,
	},
	{
		.name = "low_period",
		.get = (getter)gpiod_frequency_counter_FrequencyCounter_get_low_period,
		.doc = gpiod_frequency_counter_FrequencyCounter_get_low_period_doc,
	},
	{
		.name = "high_period",
		.get = (getter)gpiod_frequency_counter_FrequencyCounter_get_high_period,
		.doc = gpiod_frequency_counter_FrequencyCounter_get_high_period_doc,
	},
	{
		.name = "duty_cycle",
		.get = (getter)gpiod_frequency_counter_FrequencyCounter_get_duty_cycle,
		.doc = gpiod_frequency_counter_FrequencyCounter_get_duty_cycle_doc,
	},
	{}
};

static PyTypeObject gpiod_frequency_counter_FrequencyCounterType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "gpiod_frequency_counter.FrequencyCounter",
	.tp_basicsize = sizeof(gpiod_frequency_counter_FrequencyCounterObject),
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = gpiod_frequency_counter_FrequencyCounterType_doc,
	.tp_new = PyType_GenericNew,
	.tp_init = (initproc)gpiod_frequency_counter_FrequencyCounter_init,
	.tp_dealloc = (destructor)gpiod_frequency_counter_FrequencyCounter_dealloc,
	.tp_repr = (reprfunc)gpiod_frequency_counter_FrequencyCounter_repr,
	.tp_methods = gpiod_frequency_counter_FrequencyCounter_methods,
	.tp_getset = gpiod_frequency_counter_FrequencyCounter_getset,
};

PyDoc_STRVAR(gpiod_frequency_counter_Module_doc,
"Python bindings for libgpiod-frequency-counter.\n"
);

static PyModuleDef gpiod_frequency_counter_Module = {
	PyModuleDef_HEAD_INIT,
	.m_name = "gpiod_frequency_counter",
	.m_doc = gpiod_frequency_counter_Module_doc,
	.m_size = -1,
};

PyMODINIT_FUNC PyInit_gpiod_frequency_counter() {
	PyObject *module = PyModule_Create(&gpiod_frequency_counter_Module);
	if (!module) {
		return NULL;
	}
	PyTypeObject *type = &gpiod_frequency_counter_FrequencyCounterType;
	const char *name = "FrequencyCounter";
	if (PyType_Ready(type)) {
		return NULL;
	}
	Py_INCREF(type);
	if (PyModule_AddObject(module, name, (PyObject*)type) < 0) {
		return NULL;
	}
	const char *version = gpiod_frequency_counter_version_string();
	if (PyModule_AddStringConstant(module, "__version__", version) < 0) {
		return NULL;
	}
	return module;
}
