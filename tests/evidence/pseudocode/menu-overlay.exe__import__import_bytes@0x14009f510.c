
/* WARNING: Function: ___chkstk_ms replaced with injection: alloca_probe */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::import::import_bytes */

void __rustcall
menu_overlay::import::import_bytes
          (undefined8 *param_1,undefined8 param_2,undefined8 param_3,byte param_4,int *param_5,
          uint *******param_6,undefined8 param_7,undefined8 param_8,undefined4 param_9,
          undefined4 param_10)

{
  code *pcVar1;
  uint uVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  bool bVar7;
  uint *******pppppppuVar8;
  uint *******pppppppuVar9;
  uint uVar10;
  uint uVar11;
  char cVar12;
  uint uVar13;
  uint uVar14;
  undefined8 uVar15;
  undefined8 *puVar16;
  uint *******pppppppuVar17;
  longlong lVar18;
  uint *******pppppppuVar19;
  void *pvVar20;
  ulonglong uVar21;
  uint *******pppppppuVar22;
  uint uVar23;
  uint uVar24;
  uint ******ppppppuVar25;
  uint *******pppppppuVar26;
  uint uVar27;
  uint uVar28;
  uint uVar29;
  undefined *puVar30;
  uint uVar31;
  int iVar32;
  uint uVar33;
  uint uVar34;
  uint uVar35;
  uint uVar36;
  uint uVar37;
  uint *******pppppppuVar38;
  uint uVar39;
  uint uVar40;
  uint *******pppppppuVar41;
  uint uVar42;
  undefined8 local_1980;
  uint *******local_1968;
  uint *******pppppppuStack_1960;
  uint *******local_1958;
  uint *******local_1950;
  uint *******local_1948;
  uint *******local_1940;
  uint *******local_1938;
  longlong local_1930;
  uint *******local_1928;
  uint local_1920;
  uint local_191c;
  uint local_1918;
  uint local_1914;
  uint local_1910;
  uint local_190c;
  longlong local_1908;
  undefined4 local_1900;
  undefined4 uStack_18fc;
  undefined4 uStack_18f8;
  undefined4 uStack_18f4;
  uint *******local_18f0;
  undefined4 local_18e8;
  undefined4 local_18e4;
  uint *******local_18e0;
  uint *******local_18d8;
  undefined4 local_18d0;
  undefined4 uStack_18cc;
  undefined4 uStack_18c8;
  undefined4 uStack_18c4;
  undefined8 local_18c0;
  uint *******local_18b8;
  undefined4 uStack_18b0;
  undefined4 uStack_18ac;
  uint *******local_18a8;
  longlong local_18a0;
  undefined8 local_1898;
  undefined8 local_1890;
  uint *******local_1880;
  uint *******local_1878;
  uint *******local_1870;
  longlong *local_1868 [71];
  uint auStack_162c [8];
  int aiStack_160c [4];
  undefined4 uStack_15fc;
  undefined8 local_15f8;
  undefined8 local_15f0;
  undefined8 local_15e8;
  undefined8 local_15e0;
  undefined8 local_15d8;
  undefined8 local_15d0;
  undefined8 local_15c8;
  undefined8 local_15c0;
  undefined8 local_15b8;
  undefined8 local_15b0;
  undefined8 uStack_15a8;
  undefined8 local_15a0;
  undefined8 uStack_1598;
  undefined8 local_1590;
  undefined8 uStack_1588;
  undefined8 local_1580;
  undefined8 uStack_1578;
  undefined8 local_1570;
  undefined8 uStack_1568;
  undefined8 local_1560;
  undefined8 uStack_1558;
  undefined8 local_1550;
  undefined8 uStack_1548;
  undefined8 local_1540;
  undefined8 uStack_1538;
  undefined8 local_1530;
  undefined8 uStack_1528;
  undefined8 local_1520;
  undefined8 uStack_1518;
  undefined8 local_1510;
  undefined8 uStack_1508;
  undefined8 local_1500;
  undefined8 uStack_14f8;
  undefined8 local_1390;
  uint *******local_1130;
  undefined1 local_1128 [8];
  longlong *local_1120 [76];
  longlong local_ec0;
  undefined1 local_eb8 [8];
  longlong *local_eb0 [76];
  uint *******local_c50;
  undefined1 local_c48 [616];
  uint *******local_9e0;
  uint *******local_9d8;
  uint *******pppppppuStack_9d0;
  uint *******local_9c8;
  uint *******local_778;
  uint *******local_770;
  uint ******local_768;
  undefined8 local_760;
  uint *******local_508;
  uint *******local_500;
  undefined8 uStack_4f8;
  undefined8 local_2a0;
  undefined8 uStack_298;
  uint *******local_290;
  undefined8 uStack_48;
  
  uStack_48 = 0x14009f526;
  local_18e8 = param_9;
  local_18e4 = param_10;
  lVar18 = (ulonglong)param_4 * 0x2000;
  pppppppuVar22 = (uint *******)(lVar18 + 0x5329);
  if (((((param_6 < pppppppuVar22) || (*param_5 != 0x666c625f)) || (param_5[1] != 0x30000000)) ||
      ((param_5[0xc] != 0x72646863 || (param_5[0xd] != -0x3ffe0000)))) ||
     ((param_5[0x10] != 6 - (uint)param_4 || (param_5[0x11] != (int)pppppppuVar22)))) {
LAB_14009f63b:
    if (param_4 != 0) {
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      puVar16 = (undefined8 *)__rustc::__rust_alloc(0x15,1);
      if (puVar16 == (undefined8 *)0x0) goto LAB_1400a10a4;
      *puVar16 = 0x6552206120746f6e;
      puVar16[1] = 0x67726f4620686361;
      uVar15 = 0x70616d206567726f;
      goto LAB_14009f7a3;
    }
  }
  else {
    iVar32 = 0x7276706d;
    if (param_4 != 0) {
      iVar32 = 0x7261766d;
    }
    if (((param_5[0xbc] != iVar32) ||
        (uVar28 = param_5[0xbd],
        (uVar28 >> 0x18 | (uVar28 & 0xff0000) >> 8 | (uVar28 & 0xff00) << 8 | uVar28 << 0x18) !=
        ((uint)param_4 << 0xd | 0x5028))) ||
       ((param_5[(ulonglong)param_4 * 0x800 + 0x14c6] != 0x666f655f ||
        ((*(int *)((longlong)param_5 + -0xd + (longlong)pppppppuVar22) != 0x11000000 ||
         (uVar28 = *(uint *)((longlong)param_5 + -5 + (longlong)pppppppuVar22),
         (uVar28 >> 0x18 | (uVar28 & 0xff0000) >> 8 | (uVar28 & 0xff00) << 8 | uVar28 << 0x18) !=
         (int)lVar18 + 0x5318U)))))) goto LAB_14009f63b;
    if (param_4 != 0) {
      puVar30 = &DAT_1401c4125;
      uVar15 = 3;
      local_1948 = (uint *******)0x1401c3e5f;
      local_1950 = (uint *******)&DAT_00000004;
LAB_14009f6b8:
      <std::path::Path>::_join(&local_18a0,param_2,param_3,puVar30,uVar15);
      <alloc::raw_vec::RawVecInner>::try_allocate_in
                (&local_15f0,pppppppuVar22,0,1,1);
      pppppppuVar19 = local_15e0;
      pppppppuVar41 = (uint *******)local_15e8;
      if ((int)local_15f0 == 1) {
        alloc::raw_vec::handle_error(local_15e8,local_15e0);
        goto LAB_1400a111d;
      }
      local_1880 = (uint *******)local_15e8;
      local_1878 = local_15e0;
      memcpy(local_15e0,param_5,(size_t)pppppppuVar22);
      if (pppppppuVar22 == pppppppuVar41) {
        local_1870 = pppppppuVar22;
        <alloc::raw_vec::RawVec<u8>>::grow_one(&local_1880);
        pppppppuVar19 = local_1878;
      }
      *(undefined1 *)((longlong)pppppppuVar19 + (longlong)pppppppuVar22) = 0x80;
      pppppppuVar41 = (uint *******)(lVar18 + 0x532a);
      do {
        if (pppppppuVar41 == local_1880) {
          local_1870 = pppppppuVar41;
          <alloc::raw_vec::RawVec<u8>>::grow_one(&local_1880);
        }
        *(undefined1 *)((longlong)local_1878 + (longlong)pppppppuVar41) = 0;
        pppppppuVar41 = (uint *******)((longlong)pppppppuVar41 + 1);
      } while (((uint)pppppppuVar41 & 0x3f) != 0x38);
      local_1870 = pppppppuVar41;
      if ((ulonglong)((longlong)local_1880 - (longlong)pppppppuVar41) < 8) {
        <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
        <alloc::alloc::Global>(&local_1880,pppppppuVar41,8,1,1);
      }
      uVar21 = (ulonglong)(uint)((int)pppppppuVar22 * 8);
      *(ulonglong *)((longlong)local_1878 + (longlong)local_1870) =
           (uVar21 & 0xff000000) << 8 | (uVar21 & 0xff0000) << 0x18 | (uVar21 & 0xff00) << 0x28 |
           uVar21 << 0x38;
      local_1870 = local_1870 + 1;
      if (local_1870 == (uint *******)0x0) {
        uVar33 = 0x67e6096a;
        uVar28 = 0x85ae67bb;
        uVar27 = 0x72f36e3c;
        uVar23 = 0x3af54fa5;
        uVar29 = 0x7f520e51;
        uVar37 = 0x8c68059b;
        uVar36 = 0xabd9831f;
        uVar14 = 0x19cde05b;
      }
      else {
        uVar27 = 0xbb67ae85;
        uVar29 = 0xa54ff53a;
        local_1980 = 0;
        pppppppuVar19 = local_1878;
        pppppppuVar41 = local_1870;
        uVar28 = 0x6a09e667;
        uVar37 = 0x510e527f;
        uVar14 = 0x5be0cd19;
        uVar13 = 0x1f83d9ab;
        uVar23 = 0x3c6ef372;
        uVar36 = 0x9b05688c;
        do {
          uVar35 = uVar36;
          uVar39 = uVar23;
          uVar33 = uVar13;
          uVar34 = uVar14;
          uVar42 = uVar37;
          uVar31 = uVar28;
          pppppppuVar17 = (uint *******)&DAT_00000040;
          if (pppppppuVar41 < &DAT_00000040) {
            pppppppuVar17 = pppppppuVar41;
          }
          local_1500 = 0;
          uStack_14f8 = 0;
          local_1510 = 0;
          uStack_1508 = 0;
          local_1520 = 0;
          uStack_1518 = 0;
          local_1530 = 0;
          uStack_1528 = 0;
          local_1540 = 0;
          uStack_1538 = 0;
          local_1550 = 0;
          uStack_1548 = 0;
          local_1560 = 0;
          uStack_1558 = 0;
          local_1570 = 0;
          uStack_1568 = 0;
          local_1580 = 0;
          uStack_1578 = 0;
          local_1590 = 0;
          uStack_1588 = 0;
          local_15a0 = 0;
          uStack_1598 = 0;
          local_15b0 = 0;
          uStack_15a8 = 0;
          uVar28 = (uint)pppppppuVar17 >> 2;
          if (uVar28 == 0) {
            uVar15 = 4;
LAB_1400a108e:
            core::slice::index::slice_index_fail
                      (local_1980,uVar15,pppppppuVar17,&PTR_s_src_import_rs_1401c42f8);
            goto LAB_1400a111d;
          }
          uVar36 = *(uint *)pppppppuVar19;
          uVar36 = uVar36 >> 0x18 | (uVar36 & 0xff0000) >> 8 | (uVar36 & 0xff00) << 8 |
                   uVar36 << 0x18;
          local_15f0 = (uint *******)CONCAT44(local_15f0._4_4_,uVar36);
          if (uVar28 == 1) {
            uVar15 = 8;
            local_1980 = 4;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)((longlong)pppppppuVar19 + 4);
          local_15f0 = (uint *******)
                       CONCAT44(uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                                uVar23 << 0x18,uVar36);
          if (uVar28 == 2) {
            uVar15 = 0xc;
            local_1980 = 8;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 1);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15e8 = (code *)CONCAT44(local_15e8._4_4_,uVar23);
          if (uVar28 == 3) {
            uVar15 = 0x10;
            local_1980 = 0xc;
            goto LAB_1400a108e;
          }
          uVar14 = *(uint *)((longlong)pppppppuVar19 + 0xc);
          local_15e8 = (code *)CONCAT44(uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 |
                                        (uVar14 & 0xff00) << 8 | uVar14 << 0x18,uVar23);
          if (uVar28 == 4) {
            uVar15 = 0x14;
            local_1980 = 0x10;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 2);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15e0 = (uint *******)CONCAT44(local_15e0._4_4_,uVar23);
          if (uVar28 == 5) {
            uVar15 = 0x18;
            local_1980 = 0x14;
            goto LAB_1400a108e;
          }
          uVar14 = *(uint *)((longlong)pppppppuVar19 + 0x14);
          local_15e0 = (uint *******)
                       CONCAT44(uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 | (uVar14 & 0xff00) << 8 |
                                uVar14 << 0x18,uVar23);
          if (uVar28 == 6) {
            uVar15 = 0x1c;
            local_1980 = 0x18;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 3);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15d8 = (code *)CONCAT44(local_15d8._4_4_,uVar23);
          if (uVar28 == 7) {
            uVar15 = 0x20;
            local_1980 = 0x1c;
            goto LAB_1400a108e;
          }
          uVar14 = *(uint *)((longlong)pppppppuVar19 + 0x1c);
          local_15d8 = (code *)CONCAT44(uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 |
                                        (uVar14 & 0xff00) << 8 | uVar14 << 0x18,uVar23);
          if (uVar28 == 8) {
            uVar15 = 0x24;
            local_1980 = 0x20;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 4);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15d0 = (uint *******)CONCAT44(local_15d0._4_4_,uVar23);
          if (uVar28 == 9) {
            uVar15 = 0x28;
            local_1980 = 0x24;
            goto LAB_1400a108e;
          }
          uVar14 = *(uint *)((longlong)pppppppuVar19 + 0x24);
          local_15d0 = (uint *******)
                       CONCAT44(uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 | (uVar14 & 0xff00) << 8 |
                                uVar14 << 0x18,uVar23);
          if (uVar28 == 10) {
            uVar15 = 0x2c;
            local_1980 = 0x28;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 5);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15c8 = (code *)CONCAT44(local_15c8._4_4_,uVar23);
          if (uVar28 == 0xb) {
            uVar15 = 0x30;
            local_1980 = 0x2c;
            goto LAB_1400a108e;
          }
          uVar14 = *(uint *)((longlong)pppppppuVar19 + 0x2c);
          local_15c8 = (code *)CONCAT44(uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 |
                                        (uVar14 & 0xff00) << 8 | uVar14 << 0x18,uVar23);
          if (uVar28 == 0xc) {
            uVar15 = 0x34;
            local_1980 = 0x30;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 6);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15c0 = (longlong *)CONCAT44(local_15c0._4_4_,uVar23);
          if (uVar28 == 0xd) {
            uVar15 = 0x38;
            local_1980 = 0x34;
            goto LAB_1400a108e;
          }
          uVar14 = *(uint *)((longlong)pppppppuVar19 + 0x34);
          local_15c0 = (longlong *)
                       CONCAT44(uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 | (uVar14 & 0xff00) << 8 |
                                uVar14 << 0x18,uVar23);
          if (uVar28 == 0xe) {
            uVar15 = 0x3c;
            local_1980 = 0x38;
            goto LAB_1400a108e;
          }
          uVar23 = *(uint *)(pppppppuVar19 + 7);
          uVar23 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                   uVar23 << 0x18;
          local_15b8 = (code *)CONCAT44(local_15b8._4_4_,uVar23);
          if (uVar28 == 0xf) {
            uVar15 = 0x40;
            local_1980 = 0x3c;
            goto LAB_1400a108e;
          }
          local_18a8 = (uint *******)((longlong)pppppppuVar19 + (longlong)pppppppuVar17);
          pppppppuVar41 = (uint *******)((longlong)pppppppuVar41 - (longlong)pppppppuVar17);
          uVar28 = *(uint *)((longlong)pppppppuVar19 + 0x3c);
          local_15b8 = (code *)CONCAT44(uVar28 >> 0x18 | (uVar28 & 0xff0000) >> 8 |
                                        (uVar28 & 0xff00) << 8 | uVar28 << 0x18,uVar23);
          lVar18 = 0x10;
          do {
            uVar28 = auStack_162c[lVar18];
            uVar23 = *(uint *)((longlong)&local_15f8 + lVar18 * 4);
            *(uint *)((longlong)&local_15f0 + lVar18 * 4) =
                 uVar36 + aiStack_160c[lVar18] +
                 (uVar23 >> 10 ^ (uVar23 << 0xd | uVar23 >> 0x13) ^ (uVar23 << 0xf | uVar23 >> 0x11)
                 ) + (uVar28 >> 3 ^
                     (uVar28 << 0xe | uVar28 >> 0x12) ^ (uVar28 << 0x19 | uVar28 >> 7));
            lVar18 = lVar18 + 1;
            uVar36 = uVar28;
          } while (lVar18 != 0x40);
          lVar18 = 0;
          uVar10 = uVar27;
          uVar23 = uVar39;
          uVar24 = uVar29;
          uVar11 = uVar35;
          uVar13 = uVar33;
          uVar28 = uVar31;
          uVar37 = uVar42;
          uVar3 = uVar34;
          do {
            uVar36 = uVar37;
            uVar2 = uVar28;
            uVar14 = uVar13;
            uVar13 = uVar11;
            uVar40 = uVar23;
            uVar23 = uVar10;
            iVar32 = ((uVar36 << 7 | uVar36 >> 0x19) ^
                     (uVar36 << 0x15 | uVar36 >> 0xb) ^ (uVar36 << 0x1a | uVar36 >> 6)) + uVar3 +
                     *(int *)((longlong)&DAT_1401c41f8 + lVar18) +
                     ((uVar13 ^ uVar14) & uVar36 ^ uVar14) +
                     *(int *)((longlong)&local_15f0 + lVar18);
            uVar37 = uVar24 + iVar32;
            uVar28 = (uVar40 & uVar23 ^ (uVar40 ^ uVar23) & uVar2) +
                     ((uVar2 << 10 | uVar2 >> 0x16) ^
                     (uVar2 << 0x13 | uVar2 >> 0xd) ^ (uVar2 << 0x1e | uVar2 >> 2)) + iVar32;
            lVar18 = lVar18 + 4;
            uVar10 = uVar2;
            uVar24 = uVar40;
            uVar11 = uVar36;
            uVar3 = uVar14;
          } while (lVar18 != 0x100);
          uVar28 = uVar31 + uVar28;
          uVar27 = uVar27 + uVar2;
          uVar23 = uVar39 + uVar23;
          uVar29 = uVar29 + uVar40;
          uVar37 = uVar42 + uVar37;
          uVar36 = uVar35 + uVar36;
          uVar13 = uVar33 + uVar13;
          uVar14 = uVar34 + uVar14;
          pppppppuVar19 = local_18a8;
          local_1920 = uVar34;
          local_191c = uVar33;
          local_1918 = uVar35;
          local_1914 = uVar42;
          local_1910 = uVar39;
          local_190c = uVar31;
        } while (pppppppuVar41 != (uint *******)0x0);
        uVar33 = uVar28 >> 0x18 | (uVar28 & 0xff0000) >> 8 | (uVar28 & 0xff00) << 8 |
                 uVar28 * 0x1000000;
        uVar28 = uVar27 >> 0x18 | (uVar27 & 0xff0000) >> 8 | (uVar27 & 0xff00) << 8 |
                 uVar27 * 0x1000000;
        uVar27 = uVar23 >> 0x18 | (uVar23 & 0xff0000) >> 8 | (uVar23 & 0xff00) << 8 |
                 uVar23 * 0x1000000;
        uVar23 = uVar29 >> 0x18 | (uVar29 & 0xff0000) >> 8 | (uVar29 & 0xff00) << 8 |
                 uVar29 * 0x1000000;
        uVar29 = uVar37 >> 0x18 | (uVar37 & 0xff0000) >> 8 | (uVar37 & 0xff00) << 8 |
                 uVar37 * 0x1000000;
        uVar37 = uVar36 >> 0x18 | (uVar36 & 0xff0000) >> 8 | (uVar36 & 0xff00) << 8 |
                 uVar36 * 0x1000000;
        uVar36 = uVar13 >> 0x18 | (uVar13 & 0xff0000) >> 8 | (uVar13 & 0xff00) << 8 |
                 uVar13 * 0x1000000;
        uVar14 = uVar14 >> 0x18 | (uVar14 & 0xff0000) >> 8 | (uVar14 & 0xff00) << 8 |
                 uVar14 * 0x1000000;
      }
      local_15f0 = (uint *******)CONCAT44(uVar28,uVar33);
      local_15e8 = (code *)CONCAT44(uVar23,uVar27);
      local_15e0 = (uint *******)CONCAT44(uVar37,uVar29);
      local_15d8 = (code *)CONCAT44(uVar14,uVar36);
      if (local_1880 != (uint *******)0x0) {
        __rustc::__rust_dealloc(local_1878,local_1880,1);
      }
      <alloc::string::String_as_core::iter::traits::collect::FromIterator<alloc::string::String>>
      ::from_iter::
      <core::iter::adapters::map::Map<core::slice::iter::Iter<u8>,menu_overlay::import::hex::_closure_0_>>
                (&local_1908,&local_15f0,&local_15d0);
      local_508 = local_1948;
      local_500 = local_1950;
      local_2a0 = (uint *******)CONCAT44(uStack_18fc,local_1900);
      uStack_298 = (uint *******)CONCAT44(uStack_18f4,uStack_18f8);
      std::sys::fs::read_dir(&local_15f0,local_1898,local_1890);
      if ((int)local_15f0 == 2) {
        local_1878 = (uint *******)local_15e8;
        local_1880 = (uint *******)0x2;
LAB_14009fd91:
        core::ptr::drop_glue::
        <core::result::Result<std::fs::ReadDir,core::io::error::Error>>
                  (&local_1880);
        local_1130 = (uint *******)0x2;
        bVar7 = true;
      }
      else {
        memcpy(&local_1880,&local_15f0,0x270);
        local_1130 = local_1880;
        if (local_1880 == (uint *******)0x2) goto LAB_14009fd91;
        memcpy(&local_778,&local_1878,0x268);
        bVar7 = false;
      }
      memcpy(local_c48,&local_778,0x268);
      local_15f0 = (uint *******)0x0;
      local_1390 = 0;
      local_ec0 = 2;
      local_9e0 = (uint *******)&local_9e0;
      local_9d8 = (uint *******)&local_2a0;
      pppppppuStack_9d0 = (uint *******)&local_508;
      local_c50 = (uint *******)0x2;
      local_9c8 = local_9e0;
      if (bVar7) {
LAB_14009fe56:
        local_1130 = (uint *******)0x2;
        if ((int)local_ec0 == 2) {
LAB_14009fe70:
          local_ec0 = 2;
          bVar7 = false;
        }
        else {
          local_1968 = (uint *******)&local_9e0;
          do {
            <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
            ::next(&local_1880,&local_ec0);
            if ((int)local_1880 != 1) {
              if (local_ec0 != 0) {
                if (local_ec0 == 2) goto LAB_14009fe70;
                <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
                ::drop(local_eb8);
              }
              LOCK();
              *local_eb0[0] = *local_eb0[0] + -1;
              UNLOCK();
              if (*local_eb0[0] == 0) {
                <alloc::sync::Arc<std::path::PathBuf>>::
                drop_slow(local_eb0);
              }
              goto LAB_14009fe70;
            }
            memcpy(&local_778,&local_1878,600);
            cVar12 = <&mut_core::iter::adapters::flatten::try_flatten_one<core::result::Result<std::fs::DirEntry,core::io::error::Error>,(),core::ops::control_flow::ControlFlow<()>,core::iter::adapters::map::map_try_fold<std::fs::DirEntry,std::path::PathBuf,(),core::ops::control_flow::ControlFlow<()>,menu_overlay::import::kept_already::{closure#0},core::iter::adapters::filter::filter_try_fold<std::path::PathBuf,(),core::ops::control_flow::ControlFlow<()>,menu_overlay::import::kept_already::{closure#1},core::iter::traits::iterator::Iterator::any::check<std::path::PathBuf,menu_overlay::import::kept_already::{closure#2}>::{closure#0}>::{closure#0}>::{closure#0}>::{closure#0}as_core::ops::function::FnMut<((),core::result::Result<std::fs::DirEntry,core::io::error::Error>)>>
                     ::call_mut(&local_1968,&local_778);
          } while (cVar12 == '\0');
          bVar7 = true;
        }
      }
      else {
        memcpy(local_1128,local_c48,0x268);
        local_1968 = (uint *******)&local_9e0;
        do {
          <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
          ::next(&local_1880,&local_1130);
          if ((int)local_1880 != 1) {
            local_c50 = (uint *******)0x2;
            if (local_1130 != (uint *******)0x0) {
              if ((int)local_1130 == 2) goto LAB_14009fe56;
              <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
              ::drop(local_1128);
            }
            LOCK();
            *local_1120[0] = *local_1120[0] + -1;
            UNLOCK();
            if (*local_1120[0] == 0) {
              <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                        (local_1120);
            }
            goto LAB_14009fe56;
          }
          memcpy(&local_778,&local_1878,600);
          cVar12 = <&mut_core::iter::adapters::flatten::try_flatten_one<core::result::Result<std::fs::DirEntry,core::io::error::Error>,(),core::ops::control_flow::ControlFlow<()>,core::iter::adapters::map::map_try_fold<std::fs::DirEntry,std::path::PathBuf,(),core::ops::control_flow::ControlFlow<()>,menu_overlay::import::kept_already::{closure#0},core::iter::adapters::filter::filter_try_fold<std::path::PathBuf,(),core::ops::control_flow::ControlFlow<()>,menu_overlay::import::kept_already::{closure#1},core::iter::traits::iterator::Iterator::any::check<std::path::PathBuf,menu_overlay::import::kept_already::{closure#2}>::{closure#0}>::{closure#0}>::{closure#0}>::{closure#0}as_core::ops::function::FnMut<((),core::result::Result<std::fs::DirEntry,core::io::error::Error>)>>
                   ::call_mut(&local_1968,&local_778);
        } while (cVar12 == '\0');
        bVar7 = true;
      }
      core::ptr::drop_glue::
      <core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>>
                (&local_15f0);
      if (bVar7) {
        uVar15 = 0x8000000000000000;
LAB_14009ffa8:
        *param_1 = uVar15;
LAB_1400a0ec8:
        if (local_1908 != 0) {
          __rustc::__rust_dealloc(CONCAT44(uStack_18fc,local_1900),local_1908,1);
        }
        if (local_18a0 == 0) {
          return;
        }
        __rustc::__rust_dealloc(local_1898,local_18a0,1);
        return;
      }
      local_1968 = local_1948;
      pppppppuStack_1960 = local_1950;
      std::sys::fs::read_dir(&local_15f0,local_1898,local_1890);
      if ((int)local_15f0 == 2) {
        local_1878 = (uint *******)local_15e8;
        local_1880 = (uint *******)0x2;
LAB_1400a007b:
        core::ptr::drop_glue::
        <core::result::Result<std::fs::ReadDir,core::io::error::Error>>
                  (&local_1880);
        pppppppuVar41 = (uint *******)0x2;
        bVar7 = true;
      }
      else {
        memcpy(&local_1880,&local_15f0,0x270);
        pppppppuVar41 = local_1880;
        if (local_1880 == (uint *******)0x2) goto LAB_1400a007b;
        memcpy(&local_9e0,&local_1878,0x268);
        bVar7 = false;
      }
      local_15f0 = (uint *******)0x0;
      local_1390 = 0;
      local_1130 = (uint *******)0x2;
      local_ec0 = 2;
      local_c50 = pppppppuVar41;
      memcpy(local_c48,&local_9e0,0x268);
      local_18b8 = (uint *******)&local_1968;
      memcpy(&local_770,&local_9e0,0x268);
      local_778 = (uint *******)0x2;
      if (!bVar7) {
        memcpy(&local_1870,&local_9e0,0x268);
        local_1880 = (uint *******)0x0;
        local_1940 = (uint *******)&local_18b8;
        pppppppuVar19 = (uint *******)0x0;
        local_1878 = pppppppuVar41;
        while( true ) {
          <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
          ::next(&local_508,&local_1878);
          if ((int)local_508 != 1) break;
          memcpy(&uStack_298,&local_500,600);
          local_2a0 = pppppppuVar19;
          pppppppuVar19 =
               (uint *******)
               <&mut_core::iter::adapters::flatten::flatten_one<core::result::Result<std::fs::DirEntry,core::io::error::Error>,usize,core::iter::adapters::map::map_fold<std::fs::DirEntry,usize,usize,<core::iter::adapters::filter::Filter<_,_>as_core::iter::traits::iterator::Iterator>::count::to_usize<std::fs::DirEntry,menu_overlay::import::count_in::{closure#0}>::{closure#0},<usize_as_core::iter::traits::accum::Sum>::sum<core::iter::adapters::map::Map<core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>,<core::iter::adapters::filter::Filter<_,_>as_core::iter::traits::iterator::Iterator>::count::to_usize<std::fs::DirEntry,menu_overlay::import::count_in::{closure#0}>::{closure#0}>>::{closure#0}>::{closure#0}>::{closure#0}as_core::ops::function::FnMut<(usize,core::result::Result<std::fs::DirEntry,core::io::error::Error>)>>
               ::call_mut(&local_1940,pppppppuVar19,&uStack_298);
        }
        if (local_1878 != (uint *******)0x0) {
          <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
          ::drop(&local_1870);
        }
        LOCK();
        *local_1868[0] = *local_1868[0] + -1;
        UNLOCK();
        if (*local_1868[0] == 0) {
          <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                    (local_1868);
        }
        if ((uint *******)0x1f3 < pppppppuVar19) {
          local_15f0 = (uint *******)&DAT_1401c4170;
          local_15e8 = <u64_as_core::fmt::Display>::fmt;
          alloc::fmt::format::format_inner
                    (aiStack_160c + 1,&DAT_1401c4178,&local_15f0);
          param_1[3] = local_15f8;
          *(int *)(param_1 + 1) = aiStack_160c[1];
          *(int *)((longlong)param_1 + 0xc) = aiStack_160c[2];
          *(int *)(param_1 + 2) = aiStack_160c[3];
          *(undefined4 *)((longlong)param_1 + 0x14) = uStack_15fc;
          uVar15 = 0x8000000000000001;
          goto LAB_14009ffa8;
        }
      }
      local_15f0 = (uint *******)(param_5 + 0x30);
      local_15e8 = (code *)0x100;
      local_15e0 = (uint *******)0x2;
      local_15d8 = (code *)((ulonglong)local_15d8 & 0xffffffffffffff00);
      <alloc::vec::Vec<u16>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<u16,core::iter::adapters::take_while::TakeWhile<core::iter::adapters::map::Map<core::slice::iter::Chunks<u8>,menu_overlay::import::title_of::{closure#0}>,menu_overlay::import::title_of::{closure#1}>>>
      ::from_iter(&local_9e0,&local_15f0);
      pppppppuVar26 = local_9d8;
      local_15e8 = (code *)((longlong)local_9d8 + (longlong)pppppppuStack_9d0 * 2);
      local_15f0 = local_9d8;
      local_15e0 = (uint *******)((ulonglong)local_15e0 & 0xffffffffffff0000);
      <alloc::string::String_as_core::iter::traits::collect::FromIterator<char>>
      ::from_iter::
      <core::iter::adapters::map::Map<core::char::decode::DecodeUtf16<core::iter::adapters::cloned::Cloned<core::slice::iter::Iter<u16>>>,<alloc::string::String>::from_utf16_lossy::_closure_0_>>
                (&local_1880,&local_15f0);
      pppppppuVar41 = local_1878;
      <alloc::string::String_as_core::iter::traits::collect::FromIterator<char>>
      ::from_iter::
      <core::iter::adapters::filter::Filter<core::str::iter::Chars,menu_overlay::import::title_of::_closure_2_>>
                (&local_778,local_1878,(undefined1 *)((longlong)local_1870 + (longlong)local_1878));
      pppppppuVar8 = local_770;
      ppppppuVar25 = local_768;
      pvVar20 = (void *)<str>::trim_matches::<<char>::is_whitespace>(local_770);
      <alloc::raw_vec::RawVecInner>::try_allocate_in(&local_15f0);
      pppppppuVar17 = local_15e0;
      pppppppuVar19 = (uint *******)local_15e8;
      if ((int)local_15f0 == 1) {
        alloc::raw_vec::handle_error(local_15e8,local_15e0);
        goto LAB_1400a111d;
      }
      if (ppppppuVar25 != (uint ******)0x0) {
        memcpy(local_15e0,pvVar20,(size_t)ppppppuVar25);
      }
      if (local_778 != (uint *******)0x0) {
        __rustc::__rust_dealloc(pppppppuVar8,local_778,1);
      }
      if (local_1880 != (uint *******)0x0) {
        __rustc::__rust_dealloc(pppppppuVar41,local_1880,1);
      }
      if (local_9e0 != (uint *******)0x0) {
        __rustc::__rust_dealloc(pppppppuVar26,(longlong)local_9e0 * 2,2);
      }
      clean(&local_1940,pppppppuVar17,ppppppuVar25);
      if (pppppppuVar19 != (uint *******)0x0) {
        __rustc::__rust_dealloc(pppppppuVar17,pppppppuVar19,1);
      }
      local_15e8 = (code *)(local_1930 + (longlong)local_1938);
      local_15f0 = local_1938;
      local_15e0 = (uint *******)&DAT_00000028;
      <alloc::string::String_as_core::iter::traits::collect::FromIterator<char>>
      ::from_iter::
      <core::iter::adapters::take::Take<core::iter::adapters::filter::Filter<core::str::iter::Chars,menu_overlay::import::file_stem::_closure_0_>>>
                (&local_1880,&local_15f0);
      pppppppuVar41 = local_1878;
      pppppppuVar26 = local_1870;
      pvVar20 = (void *)<str>::trim_matches::<<char>::is_whitespace>(local_1878);
      <alloc::raw_vec::RawVecInner>::try_allocate_in(&local_15f0);
      pppppppuVar17 = local_15e0;
      pppppppuVar19 = (uint *******)local_15e8;
      if ((int)local_15f0 == 1) {
        alloc::raw_vec::handle_error(local_15e8,local_15e0);
        goto LAB_1400a111d;
      }
      if (pppppppuVar26 != (uint *******)0x0) {
        memcpy(local_15e0,pvVar20,(size_t)pppppppuVar26);
      }
      <alloc::vec::Vec<u8>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<u8,core::iter::adapters::map::Map<core::slice::iter::Iter<u8>,<[u8]>::to_ascii_uppercase::{closure#0}>>>
      ::from_iter(&local_15f0,pppppppuVar17,
                  (undefined1 *)((longlong)pppppppuVar17 + (longlong)pppppppuVar26));
      pppppppuVar9 = (uint *******)local_15e8;
      pppppppuVar8 = local_15f0;
      pppppppuVar38 = pppppppuVar19;
      if (local_15e0 == (uint *******)0x3) {
        if ((*(char *)((longlong)local_15e8 + 2) != 'N' || *(short *)local_15e8 != 0x4f43) &&
           ((*(char *)((longlong)local_15e8 + 2) != 'N' || *(short *)local_15e8 != 0x5250 &&
            (*(char *)((longlong)local_15e8 + 2) != 'X' || *(short *)local_15e8 != 0x5541)))) {
          if ((pppppppuVar26 != (uint *******)0x0) &&
             (*(char *)((longlong)local_15e8 + 2) != 'L' || *(short *)local_15e8 != 0x554e))
          goto LAB_1400a0635;
        }
LAB_1400a060e:
        pppppppuVar38 = (uint *******)0xffffffffffffffff;
        if (pppppppuVar19 != (uint *******)0x0) {
          __rustc::__rust_dealloc(pppppppuVar17,pppppppuVar19,1);
        }
      }
      else if ((local_15e0 == (uint *******)&DAT_00000004) &&
              (((*(char *)((longlong)local_15e8 + 2) == 'M' && *(short *)local_15e8 == 0x4f43 ||
                (*(char *)((longlong)local_15e8 + 2) == 'T' && *(short *)local_15e8 == 0x504c)) &&
               (0x2f < *(byte *)((longlong)local_15e8 + 3))))) {
        if ((pppppppuVar26 == (uint *******)0x0) || (*(byte *)((longlong)local_15e8 + 3) < 0x3a))
        goto LAB_1400a060e;
      }
      else if (pppppppuVar26 == (uint *******)0x0) goto LAB_1400a060e;
LAB_1400a0635:
      if (pppppppuVar8 != (uint *******)0x0) {
        __rustc::__rust_dealloc(pppppppuVar9,pppppppuVar8,1);
      }
      if (local_1880 != (uint *******)0x0) {
        __rustc::__rust_dealloc(pppppppuVar41,local_1880,1);
      }
      if (pppppppuVar38 != (uint *******)0xffffffffffffffff) {
        pppppppuStack_1960 = pppppppuVar17;
        local_1968 = pppppppuVar38;
        local_1958 = pppppppuVar26;
        goto LAB_1400a0718;
      }
      pppppppuVar41 = (uint *******)CONCAT44(uStack_18fc,local_1900);
      uVar21 = CONCAT44(uStack_18f4,uStack_18f8);
      if (uVar21 < 9) {
        if (uVar21 == 8) {
LAB_1400a06c2:
          local_1878 = (uint *******)&DAT_00000008;
          local_15f0 = (uint *******)&local_1880;
          local_15e8 = (code *)&<&str_as_core::fmt::Display>::fmt;
          local_1880 = pppppppuVar41;
          alloc::fmt::format::format_inner(&local_1968,&DAT_1401c3e48,&local_15f0)
          ;
LAB_1400a0718:
          local_778 = local_1948;
          local_770 = local_1950;
          local_768 = (uint ******)&DAT_1401c4128;
          local_760 = 5;
          iVar32 = 1;
          local_18e0 = pppppppuStack_1960;
          local_18d8 = local_1958;
          do {
            pppppppuVar19 = local_18d8;
            pppppppuVar41 = local_18e0;
            local_1880 = (uint *******)CONCAT44(local_1880._4_4_,iVar32);
            if (iVar32 == 1) {
              <alloc::raw_vec::RawVecInner>::try_allocate_in
                        (&local_15f0,local_18d8,0,1,1);
              pppppppuVar26 = local_15e0;
              pppppppuVar17 = (uint *******)local_15e8;
              if ((char)local_15f0 != '\0') {
                alloc::raw_vec::handle_error(local_15e8,local_15e0);
                goto LAB_1400a111d;
              }
              if (pppppppuVar19 != (uint *******)0x0) {
                memcpy(local_15e0,pppppppuVar41,(size_t)pppppppuVar19);
              }
              local_2a0 = pppppppuVar17;
              uStack_298 = pppppppuVar26;
              local_290 = pppppppuVar19;
            }
            else {
              local_15f0 = (uint *******)&local_18e0;
              local_15e8 = (code *)&<&str_as_core::fmt::Display>::fmt;
              local_15d8 = <i32_as_core::fmt::Display>::fmt;
              local_15e0 = (uint *******)&local_1880;
              alloc::fmt::format::format_inner
                        (&local_2a0,&DAT_1401c3e54,&local_15f0);
            }
            local_508 = local_2a0;
            local_500 = uStack_298;
            uStack_4f8 = local_290;
            local_18f0 = (uint *******)&local_508;
            local_1928 = (uint *******)&local_778;
            local_15f0 = (uint *******)&local_18f0;
            local_15e8 = <&mut_alloc::string::String_as_core::fmt::Display>
                         ::fmt;
            local_15e0 = (uint *******)&local_1928;
            local_15d8 = <&&str_as_core::fmt::Display>::fmt;
            alloc::fmt::format::format_inner
                      (&local_9e0,&DAT_1401c3de8,&local_15f0);
            pppppppuVar41 = local_9d8;
            <std::path::Path>::_join
                      (&local_1880,local_1898,local_1890,local_9d8,pppppppuStack_9d0);
            if (local_9e0 != (uint *******)0x0) {
              __rustc::__rust_dealloc(pppppppuVar41,local_9e0,1);
            }
            pppppppuVar41 = local_1878;
            std::sys::fs::metadata(&local_15f0,local_1878);
            if ((int)local_15f0 == 2) {
              if ((1 < ((uint)local_15e8 & 3) - 2) && (((ulonglong)local_15e8 & 3) != 0)) {
                local_15e8 = (code *)((longlong)local_15e8 + -1);
                local_15f0 = (uint *******)CONCAT71(local_15f0._1_7_,3);
                <core::io::error::CustomOwner_as_core::ops::drop::Drop>
                ::drop(&local_15e8);
              }
              if (local_1880 != (uint *******)0x0) {
                __rustc::__rust_dealloc(pppppppuVar41,local_1880,1);
              }
              local_1928 = &local_768;
              local_15f0 = (uint *******)&local_18f0;
              local_15e8 = <&mut_alloc::string::String_as_core::fmt::Display>
                           ::fmt;
              local_15e0 = (uint *******)&local_1928;
              local_15d8 = <&&str_as_core::fmt::Display>::fmt;
              alloc::fmt::format::format_inner
                        (&local_9e0,&DAT_1401c3de8,&local_15f0);
              <std::path::Path>::join::<alloc::string::String>
                        (&local_1880,local_1898,local_1890,&local_9e0);
              pppppppuVar41 = local_1878;
              std::sys::fs::metadata(&local_15f0,local_1878);
              if ((int)local_15f0 != 2) goto LAB_1400a0b04;
              if ((1 < ((uint)local_15e8 & 3) - 2) && (((ulonglong)local_15e8 & 3) != 0)) {
                local_15e8 = (code *)((longlong)local_15e8 + -1);
                local_15f0 = (uint *******)CONCAT71(local_15f0._1_7_,3);
                <core::io::error::CustomOwner_as_core::ops::drop::Drop>
                ::drop(&local_15e8);
              }
              if (local_1880 != (uint *******)0x0) {
                __rustc::__rust_dealloc(pppppppuVar41);
              }
              local_18b8 = local_500;
              uStack_18b0 = (undefined4)uStack_4f8;
              uStack_18ac = uStack_4f8._4_4_;
              if (local_508 != (uint *******)0xffffffffffffffff) goto LAB_1400a0b48;
            }
            else {
LAB_1400a0b04:
              if (local_1880 != (uint *******)0x0) {
                __rustc::__rust_dealloc(pppppppuVar41);
              }
              if (local_508 != (uint *******)0x0) {
                __rustc::__rust_dealloc(local_500);
              }
            }
            iVar32 = iVar32 + 1;
          } while( true );
        }
      }
      else if (-0x41 < *(char *)(pppppppuVar41 + 1)) goto LAB_1400a06c2;
      core::str::slice_error_fail
                (pppppppuVar41,uVar21,0,8,&PTR_s_src_import_rs_1401c3e30);
LAB_1400a111d:
                    /* WARNING: Does not return */
      pcVar1 = (code *)invalidInstructionException();
      (*pcVar1)();
    }
    if ((*(byte *)(param_5 + 0x1a) < 7) && ((0x58U >> (*(byte *)(param_5 + 0x1a) & 0x1f) & 1) != 0))
    {
      puVar30 = &DAT_1401c411d;
      uVar15 = 8;
      local_1948 = (uint *******)0x1401c3e5c;
      local_1950 = (uint *******)0x3;
      goto LAB_14009f6b8;
    }
  }
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar16 = (undefined8 *)__rustc::__rust_alloc(0x15,1);
  if (puVar16 == (undefined8 *)0x0) {
LAB_1400a10a4:
    alloc::raw_vec::handle_error(1,0x15);
                    /* WARNING: Does not return */
    pcVar1 = (code *)invalidInstructionException();
    (*pcVar1)();
  }
  *puVar16 = 0x6552206120746f6e;
  puVar16[1] = 0x656d616720686361;
  uVar15 = 0x6570797420656d61;
LAB_14009f7a3:
  *(undefined8 *)((longlong)puVar16 + 0xd) = uVar15;
  param_1[1] = 0x15;
  param_1[2] = puVar16;
  param_1[3] = 0x15;
  *param_1 = 0x8000000000000001;
  return;
LAB_1400a0b48:
  local_9d8 = local_500;
  pppppppuStack_9d0 = uStack_4f8;
  local_9e0 = local_508;
  local_15e8 = <alloc::string::String_as_core::fmt::Display>::fmt
  ;
  local_15e0 = (uint *******)&local_778;
  local_15d8 = (code *)&<&str_as_core::fmt::Display>::fmt;
  local_15f0 = (uint *******)&local_9e0;
  alloc::fmt::format::format_inner(&local_1880,&DAT_1401c3de8,&local_15f0);
  <std::path::Path>::join::<alloc::string::String>
            (&local_15f0,local_1898,local_1890,&local_1880);
  if (local_9e0 != (uint *******)0x0) {
    __rustc::__rust_dealloc(local_9d8,local_9e0,1);
  }
  pppppppuVar17 = local_15e0;
  pppppppuVar19 = (uint *******)local_15e8;
  pppppppuVar41 = local_15f0;
  <std::time::SystemTime>::duration_since
            (&local_15f0,&local_18e8,0xd53e8000,0x19db1de);
  local_9e0 = (uint *******)local_15e8;
  if ((char)local_15f0 != '\0') {
    local_9e0 = (uint *******)0x0;
  }
  clean(&local_1880,param_7,param_8);
  local_15f0 = (uint *******)&local_1940;
  local_15e8 = <alloc::string::String_as_core::fmt::Display>::fmt
  ;
  local_15d8 = <alloc::string::String_as_core::fmt::Display>::fmt
  ;
  local_15d0 = (uint *******)&local_9e0;
  local_15c8 = <u64_as_core::fmt::Display>::fmt;
  local_15c0 = &local_1908;
  local_15b8 = <alloc::string::String_as_core::fmt::Display>::fmt
  ;
  local_15e0 = (uint *******)&local_1880;
  alloc::fmt::format::format_inner(&local_778,&DAT_1401c412d,&local_15f0);
  if (local_1880 != (uint *******)0x0) {
    __rustc::__rust_dealloc(local_1878,local_1880,1);
  }
  ppppppuVar25 = local_768;
  pppppppuVar9 = local_770;
  pppppppuVar8 = local_778;
  <std::path::Path>::_with_extension
            (&local_1880,pppppppuVar19,pppppppuVar17,&DAT_1401c4128,5);
  pppppppuVar26 = local_1878;
  write_whole(&local_18d0,local_1878,local_1870,pppppppuVar9,ppppppuVar25);
  if ((CONCAT44(uStack_18cc,local_18d0) == -1) &&
     (write_whole(&local_18d0,pppppppuVar19,pppppppuVar17,param_5,pppppppuVar22),
     CONCAT44(uStack_18cc,local_18d0) == -1)) {
    if (local_1880 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuVar26,local_1880,1);
    }
    pppppppuVar22 = (uint *******)&local_1940;
    if (local_1930 == 0) {
      pppppppuVar22 = (uint *******)&local_1968;
    }
    param_1[2] = pppppppuVar22[2];
    uVar4 = *(undefined4 *)((longlong)pppppppuVar22 + 4);
    uVar5 = *(undefined4 *)(pppppppuVar22 + 1);
    uVar6 = *(undefined4 *)((longlong)pppppppuVar22 + 0xc);
    *(undefined4 *)param_1 = *(undefined4 *)pppppppuVar22;
    *(undefined4 *)((longlong)param_1 + 4) = uVar4;
    *(undefined4 *)(param_1 + 1) = uVar5;
    *(undefined4 *)((longlong)param_1 + 0xc) = uVar6;
    *(byte *)(param_1 + 3) = param_4;
    if (pppppppuVar8 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuVar9,pppppppuVar8,1);
    }
    if (pppppppuVar41 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuVar19,pppppppuVar41,1);
    }
    pppppppuVar22 = local_1940;
    if (local_1930 != 0) {
      pppppppuVar22 = local_1968;
      local_1938 = pppppppuStack_1960;
    }
  }
  else {
    <std::path::Path>::_with_extension
              (&local_15f0,pppppppuVar19,pppppppuVar17,&DAT_1401c4128,5);
    uVar21 = std::fs::remove_file::<std::path::PathBuf>
                       (&local_15f0);
    if ((uVar21 != 0) && ((1 < ((uint)uVar21 & 3) - 2 && ((uVar21 & 3) != 0)))) {
      local_15e8 = (code *)(uVar21 - 1);
      local_15f0 = (uint *******)CONCAT71(local_15f0._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&local_15e8);
    }
    param_1[3] = local_18c0;
    *(undefined4 *)(param_1 + 1) = local_18d0;
    *(undefined4 *)((longlong)param_1 + 0xc) = uStack_18cc;
    *(undefined4 *)(param_1 + 2) = uStack_18c8;
    *(undefined4 *)((longlong)param_1 + 0x14) = uStack_18c4;
    *param_1 = 0x8000000000000001;
    if (local_1880 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuVar26,local_1880,1);
    }
    if (pppppppuVar8 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuVar9,pppppppuVar8,1);
    }
    if (pppppppuVar41 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuVar19,pppppppuVar41,1);
    }
    pppppppuVar22 = local_1940;
    if (local_1968 != (uint *******)0x0) {
      __rustc::__rust_dealloc(pppppppuStack_1960,local_1968,1);
      pppppppuVar22 = local_1940;
    }
  }
  if (pppppppuVar22 != (uint *******)0x0) {
    __rustc::__rust_dealloc(local_1938,pppppppuVar22,1);
  }
  goto LAB_1400a0ec8;
}


