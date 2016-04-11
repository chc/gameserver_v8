#include "../PythonInterface.h"
#include "../SAMP/SAMPRakPeer.h"
static PyObject *
frontend_createmodal(PyObject *self, PyObject *args)
{
	PyObject *dict;
	PyObject *user;
    if (!PyArg_ParseTuple(args, "OO", &user, &dict))
        return NULL;
    if(PyDict_Check(dict)) {
		PyObject *title = PyDict_GetItemString(dict, "title");
		PyObject *message = PyDict_GetItemString(dict, "message");
		PyObject *type = PyDict_GetItemString(dict, "type");
		PyObject *buttons = PyDict_GetItemString(dict, "buttons");
		PyObject *callback = PyDict_GetItemString(dict, "callback");
		PyObject *extra = PyDict_GetItemString(dict, "extra");
		if(title && message && type) {
			const char *ptitle = PythonScriptInterface::copyPythonString(title);
			const char *msg = PythonScriptInterface::copyPythonString(message);
			const char *b1 = NULL, *b2 = NULL;
			long native_type = PyLong_AsLong(type);
			ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(user);

			PyObject *button1 = NULL, *button2 = NULL;
   			if(PyList_Check(buttons)) {
   				PyObject *seq = PySequence_Fast(buttons, "expected a sequence");
				button1 = PyList_GET_ITEM(seq, 0);
				button2 = PyList_GET_ITEM(seq, 1);
				
				if(button1)
					b1 = PythonScriptInterface::copyPythonString(button1);
				if(button2)
					b2 = PythonScriptInterface::copyPythonString(button2);
			}
			tbl->last_dialog_callback = callback;
			Py_INCREF(tbl->last_dialog_callback);
			tbl->user->ShowPlayerDialog(666, native_type, ptitle, msg, b1, b2);
			if(b1)
				free((void *)b1);
			if(b2)
				free((void *)b2);

			free((void *)ptitle);
			free((void *)msg);
		}
	}
    Py_RETURN_NONE;
}

static PyMethodDef FrontendMethods[] = {
    {"CreateModal",  frontend_createmodal, METH_VARARGS, "Display a modal to the client"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef frontend_module = {
   PyModuleDef_HEAD_INIT,
   "Frontend",   /* name of module */
   NULL, /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   FrontendMethods
};

PyMODINIT_FUNC
PyInit_FrontEnd(void)
{
	PyObject *m;

	m = PyModule_Create(&frontend_module);

	PyObject *temp_obj = PyLong_FromLong(0);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "MSG_BOX", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(1);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "INPUT_BOX", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(2);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "LIST_BOX", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(3);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "PASSWORD_INPUT", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(4);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "TABLIST_INPUT", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(5);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "TABLIST_HEADER_INPUT", (PyObject *)temp_obj);
    return m;
}