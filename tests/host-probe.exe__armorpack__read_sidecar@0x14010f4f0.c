
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::armorpack::read_sidecar */

void __rustcall
host_probe::armorpack::read_sidecar
          (undefined8 *param_1,undefined4 **param_2,longlong param_3,undefined4 *param_4,
          ulonglong param_5)

{
  undefined8 uVar1;
  code *pcVar2;
  int *piVar3;
  ulonglong uVar4;
  longlong lVar5;
  undefined4 *puVar6;
  longlong lVar7;
  undefined4 *local_178;
  undefined8 local_170;
  undefined8 uStack_168;
  longlong local_160;
  undefined4 local_158;
  undefined4 uStack_154;
  undefined4 uStack_150;
  undefined4 uStack_14c;
  ulonglong local_148;
  undefined4 *local_138;
  undefined8 local_130;
  undefined4 **local_128;
  code *local_120;
  undefined4 **local_118;
  code *local_110;
  undefined4 **local_108;
  code *local_100;
  undefined8 *local_f8;
  code *local_f0;
  undefined8 *local_e8;
  code *local_e0;
  undefined4 ***local_d8;
  code *local_d0;
  undefined4 *local_c8;
  undefined8 local_c0;
  undefined4 uStack_b8;
  undefined4 uStack_b4;
  ulonglong local_b0;
  longlong local_a8;
  undefined4 *local_a0;
  undefined8 local_98;
  undefined1 local_90;
  undefined4 **local_88;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  undefined4 local_70;
  undefined4 uStack_6c;
  undefined4 uStack_68;
  undefined4 uStack_64;
  undefined8 local_60;
  undefined4 *local_58 [4];
  
  uVar4 = param_5;
  local_128 = (undefined4 **)&local_170;
  local_120 = <std::path::Display_as_core::fmt::Display>::fmt;
  local_170 = param_2;
  uStack_168 = param_3;
  local_c8 = param_4;
  alloc::fmt::format::format_inner(&local_a8,&DAT_1406b1c09,&local_128);
  local_90 = 1;
  std::fs::read::inner(&local_c0,local_a0,local_98);
  if (local_c0 == 0xffffffffffffffff) {
    local_178 = (undefined4 *)CONCAT44(uStack_b4,uStack_b8);
    local_138 = local_a0;
    local_130 = local_98;
    local_128 = &local_138;
    local_120 = <std::path::Display_as_core::fmt::Display>::fmt;
    local_118 = &local_178;
    local_110 = <core::io::error::Error_as_core::fmt::Display>::
                fmt;
    alloc::fmt::format::format_inner(&local_170,&DAT_1406b0d4b,&local_128);
    if ((1 < ((uint)local_178 & 3) - 2) && (((ulonglong)local_178 & 3) != 0)) {
      local_120 = (code *)((longlong)local_178 - 1);
      local_128 = (undefined4 **)CONCAT71(local_128._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&local_120);
    }
    param_1[3] = local_160;
    *(undefined4 *)(param_1 + 1) = (undefined4)local_170;
    *(undefined4 *)((longlong)param_1 + 0xc) = local_170._4_4_;
    *(undefined4 *)(param_1 + 2) = (undefined4)uStack_168;
    *(undefined4 *)((longlong)param_1 + 0x14) = uStack_168._4_4_;
    *param_1 = 0xffffffffffffffff;
    uStack_80 = (undefined4)uStack_168;
    uStack_7c = uStack_168._4_4_;
    local_88 = local_170;
    goto joined_r0x00014010f807;
  }
  local_158 = (undefined4)local_c0;
  uStack_154 = local_c0._4_4_;
  uStack_150 = uStack_b8;
  uStack_14c = uStack_b4;
  local_148 = local_b0;
  if ((local_b0 < 0x20) || (piVar3 = (int *)CONCAT44(uStack_b4,uStack_b8), *piVar3 != 0x4c525252)) {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar6 = (undefined4 *)__rustc::__rust_alloc(0x25,1);
    if (puVar6 == (undefined4 *)0x0) {
      alloc::raw_vec::handle_error(1,0x25);
LAB_14010f979:
                    /* WARNING: Does not return */
      pcVar2 = (code *)invalidInstructionException();
      (*pcVar2)();
    }
    *(undefined8 *)(puVar6 + 4) = 0x74276e7369207261;
    *(undefined8 *)(puVar6 + 6) = 0x4c525252206e6120;
    *puVar6 = 0x6f6c6572;
    puVar6[1] = 0x69746163;
    puVar6[2] = 0x73206e6f;
    puVar6[3] = 0x63656469;
    *(undefined8 *)((longlong)puVar6 + 0x1d) = 0x656c6966204c5252;
    param_1[1] = 0x25;
    param_1[2] = puVar6;
    param_1[3] = 0x25;
LAB_14010f6fd:
    *param_1 = 0xffffffffffffffff;
  }
  else {
    local_138 = &local_158;
    local_88 = (undefined4 **)(ulonglong)(uint)piVar3[2];
    local_58[0] = *(undefined4 **)(piVar3 + 3);
    uVar1 = *(undefined8 *)(piVar3 + 5);
    local_c0 = (ulonglong)(uint)piVar3[7];
    local_178 = (undefined4 *)CONCAT44(local_178._4_4_,piVar3[1]);
    if ((uVar4 != local_c0 || (local_58[0] != param_4 || piVar3[1] != 1)) ||
       (local_b0 != (longlong)local_88 * 4 + 0x20U)) {
      local_128 = &local_178;
      local_120 = <u32_as_core::fmt::Display>::fmt;
      local_118 = local_58;
      local_110 = <usize_as_core::fmt::LowerHex>::fmt;
      local_108 = &local_c8;
      local_100 = <usize_as_core::fmt::LowerHex>::fmt;
      local_f0 = <usize_as_core::fmt::LowerHex>::fmt;
      local_e8 = &param_5;
      local_e0 = <usize_as_core::fmt::LowerHex>::fmt;
      local_d8 = &local_88;
      local_d0 = <u64_as_core::fmt::Display>::fmt;
      local_f8 = &local_c0;
      alloc::fmt::format::format_inner(&local_70,&DAT_1406b1c45,&local_128);
      param_1[3] = local_60;
      *(undefined4 *)(param_1 + 1) = local_70;
      *(undefined4 *)((longlong)param_1 + 0xc) = uStack_6c;
      *(undefined4 *)(param_1 + 2) = uStack_68;
      *(undefined4 *)((longlong)param_1 + 0x14) = uStack_64;
      goto LAB_14010f6fd;
    }
    local_120 = (code *)0x0;
    local_128 = &local_138;
    local_118 = local_88;
    <alloc::vec::Vec<u32>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<u32,core::iter::adapters::map::Map<core::ops::range::Range<usize>,host_probe::armorpack::read_sidecar::{closure#3}>>>
    ::from_iter(&local_170,&local_128);
    lVar5 = uStack_168;
    if (local_160 != 0) {
      lVar7 = 0;
      do {
        if (uVar4 < (ulonglong)*(uint *)(uStack_168 + lVar7) + 4) {
          __rustc::__rust_no_alloc_shim_is_unstable_v2();
          puVar6 = (undefined4 *)__rustc::__rust_alloc(0x31,1);
          if (puVar6 == (undefined4 *)0x0) {
            alloc::raw_vec::handle_error(1,0x31);
            goto LAB_14010f979;
          }
          *(undefined8 *)(puVar6 + 8) = 0x6174206568742065;
          *(undefined8 *)(puVar6 + 10) = 0x6f69746365732067;
          *(undefined8 *)(puVar6 + 4) = 0x746e696f70207261;
          *(undefined8 *)(puVar6 + 6) = 0x64697374756f2073;
          *puVar6 = 0x6f6c6572;
          puVar6[1] = 0x69746163;
          puVar6[2] = 0x73206e6f;
          puVar6[3] = 0x63656469;
          *(undefined1 *)(puVar6 + 0xc) = 0x6e;
          param_1[1] = 0x31;
          param_1[2] = puVar6;
          param_1[3] = 0x31;
          *param_1 = 0xffffffffffffffff;
          if (local_170 != (undefined4 **)0x0) {
            __rustc::__rust_dealloc(lVar5,(longlong)local_170 << 2,4);
            return;
          }
          goto LAB_14010f704;
        }
        lVar7 = lVar7 + 4;
      } while (local_160 << 2 != lVar7);
    }
    param_1[2] = local_160;
    *(undefined4 *)param_1 = (undefined4)local_170;
    *(undefined4 *)((longlong)param_1 + 4) = local_170._4_4_;
    *(undefined4 *)(param_1 + 1) = (undefined4)uStack_168;
    *(undefined4 *)((longlong)param_1 + 0xc) = uStack_168._4_4_;
    param_1[3] = param_4;
    param_1[4] = uVar1;
    param_1[5] = uVar4;
  }
LAB_14010f704:
  if (CONCAT44(uStack_154,local_158) != 0) {
    __rustc::__rust_dealloc
              (CONCAT44(uStack_14c,uStack_150),CONCAT44(uStack_154,local_158),1);
    return;
  }
joined_r0x00014010f807:
  if (local_a8 == 0) {
    return;
  }
  __rustc::__rust_dealloc(local_a0,local_a8,1);
  return;
}


