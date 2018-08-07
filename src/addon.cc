#include <stdexcept>
#include <node_api.h>
#include "addon.h"

#define napiErrChk(status) \
  if (status != napi_ok){ \
    const napi_extended_error_info* err_info_ptr; \
    napi_get_last_error_info(env, &err_info_ptr); \
    throw std::runtime_error(err_info_ptr->error_message); \
  }

#define processRuntimeError(deferred) \
  napi_value error, error_code, error_msg; \
  napi_create_string_utf8(env, "ERROR", NAPI_AUTO_LENGTH, &error_code); \
  napi_create_string_utf8(env, e.what(), NAPI_AUTO_LENGTH, &error_msg); \
  napi_create_error(env, error_code, error_msg, &error); \
  napi_reject_deferred(env, deferred, error);

napi_value create_promise(napi_env env, napi_deferred& deferred, napi_async_execute_callback execute, napi_async_complete_callback complete, void* pArgArr){
  napi_value promise;
  napiErrChk(napi_create_promise(env, &deferred, &promise));
  
  napi_async_work asyncWorkResult;
  napi_value arg1, arg2;
  napiErrChk(napi_create_object(env, &arg1));
  napiErrChk(napi_create_object(env, &arg2));
  napiErrChk(napi_create_async_work(env, arg1, arg2, execute, complete, pArgArr, &asyncWorkResult));
  napiErrChk(napi_queue_async_work(env, asyncWorkResult));
  
  return promise;
}

/*=============================================================*
* FUNCTION: example_method
*==============================================================*/
napi_deferred deferred_example_method;

void execute_example_method(napi_env env, void* data){
  arg_example_method* _data = (arg_example_method*)data;
  _data->output = _data->input1 * _data->input2;
}

void complete_example_method(napi_env env, napi_status status, void* data){
  try {
    napiErrChk(status);
    arg_example_method* _data = (arg_example_method*)data;

    napi_value ans;
    napiErrChk(napi_create_double(env, _data->output, &ans));

    napi_value result;
    napiErrChk(napi_create_object(env, &result));
#define DECLARE_NAPI_MEMBER(name) \
  {#name, 0, 0, 0, 0, name, napi_enumerable, 0}
    napi_property_descriptor descriptors[] = {
      DECLARE_NAPI_MEMBER(ans)
    };
    napiErrChk(napi_define_properties(env, result, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    napiErrChk(napi_resolve_deferred(env, deferred_example_method, result));
  } catch(const std::runtime_error& e){
    processRuntimeError(deferred_example_method); // promise is rejected with e.what()
  }
  deferred_example_method = NULL;
}

napi_value example_method(napi_env env, const napi_callback_info info){
  static arg_example_method argArr;
  try {
    size_t argc = 2;
    napi_value args[2];
    napiErrChk(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    napiErrChk(napi_get_value_double(env, args[0], &(argArr.input1)));
    napiErrChk(napi_get_value_double(env, args[1], &(argArr.input2)));
    
    return create_promise(env, deferred_example_method, execute_example_method, complete_example_method, &argArr);
  } catch(const std::runtime_error&){
    return NULL;
  }
}

#define DECLARE_NAPI_METHOD(name) \
  {#name, 0, name, 0, 0, 0, napi_default, 0}
  
napi_value Init(napi_env env, napi_value exports){
  napi_status status;
  napi_property_descriptor descriptors[] = {
    DECLARE_NAPI_METHOD(example_method)
  };
  status = napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors);
  return exports;
}

NAPI_MODULE(addon, Init)