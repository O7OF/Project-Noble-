
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::own_variant_path */

void __rustcall
host_probe::own_variant_path
          (size_t *param_1,longlong param_2,longlong param_3,undefined8 param_4)

{
  void *_Src;
  size_t _Size;
  code *pcVar1;
  char cVar2;
  void *_Dst;
  undefined8 uVar3;
  size_t local_68;
  void *local_60;
  size_t local_58;
  longlong local_50;
  undefined8 local_48;
  undefined8 local_40;
  
  if (*(longlong *)(param_2 + 0x30) == -1) {
    *param_1 = 0xffffffffffffffff;
    return;
  }
  _Src = *(void **)(param_2 + 0x38);
  _Size = *(size_t *)(param_2 + 0x40);
  if ((param_3 == 0) ||
     (cVar2 = <std::path::Path>::is_absolute(_Src,_Size), cVar2 != '\0')) {
    if ((longlong)_Size < 0) {
      uVar3 = 0;
LAB_1401c32f1:
      alloc::raw_vec::handle_error(uVar3,_Size);
                    /* WARNING: Does not return */
      pcVar1 = (code *)invalidInstructionException();
      (*pcVar1)();
    }
    if (_Size == 0) {
      _Dst = (void *)0x1;
      local_68 = 0;
      local_58 = 0;
    }
    else {
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      uVar3 = 1;
      _Dst = (void *)__rustc::__rust_alloc(_Size,1);
      if (_Dst == (void *)0x0) goto LAB_1401c32f1;
      memcpy(_Dst,_Src,_Size);
      local_68 = _Size;
      local_58 = _Size;
    }
  }
  else {
    <std::path::Path>::_join(&local_50,param_3,param_4,_Src,_Size);
    <core::wtf8::Wtf8>::to_string_lossy(&local_68,local_48,local_40);
    _Dst = local_60;
    if (local_68 == 0xffffffffffffffff) {
      if ((longlong)local_58 < 0) {
        uVar3 = 0;
LAB_1401c33cc:
        alloc::raw_vec::handle_error(uVar3,local_58);
                    /* WARNING: Does not return */
        pcVar1 = (code *)invalidInstructionException();
        (*pcVar1)();
      }
      if (local_58 == 0) {
        _Dst = (void *)0x1;
        local_68 = 0;
        local_58 = 0;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar3 = 1;
        _Dst = (void *)__rustc::__rust_alloc(local_58,1);
        if (_Dst == (void *)0x0) goto LAB_1401c33cc;
        memcpy(_Dst,local_60,local_58);
        local_68 = local_58;
      }
    }
    if (local_50 != 0) {
      __rustc::__rust_dealloc(local_48,local_50,1);
      return;
    }
  }
  *param_1 = local_68;
  param_1[1] = (size_t)_Dst;
  param_1[2] = local_58;
  return;
}


