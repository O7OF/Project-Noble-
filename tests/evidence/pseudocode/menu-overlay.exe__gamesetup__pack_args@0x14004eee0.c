
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::gamesetup::pack_args */

longlong * __rustcall
menu_overlay::gamesetup::pack_args
          (longlong *param_1,longlong param_2,ulonglong param_3)

{
  longlong *plVar1;
  char *pcVar2;
  code *pcVar3;
  longlong lVar4;
  longlong lVar5;
  undefined8 *puVar6;
  longlong lVar7;
  undefined8 *puVar8;
  ulonglong uVar9;
  longlong lVar10;
  undefined1 auVar11 [16];
  undefined1 auVar12 [16];
  longlong local_f0;
  longlong lStack_e8;
  longlong local_e0;
  longlong local_d8;
  longlong local_d0;
  longlong local_c8;
  longlong local_c0;
  undefined8 *local_b8 [3];
  longlong local_a0;
  longlong lStack_98;
  undefined8 *local_90;
  longlong local_80;
  longlong lStack_78;
  undefined8 *local_70;
  undefined8 *local_68;
  undefined8 local_60;
  longlong local_58;
  longlong local_50;
  undefined8 *puStack_48;
  
  if (param_3 != 0) {
    lVar4 = param_3 * 0x18;
    lVar5 = 0;
    lVar7 = lVar4;
    lVar10 = param_2;
    do {
      if ((*(longlong *)(lVar10 + 0x10) == 0xc) &&
         ((int)(*(longlong **)(lVar10 + 8))[1] == 0x6b636170 &&
          **(longlong **)(lVar10 + 8) == 0x2d726f6d72612d2d)) {
        if (lVar5 + 1U < param_3) {
          <alloc::string::String_as_core::clone::Clone>::clone
                    (&local_c8);
          puVar8 = local_b8[0];
          local_d8 = local_c8;
          if (local_c8 != -1) {
            local_d0 = local_c0;
            local_f0 = 0;
            lStack_e8 = 8;
            local_e0 = 0;
            uVar9 = param_3;
            lVar7 = lVar4;
            goto LAB_14004efdd;
          }
        }
        break;
      }
      lVar5 = lVar5 + 1;
      lVar7 = lVar7 + -0x18;
      lVar10 = lVar10 + 0x18;
    } while (lVar7 != 0);
  }
  *param_1 = 0;
  param_1[1] = 8;
  param_1[2] = 0;
  return param_1;
  while( true ) {
    uVar9 = uVar9 - 1;
    lVar7 = lVar7 + -0x18;
    if (lVar7 == 0) break;
LAB_14004efdd:
    if ((*(longlong *)(param_2 + -8 + lVar7) == 0xc) &&
       (plVar1 = *(longlong **)(param_2 + -0x10 + lVar7),
       (int)plVar1[1] == 0x7370616d && *plVar1 == 0x2d61727478652d2d)) {
      if (uVar9 < param_3) {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        puVar6 = (undefined8 *)__rustc::__rust_alloc(0xc,1);
        if (puVar6 == (undefined8 *)0x0) goto LAB_14004f310;
        *puVar6 = 0x2d61727478652d2d;
        *(undefined4 *)(puVar6 + 1) = 0x7370616d;
        <alloc::string::String_as_core::clone::Clone>::clone
                  (&local_80,lVar7 + param_2);
        local_90 = local_70;
        local_a0 = local_80;
        lStack_98 = lStack_78;
        local_c8 = 0;
        local_c0 = 2;
        local_b8[0] = (undefined8 *)0xc;
        local_b8[2] = (undefined8 *)0xc;
        local_b8[1] = puVar6;
        <alloc::vec::Vec<alloc::string::String>>::extend_trusted
        ::
        <core::array::iter::IntoIter<alloc::string::String,2usize>>
                  (&local_f0,&local_c8);
      }
      break;
    }
  }
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar6 = (undefined8 *)__rustc::__rust_alloc(0xc,1);
  if (puVar6 == (undefined8 *)0x0) {
LAB_14004f310:
    alloc::raw_vec::handle_error(1,0xc);
LAB_14004f336:
                    /* WARNING: Does not return */
    pcVar3 = (code *)invalidInstructionException();
    (*pcVar3)();
  }
  *puVar6 = 0x2d726f6d72612d2d;
  *(undefined4 *)(puVar6 + 1) = 0x6b636170;
  local_58 = local_d8;
  local_50 = local_d0;
  puStack_48 = puVar8;
  local_80 = 0;
  lStack_78 = 2;
  local_70 = (undefined8 *)0xc;
  local_60 = 0xc;
  local_68 = puVar6;
  if ((ulonglong)(local_f0 - local_e0) < 2) {
    <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
    <alloc::alloc::Global>(&local_f0,local_e0,2,8,0x18);
  }
  lVar5 = lStack_78;
  lVar10 = local_80;
  lVar7 = local_e0;
  local_c8 = local_80;
  local_c0 = lStack_78;
  lStack_98 = local_50;
  local_90 = puStack_48;
  local_b8[2] = (undefined8 *)local_60;
  local_a0 = local_58;
  local_b8[0] = local_70;
  local_b8[1] = local_68;
  if (lStack_78 - local_80 != 0) {
    memcpy((void *)(lStack_e8 + local_e0 * 0x18),local_b8 + local_80 * 3,
           (lStack_78 - local_80) * 0x18);
    local_e0 = (lVar5 + lVar7) - lVar10;
  }
  lVar7 = 0;
  do {
    if (*(longlong *)(param_2 + 0x10 + lVar7) == 0x13) {
      pcVar2 = *(char **)(param_2 + 8 + lVar7);
      auVar11[0] = -(*pcVar2 == DAT_1401bec70);
      auVar11[1] = -(pcVar2[1] == DAT_1401bec70._1_1_);
      auVar11[2] = -(pcVar2[2] == DAT_1401bec70._2_1_);
      auVar11[3] = -(pcVar2[3] == DAT_1401bec70._3_1_);
      auVar11[4] = -(pcVar2[4] == DAT_1401bec70._4_1_);
      auVar11[5] = -(pcVar2[5] == DAT_1401bec70._5_1_);
      auVar11[6] = -(pcVar2[6] == DAT_1401bec70._6_1_);
      auVar11[7] = -(pcVar2[7] == DAT_1401bec70._7_1_);
      auVar11[8] = -(pcVar2[8] == DAT_1401bec70._8_1_);
      auVar11[9] = -(pcVar2[9] == DAT_1401bec70._9_1_);
      auVar11[10] = -(pcVar2[10] == DAT_1401bec70._10_1_);
      auVar11[0xb] = -(pcVar2[0xb] == DAT_1401bec70._11_1_);
      auVar11[0xc] = -(pcVar2[0xc] == DAT_1401bec70._12_1_);
      auVar11[0xd] = -(pcVar2[0xd] == DAT_1401bec70._13_1_);
      auVar11[0xe] = -(pcVar2[0xe] == DAT_1401bec70._14_1_);
      auVar11[0xf] = -(pcVar2[0xf] == DAT_1401bec70._15_1_);
      auVar12[0] = -(pcVar2[3] == DAT_1401bec60);
      auVar12[1] = -(pcVar2[4] == DAT_1401bec60._1_1_);
      auVar12[2] = -(pcVar2[5] == DAT_1401bec60._2_1_);
      auVar12[3] = -(pcVar2[6] == DAT_1401bec60._3_1_);
      auVar12[4] = -(pcVar2[7] == DAT_1401bec60._4_1_);
      auVar12[5] = -(pcVar2[8] == DAT_1401bec60._5_1_);
      auVar12[6] = -(pcVar2[9] == DAT_1401bec60._6_1_);
      auVar12[7] = -(pcVar2[10] == DAT_1401bec60._7_1_);
      auVar12[8] = -(pcVar2[0xb] == DAT_1401bec60._8_1_);
      auVar12[9] = -(pcVar2[0xc] == DAT_1401bec60._9_1_);
      auVar12[10] = -(pcVar2[0xd] == DAT_1401bec60._10_1_);
      auVar12[0xb] = -(pcVar2[0xe] == DAT_1401bec60._11_1_);
      auVar12[0xc] = -(pcVar2[0xf] == DAT_1401bec60._12_1_);
      auVar12[0xd] = -(pcVar2[0x10] == DAT_1401bec60._13_1_);
      auVar12[0xe] = -(pcVar2[0x11] == DAT_1401bec60._14_1_);
      auVar12[0xf] = -(pcVar2[0x12] == DAT_1401bec60._15_1_);
      auVar12 = auVar12 & auVar11;
      if ((ushort)((ushort)(SUB161(auVar12 >> 7,0) & 1) |
                   (ushort)(SUB161(auVar12 >> 0xf,0) & 1) << 1 |
                   (ushort)(SUB161(auVar12 >> 0x17,0) & 1) << 2 |
                   (ushort)(SUB161(auVar12 >> 0x1f,0) & 1) << 3 |
                   (ushort)(SUB161(auVar12 >> 0x27,0) & 1) << 4 |
                   (ushort)(SUB161(auVar12 >> 0x2f,0) & 1) << 5 |
                   (ushort)(SUB161(auVar12 >> 0x37,0) & 1) << 6 |
                   (ushort)(SUB161(auVar12 >> 0x3f,0) & 1) << 7 |
                   (ushort)(SUB161(auVar12 >> 0x47,0) & 1) << 8 |
                   (ushort)(SUB161(auVar12 >> 0x4f,0) & 1) << 9 |
                   (ushort)(SUB161(auVar12 >> 0x57,0) & 1) << 10 |
                   (ushort)(SUB161(auVar12 >> 0x5f,0) & 1) << 0xb |
                   (ushort)(SUB161(auVar12 >> 0x67,0) & 1) << 0xc |
                   (ushort)(SUB161(auVar12 >> 0x6f,0) & 1) << 0xd |
                   (ushort)(SUB161(auVar12 >> 0x77,0) & 1) << 0xe |
                  (ushort)(byte)(auVar12[0xf] >> 7) << 0xf) == 0xffff) {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        puVar8 = (undefined8 *)__rustc::__rust_alloc(0x13,1);
        lVar4 = local_e0;
        if (puVar8 == (undefined8 *)0x0) {
          alloc::raw_vec::handle_error(1,0x13);
          goto LAB_14004f336;
        }
        *puVar8 = 0x2d6574696c652d2d;
        puVar8[1] = 0x6c732d726f6d7261;
        *(undefined4 *)((longlong)puVar8 + 0xf) = 0x73746f6c;
        if (local_e0 == local_f0) {
          <alloc::raw_vec::RawVec<alloc::string::String>>::
          grow_one(&local_f0);
        }
        *(undefined8 *)(lStack_e8 + lVar4 * 0x18) = 0x13;
        *(undefined8 **)(lStack_e8 + 8 + lVar4 * 0x18) = puVar8;
        *(undefined8 *)(lStack_e8 + 0x10 + lVar4 * 0x18) = 0x13;
        local_e0 = lVar4 + 1;
        break;
      }
    }
    lVar7 = lVar7 + 0x18;
  } while (lVar4 != lVar7);
  param_1[2] = local_e0;
  *param_1 = local_f0;
  param_1[1] = lStack_e8;
                    /* WARNING: Read-only address (ram,0x0001401bec60) is written */
                    /* WARNING: Read-only address (ram,0x0001401bec70) is written */
  return param_1;
}


