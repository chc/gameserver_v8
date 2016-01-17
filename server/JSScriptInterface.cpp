#include "CHCGameServer.h"
#include <stdlib.h>
#include "JSScriptInterface.h"

v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate, CHCGameServer *gameserver);
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
            char* argv[]);
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
                   v8::Local<v8::Value> name, bool print_result,
                   bool report_exceptions);
void Print(const v8::FunctionCallbackInfo<v8::Value>& args);

v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);


// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

class ShellArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  virtual void* Allocate(size_t length) {
    void* data = AllocateUninitialized(length);
    return data == NULL ? data : memset(data, 0, length);
  }
  virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
  virtual void Free(void* data, size_t) { free(data); }
};



JSScriptInterface::JSScriptInterface(CHCGameServer *gameserver) : IScriptInterface(gameserver) {
	v8::V8::InitializeICU();
	v8::V8::InitializeExternalStartupData(".");
	v8::Platform* platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();
	ShellArrayBufferAllocator array_buffer_allocator;
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = &array_buffer_allocator;
	v8::Isolate* isolate = v8::Isolate::New(create_params);
	isolate->SetData(0, this);

    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = CreateShellContext(isolate, gameserver);
    if (context.IsEmpty()) {
      fprintf(stderr, "Error creating context\n");
    }
    v8::Context::Scope context_scope(context);




    const char* str = "test.js";
    // Use all other arguments as names of files to load and run.
    v8::Local<v8::String> file_name =
        v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal)
            .ToLocalChecked();
    v8::Local<v8::String> source;
    if (!ReadFile(isolate, str).ToLocal(&source)) {
      fprintf(stderr, "Error reading '%s'\n", str);
      return;
    }
    bool success = ExecuteString(isolate, source, file_name, false, true);
    while (v8::platform::PumpMessageLoop(platform, isolate)) continue;

/*
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete platform;
*/
}
void JSScriptInterface::run() {

}

void JSScriptInterface::GetGameserverName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	v8::Local<v8::String> name  = v8::String::NewFromUtf8(isolate, interface->getGameServer()->getName());
	v8::EscapableHandleScope scope(isolate);
	info.GetReturnValue().Set(scope.Escape(name));
}
void JSScriptInterface::SetGameserverName(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	v8::String::Utf8Value name(value->ToString());
	interface->getGameServer()->setName(*name);
}


void JSScriptInterface::SetGameserverVersion(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	v8::String::Utf8Value name(value->ToString());
	interface->getGameServer()->setVersion(*name);

}
void JSScriptInterface::GetGameserverVersion(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	v8::Local<v8::String> name  = v8::String::NewFromUtf8(isolate, interface->getGameServer()->getVersion());
	v8::EscapableHandleScope scope(isolate);
	info.GetReturnValue().Set(scope.Escape(name));
}


void JSScriptInterface::SetGameserverMap(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	 v8::String::Utf8Value name(value->ToString());
	 interface->getGameServer()->setMap(*name);

}
void JSScriptInterface::GetGameserverMap(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, interface->getGameServer()->getMap());
	v8::EscapableHandleScope scope(isolate);
	info.GetReturnValue().Set(scope.Escape(name));
}


void JSScriptInterface::SetGameserverPlayerLimit(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	 uint32_t val = value->Int32Value();
	 interface->getGameServer()->setMaxPlayers(val);
}
void JSScriptInterface::GetGameserverPlayerLimit(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
	v8::Isolate *isolate = info.GetIsolate();
	JSScriptInterface *interface = (JSScriptInterface *)isolate->GetData(0);
	info.GetReturnValue().Set(interface->getGameServer()->getMaxPlayers());
}


// Creates a new execution environment containing the built-in
// functions.
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate, CHCGameServer *gameserver) {
  // Create a template for the global object.
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
  v8::Local<v8::ObjectTemplate> gameserver_templ = v8::ObjectTemplate::New(isolate);

  // Bind the global 'print' function to the C++ Print callback.
  global->Set(
      v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal)
          .ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, Print));

  
  //gameserver_templ->SetInternalFieldCount(1);
  gameserver_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "name"), JSScriptInterface::GetGameserverName, JSScriptInterface::SetGameserverName);
  gameserver_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "version"), JSScriptInterface::GetGameserverVersion, JSScriptInterface::SetGameserverVersion);
  gameserver_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "map"), JSScriptInterface::GetGameserverMap, JSScriptInterface::SetGameserverMap);
  gameserver_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "player_limit"), JSScriptInterface::GetGameserverPlayerLimit, JSScriptInterface::SetGameserverPlayerLimit);

  global->Set(
      v8::String::NewFromUtf8(isolate, "GameServer", v8::NewStringType::kNormal)
          .ToLocalChecked(),
      gameserver_templ);


  return v8::Context::New(isolate, NULL, global);
}


// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

// Reads a file into a v8 string.
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return v8::MaybeLocal<v8::String>();

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (size_t i = 0; i < size;) {
    i += fread(&chars[i], 1, size - i, file);
    if (ferror(file)) {
      fclose(file);
      return v8::MaybeLocal<v8::String>();
    }
  }
  fclose(file);
  v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
      isolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
  delete[] chars;
  return result;
}


// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
                   v8::Local<v8::Value> name, bool print_result,
                   bool report_exceptions) {
  v8::HandleScope handle_scope(isolate);
  v8::TryCatch try_catch(isolate);
  v8::ScriptOrigin origin(name);
  v8::Local<v8::Context> context(isolate->GetCurrentContext());
  v8::Local<v8::Script> script;
  if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
    // Print errors that happened during compilation.
    if (report_exceptions)
      ReportException(isolate, &try_catch);
    return false;
  } else {
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
      //assert(try_catch.HasCaught());
      // Print errors that happened during execution.
      if (report_exceptions)
        ReportException(isolate, &try_catch);
      return false;
    } else {
      //assert(!try_catch.HasCaught());
      if (print_result && !result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value.
        v8::String::Utf8Value str(result);
        const char* cstr = ToCString(str);
        printf("%s\n", cstr);
      }
      return true;
    }
  }
}


void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Local<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    fprintf(stderr, "%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(
        message->GetSourceLine(context).ToLocalChecked());
    const char* sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    v8::Local<v8::Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString() &&
        v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
      v8::String::Utf8Value stack_trace(stack_trace_string);
      const char* stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}
