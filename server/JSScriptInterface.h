#ifndef _JSSCRIPTINTERFACE_H
#define _JSSCRIPTINTERFACE_H
#include "main.h"
#include "ScriptInterface.h"
#include <include/v8.h>
#include <include/libplatform/libplatform.h>

class JSScriptInterface : public IScriptInterface {
public:
	JSScriptInterface(CHCGameServer *gameserver);
	void run();

	/*
		Global JS GameServer Setter/Getter handlers
	*/
	static void GetGameserverName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetGameserverName(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetGameserverVersion(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetGameserverVersion(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetGameserverPlayerLimit(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetGameserverPlayerLimit(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetGameserverMap(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetGameserverMap(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};
#endif //_JSSCRIPTINTERFACE_H