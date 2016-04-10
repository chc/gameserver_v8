#include "PythonInterface.h"
#include "CHCGameServer.h"
#include "SAMP/SAMPRakPeer.h"
#include <arpa/inet.h>
#include <Python.h>
#include <structmember.h>

static PythonScriptInterface *gbl_pi_interface = NULL;
//Connection class
typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} gs_ConnectionObject;
static PyTypeObject gs_ConnectionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "CoreServer.Connection",             /*tp_name*/
    sizeof(gs_ConnectionObject), /*tp_basicsize*/
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
    "Connection Object",           /* tp_doc */
};

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} gs_CommandHandlerObject;
static PyTypeObject gs_CommandHandlerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "CoreServer.CommandHandler",             /*tp_name*/
    sizeof(gs_CommandHandlerObject), /*tp_basicsize*/
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
    "Command Handler Object",           /* tp_doc */
};

typedef struct {
    PyObject_HEAD
    PyObject *connection;
    /* Type-specific fields go here. */
} gs_BaseEntityObject;
static PyTypeObject gs_BaseEntityType = {
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

static PyObject *
gs_conn_sendmsg(PyObject *self, PyObject *args)
{
	
    const char *msg;
    unsigned colour;
    int sts;

    if (!PyArg_ParseTuple(args, "Is", &colour, &msg))
        return NULL;
    ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
    tbl->user->SendClientMessage(colour, msg);
    Py_RETURN_NONE;
}

static PyObject *
gs_conn_setconnhandler(PyObject *self, PyObject *args)
{
	gbl_pi_interface->mp_connection_handler = (PyTypeObject *)args;
    Py_RETURN_NONE;
}

static PyMethodDef GameserverMethods[] = {
     {"SetConnectionHandler", gs_conn_setconnhandler, METH_O, "Sets the connection handler"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef coreserver_module = {
   PyModuleDef_HEAD_INIT,
   "CoreServer",   /* name of module */
   NULL, /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   GameserverMethods
};

static PyObject *
pyi_conn_getip(PyObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	const struct sockaddr_in *addr = tbl->user->getAddress();
    return PyUnicode_FromString(inet_ntoa(addr->sin_addr));
}
static PyObject *
pyi_conn_getport(PyObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	const struct sockaddr_in *addr = tbl->user->getAddress();
    return PyLong_FromLong(htons(addr->sin_port));
}
static PyObject *
gs_conn_setentity(PyObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	 PyObject* obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
        Py_RETURN_NONE;
    tbl->entity = obj;
	Py_RETURN_NONE;
}


static PyObject *
pyi_conn_disconnect(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

static PyObject *
pyi_cmdhndlr_registercmds(PyObject *self, PyObject *args)
{
    PyObject* obj;
    PyObject *seq;
    PyObject *cmd_dict;

	Py_ssize_t pos = 0;
    int len;
    if (!PyArg_ParseTuple(args, "O", &obj))
        Py_RETURN_NONE;
 	seq = PySequence_Fast(obj, "expected a sequence");
    len = PySequence_Size(obj);
    ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	if(PyList_Check(obj)) {
		for(int i=0;i<len;i++) {
			cmd_dict = PyList_GET_ITEM(seq, i);
			if(PyDict_Check(cmd_dict)) {
				ScriptCommand *cmd = PythonScriptInterface::GetScriptCmdFromPyDict(self, cmd_dict);
				tbl->registered_commands.push_back(cmd);
			}

		}
	}
	Py_RETURN_NONE;  
}

static PyObject *pyi_baseentity_spawn(gs_BaseEntityObject *self, PyObject *args)
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

 static PyMethodDef conn_methods[] = {
    								{"GetIP",  pyi_conn_getip, METH_VARARGS,
 								    "Gets a connections IP"},
    								{"GetPort",  pyi_conn_getport, METH_VARARGS,
 								    "Gets a connections source port"},
 								    {"Disconnect", pyi_conn_disconnect, METH_VARARGS, "Disconnects a client"},
 								    {"SendMessage",  gs_conn_sendmsg, METH_VARARGS, "Sends a console message"},
 								    {"SetEntity",  gs_conn_setentity, METH_VARARGS, "Sends a console message"},
    								{NULL, NULL, 0, NULL}};

static PyMethodDef cmdhandler_methods[] = {
    								{"RegisterCommands",  pyi_cmdhndlr_registercmds, METH_VARARGS,
 								    "Registers commands to be processed by the handler"},
    								{NULL, NULL, 0, NULL}};
static PyObject *
Entity_gethp(gs_BaseEntityObject *self, void *closure)
{
    return PyLong_FromLong(100);
}

static int
Entity_sethp(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		tbl->user->SetHealth(PyFloat_AsDouble(value));
	}
	return 0;
}

static PyObject *
Entity_getarmour(gs_BaseEntityObject *self, void *closure)
{
    return PyLong_FromLong(100);
}

static int
Entity_setarmour(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		tbl->user->SetArmour(PyFloat_AsDouble(value));
	}
	return 0;
}

static PyObject *
Entity_getmodel(gs_BaseEntityObject *self, void *closure)
{
    return PyLong_FromLong(100);
}

static int
Entity_setmodel(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		tbl->user->SetSkin(PyLong_AsLong(value));
	}
	return 0;
}



static PyObject *
Entity_getpos(gs_BaseEntityObject *self, void *closure)
{
    return PyLong_FromLong(100);
}

static int
Entity_setpos(gs_BaseEntityObject *self, PyObject *value, void *closure)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self->connection);
	if(tbl) {
		if(PyList_Check(value)) {
			float x,y,z;
			PyObject *seq = PySequence_Fast(value, "expected a sequence");
			PyObject *pyx = PyList_GET_ITEM(seq, 0);
			PyObject *pyy = PyList_GET_ITEM(seq, 1);
			PyObject *pyz = PyList_GET_ITEM(seq, 2);
			x = PyFloat_AsDouble(pyx);
			y = PyFloat_AsDouble(pyy);
			z = PyFloat_AsDouble(pyz);
			tbl->user->SetPosition(x,y,z);
		}
	}
	return 0;
}

