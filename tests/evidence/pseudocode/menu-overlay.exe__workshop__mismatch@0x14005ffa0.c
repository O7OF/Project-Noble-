
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::workshop::mismatch */

undefined8 * __rustcall
menu_overlay::workshop::mismatch
          (undefined8 *param_1,undefined8 param_2,undefined8 param_3,longlong param_4)

{
  size_t _Size;
  undefined4 *_Buf2;
  int iVar1;
  undefined8 *puVar2;
  undefined4 *puVar3;
  code *pcVar4;
  longlong *plVar5;
  undefined8 uVar6;
  size_t sVar7;
  longlong local_98;
  longlong *local_90;
  code *pcStack_88;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined4 local_70;
  undefined4 uStack_6c;
  undefined4 uStack_68;
  undefined4 uStack_64;
  longlong local_60;
  undefined4 local_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  undefined8 local_48;
  
  mods::read(&local_90);
  pcVar4 = pcStack_88;
  plVar5 = local_90;
  if (local_90 == (longlong *)0xffffffffffffffff) {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar2 = (undefined8 *)__rustc::__rust_alloc(0x17,1);
    if (puVar2 != (undefined8 *)0x0) {
      *puVar2 = 0x6261646165726e75;
      puVar2[1] = 0x6e49646f4d20656c;
      *(undefined8 *)((longlong)puVar2 + 0xf) = 0x6e6f736a2e6f666e;
      *param_1 = 0x17;
      param_1[1] = puVar2;
      param_1[2] = 0x17;
      return param_1;
    }
    alloc::raw_vec::handle_error(1,0x17);
                    /* WARNING: Does not return */
    pcVar4 = (code *)invalidInstructionException();
    (*pcVar4)();
  }
  local_98 = local_60;
  if (local_60 != *(longlong *)(param_4 + 0x30)) {
    local_90 = &local_98;
    pcStack_88 = <u64_as_core::fmt::Display>::fmt;
    alloc::fmt::format::format_inner(&local_58,&DAT_1401c0e50,&local_90);
    param_1[2] = local_48;
    *(undefined4 *)param_1 = local_58;
    *(undefined4 *)((longlong)param_1 + 4) = uStack_54;
    *(undefined4 *)(param_1 + 1) = uStack_50;
    *(undefined4 *)((longlong)param_1 + 0xc) = uStack_4c;
    if (plVar5 != (longlong *)0x0) {
      __rustc::__rust_dealloc(pcVar4,plVar5,1);
    }
    plVar5 = (longlong *)CONCAT44(uStack_74,uStack_78);
    if (plVar5 == (longlong *)0x0) {
      return param_1;
    }
    pcVar4 = (code *)CONCAT44(uStack_6c,local_70);
    goto LAB_14006019f;
  }
  _Size = *(size_t *)(param_4 + 0x10);
  _Buf2 = (undefined4 *)CONCAT44(uStack_6c,local_70);
  if (_Size == 0) {
LAB_14006010b:
    *param_1 = 0xffffffffffffffff;
    if (CONCAT44(uStack_74,uStack_78) != 0) {
      __rustc::__rust_dealloc(_Buf2,CONCAT44(uStack_74,uStack_78),1);
    }
  }
  else {
    sVar7 = CONCAT44(uStack_64,uStack_68);
    if (_Size == sVar7) {
      iVar1 = memcmp(*(void **)(param_4 + 8),_Buf2,_Size);
      if (iVar1 == 0) goto LAB_14006010b;
LAB_140060137:
      uVar6 = CONCAT44(uStack_74,uStack_78);
      puVar3 = _Buf2;
    }
    else {
      if (sVar7 != 0) goto LAB_140060137;
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      uVar6 = 7;
      puVar3 = (undefined4 *)__rustc::__rust_alloc(7,1);
      if (puVar3 == (undefined4 *)0x0) {
        alloc::raw_vec::handle_error(1,7);
                    /* WARNING: Does not return */
        pcVar4 = (code *)invalidInstructionException();
        (*pcVar4)();
      }
      *(undefined4 *)((longlong)puVar3 + 3) = 0x6e776f6e;
      *puVar3 = 0x6e6b6e75;
      if (CONCAT44(uStack_74,uStack_78) != 0) {
        __rustc::__rust_dealloc(_Buf2,CONCAT44(uStack_74,uStack_78),1);
      }
      sVar7 = 7;
    }
    *param_1 = uVar6;
    param_1[1] = puVar3;
    param_1[2] = sVar7;
  }
  if (plVar5 == (longlong *)0x0) {
    return param_1;
  }
LAB_14006019f:
  __rustc::__rust_dealloc(pcVar4,plVar5,1);
  return param_1;
}


