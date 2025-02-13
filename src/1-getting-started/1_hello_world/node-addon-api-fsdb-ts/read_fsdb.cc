#include <napi.h>

#include <vector>

#include "ffrAPI.h"

//
// NOVAS_FSDB is internally used in NOVAS
//
#ifdef NOVAS_FSDB
#undef NOVAS_FSDB
#endif

#ifndef TRUE
const int TRUE = 1;
#endif

#ifndef FALSE
const int FALSE = 0;
#endif

static void __DumpScope(fsdbTreeCBDataScope *scope);
static void __DumpVar(fsdbTreeCBDataVar *var);
static bool_T MyTreeCB(fsdbTreeCBType cb_type, void *client_data,
                       void *tree_cb_data);

// Logical offsets in scope 'Top'
static std::vector<fsdbLUOff> scope_offset;

char *fsdb_name =
    "/home/heyfey/verdi/2022.06/share/FsdbReader_build/example/demoL1.fsdb";

Napi::String readFsdb(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  fsdbRC rc = ffrObject::ffrCheckFile(fsdb_name);
  if (rc == FSDB_RC_FILE_DOES_NOT_EXIST) {
    return Napi::String::New(env, "not exist");
  } else if (rc == FSDB_RC_FILE_IS_NOT_READABLE) {
    return Napi::String::New(env, "not readable");
  } else if (rc == FSDB_RC_FILE_IS_A_DIRECTORY) {
    return Napi::String::New(env, "is directory");
  } else if (rc == FALSE) {
    // return Napi::String::New(env, "ffrCheckFile return false");
  } else if (rc == FSDB_RC_SUCCESS) {
    // return Napi::String::New(env, "rc success");
  }

  if (FALSE == ffrObject::ffrIsFSDB(fsdb_name)) {
    return Napi::String::New(env, "ffrIsFSDB return false");
  }

  // Don't rely on checkFSDB. Prefer use ffrCheckFile + ffrIsFSDB
  // if (FSDB_RC_FAILURE == ffrObject::ffrCheckFSDB(fsdb_name)) {
  //     return Napi::String::New(env, "ffrCheckFSDB failed");
  // }

  ffrObject *fsdb_obj = ffrObject::ffrOpen3(fsdb_name);
  if (nullptr == fsdb_obj) {
    return Napi::String::New(env, "ffrObject::ffrOpen() failed.");
  }
  fsdb_obj->ffrSetTreeCBFunc(MyTreeCB, NULL);

  fsdb_obj->ffrReadScopeTree();
  for (size_t i = 0; i < scope_offset.size(); i++) {
    if (FSDB_RC_FAILURE == fsdb_obj->ffrReadVarByLogUOff(&scope_offset[i])) {
      return Napi::String::New(env, "ffrReadVarByLogUOff failed");
    }
  }

  fsdb_obj->ffrClose();
  return Napi::String::New(env, "Success.");
}

//
// NAME : MyTreeCB
//
// DESCRIPTION: A callback function used by fsdb reader
//
// PARAMETERS : See fsdb reader document.
//
// RETURN : See fsdb reader document.
//

static bool_T MyTreeCB(fsdbTreeCBType cb_type, void *client_data,
                       void *tree_cb_data) {
  fsdbTreeCBDataScope *pScope;
  fsdbTreeCBDataUpscope *pUpscope;
  fsdbTreeCBDataVar *pVar;

  switch (cb_type) {
    case FSDB_TREE_CBT_BEGIN_TREE:
      fprintf(stderr, "<BeginTree>\n");
      break;

    case FSDB_TREE_CBT_SCOPE:
      // The first time 'Top' is entered, record its logical offset.
      // Other logical offsets of 'Top' will be known when an
      // UPSCOPE sets the current traverse point to 'Top'.
      pScope = (fsdbTreeCBDataScope *)tree_cb_data;
      scope_offset.push_back(pScope->var_start_log_uoff);
      __DumpScope(pScope);
      break;

    case FSDB_TREE_CBT_UPSCOPE:
      pUpscope = (fsdbTreeCBDataUpscope *)tree_cb_data;
      scope_offset.push_back(pUpscope->var_start_log_uoff);
      fprintf(stderr, "<Upscope>\n");
      break;

    case FSDB_TREE_CBT_VAR:
      pVar = (fsdbTreeCBDataVar *)tree_cb_data;
      __DumpVar(pVar);
      break;

    case FSDB_TREE_CBT_END_TREE:
      fprintf(stderr, "<EndTree>\n\n");
      break;

    case FSDB_TREE_CBT_ARRAY_BEGIN:
      fprintf(stderr, "<BeginArray>\n");
      break;

    case FSDB_TREE_CBT_ARRAY_END:
      fprintf(stderr, "<EndArray>\n\n");
      break;

    case FSDB_TREE_CBT_RECORD_BEGIN:
      fprintf(stderr, "<BeginRecord>\n");
      break;

    case FSDB_TREE_CBT_RECORD_END:
      fprintf(stderr, "<EndRecord>\n\n");
      break;

    case FSDB_TREE_CBT_FILE_TYPE:
      break;

    case FSDB_TREE_CBT_SIMULATOR_VERSION:
      break;

    case FSDB_TREE_CBT_SIMULATION_DATE:
      break;

    case FSDB_TREE_CBT_X_AXIS_SCALE:
      break;

    case FSDB_TREE_CBT_END_ALL_TREE:
      break;

    default:
      return TRUE;
  }

  return TRUE;
}