static PyMemberDef BaseEntity_members[] = {
    {"connection", T_OBJECT, offsetof(gs_BaseEntityObject, connection), 0,
     "Entitys connection"},
    {NULL}  /* Sentinel */
};

static PyGetSetDef Entity_getseters[] = {
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
    {NULL}  /* Sentinel */
};
static PyMethodDef BaseEntity_methods[] = {
    								{"Spawn",  (PyCFunction)pyi_baseentity_spawn, METH_VARARGS,
 								    "Registers commands to be processed by the handler"},
    								{NULL, NULL, 0, NULL}};
static PyObject *
gs_Conn_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *self;

    self = (PyObject *)type->tp_alloc(type, 0);
    gbl_pi_interface->mp_current_info_table->connection_object = self;
    return (PyObject *)self;
}
PyMODINIT_FUNC
PyInit_CoreServer(void)
{
	PyObject *m;
    gs_ConnectionType.tp_new = gs_Conn_new;//PyType_GenericNew;
    gs_CommandHandlerType.tp_new = PyType_GenericNew;

    gs_BaseEntityType.tp_new = PyType_GenericNew;
    gs_BaseEntityType.tp_getset = Entity_getseters;
    gs_BaseEntityType.tp_members = (struct PyMemberDef *)&BaseEntity_members;
    gs_BaseEntityType.tp_methods = (struct PyMethodDef *)&BaseEntity_methods;

    gs_ConnectionType.tp_methods = (struct PyMethodDef*)&conn_methods; 

    gs_CommandHandlerType.tp_methods = (struct PyMethodDef *)&cmdhandler_methods;
    
    if (PyType_Ready(&gs_ConnectionType) < 0)
        return NULL;

   if (PyType_Ready(&gs_CommandHandlerType) < 0)
        return NULL;

    
    if (PyType_Ready(&gs_BaseEntityType) < 0)
        return NULL;
	m = PyModule_Create(&coreserver_module);

	Py_INCREF(&gs_ConnectionType);
	PyModule_AddObject(m, "Connection", (PyObject *)&gs_ConnectionType);

	Py_INCREF(&gs_CommandHandlerType);
	PyModule_AddObject(m, "CommandHandler", (PyObject *)&gs_CommandHandlerType);

	Py_INCREF(&gs_BaseEntityType);
	PyModule_AddObject(m, "BaseEntity", (PyObject *)&gs_BaseEntityType);
    return m;
}

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


PythonScriptInterface::PythonScriptInterface(CHCGameServer *gameserver) : IScriptInterface(gameserver) {
  gbl_pi_interface = this;
  mp_current_info_table = NULL;

  PyImport_AppendInittab("CoreServer", PyInit_CoreServer);
  PyImport_AppendInittab("Frontend", PyInit_FrontEnd);

  Py_Initialize();
  FILE *fd = fopen("scripts/__init__.py", "rb");
  PyRun_AnyFile(fd, NULL);
  fclose(fd);
 
}

