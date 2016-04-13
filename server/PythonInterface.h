#ifndef _PYSCRIPTINTERFACE_H
#define _PYSCRIPTINTERFACE_H
#include "main.h"
#include "ScriptInterface.h"
#include <server/SAMP/SAMPDriver.h>
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
	SAMPPlayer *bot_user;
	PyObject* last_dialog_callback;
	std::vector<ScriptCommand *> registered_commands; //curently 1 allocated per cmd per user... make globally shared
} ClientInfoTable;

//defined in other 


class PythonScriptInterface : public IScriptInterface {
public:
	PythonScriptInterface(CHCGameServer *gameserver);
	~PythonScriptInterface();
	ClientInfoTable *findClient(void *user, bool create = false);
	ClientInfoTable *findClientByConnObj(PyObject *conn);
	ClientInfoTable *findClientByEntity(PyObject *entity, bool create = false, bool create_bot = false);
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
extern PythonScriptInterface *gbl_pi_interface;
extern PyTypeObject gs_BaseEntityType;
extern PyGetSetDef Entity_getseters[];
extern PyMethodDef BaseEntity_methods[];
extern PyMemberDef BaseEntity_members[];
extern PyTypeObject gs_CommandHandlerType;
extern PyTypeObject gs_ConnectionType;

typedef struct {
    PyObject_HEAD
    PyObject *connection;
    /* Type-specific fields go here. */
} gs_BaseEntityObject;

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} gs_CommandHandlerObject;

//Connection class
typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} gs_ConnectionObject;

#endif //_PYSCRIPTINTERFACE_H