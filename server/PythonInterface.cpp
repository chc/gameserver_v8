#include "main.h"
#include "PythonInterface.h"
#include "CHCGameServer.h"
#include "SAMP/SAMPPlayer.h"
#include "SAMP/SAMPRakPeer.h"
#include <Python.h>
#include <structmember.h>

PythonScriptInterface *gbl_pi_interface = NULL;

PyMODINIT_FUNC PyInit_FrontEnd(void);
PyMODINIT_FUNC PyInit_CoreServer(void);
PyMODINIT_FUNC PyInit_SAMP(void);


PythonScriptInterface::PythonScriptInterface(CHCGameServer *gameserver) : IScriptInterface(gameserver) {
  gbl_pi_interface = this;
  mp_current_info_table = NULL;

  PyImport_AppendInittab("CoreServer", PyInit_CoreServer);
  PyImport_AppendInittab("Frontend", PyInit_FrontEnd);
  PyImport_AppendInittab("SAMP", PyInit_SAMP);

  Py_SetPythonHome(L"E:\\Code\\Python-3.5.0"); //temp windows fix

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
	if(conn == NULL) return NULL;
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
ClientInfoTable *PythonScriptInterface::findClientByEntity(PyObject *entity, bool create, bool create_bot) {
	ClientInfoTable *table;
	std::vector<ClientInfoTable *>::iterator it = m_clients.begin();
	while(it != m_clients.end()) {
		table = *it;
		if(table->entity == (void *)entity) {
			return table;
		}
	}
	if(create) {
		table = (ClientInfoTable *)malloc(sizeof(ClientInfoTable));
		memset(table, 0, sizeof(ClientInfoTable));
		table->entity = entity;

		if(create_bot) {
			table->bot_user = getGameServer()->getSAMPDriver()->CreateBot();
		}
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
			Py_XDECREF(tbl->connection_object);
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
		case CHCGS_SpawnSelect: {
			if(tbl->entity) {
				PyObject_CallMethod(tbl->connection_object, "OnSpawnSelect", "i", extra);
			}
			break;
		}
		case CHCGS_ChatMessage: {
			PyObject_CallMethod(tbl->connection_object, "OnChatMessage", "s", extra);
			break;
		}
		case CHCGS_UIClick: {
			if(tbl->mouse_callback) {
				PyObject *arglist;
				if(extra == (void *)-1) {
					arglist = Py_BuildValue("Os", tbl->connection_object, NULL);
				} else {
					arglist = Py_BuildValue("Oi", tbl->connection_object,extra);
				}
				PyObject *ret = PyObject_CallObject(tbl->mouse_callback, arglist);
				PyErr_Print();
				Py_XDECREF(arglist);
				Py_XDECREF(ret);
			}
			break;
		}
		case CHCGS_PlayerDeath: {
			SAMPDeathInfo *death_info = (SAMPDeathInfo *)extra;
			SAMPDriver *driver = this->getGameServer()->getSAMPDriver();
			printf("Send death: %d  %d\n", ((SAMPPlayer *)user)->GetPlayerID(), death_info->killer_id);
			driver->BroadcastDeath((SAMPPlayer *)user, (SAMPPlayer *)driver->findPlayerByID(death_info->killer_id), 0);
			break;
		}
	}
}
char *PythonScriptInterface::copyPythonString(PyObject *string) {
	if(string) {
		PyObject *bstr = PyUnicode_AsASCIIString(string);
		if(bstr && PyBytes_Check(bstr)) {
			return strdup(PyBytes_AsString(bstr));
		}
	}
	return NULL;
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