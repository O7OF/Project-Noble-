
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::set_map */

void __rustcall
host_probe::set_map
          (longlong param_1,ulonglong param_2,undefined8 param_3,undefined8 param_4)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  code *pcVar3;
  undefined8 *puVar4;
  int iVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  void *pvVar8;
  longlong lVar9;
  undefined4 local_f8;
  undefined4 uStack_f4;
  undefined4 uStack_f0;
  undefined4 uStack_ec;
  undefined4 *local_e8;
  void *local_e0;
  undefined8 local_d8;
  undefined8 uStack_d0;
  void **local_c8;
  code *local_c0;
  undefined4 *local_b8;
  code *local_b0;
  undefined4 local_a4;
  undefined8 local_a0;
  undefined8 local_98;
  undefined8 local_90;
  longlong local_88;
  ulonglong local_80;
  longlong local_78;
  void *local_70;
  undefined4 *local_68;
  undefined8 local_60;
  undefined8 local_58;
  void *local_50;
  undefined4 *local_48;
  
  local_60 = param_3;
  local_58 = param_4;
  ::<str>::to_lowercase(&local_d8,param_3,param_4);
  puVar4 = uStack_d0;
  ::<str>::replace::<char>(&local_78,uStack_d0,local_c8);
  local_88 = param_1;
  if (local_d8 != (undefined8 *)0x0) {
    __rustc::__rust_dealloc(puVar4,local_d8,1);
    return;
  }
  local_e0 = local_70;
  lVar9 = 0;
  local_80 = param_2;
  do {
    puVar2 = *(undefined4 **)((longlong)&DAT_1406d5648 + lVar9);
    if (puVar2 == local_68) {
      pvVar8 = *(void **)((longlong)&PTR_s_50_panopticon_1406d5640 + lVar9);
      iVar5 = memcmp(pvVar8,local_e0,(size_t)local_68);
      if (iVar5 != 0) goto LAB_1401d1ebb;
      uVar6 = *(undefined8 *)((longlong)&PTR_s_Boardwalk_1406d5658 + lVar9);
      uVar7 = *(undefined8 *)((longlong)&DAT_1406d5660 + lVar9);
LAB_1401d1f9e:
      local_a4 = *(undefined4 *)((longlong)&DAT_1406d5650 + lVar9);
      uVar1 = *(undefined4 *)((longlong)&DAT_1406d5654 + lVar9);
      local_a0 = uVar6;
      local_98 = uVar7;
      local_50 = pvVar8;
      local_48 = local_68;
      if (local_80 < 0x14) {
        core::slice::index::slice_index_fail
                  (0x10,0x14,local_80,&PTR_s_src_main_rs_1406d7258);
      }
      else {
        *(undefined4 *)(local_88 + 0x10) = local_a4;
        if (local_80 < 0x24) {
          uVar7 = 0x24;
          uVar6 = 0x14;
        }
        else {
          *(undefined4 *)(local_88 + 0x14) = local_a4;
          *(undefined4 *)(local_88 + 0x18) = 0x88880000;
          *(undefined8 *)(local_88 + 0x1c) = 0;
          if (0x1d5cf < local_80) {
            *(undefined4 *)(local_88 + 0x1d5c0) = local_a4;
            *(undefined4 *)(local_88 + 0x1d5c4) = 0x88880000;
            *(undefined8 *)(local_88 + 0x1d5c8) = 0;
            *(undefined4 *)(local_88 + 0x1d334) = uVar1;
            *(undefined4 *)(local_88 + 0x1d5bc) = uVar1;
            local_d8 = &local_a0;
            uStack_d0 = &<&str_as_core::fmt::Display>::fmt;
            local_c8 = &local_50;
            local_c0 = (code *)&<&str_as_core::fmt::Display>::fmt;
            local_b8 = &local_a4;
            local_b0 = <u32_as_core::fmt::Display>::fmt;
            alloc::fmt::format::format_inner(&local_f8,&DAT_1406d7270,&local_d8);
            host::log(CONCAT44(uStack_ec,uStack_f0),local_e8);
            if (CONCAT44(uStack_f4,local_f8) != 0) {
              __rustc::__rust_dealloc
                        (CONCAT44(uStack_ec,uStack_f0),CONCAT44(uStack_f4,local_f8),1);
              return;
            }
            if (local_78 != 0) {
              __rustc::__rust_dealloc(local_e0,local_78,1);
              return;
            }
            return;
          }
          uVar7 = 0x1d5d0;
          uVar6 = 0x1d5c0;
        }
        core::slice::index::slice_index_fail
                  (uVar6,uVar7,local_80,&PTR_s_src_main_rs_1406d7298);
      }
      goto LAB_1401d21d7;
    }
LAB_1401d1ebb:
    uVar6 = *(undefined8 *)((longlong)&PTR_s_Boardwalk_1406d5658 + lVar9);
    uVar7 = *(undefined8 *)((longlong)&DAT_1406d5660 + lVar9);
    ::<str>::to_lowercase(&local_d8,uVar6,uVar7);
    puVar4 = uStack_d0;
    ::<str>::replace::<char>(&local_f8,uStack_d0,local_c8);
    if (local_e8 == local_68) {
      pvVar8 = (void *)CONCAT44(uStack_ec,uStack_f0);
      iVar5 = memcmp(pvVar8,local_e0,(size_t)local_68);
      if (CONCAT44(uStack_f4,local_f8) != 0) {
        __rustc::__rust_dealloc(pvVar8,CONCAT44(uStack_f4,local_f8),1);
        return;
      }
      if (local_d8 != (undefined8 *)0x0) {
        __rustc::__rust_dealloc(puVar4,local_d8,1);
        return;
      }
      if (iVar5 == 0) {
        pvVar8 = *(void **)((longlong)&PTR_s_50_panopticon_1406d5640 + lVar9);
        local_68 = puVar2;
        goto LAB_1401d1f9e;
      }
    }
    else {
      if (CONCAT44(uStack_f4,local_f8) != 0) {
        __rustc::__rust_dealloc
                  (CONCAT44(uStack_ec,uStack_f0),CONCAT44(uStack_f4,local_f8),1);
        return;
      }
      if (local_d8 != (undefined8 *)0x0) {
        __rustc::__rust_dealloc(puVar4,local_d8,1);
        return;
      }
    }
    lVar9 = lVar9 + 0x28;
  } while (lVar9 != 800);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<(u32,&str,u32,&str)>,host_probe::set_map::{closure#1}>>>
  ::from_iter(&local_a0,&PTR_s_50_panopticon_1406d5640,&DAT_1406d5960);
  alloc::str::join_generic_copy::<str,u8,alloc::string::String>
            (&local_d8,local_98,local_90,&DAT_1406d5e5a,2);
  local_e8 = (undefined4 *)local_c8;
  local_f8 = (undefined4)local_d8;
  uStack_f4 = local_d8._4_4_;
  uStack_f0 = (undefined4)uStack_d0;
  uStack_ec = uStack_d0._4_4_;
  local_d8 = &local_60;
  uStack_d0 = &<&str_as_core::fmt::Display>::fmt;
  local_c8 = (void **)&local_f8;
  local_c0 = <alloc::string::String_as_core::fmt::Display>::fmt;
  core::panicking::panic_fmt(&DAT_1406d72b0,&local_d8,&PTR_s_src_main_rs_1406d72d0)
  ;
LAB_1401d21d7:
                    /* WARNING: Does not return */
  pcVar3 = (code *)invalidInstructionException();
  (*pcVar3)();
}


