#include "../PythonInterface.h"
#include "../SAMP/SAMPRakPeer.h"
#include <structmember.h>

PyObject *pyi_baseentity_spawn(gs_BaseEntityObject *self, PyObject *args);
int Entity_setpos(gs_BaseEntityObject *self, PyObject *value, void *closure);
PyObject *Entity_getpos(gs_BaseEntityObject *self, void *closure);
int Entity_setmodel(gs_BaseEntityObject *self, PyObject *value, void *closure);
PyObject *Entity_getmodel(gs_BaseEntityObject *self, void *closure);
int Entity_setarmour(gs_BaseEntityObject *self, PyObject *value, void *closure);
PyObject *Entity_getarmour(gs_BaseEntityObject *self, void *closure);
int Entity_sethp(gs_BaseEntityObject *self, PyObject *value, void *closure);
PyObject *Entity_gethp(gs_BaseEntityObject *self, void *closure);
int Entity_setname(gs_BaseEntityObject *self, PyObject *value, void *closure);
PyObject *Entity_getname(gs_BaseEntityObject *self, void *closure);

PyTypeObject gs_BaseEntityType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "CoreServer.BaseEntity",             /*tp_name*/
    sizeof(gs_BaseEntityObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Base Entity Object",           /* tp_doc */
};

PyMemberDef BaseEntity_members[] = {
    {"connection", T_OBJECT, offsetof(gs_BaseEntityObject, connection), 0,
     "Entitys connection"},
    {NULL}  /* Sentinel */
};

PyGetSetDef Entity_getseters[] = {
    {"Health", 
     (getter)Entity_gethp, (setter)Entity_sethp,
     "Entity Health",
     NULL},
     {"Armour", 
     (getter)Entity_getarmour, (setter)Entity_setarmour,
     "Entity Health",
     NULL},
     {"Model", 
     (getter)Entity_getmodel, (setter)Entity_setmodel,
     "Entity Model",
     NULL},
     {"Position", 
     (getter)Entity_getpos, (setter)Entity_setpos,
     "Entity Position",
     NULL},
    {"Name", 
     (getter)Entity_getname, (setter)Entity_setname,
     "Entity Health",
     NULL},
    {NULL}  /* Sentinel */
};
PyMethodDef BaseEntity_methods[] = {
    								{"Spawn",  (PyCFunction)pyi_baseentity_spawn, METH_VARARGS,
 								    "Registers commands to be processed by the handler"},
    								{NULL, NULL, 0, NULL}};

PyObject *Entity_gethp(gs_BaseEntityObject *self, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
 		return PyLong_FromLong(tbl->user->GetHealth());
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl)
			return PyFloat_FromDouble(tbl->bot_user->health);
	}
    return PyLong_FromLong(0);
}

int Entity_sethp(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		tbl->user->SetHealth(PyFloat_AsDouble(value));
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl) {
			tbl->bot_user->health = PyFloat_AsDouble(value);
		}
	}
	return 0;
}

PyObject *Entity_getarmour(gs_BaseEntityObject *self, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
 		return PyLong_FromLong(tbl->user->GetArmour());
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl)
			return PyFloat_FromDouble(tbl->bot_user->armour);
	}
    return PyLong_FromLong(0);
}

int Entity_setarmour(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		if(tbl->user) {
			tbl->user->SetArmour(PyFloat_AsDouble(value));
		}
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self);
		if(tbl) {
			tbl->bot_user->health = PyFloat_AsDouble(value);
		}
	}
	return 0;
}

PyObject *Entity_getmodel(gs_BaseEntityObject *self, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
 		return PyLong_FromLong(tbl->user->GetModelID());
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl)
			return PyFloat_FromDouble(tbl->bot_user->modelid);
	}
    return PyLong_FromLong(100);
}

int Entity_setmodel(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		tbl->user->SetSkin(PyLong_AsLong(value));
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl) {
			tbl->bot_user->modelid = PyLong_AsLong(value);
		}	
	}
	return 0;
}



PyObject *Entity_getpos(gs_BaseEntityObject *self, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	PyObject *list = PyList_New(3);
	float *pos;
	if(tbl) {
		Py_INCREF(list);
		if(tbl) {
			pos = tbl->user->GetPosition();
			PyList_SET_ITEM(list, 0, PyFloat_FromDouble(pos[0]));
			PyList_SET_ITEM(list, 1, PyFloat_FromDouble(pos[1]));
			PyList_SET_ITEM(list, 2, PyFloat_FromDouble(pos[2]));
			Py_INCREF(list);		
			return list;
		}
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl) {
			pos = (float *)&tbl->bot_user->pos;
			PyList_SET_ITEM(list, 0, PyFloat_FromDouble(pos[0]));
			PyList_SET_ITEM(list, 1, PyFloat_FromDouble(pos[1]));
			PyList_SET_ITEM(list, 2, PyFloat_FromDouble(pos[2]));
			Py_INCREF(list);		
			return list;
		}
	}
    return PyLong_FromLong(100);
}

int Entity_setpos(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	float x,y,z;
	if(PyList_Check(value)) {		
		PyObject *seq = PySequence_Fast(value, "expected a sequence");
		PyObject *pyx = PyList_GET_ITEM(seq, 0);
		PyObject *pyy = PyList_GET_ITEM(seq, 1);
		PyObject *pyz = PyList_GET_ITEM(seq, 2);
		x = PyFloat_AsDouble(pyx);
		y = PyFloat_AsDouble(pyy);
		z = PyFloat_AsDouble(pyz);
	}
	if(tbl) {
			tbl->user->SetPosition(x,y,z);
	} else {
		tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
		if(tbl) {
			tbl->bot_user->pos[0] = x;
			tbl->bot_user->pos[1] = y;
			tbl->bot_user->pos[2] = z;
		}
	}
	return 0;
}

PyObject *pyi_baseentity_spawn(gs_BaseEntityObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		PyObject* dict;
		if (!PyArg_ParseTuple(args, "O", &dict)) {
			tbl->user->SpawnPlayer(0.0, 0.0, 15.0, 1);
		} else {
			float x = 0.0,y = 0.0,z = 0.0;
			int skin = 0;
			int team = -1;
			PyObject *pos = PyDict_GetItemString(dict, "position");
			PyObject *model = PyDict_GetItemString(dict, "model");
			if(pos) {
				PyObject *pyx = PyList_GET_ITEM(pos, 0);
				PyObject *pyy = PyList_GET_ITEM(pos, 1);
				PyObject *pyz = PyList_GET_ITEM(pos, 2);
				x = PyFloat_AsDouble(pyx);
				y = PyFloat_AsDouble(pyy);
				z = PyFloat_AsDouble(pyz);
			}
			if(model) {
				skin = PyLong_AsLong(model);
			}
			tbl->user->SpawnPlayer(x, y, z, skin);	
		}
	}
	Py_RETURN_NONE;
}
int Entity_setname(gs_BaseEntityObject *self, PyObject *value, void *closure) {
	PyObject *bstr = PyUnicode_AsASCIIString(value);
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(PyBytes_Check(bstr)) {
		if(tbl) {
			tbl->user->SetName(PyBytes_AsString(bstr));
		} else {
			tbl = gbl_pi_interface->findClientByEntity((PyObject *)self, true, true);
			if(tbl) {
				strcpy(tbl->bot_user->name, PyBytes_AsString(bstr));
			}
		}
	}
	return 0;
}
PyObject *Entity_getname(gs_BaseEntityObject *self, void *closure) {

}