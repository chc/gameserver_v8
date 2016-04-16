#include "../PythonInterface.h"
#include "../SAMP/SAMPRakPeer.h"
#include <structmember.h>
#include <server/CHCGameServer.h>


PyTypeObject gs_ConnectionType = {
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


PyTypeObject gs_CommandHandlerType = {
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

PyObject *gs_conn_sendmsg(PyObject *self, PyObject *args);
PyObject *gs_conn_setconnhandler(PyObject *self, PyObject *args);
PyObject *pyi_conn_getip(PyObject *self, PyObject *args);
PyObject *pyi_conn_getport(PyObject *self, PyObject *args);
PyObject *gs_conn_setentity(PyObject *self, PyObject *args);
PyObject *pyi_conn_disconnect(PyObject *self, PyObject *args);
PyObject *gs_conn_addentity(PyObject *self, PyObject *args);
PyObject *pyi_cmdhndlr_registercmds(PyObject *self, PyObject *args);

static PyMethodDef GameserverMethods[] = {
     {"SetConnectionHandler", gs_conn_setconnhandler, METH_O, "Sets the connection handler"},
     {"AddEntity", gs_conn_addentity, METH_VARARGS, "Sets the connection handler"},
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


PyMethodDef conn_methods[] = {
                                    {"GetIP",  pyi_conn_getip, METH_VARARGS,
                                    "Gets a connections IP"},
                                    {"GetPort",  pyi_conn_getport, METH_VARARGS,
                                    "Gets a connections source port"},
                                    {"Disconnect", pyi_conn_disconnect, METH_VARARGS, "Disconnects a client"},
                                    {"SendMessage",  gs_conn_sendmsg, METH_VARARGS, "Sends a console message"},
                                    {"SetEntity",  gs_conn_setentity, METH_VARARGS, "Registers an entity to a connection"},
                                    {NULL, NULL, 0, NULL}};

PyMethodDef cmdhandler_methods[] = {
                                    {"RegisterCommands",  pyi_cmdhndlr_registercmds, METH_VARARGS,
                                    "Registers commands to be processed by the handler"},
                                    {NULL, NULL, 0, NULL}};

PyObject *gs_conn_sendmsg(PyObject *self, PyObject *args)
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

PyObject *gs_conn_setconnhandler(PyObject *self, PyObject *args)
{
	gbl_pi_interface->mp_connection_handler = (PyTypeObject *)args;
    Py_INCREF(gbl_pi_interface->mp_connection_handler);
    Py_RETURN_NONE;
}



PyObject *pyi_conn_getip(PyObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	const struct sockaddr_in *addr = tbl->user->getAddress();
    return PyUnicode_FromString(inet_ntoa(addr->sin_addr));
}
PyObject *pyi_conn_getport(PyObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	const struct sockaddr_in *addr = tbl->user->getAddress();
    return PyLong_FromLong(htons(addr->sin_port));
}
PyObject *gs_conn_setentity(PyObject *self, PyObject *args)
{
	ClientInfoTable *tbl = gbl_pi_interface->findClientByConnObj(self);
	 PyObject* obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
        Py_RETURN_NONE;
    tbl->entity = obj;
	Py_RETURN_NONE;
}


PyObject *pyi_conn_disconnect(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

PyObject *pyi_cmdhndlr_registercmds(PyObject *self, PyObject *args)
{
    PyObject* obj;
    PyObject *seq;
    PyObject *cmd_dict;

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


PyObject *gs_conn_addentity(PyObject *self, PyObject *args) {
    ClientInfoTable *tbl;
    PyObject* obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
        Py_RETURN_NONE;

    tbl = gbl_pi_interface->findClientByEntity((PyObject *)obj);
    printf("Got tbl: %p\n", tbl);
    SAMPDriver *driver = gbl_pi_interface->getGameServer()->getSAMPDriver();
    driver->AddBot(tbl->bot_user);
    Py_RETURN_NONE;
}




PyObject *gs_Conn_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
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
