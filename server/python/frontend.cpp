#include "../PythonInterface.h"
#include "../SAMP/SAMPDriver.h"
#include "../SAMP/SAMPRakPeer.h"
#include <server/CHCGameServer.h>
static PyObject *
frontend_createmodal(PyObject *self, PyObject *args)
{
	PyObject *dict;
	PyObject *user;
    if (!PyArg_ParseTuple(args, "OO", &user, &dict))
        Py_RETURN_NONE;
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
void samp_frontend_load_td_font(PyObject *font_data, SAMPTextDraw *td) {
	if(!font_data) return;
	if(PyDict_Check(font_data)) {
		PyObject *py_text = PyDict_GetItemString(font_data, "text");
		PyObject *py_colour = PyDict_GetItemString(font_data, "colour");
		PyObject* py_proportional = PyDict_GetItemString(font_data, "proportional");
		PyObject* py_shadow = PyDict_GetItemString(font_data, "shadow");
		PyObject* py_width = PyDict_GetItemString(font_data, "width");
		PyObject* py_height = PyDict_GetItemString(font_data, "height");

		PyObject* py_style = PyDict_GetItemString(font_data, "style");

		PyObject* py_alignment = PyDict_GetItemString(font_data, "alignment");

		int alignment = 0;
		if(py_alignment) {
			alignment = PyLong_AsLong(py_alignment);
		}

		td->flags &= ~(SAMPTD_IsLeftAligned|SAMPTD_IsRightAligned|SAMPTD_IsCenterAligned);
		switch(alignment) {
			case 0:
				td->flags |= SAMPTD_IsLeftAligned;
				break;
			case 1:
				td->flags |= SAMPTD_IsRightAligned;
				break;
			case 2:
				td->flags |= SAMPTD_IsCenterAligned;
				break;
		}
		
		
		bool is_proportional = py_proportional == Py_True;
		bool has_Shadow = py_shadow == Py_True;
		if(py_text) {
			const char *str = PythonScriptInterface::copyPythonString(py_text);
			if(str) {
				strcpy(td->text, str);
				free((void *)str);
			}
		}
		if(py_style) {
			td->style = PyLong_AsLong(py_style);
		}
		if(has_Shadow) {
			td->shadow = true;
		}
		if(is_proportional) {
			td->flags |= SAMPTD_IsProportional;
		}
		if(py_colour) {
			uint32_t colour = PyLong_AsLong(py_colour);
			td->font_colour = colour;
		}
		if(py_width && py_height) {
			td->font_width = PyFloat_AsDouble(py_width);
			td->font_height = PyFloat_AsDouble(py_height);
		}

	}
}
PyObject *frontend_createuielement(PyObject *self, PyObject* args) {
	PyObject *dict;
    if (!PyArg_ParseTuple(args, "O", &dict))
        Py_RETURN_NONE;
    if(PyDict_Check(dict)) {
	    SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
    	SAMPTextDraw *td = driver->CreateTextDraw();
    	PyObject *py_box_colour = PyDict_GetItemString(dict, "box_colour");
    	PyObject *py_box_width = PyDict_GetItemString(dict, "box_width");
    	PyObject *py_box_height = PyDict_GetItemString(dict, "box_height");

    	PyObject *py_x, *py_y;
    	py_x = PyDict_GetItemString(dict, "x");
    	py_y = PyDict_GetItemString(dict, "y");

    	if(py_x && py_y) {
    		td->x = PyFloat_AsDouble(py_x);
    		td->y = PyFloat_AsDouble(py_y);
    	}
    	uint32_t box_col = 0;
    	
    	if(py_box_colour) {
    		 box_col = PyLong_AsLong(py_box_colour);
    	}
    	if(py_box_width && py_box_height) {
    		float box_width = PyFloat_AsDouble(py_box_width), box_height = PyFloat_AsDouble(py_box_height);
    		td->box_width = box_width;
    		td->box_height = box_height;
    	}
    	td->selectable = PyDict_GetItemString(dict, "selectable") == Py_True;
    	PyObject *py_font = PyDict_GetItemString(dict, "font");
    	samp_frontend_load_td_font(py_font, td);
    	td->box_colour = box_col;
    	if(PyDict_GetItemString(dict, "box") == Py_True) {
    		td->flags |= SAMPTD_IsBox;
    	}
    	return PyLong_FromLong(td->id);
    }

	Py_RETURN_NONE;
}

PyObject *frontend_displayuielements(PyObject *self, PyObject* args) {
	PyObject *arr, *conn;
    if (!PyArg_ParseTuple(args, "OO", &conn, &arr))
        Py_RETURN_NONE;
    if(arr) {
    	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(conn);
    	SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
    	PyObject *seq = PySequence_Fast(arr, "expected a sequence");
    	if(tbl && seq) {
    		Py_ssize_t len =  PyList_Size(seq);
    		for(int i=0;i<len;i++) {
    			PyObject *py_id = PyList_GET_ITEM(seq, i);
    			int id = PyLong_AsLong(py_id);
    			SAMPTextDraw *td = driver->FindTextDrawByID(id);
    			if(td) {
    				tbl->user->ShowTextDraw(td);
    			}
    		}
    	}
	}
	Py_RETURN_NONE;
}
PyObject *frontend_hideuielements(PyObject *self, PyObject *args) {
	PyObject *arr, *conn;
    if (!PyArg_ParseTuple(args, "OO", &conn, &arr))
        Py_RETURN_NONE;
    if(arr) {
    	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(conn);
    	SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
    	PyObject *seq = PySequence_Fast(arr, "expected a sequence");
    	if(tbl && seq) {
    		Py_ssize_t len =  PyList_Size(seq);
    		for(int i=0;i<len;i++) {
    			PyObject *py_id = PyList_GET_ITEM(seq, i);
    			int id = PyLong_AsLong(py_id);
    			SAMPTextDraw *td = driver->FindTextDrawByID(id);
    			if(td) {
    				tbl->user->HideTextDraw(td);
    			}
    		}
    	}
	}
	Py_RETURN_NONE;
}
/*
	Hover colour is a SAMP thing, should probs be a variable in the SAMP module instead
*/
PyObject *frontend_activatemouse(PyObject *self, PyObject *args) {
	PyObject *status, *conn, *options, *callback;
	uint32_t hover_colour = 0xFFFFFFFF;
	bool enabled = true;
	if(!PyArg_ParseTuple(args, "OO", &conn, &options)) {
		Py_RETURN_NONE;
	}
	if(PyDict_Check(options)) {
		PyObject *obj = PyDict_GetItemString(options, "enabled");
		enabled = obj == Py_True;
		obj = PyDict_GetItemString(options, "hover_colour");
		if(obj)
			hover_colour = PyLong_AsUnsignedLong(obj);

		callback = PyDict_GetItemString(options, "callback");
	}
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(conn);
    SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
	tbl->user->SelectTextDraw(hover_colour, status == Py_True);
	Py_XDECREF(tbl->mouse_callback);
	tbl->mouse_callback = callback;
	Py_INCREF(callback);
	Py_RETURN_NONE;
}
static PyMethodDef FrontendMethods[] = {
	//CreateUIElement
    {"CreateModal", frontend_createmodal, METH_VARARGS, "Display a modal to the client"},
    {"CreateUIElement", frontend_createuielement, METH_VARARGS, "Creates a UI Element"},
    {"DisplayUIElements", frontend_displayuielements, METH_VARARGS, "Displays UI arguments"},
    {"HideUIElements", frontend_hideuielements, METH_VARARGS, "Displays UI arguments"},
    {"ActivateMouse", frontend_activatemouse, METH_VARARGS, "Sets the mouses activity state"},
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

	temp_obj = PyLong_FromLong(0);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "TEXT_ALIGNMENT_LEFT", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(1);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "TEXT_ALIGNMENT_RIGHT", (PyObject *)temp_obj);

	temp_obj = PyLong_FromLong(2);

	Py_INCREF(temp_obj);

	PyModule_AddObject(m, "TEXT_ALIGNMENT_CENTER", (PyObject *)temp_obj);

    return m;
}