PythonScriptInterface::~PythonScriptInterface() {
	Py_DECREF(&gs_ConnectionType);
	Py_Finalize();
}
void PythonScriptInterface::run() {

}
ClientInfoTable *PythonScriptInterface::findClientByConnObj(PyObject *conn) {
	ClientInfoTable *table;
	std::vector<ClientInfoTable *>::iterator it = m_clients.begin();
	while(it != m_clients.end()) {
		table = *it;
		if(table->connection_object == conn) {
			return table;
		}
		it++;
	}

	return NULL;
}
ClientInfoTable *PythonScriptInterface::findClient(void *user, bool create) {
	ClientInfoTable *table;
	std::vector<ClientInfoTable *>::iterator it = m_clients.begin();
	while(it != m_clients.end()) {
		table = *it;
		if(table->user == user) {
			return table;
		}
		it++;
	}
	if(create) {
		table = (ClientInfoTable *)malloc(sizeof(ClientInfoTable));
		memset(table, 0, sizeof(ClientInfoTable));
		table->user = (SAMPRakPeer *)user;
		m_clients.push_back(table);
		return table;
	}

	return NULL;
}
void PythonScriptInterface::HandleClientCommand(void *user, const char *extra) {
	ClientInfoTable *tbl = findClient(user);
	if(tbl) {
		std::vector<ScriptCommand *>::iterator it = tbl->registered_commands.begin();
		ScriptCommand *cmd;
		while(it != tbl->registered_commands.end()) {
			cmd = *it;
			if(strncasecmp(cmd->name,&extra[1], strlen(cmd->name)) == 0) {
				PyObject *ret = PyObject_CallFunction(cmd->run_func, "s", extra);
				PyErr_Print();
				Py_XDECREF(ret);
				break;
			}
			it++;
		}
	}
}
void PythonScriptInterface::HandleEvent(int event_id, void *user, void *extra) {
	ClientInfoTable *tbl = findClient(user, true);
	mp_current_info_table = tbl;
	PyObject* sys_mod_dict = PyImport_GetModuleDict();
	PyObject* main_mod = PyMapping_GetItemString(sys_mod_dict, "__main__");
	switch(event_id) {
		case CHCGS_ClientConnectEvent: {
			tbl->connection_object = PyObject_CallMethod(main_mod, mp_connection_handler->tp_name, "");
			PyErr_Print();
			printf("Client connect %p | %p (%s)\n", tbl->connection_object, mp_connection_handler, mp_connection_handler->tp_name);
			break;
		}
		case CHCGS_ClientDisconnectEvent: {
			printf("Client disconnect\n");
			break;
		}
		case CHCGS_EnterWorld: {
			if(tbl->entity)
				PyObject_CallMethod(tbl->entity, "OnEnterWorld", "");
			break;
		}
		case CHCGS_ClientCommand: {
			HandleClientCommand(user, (const char *)extra);
			break;
		}
		case CHCGS_DialogResponse: {
			DialogEvent *devent = (DialogEvent *)extra;
			if(tbl->last_dialog_callback) {
				PyObject *arglist = Py_BuildValue("iis",(int)devent->list_index, (int)devent->button_id,devent->input);
				PyObject *ret = PyObject_CallObject(tbl->last_dialog_callback, arglist);
				Py_DECREF(tbl->last_dialog_callback);
				tbl->last_dialog_callback = NULL;
				PyErr_Print();
				Py_XDECREF(arglist);
				Py_XDECREF(ret);
			}
			break;
		}
	}
}
char *PythonScriptInterface::copyPythonString(PyObject *string) {
	PyObject *bstr = PyUnicode_AsASCIIString(string);
	if(bstr && PyBytes_Check(bstr)) {
		return strdup(PyBytes_AsString(bstr));
	}
	return strdup(""); //change this to null when you write the cleaner cleanup...
}
ScriptCommand *PythonScriptInterface::GetScriptCmdFromPyDict(PyObject *self, PyObject *dict) {
	PyObject *primarycmd = PyDict_GetItemString(dict, "primary_command");
	PyObject *function = PyDict_GetItemString(dict, "function");
	PyObject *aliases = PyDict_GetItemString(dict, "aliases");
	if(function && primarycmd) {
		ScriptCommand *cmd = (ScriptCommand *)malloc(sizeof(ScriptCommand));
		cmd->name = copyPythonString(primarycmd);

		Py_INCREF(function);
		cmd->run_func = function;
		Py_INCREF(self);
		cmd->self = self;
		return cmd;
	}
	return NULL;
}