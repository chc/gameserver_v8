#include "../PythonInterface.h"
#include "../SAMP/SAMPRakPeer.h"
#include "../SAMP/SAMPDriver.h"
#include <server/CHCGameServer.h>
#include <structmember.h>

typedef struct {
	PyObject *pickup_event;
} SAMPPyPickup;

struct {
	PyTypeObject *mp_base_pickup;
	PyTypeObject *mp_base_text_label;
	PyTypeObject *mp_base_vehicle;
} SAMPScriptState;

std::vector<SAMPPyPickup *> py_samp_pickup_list;

PyObject *pyi_samp_createpickup(PyObject *self, PyObject *args);
PyObject *pyi_samp_destroypickup(PyObject *self, PyObject *args);
PyObject *pyi_samp_create3dlabel(PyObject *self, PyObject *args);
PyObject *pyi_samp_destroy3dtextlabel(PyObject *self, PyObject *args);
PyObject *pyi_samp_createvehicle(PyObject *self, PyObject *args);

PyObject *pyi_samp_setpickupentity(PyObject *self, PyObject *args);
PyObject *pyi_samp_set3dtextextentity(PyObject *self, PyObject *args);
PyObject *pyi_samp_setvehicleentity(PyObject *self, PyObject *args);
PyMethodDef SAMP_methods[] = {
    								{"CreatePickup",  (PyCFunction)pyi_samp_createpickup, METH_VARARGS,
 								    "Creates a pickup"},
 								    {"DestroyPickup",  (PyCFunction)pyi_samp_destroypickup, METH_VARARGS,
 								    "Creates a pickup"},
 								    {"CreateVehicle",  (PyCFunction)pyi_samp_createvehicle, METH_VARARGS,
 								    "Creates a vehicle"},
 								    {"Create3DTextLabel",  (PyCFunction)pyi_samp_create3dlabel, METH_VARARGS,
 								    "Creates a 3d Text Label"},
 								    {"Destroy3DTextLabel",  (PyCFunction)pyi_samp_destroy3dtextlabel, METH_VARARGS,
 								    "Destroys a 3d Text Label"},
 								    {"SetPickupEntity",  (PyCFunction)pyi_samp_setpickupentity, METH_O,
 								    "Sets the pickup entity"},
 								    {"Set3DTextLabelEntity",  (PyCFunction)pyi_samp_set3dtextextentity, METH_O,
 								    "Sets the 3d Text Label Entity"},
 								    {"SetVehicleEntity",  (PyCFunction)pyi_samp_setvehicleentity, METH_O,
 								    "Sets the vehicle entity"},
    								{NULL, NULL, 0, NULL}};

struct PyModuleDef samp_module = {
   PyModuleDef_HEAD_INIT,
   "SAMP",   /* name of module */
   NULL, /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   SAMP_methods
};




PyMODINIT_FUNC
PyInit_SAMP(void)
{
	PyObject *m = PyModule_Create(&samp_module);
	return m;

}



