#include <node_api.h>
#include <memory>
#include "addon.h"

napi_value create_promise(napi_env env, napi_deferred& deferred, napi_async_execute_callback execute, napi_async_complete_callback complete, void* pArgArr){
  napi_status status;
  
  napi_value promise;
  status = napi_create_promise(env, &deferred, &promise);
  
  napi_async_work asyncWorkResult;
  napi_value arg1, arg2;
  status = napi_create_object(env, &arg1);
  status = napi_create_object(env, &arg2);
  status = napi_create_async_work(env, arg1, arg2, execute, complete, pArgArr, &asyncWorkResult);
  status = napi_queue_async_work(env, asyncWorkResult);
  
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

void complete_example_method(napi_env env, napi_status nd_status, void* data){
  arg_example_method* _data = (arg_example_method*)data;
  
  napi_value error, error_code, error_msg;
  napi_value result;
  napi_status status;
  const napi_extended_error_info* err_info_ptr;
  
  if (nd_status != napi_ok){
    status = napi_get_last_error_info(env, &err_info_ptr);
    status = napi_create_string_utf8(env, "NAPI_ERROR", NAPI_AUTO_LENGTH, &error_code);
    status = napi_create_string_utf8(env, err_info_ptr->error_message, NAPI_AUTO_LENGTH, &error_msg);
    status = napi_create_error(env, error_code, error_msg, &error);
    status = napi_reject_deferred(env, deferred_example_method, error);
  } else {
    status = napi_create_double(env, _data->output, &result);
    status = napi_resolve_deferred(env, deferred_example_method, result);
  }
  
  deferred_example_method = NULL;
}

napi_value example_method(napi_env env, const napi_callback_info info){
  napi_status status;
  
  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  static arg_example_method argArr;
  status = napi_get_value_double(env, args[0], &(argArr.input1));
  status = napi_get_value_double(env, args[1], &(argArr.input2));
  
  return create_promise(env, deferred_example_method, execute_example_method, complete_example_method, &argArr);
}

#define DECLARE_NAPI_METHOD(name, func) \
  {name, 0, func, 0, 0, 0, napi_default, 0}
  
napi_value Init(napi_env env, napi_value exports){
  napi_status status;
  napi_property_descriptor descriptors[] = {
    DECLARE_NAPI_METHOD("example_method", example_method)
  };
  status = napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors);
  return exports;
}

NAPI_MODULE(addon, Init)