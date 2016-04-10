#ifndef _PYSCRIPTINTERFACE_H
#define _PYSCRIPTINTERFACE_H
#include "main.h"
#include "ScriptInterface.h"
#include <vector>
#include <Python.h>
class SAMPRakPeer;

typedef struct {
	const char *name;
	std::vector<const char *> aliases;
	PyObject *run_func;
	PyObject *self;
} ScriptCommand;


typedef struct {
	SAMPRakPeer *user;
	PyObject* connection_object;
	PyObject* entity;
	PyObject* last_dialog_callback;
	std::vector<ScriptCommand *> registered_commands; //curently 1 allocated per cmd per user... make globally shared
} ClientInfoTable;


class PythonScriptInterface : public IScriptInterface {
public:
	PythonScriptInterface(CHCGameServer *gameserver);
	~PythonScriptInterface();
	ClientInfoTable *findClient(void *user, bool create = false);
	ClientInfoTable *findClientByConnObj(PyObject *conn);
	void run();
	void HandleEvent(int event_id, void *user, void *extra);
	void HandleClientCommand(void *user, const char *extra);
	std::vector<ClientInfoTable *> m_clients;
	std::vector<ScriptCommand *> m_commands;
	ClientInfoTable *mp_current_info_table; //used for adding internal data from PyObjects

	PyTypeObject *mp_connection_handler;
	static char *copyPythonString(PyObject *string);


	//stuff that should private
	static ScriptCommand *GetScriptCmdFromPyDict(PyObject *self, PyObject *dict);

};
#endif //_PYSCRIPTINTERFACE_H