PyObject *pyi_samp_createpickup(PyObject *self, PyObject *args) {
	PyObject* dict;

	int modelid;
	float x,y,z;
	int pickup_type;
	int world, stream_index;

	int pickup_id = -1;

	SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
	if (PyArg_ParseTuple(args, "O", &dict)) {
		if(PyDict_Check(dict)) {
			//SAMP.CreatePickup({'model': 1222, 'position': [1529.6,-1691.2,13.3], 'pickup_type': 1, 'world': 0, 'stream_index': 0, 'pickup_event': pickup_event})
			PyObject *model = PyDict_GetItemString(dict, "model");
			PyObject *position = PyDict_GetItemString(dict, "position");
			PyObject *pickup_type = PyDict_GetItemString(dict, "pickup_type");
			PyObject *world = PyDict_GetItemString(dict, "world");
			PyObject *stream_index = PyDict_GetItemString(dict, "stream_index");
			PyObject *pickup_event = PyDict_GetItemString(dict, "pickup_event");
			if(pickup_event) {
				Py_INCREF(pickup_event);
			}
			if(position && PyList_Check(position)) {
				PyObject *pyx = PyList_GET_ITEM(position, 0);
				PyObject *pyy = PyList_GET_ITEM(position, 1);
				PyObject *pyz = PyList_GET_ITEM(position, 2);
				x = PyFloat_AsDouble(pyx);
				y = PyFloat_AsDouble(pyy);
				z = PyFloat_AsDouble(pyz);
			}
			if(model) {
				modelid = PyLong_AsLong(model);
			}
			pickup_id = driver->createPickup(modelid, 1, x,y, z);
		}
	}
	return PyLong_FromLong(pickup_id);
}
PyObject *pyi_samp_destroypickup(PyObject *self, PyObject *args) {
	SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
	int pickup_id;
	if (PyArg_ParseTuple(args, "i", &pickup_id)) {
		driver->destroyPickup(pickup_id);
	}
	Py_RETURN_NONE;

}
PyObject *pyi_samp_create3dlabel(PyObject *self, PyObject *args) {
	PyObject* dict;

	int modelid;
	float x,y,z;
	int pickup_type;
	int world, stream_index;

	int textdraw_id = -1;

	float draw_dist = 500.0;

	SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
	if (PyArg_ParseTuple(args, "O", &dict)) {
		if(PyDict_Check(dict)) {
			PyObject *text = PyDict_GetItemString(dict, "text");
			PyObject *position = PyDict_GetItemString(dict, "position");
			PyObject *world = PyDict_GetItemString(dict, "world");
			PyObject *stream_index = PyDict_GetItemString(dict, "stream_index");
			PyObject *los = PyDict_GetItemString(dict, "test_los");
			PyObject *colour = PyDict_GetItemString(dict, "colour");
			PyObject *dist = PyDict_GetItemString(dict, "draw_distance");
			uint32_t col = 0xFFFFFFFF;
			bool test_los = false;
			if(position && PyList_Check(position)) {
				PyObject *pyx = PyList_GET_ITEM(position, 0);
				PyObject *pyy = PyList_GET_ITEM(position, 1);
				PyObject *pyz = PyList_GET_ITEM(position, 2);
				x = PyFloat_AsDouble(pyx);
				y = PyFloat_AsDouble(pyy);
				z = PyFloat_AsDouble(pyz);
			}
			if(dist) {
				draw_dist = PyFloat_AsDouble(dist);
			}
			if(colour) {
				col = PyLong_AsLong(colour);
			}
			test_los = los == Py_True;
			const char *str = PythonScriptInterface::copyPythonString(text);

			textdraw_id = driver->create3DTextLabel(str, col, x, y, z, draw_dist, test_los, 0, 0);
			if(str)
				free((void *)str);
		}
	}
	return PyLong_FromLong(textdraw_id);
}
PyObject *pyi_samp_destroy3dtextlabel(PyObject *self, PyObject *args) {
	int id = 0;
	if (PyArg_ParseTuple(args, "i", &id)) {
		SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
		driver->destroy3DTextLabel(id);
	}
	Py_RETURN_NONE;
}
PyObject *pyi_samp_createvehicle(PyObject *self, PyObject *args) {
	PyObject* dict;
	float x,y,z;
	int modelid;
	int worldid = 0, n_stream_index = 0;
	SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
	if (PyArg_ParseTuple(args, "O", &dict)) {
		if(PyDict_Check(dict)) {
			int c1 = -1, c2 = -1;
			PyObject *model = PyDict_GetItemString(dict, "model");
			PyObject *position = PyDict_GetItemString(dict, "position");
			PyObject *world = PyDict_GetItemString(dict, "world");

			PyObject *colours = PyDict_GetItemString(dict, "colour");
			PyObject *stream_index = PyDict_GetItemString(dict, "stream_index");
			
			if(position && PyList_Check(position)) {
				PyObject *pyx = PyList_GET_ITEM(position, 0);
				PyObject *pyy = PyList_GET_ITEM(position, 1);
				PyObject *pyz = PyList_GET_ITEM(position, 2);
				x = PyFloat_AsDouble(pyx);
				y = PyFloat_AsDouble(pyy);
				z = PyFloat_AsDouble(pyz);
			}
			if(colours && PyList_Check(colours)) {
				PyObject *pc1 = PyList_GET_ITEM(colours, 0);
				PyObject *pc2 = PyList_GET_ITEM(colours, 1);
				c1 = PyLong_AsLong(pc1);
				c2 = PyLong_AsLong(pc2);
			}
			modelid = PyLong_AsLong(model);
			return PyLong_FromLong(driver->CreateVehicle(modelid,x,y,z,0.0,c1,c2));
		}
	}
	Py_RETURN_NONE;
}
PyObject *pyi_samp_setpickupentity(PyObject *self, PyObject *args) {
	SAMPScriptState.mp_base_pickup = (PyTypeObject *)args;
    Py_INCREF(SAMPScriptState.mp_base_pickup);
	Py_RETURN_NONE;
}
PyObject *pyi_samp_set3dtextextentity(PyObject *self, PyObject *args) {
	SAMPScriptState.mp_base_text_label = (PyTypeObject *)args;
    Py_INCREF(SAMPScriptState.mp_base_text_label);
	Py_RETURN_NONE;
}
PyObject *pyi_samp_setvehicleentity(PyObject *self, PyObject *args) {
	SAMPScriptState.mp_base_vehicle = (PyTypeObject *)args;
    Py_INCREF(SAMPScriptState.mp_base_vehicle);
	Py_RETURN_NONE;
}