
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::set_map_variant */

void __rustcall
host_probe::set_map_variant
          (longlong param_1,ulonglong param_2,longlong param_3,ulonglong param_4,undefined8 param_5)

{
  code *pcVar1;
  void *_Src;
  undefined *puVar2;
  undefined **ppuVar3;
  char local_ca;
  char local_c9;
  ulonglong local_c8;
  int local_bc;
  ulonglong *local_b8;
  code *local_b0;
  ulonglong *local_a8;
  code *local_a0;
  undefined **local_98;
  undefined8 *local_90;
  int *local_88;
  code *local_80;
  ulonglong local_78;
  undefined8 local_70;
  undefined *local_68;
  undefined8 local_60;
  longlong local_58;
  undefined8 local_50;
  undefined8 local_48;
  
  local_70 = param_5;
  local_78 = param_4;
  std::fs::read::inner(&local_b8,param_4);
  pcVar1 = local_b0;
  if (local_b8 == (ulonglong *)0xffffffffffffffff) {
    set_map_variant::_closure_0_(&local_78,local_b0);
                    /* WARNING: Does not return */
    pcVar1 = (code *)invalidInstructionException();
    (*pcVar1)();
  }
  if (local_a8 < (char *)0x7329) {
    core::panicking::panic_fmt
              ("map variant file is too small",0x3b,&PTR_s_src_main_rs_1406d5398);
  }
  else {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    _Src = (void *)__rustc::__rust_alloc_zeroed(0x20000,1);
    if (_Src == (void *)0x0) {
      alloc::raw_vec::handle_error(1,0x20000);
    }
    else {
      local_ca = '\0';
      (*(code *)(param_3 + 0x6bff8))(_Src);
      local_c9 = (*(code *)(param_3 + 0x17af00))(pcVar1,0x7329,_Src,&local_ca);
      if ((local_c9 == '\0') || (local_ca == '\0')) {
        local_b8 = (ulonglong *)&local_c9;
        local_b0 = <u8_as_core::fmt::Display>::fmt;
        local_a0 = <u8_as_core::fmt::Display>::fmt;
        ppuVar3 = &PTR_s_src_main_rs_1406d53f8;
        puVar2 = &DAT_1406d53b0;
        local_a8 = (ulonglong *)&local_ca;
LAB_1401c2c21:
        core::panicking::panic_fmt(puVar2,&local_b8,ppuVar3);
      }
      else {
        local_c8 = 0x20000;
        do {
          if (*(char *)((longlong)_Src + (local_c8 - 1)) != '\0') {
LAB_1401c2a5c:
            if (local_c8 < 0xe801) goto LAB_1401c2a6d;
            local_b8 = &local_c8;
            local_b0 = <usize_as_core::fmt::LowerHex>::fmt;
            ppuVar3 = &PTR_s_src_main_rs_1406d5460;
            puVar2 = &DAT_1406d5410;
            goto LAB_1401c2c21;
          }
          if (*(char *)((longlong)_Src + (local_c8 - 2)) != '\0') {
            local_c8 = local_c8 - 1;
            goto LAB_1401c2a5c;
          }
          if (*(char *)((longlong)_Src + (local_c8 - 3)) != '\0') {
            local_c8 = local_c8 - 2;
            goto LAB_1401c2a5c;
          }
          if (*(char *)((longlong)_Src + (local_c8 - 4)) != '\0') {
            local_c8 = local_c8 - 3;
            goto LAB_1401c2a5c;
          }
          local_c8 = local_c8 - 4;
        } while (local_c8 != 0);
        local_c8 = 0;
LAB_1401c2a6d:
        if (0x2bb07 < param_2) {
          memcpy((void *)(param_1 + 0x1d308),_Src,0xe800);
          local_bc = *(int *)(param_1 + 0x1d5c0);
          *(int *)(param_1 + 0x10) = local_bc;
          *(int *)(param_1 + 0x14) = local_bc;
          if (local_bc - 0xbeU < 0x14) {
            local_68 = *(undefined **)((&PTR_PTR_1406da7e8)[local_bc - 0xbeU] + 0x18);
            local_60 = *(undefined8 *)((&PTR_PTR_1406da7e8)[local_bc - 0xbeU] + 0x20);
          }
          else {
            local_68 = &DAT_1406d5960;
            local_60 = 7;
          }
          local_b8 = &local_78;
          local_b0 = (code *)&<&str_as_core::fmt::Display>::fmt;
          local_a8 = &local_c8;
          local_a0 = <usize_as_core::fmt::LowerHex>::fmt;
          local_98 = &local_68;
          local_90 = &<&str_as_core::fmt::Display>::fmt;
          local_88 = &local_bc;
          local_80 = <u32_as_core::fmt::Display>::fmt;
          alloc::fmt::format::format_inner(&local_58,&DAT_1406d5967,&local_b8);
          host::log(local_50,local_48);
          if (local_58 != 0) {
            __rustc::__rust_dealloc(local_50,local_58,1);
            return;
          }
          __rustc::__rust_dealloc(_Src,0x20000,1);
          return;
        }
        core::slice::index::slice_index_fail
                  (0x1d308,0x2bb08,param_2,&PTR_s_src_main_rs_1406d5478);
      }
    }
  }
                    /* WARNING: Does not return */
  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}