static void __DumpScope(fsdbTreeCBDataScope *scope) {
  str_T type;

  switch (scope->type) {
    case FSDB_ST_VCD_MODULE:
      type = (str_T) "module";
      break;

    case FSDB_ST_VCD_TASK:
      type = (str_T) "task";
      break;

    case FSDB_ST_VCD_FUNCTION:
      type = (str_T) "function";
      break;

    case FSDB_ST_VCD_BEGIN:
      type = (str_T) "begin";
      break;

    case FSDB_ST_VCD_FORK:
      type = (str_T) "fork";
      break;

    default:
      type = (str_T) "unknown_scope_type";
      break;
  }

  fprintf(stderr, "<Scope> name:%s  type:%s\n", scope->name, type);
}

static void __DumpVar(fsdbTreeCBDataVar *var) {
  str_T type;
  str_T bpb;

  switch (var->bytes_per_bit) {
    case FSDB_BYTES_PER_BIT_1B:
      bpb = (str_T) "1B";
      break;

    case FSDB_BYTES_PER_BIT_2B:
      bpb = (str_T) "2B";
      break;

    case FSDB_BYTES_PER_BIT_4B:
      bpb = (str_T) "4B";
      break;

    case FSDB_BYTES_PER_BIT_8B:
      bpb = (str_T) "8B";
      break;

    default:
      bpb = (str_T) "?B";
      break;
  }

  switch (var->type) {
    case FSDB_VT_VCD_EVENT:
      type = (str_T) "event";
      break;

    case FSDB_VT_VCD_INTEGER:
      type = (str_T) "integer";
      break;

    case FSDB_VT_VCD_PARAMETER:
      type = (str_T) "parameter";
      break;

    case FSDB_VT_VCD_REAL:
      type = (str_T) "real";
      break;

    case FSDB_VT_VCD_REG:
      type = (str_T) "reg";
      break;

    case FSDB_VT_VCD_SUPPLY0:
      type = (str_T) "supply0";
      break;

    case FSDB_VT_VCD_SUPPLY1:
      type = (str_T) "supply1";
      break;

    case FSDB_VT_VCD_TIME:
      type = (str_T) "time";
      break;

    case FSDB_VT_VCD_TRI:
      type = (str_T) "tri";
      break;

    case FSDB_VT_VCD_TRIAND:
      type = (str_T) "triand";
      break;

    case FSDB_VT_VCD_TRIOR:
      type = (str_T) "trior";
      break;

    case FSDB_VT_VCD_TRIREG:
      type = (str_T) "trireg";
      break;

    case FSDB_VT_VCD_TRI0:
      type = (str_T) "tri0";
      break;

    case FSDB_VT_VCD_TRI1:
      type = (str_T) "tri1";
      break;

    case FSDB_VT_VCD_WAND:
      type = (str_T) "wand";
      break;

    case FSDB_VT_VCD_WIRE:
      type = (str_T) "wire";
      break;

    case FSDB_VT_VCD_WOR:
      type = (str_T) "wor";
      break;

    case FSDB_VT_VHDL_SIGNAL:
      type = (str_T) "signal";
      break;

    case FSDB_VT_VHDL_VARIABLE:
      type = (str_T) "variable";
      break;

    case FSDB_VT_VHDL_CONSTANT:
      type = (str_T) "constant";
      break;

    case FSDB_VT_VHDL_FILE:
      type = (str_T) "file";
      break;

    case FSDB_VT_VCD_MEMORY:
      type = (str_T) "vcd_memory";
      break;

    case FSDB_VT_VHDL_MEMORY:
      type = (str_T) "vhdl_memory";
      break;

    case FSDB_VT_VCD_MEMORY_DEPTH:
      type = (str_T) "vcd_memory_depth_or_range";
      break;

    case FSDB_VT_VHDL_MEMORY_DEPTH:
      type = (str_T) "vhdl_memory_depth";
      break;

    default:
      type = (str_T) "unknown_var_type";
      break;
  }

  fprintf(stderr, "<Var>  name:%s  l:%u  r:%u  type:%s  ", var->name,
          var->lbitnum, var->rbitnum, type);
  fprintf(stderr, "idcode:%llu  dtidcode:%u  bpb:%s\n", var->u.idcode,
          var->dtidcode, bpb);
}

Napi::String Method(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "world");
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "hello"),
              Napi::Function::New(env, Method));
  exports.Set(Napi::String::New(env, "readFsdb"),
              Napi::Function::New(env, readFsdb));
  return exports;
}

NODE_API_MODULE(hello, Init)
