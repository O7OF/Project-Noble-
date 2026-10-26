
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::mods::folder_in_library */

undefined8 __rustcall
menu_overlay::mods::folder_in_library
          (undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  code *pcVar1;
  void *pvVar2;
  void *pvVar3;
  longlong *_Size;
  undefined1 *_Size_00;
  undefined8 uVar4;
  longlong local_d8;
  undefined8 local_d0;
  undefined8 local_c8;
  longlong local_b8;
  undefined8 local_b0;
  undefined8 local_a8;
  longlong local_98;
  undefined8 local_90;
  undefined8 local_88;
  longlong local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined1 local_54 [20];
  
  <std::path::Path>::_join(&local_78,param_2,param_3,"steamapps",9);
  <std::path::Path>::_join(&local_98,local_70,local_68,&DAT_1401c8620,8);
  <std::path::Path>::_join(&local_b8,local_90,local_88,&DAT_1401c8628,7);
  _Size = &local_d8;
  pvVar2 = (void *)<u32>::_fmt(0xee75a,_Size,10);
  if ((longlong)_Size < 0) {
    uVar4 = 0;
LAB_1400de8f0:
    alloc::raw_vec::handle_error(uVar4,_Size);
  }
  else {
    if (_Size == (longlong *)0x0) {
      pvVar3 = (void *)0x1;
    }
    else {
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      uVar4 = 1;
      pvVar3 = (void *)__rustc::__rust_alloc(_Size,1);
      if (pvVar3 == (void *)0x0) goto LAB_1400de8f0;
      memcpy(pvVar3,pvVar2,(size_t)_Size);
    }
    <std::path::Path>::_join(&local_d8,local_b0,local_a8,pvVar3,_Size);
    if (_Size != (longlong *)0x0) {
      __rustc::__rust_dealloc(pvVar3,_Size,1);
    }
    _Size_00 = local_54;
    pvVar2 = (void *)<u64>::_fmt(param_4,_Size_00,0x14);
    if ((longlong)_Size_00 < 0) {
      uVar4 = 0;
    }
    else {
      if (_Size_00 == (undefined1 *)0x0) {
        pvVar3 = (void *)0x1;
LAB_1400de9e9:
        <std::path::Path>::_join(param_1,local_d0,local_c8,pvVar3,_Size_00);
        if (_Size_00 != (undefined1 *)0x0) {
          __rustc::__rust_dealloc(pvVar3,_Size_00,1);
        }
        if (local_d8 != 0) {
          __rustc::__rust_dealloc(local_d0,local_d8,1);
        }
        if (local_b8 != 0) {
          __rustc::__rust_dealloc(local_b0,local_b8,1);
        }
        if (local_98 != 0) {
          __rustc::__rust_dealloc(local_90,local_98,1);
        }
        if (local_78 != 0) {
          __rustc::__rust_dealloc(local_70,local_78,1);
        }
        return param_1;
      }
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      uVar4 = 1;
      pvVar3 = (void *)__rustc::__rust_alloc(_Size_00,1);
      if (pvVar3 != (void *)0x0) {
        memcpy(pvVar3,pvVar2,(size_t)_Size_00);
        goto LAB_1400de9e9;
      }
    }
    alloc::raw_vec::handle_error(uVar4,_Size_00);
  }
                    /* WARNING: Does not return */
  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}


