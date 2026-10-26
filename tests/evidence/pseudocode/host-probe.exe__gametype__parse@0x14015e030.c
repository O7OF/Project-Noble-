
/* WARNING: Removing unreachable block (ram,0x00014015ef61) */
/* WARNING: Removing unreachable block (ram,0x00014015f217) */
/* WARNING: Removing unreachable block (ram,0x00014015f207) */
/* WARNING: Removing unreachable block (ram,0x00014015ed73) */
/* WARNING: Removing unreachable block (ram,0x00014015e841) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::gametype::parse */

undefined8 * __rustcall
host_probe::gametype::parse
          (undefined8 *param_1,longlong param_2,ulonglong param_3)

{
  ulonglong uVar1;
  undefined1 auVar2 [16];
  undefined1 auVar3 [16];
  code *pcVar4;
  bool bVar5;
  undefined1 auVar6 [16];
  bool bVar7;
  undefined1 uVar8;
  char cVar9;
  ushort uVar10;
  uint uVar11;
  ulonglong uVar12;
  char *pcVar13;
  char *pcVar14;
  byte *pbVar15;
  undefined8 *puVar16;
  undefined8 uVar17;
  longlong lVar18;
  ulonglong uVar19;
  char *pcVar20;
  char *pcVar21;
  char *pcVar22;
  ulonglong uVar23;
  ulonglong uVar24;
  uint uVar25;
  int iVar26;
  ulonglong uVar27;
  char *pcVar28;
  longlong lVar29;
  byte bVar30;
  byte bVar31;
  byte bVar32;
  byte bVar33;
  byte bVar34;
  byte bVar35;
  byte bVar36;
  byte bVar37;
  byte bVar39;
  byte bVar40;
  byte bVar41;
  byte bVar42;
  byte bVar43;
  byte bVar44;
  byte bVar45;
  byte bVar46;
  undefined1 auVar38 [16];
  byte bVar47;
  byte bVar48;
  byte bVar49;
  byte bVar50;
  byte bVar51;
  byte bVar52;
  byte bVar53;
  byte bVar54;
  byte bVar55;
  byte bVar56;
  byte bVar57;
  byte bVar58;
  byte bVar59;
  byte bVar60;
  byte bVar61;
  byte bVar62;
  undefined1 auVar63 [16];
  ulonglong *local_150;
  undefined1 auStack_110 [16];
  longlong lStack_100;
  longlong local_f8;
  undefined1 uStack_f0;
  undefined1 uStack_ef;
  undefined1 uStack_ee;
  undefined1 uStack_ed;
  undefined4 uStack_ec;
  undefined8 *local_e8;
  char *local_e0;
  byte *local_d8;
  byte *local_d0;
  undefined8 local_c8;
  undefined1 local_c0 [128];
  
  auVar6 = _DAT_1406be850;
  auStack_110 = (undefined1  [16])0x0;
  lStack_100 = 8;
  local_f8 = 0;
  uStack_ee = 2;
  uStack_f0 = 0;
  uStack_ed = 2;
  uStack_ec = CONCAT31(uStack_ec._1_3_,2);
  uVar23 = 0;
  uVar24 = 0;
  local_e8 = param_1;
  do {
    bVar7 = false;
    uVar12 = uVar24;
joined_r0x00014015e144:
    do {
      uVar24 = uVar23;
      if (param_3 < uVar23) {
LAB_14015e1b3:
        bVar7 = true;
        bVar5 = true;
        uVar27 = param_3;
        uVar23 = uVar24;
        uVar24 = uVar12;
        if (param_3 != uVar12) goto LAB_14015e1ce;
        goto LAB_14015f175;
      }
      uVar1 = param_2 + uVar23;
      uVar24 = param_3;
      if (param_3 - uVar23 < 0x10) {
        if (param_3 != uVar23) {
          uVar19 = 0;
          do {
            if (*(char *)(uVar1 + uVar19) == '\n') goto LAB_14015e18e;
            uVar19 = uVar19 + 1;
          } while (param_3 - uVar23 != uVar19);
        }
        goto LAB_14015e1b3;
      }
      uVar19 = uVar1;
      uVar27 = core::slice::memchr::memchr_aligned(10);
      if ((uVar27 & 1) == 0) goto LAB_14015e1b3;
LAB_14015e18e:
      uVar27 = uVar19 + uVar23 + 1;
      uVar24 = uVar23 + uVar19;
      uVar23 = uVar27;
    } while ((param_3 <= uVar24) || (*(char *)(uVar1 + uVar19) != '\n'));
    bVar5 = false;
    uVar24 = uVar27;
LAB_14015e1ce:
    pcVar20 = (char *)(uVar27 - uVar12);
    pcVar28 = pcVar20;
    if ((pcVar20 != (char *)0x0) && (*(char *)(param_2 + -1 + uVar27) == '\n')) {
      pcVar28 = pcVar20 + -1;
      if (pcVar28 == (char *)0x0) {
        pcVar28 = (char *)0x0;
      }
      else if (*(char *)(param_2 + -2 + uVar27) == '\r') {
        pcVar28 = pcVar20 + -2;
      }
    }
    pcVar20 = (char *)::<str>::trim_matches::<<char>::is_whitespace>(param_2 + uVar12);
    if ((pcVar28 != (char *)0x0) && (*pcVar20 == '#')) {
LAB_14015e130:
      uVar12 = uVar24;
      if (bVar5) {
LAB_14015f175:
        if (auStack_110._0_8_ == 0) {
          *local_e8 = 0xffffffffffffffff;
        }
        else {
          cVar9 = is_ours(1);
          if (cVar9 == '\0') {
            local_e8[8] = local_f8;
            local_e8[9] = CONCAT44(uStack_ec,
                                   CONCAT13(uStack_ed,
                                            CONCAT12(uStack_ee,CONCAT11(uStack_ef,uStack_f0))));
            local_e8[6] = auStack_110._8_8_;
            local_e8[7] = lStack_100;
            local_e8[4] = 1;
            local_e8[5] = auStack_110._0_8_;
            local_e8[2] = 0;
            local_e8[3] = 0;
            *local_e8 = 0;
            local_e8[1] = 1;
            return local_e8;
          }
          *local_e8 = 0xffffffffffffffff;
        }
        if (local_f8 != 0) {
          puVar16 = (undefined8 *)(lStack_100 + 8);
          lVar29 = local_f8;
          do {
            if (puVar16[-1] != 0) {
              puVar16 = (undefined8 *)
                        __rustc::__rust_dealloc(*puVar16,puVar16[-1],1);
              return puVar16;
            }
            puVar16 = puVar16 + 4;
            lVar29 = lVar29 + -1;
          } while (lVar29 != 0);
        }
        if (auStack_110._8_8_ == 0) {
                    /* WARNING: Read-only address (ram,0x0001406be850) is written */
          return local_e8;
        }
        puVar16 = (undefined8 *)
                  __rustc::__rust_dealloc(lStack_100,auStack_110._8_8_ << 5,8);
        return puVar16;
      }
      goto joined_r0x00014015e144;
    }
    pcVar13 = (char *)0x0;
    while( true ) {
      pcVar22 = pcVar20 + (longlong)pcVar13;
      if (&DAT_0000000f < pcVar28 + -(longlong)pcVar13) {
        uVar12 = core::slice::memchr::memchr_aligned(0x3d);
        pcVar21 = pcVar22;
        if ((uVar12 & 1) == 0) goto LAB_14015e130;
      }
      else {
        if (pcVar28 == pcVar13) goto LAB_14015e130;
        pcVar21 = (char *)0x0;
        while (pcVar22[(longlong)pcVar21] != '=') {
          pcVar21 = pcVar21 + 1;
          if (pcVar28 + -(longlong)pcVar13 == pcVar21) goto LAB_14015e130;
        }
      }
      pcVar22 = pcVar21 + (longlong)pcVar13 + 1;
      pcVar21 = pcVar21 + (longlong)pcVar13;
      if ((pcVar21 < pcVar28) && (pcVar20[(longlong)pcVar21] == '=')) break;
      pcVar13 = pcVar22;
      if (pcVar28 < pcVar22) goto LAB_14015e130;
    }
    lVar29 = ::<str>::trim_matches::<<char>::is_whitespace>(pcVar20);
    if ((longlong)pcVar21 < 0) {
      uVar17 = 0;
LAB_14015f30c:
      alloc::raw_vec::handle_error(uVar17);
      goto LAB_14015f31f;
    }
    if (pcVar21 == (char *)0x0) {
      local_150 = (ulonglong *)0x1;
      goto LAB_14015e487;
    }
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    local_150 = (ulonglong *)__rustc::__rust_alloc(pcVar21,1);
    uVar17 = 1;
    if (local_150 == (ulonglong *)0x0) goto LAB_14015f30c;
    if (0xffffffffffffffe0 < (ulonglong)(lVar29 - (longlong)local_150) || pcVar21 < &DAT_00000008) {
      pcVar13 = (char *)0x0;
      goto LAB_14015e40b;
    }
    if (pcVar21 < &DAT_00000020) {
      pcVar14 = (char *)0x0;
      goto LAB_14015e3b0;
    }
    pcVar13 = (char *)((ulonglong)pcVar21 & 0x7fffffffffffffe0);
    pcVar14 = (char *)0x0;
    do {
      auVar2 = *(undefined1 (*) [16])(pcVar14 + lVar29);
      auVar3 = *(undefined1 (*) [16])(pcVar14 + lVar29 + 0x10);
      bVar30 = auVar2[0] + 0xbf;
      bVar31 = auVar2[1] + 0xbf;
      bVar32 = auVar2[2] + 0xbf;
      bVar33 = auVar2[3] + 0xbf;
      bVar34 = auVar2[4] + 0xbf;
      bVar35 = auVar2[5] + 0xbf;
      bVar36 = auVar2[6] + 0xbf;
      bVar37 = auVar2[7] + 0xbf;
      bVar39 = auVar2[8] + 0xbf;
      bVar40 = auVar2[9] + 0xbf;
      bVar41 = auVar2[10] + 0xbf;
      bVar42 = auVar2[0xb] + 0xbf;
      bVar43 = auVar2[0xc] + 0xbf;
      bVar44 = auVar2[0xd] + 0xbf;
      bVar45 = auVar2[0xe] + 0xbf;
      bVar46 = auVar2[0xf] + 0xbf;
      bVar47 = auVar3[0] + 0xbf;
      bVar48 = auVar3[1] + 0xbf;
      bVar49 = auVar3[2] + 0xbf;
      bVar50 = auVar3[3] + 0xbf;
      bVar51 = auVar3[4] + 0xbf;
      bVar52 = auVar3[5] + 0xbf;
      bVar53 = auVar3[6] + 0xbf;
      bVar54 = auVar3[7] + 0xbf;
      bVar55 = auVar3[8] + 0xbf;
      bVar56 = auVar3[9] + 0xbf;
      bVar57 = auVar3[10] + 0xbf;
      bVar58 = auVar3[0xb] + 0xbf;
      bVar59 = auVar3[0xc] + 0xbf;
      bVar60 = auVar3[0xd] + 0xbf;
      bVar61 = auVar3[0xe] + 0xbf;
      bVar62 = auVar3[0xf] + 0xbf;
      auVar63[0] = -((byte)((0x19 < bVar30) * '\x19' | (0x19 >= bVar30) * bVar30) == bVar30);
      auVar63[1] = -((byte)((0x19 < bVar31) * '\x19' | (0x19 >= bVar31) * bVar31) == bVar31);
      auVar63[2] = -((byte)((0x19 < bVar32) * '\x19' | (0x19 >= bVar32) * bVar32) == bVar32);
      auVar63[3] = -((byte)((0x19 < bVar33) * '\x19' | (0x19 >= bVar33) * bVar33) == bVar33);
      auVar63[4] = -((byte)((0x19 < bVar34) * '\x19' | (0x19 >= bVar34) * bVar34) == bVar34);
      auVar63[5] = -((byte)((0x19 < bVar35) * '\x19' | (0x19 >= bVar35) * bVar35) == bVar35);
      auVar63[6] = -((byte)((0x19 < bVar36) * '\x19' | (0x19 >= bVar36) * bVar36) == bVar36);
      auVar63[7] = -((byte)((0x19 < bVar37) * '\x19' | (0x19 >= bVar37) * bVar37) == bVar37);
      auVar63[8] = -((byte)((0x19 < bVar39) * '\x19' | (0x19 >= bVar39) * bVar39) == bVar39);
      auVar63[9] = -((byte)((0x19 < bVar40) * '\x19' | (0x19 >= bVar40) * bVar40) == bVar40);
      auVar63[10] = -((byte)((0x19 < bVar41) * '\x19' | (0x19 >= bVar41) * bVar41) == bVar41);
      auVar63[0xb] = -((byte)((0x19 < bVar42) * '\x19' | (0x19 >= bVar42) * bVar42) == bVar42);
      auVar63[0xc] = -((byte)((0x19 < bVar43) * '\x19' | (0x19 >= bVar43) * bVar43) == bVar43);
      auVar63[0xd] = -((byte)((0x19 < bVar44) * '\x19' | (0x19 >= bVar44) * bVar44) == bVar44);
      auVar63[0xe] = -((byte)((0x19 < bVar45) * '\x19' | (0x19 >= bVar45) * bVar45) == bVar45);
      auVar63[0xf] = -((byte)((0x19 < bVar46) * '\x19' | (0x19 >= bVar46) * bVar46) == bVar46);
      auVar38[0] = -((byte)((0x19 < bVar47) * '\x19' | (0x19 >= bVar47) * bVar47) == bVar47);
      auVar38[1] = -((byte)((0x19 < bVar48) * '\x19' | (0x19 >= bVar48) * bVar48) == bVar48);
      auVar38[2] = -((byte)((0x19 < bVar49) * '\x19' | (0x19 >= bVar49) * bVar49) == bVar49);
      auVar38[3] = -((byte)((0x19 < bVar50) * '\x19' | (0x19 >= bVar50) * bVar50) == bVar50);
      auVar38[4] = -((byte)((0x19 < bVar51) * '\x19' | (0x19 >= bVar51) * bVar51) == bVar51);
      auVar38[5] = -((byte)((0x19 < bVar52) * '\x19' | (0x19 >= bVar52) * bVar52) == bVar52);
      auVar38[6] = -((byte)((0x19 < bVar53) * '\x19' | (0x19 >= bVar53) * bVar53) == bVar53);
      auVar38[7] = -((byte)((0x19 < bVar54) * '\x19' | (0x19 >= bVar54) * bVar54) == bVar54);
      auVar38[8] = -((byte)((0x19 < bVar55) * '\x19' | (0x19 >= bVar55) * bVar55) == bVar55);
      auVar38[9] = -((byte)((0x19 < bVar56) * '\x19' | (0x19 >= bVar56) * bVar56) == bVar56);
      auVar38[10] = -((byte)((0x19 < bVar57) * '\x19' | (0x19 >= bVar57) * bVar57) == bVar57);
      auVar38[0xb] = -((byte)((0x19 < bVar58) * '\x19' | (0x19 >= bVar58) * bVar58) == bVar58);
      auVar38[0xc] = -((byte)((0x19 < bVar59) * '\x19' | (0x19 >= bVar59) * bVar59) == bVar59);
      auVar38[0xd] = -((byte)((0x19 < bVar60) * '\x19' | (0x19 >= bVar60) * bVar60) == bVar60);
      auVar38[0xe] = -((byte)((0x19 < bVar61) * '\x19' | (0x19 >= bVar61) * bVar61) == bVar61);
      auVar38[0xf] = -((byte)((0x19 < bVar62) * '\x19' | (0x19 >= bVar62) * bVar62) == bVar62);
      *(undefined1 (*) [16])((longlong)local_150 + (longlong)pcVar14) = auVar63 & auVar6 | auVar2;
      *(undefined1 (*) [16])((longlong)(local_150 + 2) + (longlong)pcVar14) =
           auVar38 & auVar6 | auVar3;
      pcVar14 = pcVar14 + 0x20;
    } while (pcVar13 != pcVar14);
    if (pcVar21 != pcVar13) {
      pcVar14 = pcVar13;
      if (((ulonglong)pcVar21 & 0x18) != 0) {
LAB_14015e3b0:
        pcVar13 = (char *)((ulonglong)pcVar21 & 0x7ffffffffffffff8);
        do {
          uVar12 = *(ulonglong *)(pcVar14 + lVar29);
          bVar30 = (char)uVar12 + 0xbf;
          bVar31 = (char)(uVar12 >> 8) + 0xbf;
          bVar32 = (char)(uVar12 >> 0x10) + 0xbf;
          bVar33 = (char)(uVar12 >> 0x18) + 0xbf;
          bVar34 = (char)(uVar12 >> 0x20) + 0xbf;
          bVar35 = (char)(uVar12 >> 0x28) + 0xbf;
          bVar36 = (char)(uVar12 >> 0x30) + 0xbf;
          bVar37 = (char)(uVar12 >> 0x38) + 0xbf;
          *(ulonglong *)((longlong)local_150 + (longlong)pcVar14) =
               ~CONCAT17(-((byte)((bVar37 < 0x1a) * '\x1a' | (bVar37 >= 0x1a) * bVar37) == bVar37),
                         CONCAT16(-((byte)((bVar36 < 0x1a) * '\x1a' | (bVar36 >= 0x1a) * bVar36) ==
                                   bVar36),CONCAT15(-((byte)((bVar35 < 0x1a) * '\x1a' |
                                                            (bVar35 >= 0x1a) * bVar35) == bVar35),
                                                    CONCAT14(-((byte)((bVar34 < 0x1a) * '\x1a' |
                                                                     (bVar34 >= 0x1a) * bVar34) ==
                                                              bVar34),CONCAT13(-((byte)((bVar33 < 
                                                  0x1a) * '\x1a' | (bVar33 >= 0x1a) * bVar33) ==
                                                  bVar33),CONCAT12(-((byte)((bVar32 < 0x1a) * '\x1a'
                                                                           | (bVar32 >= 0x1a) *
                                                                             bVar32) == bVar32),
                                                                   CONCAT11(-((byte)((bVar31 < 0x1a)
                                                                                     * '\x1a' |
                                                                                    (bVar31 >= 0x1a)
                                                                                    * bVar31) ==
                                                                             bVar31),-((byte)((
                                                  bVar30 < 0x1a) * '\x1a' |
                                                  (bVar30 >= 0x1a) * bVar30) == bVar30)))))))) &
               0x2020202020202020 | uVar12;
          pcVar14 = pcVar14 + 8;
        } while (pcVar13 != pcVar14);
        if (pcVar21 == pcVar13) goto LAB_14015e487;
      }
LAB_14015e40b:
      pcVar14 = pcVar13 + 1;
      if (((ulonglong)pcVar21 & 1) != 0) {
        *(char *)((longlong)local_150 + (longlong)pcVar13) =
             ((byte)(pcVar13[lVar29] + 0xbfU) < 0x1a) << 5 | pcVar13[lVar29];
        pcVar13 = pcVar14;
      }
      while (pcVar21 != pcVar14) {
        *(char *)((longlong)local_150 + (longlong)pcVar13) =
             ((byte)(pcVar13[lVar29] + 0xbfU) < 0x1a) << 5 | pcVar13[lVar29];
        pcVar13[(longlong)local_150 + 1] =
             ((byte)(pcVar13[lVar29 + 1] + 0xbfU) < 0x1a) << 5 | pcVar13[lVar29 + 1];
        pcVar14 = pcVar13 + 2;
        pcVar13 = pcVar14;
      }
    }
LAB_14015e487:
    pcVar28 = pcVar28 + -(longlong)pcVar22;
    pbVar15 = (byte *)::<str>::trim_matches::<<char>::is_whitespace>(pcVar22 + (longlong)pcVar20);
    lVar29 = local_f8;
    switch(pcVar21) {
    case (char *)0x4:
      if ((int)*local_150 == 0x656d616e) {
        local_d0 = pbVar15 + (longlong)pcVar28;
        local_c8 = 0x20;
        local_d8 = pbVar15;
        <alloc::string::String_as_core::iter::traits::collect::FromIterator<char>>
        ::from_iter::
        <core::iter::adapters::take::Take<core::str::iter::Chars>>
                  (local_c0,&local_d8);
        goto LAB_14015ef1f;
      }
      if ((int)*local_150 == 0x65736162) {
        if (-1 < (longlong)pcVar28) {
          if (pcVar28 == (char *)0x0) {
            local_e0 = (char *)0x1;
          }
          else {
            __rustc::__rust_no_alloc_shim_is_unstable_v2
                      (&switchD_14015e4bf::switchdataD_1406be890,local_e0);
            local_e0 = (char *)__rustc::__rust_alloc(pcVar28,1);
            if (local_e0 == (char *)0x0) {
              uVar17 = 1;
              local_e0 = pcVar28;
              goto LAB_14015f315;
            }
            memcpy(local_e0,pbVar15,(size_t)pcVar28);
          }
          auStack_110._0_8_ = pcVar28;
          goto LAB_14015ef1f;
        }
        uVar17 = 0;
LAB_14015f315:
        alloc::raw_vec::handle_error(uVar17,local_e0);
LAB_14015f31f:
                    /* WARNING: Does not return */
        pcVar4 = (code *)invalidInstructionException();
        (*pcVar4)();
      }
      break;
    case (char *)0x6:
      if (*(short *)((longlong)local_150 + 4) != 0x7365 || (int)*local_150 != 0x74696c65) break;
      if (pcVar28 == (char *)0x0) {
        uVar25 = 0;
        bVar5 = true;
        goto LAB_14015eeca;
      }
      if (pcVar28 == (char *)0x1) {
        bVar30 = *pbVar15;
        bVar5 = true;
        uVar25 = 0;
        if ((bVar30 == 0x2b) || (bVar30 == 0x2d)) goto LAB_14015eeca;
      }
      else {
        bVar30 = *pbVar15;
      }
      if (bVar30 == 0x2d) {
        pcVar20 = pcVar28 + -1;
        if (&DAT_00000008 < pcVar28) {
          uVar11 = 0;
          do {
            pbVar15 = pbVar15 + 1;
            iVar26 = (int)((longlong)(int)uVar11 * 10);
            if ((longlong)iVar26 != (longlong)(int)uVar11 * 10) goto code_r0x00014015f0dd;
            uVar25 = *pbVar15 - 0x30;
            if (9 < uVar25) goto LAB_14015ee1c;
            uVar11 = iVar26 - uVar25;
            if (SBORROW4(iVar26,uVar25)) {
              uVar25 = 0;
              bVar5 = true;
              goto LAB_14015eeca;
            }
            pcVar20 = pcVar20 + -1;
          } while (pcVar20 != (char *)0x0);
        }
        else {
          uVar11 = 0;
          if (pcVar20 != (char *)0x0) {
            pcVar20 = (char *)0x1;
            uVar11 = 0;
            do {
              if (9 < pbVar15[(longlong)pcVar20] - 0x30) {
LAB_14015ee1c:
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eeca;
              }
              uVar11 = uVar11 * 10 - (pbVar15[(longlong)pcVar20] - 0x30);
              pcVar20 = pcVar20 + 1;
            } while (pcVar28 != pcVar20);
          }
        }
LAB_14015eec3:
        bVar5 = false;
        uVar25 = uVar11;
      }
      else {
        if (bVar30 == 0x2b) {
          pbVar15 = pbVar15 + 1;
          pcVar28 = pcVar28 + -1;
        }
        if (&DAT_00000007 < pcVar28) {
          uVar11 = 0;
          do {
            iVar26 = (int)((longlong)(int)uVar11 * 10);
            if ((longlong)iVar26 != (longlong)(int)uVar11 * 10) {
code_r0x00014015f0dd:
              uVar25 = 0;
              bVar5 = true;
              goto LAB_14015eeca;
            }
            uVar25 = *pbVar15 - 0x30;
            if (9 < uVar25) goto LAB_14015ee1c;
            uVar11 = iVar26 + uVar25;
            if (SCARRY4(iVar26,uVar25)) {
              uVar25 = 0;
              bVar5 = true;
              goto LAB_14015eeca;
            }
            pbVar15 = pbVar15 + 1;
            pcVar28 = pcVar28 + -1;
          } while (pcVar28 != (char *)0x0);
          goto LAB_14015eec3;
        }
        if (pcVar28 == (char *)0x0) {
          uVar11 = 0;
          goto LAB_14015eec3;
        }
        uVar11 = *pbVar15 - 0x30;
        uVar25 = 0;
        bVar5 = true;
        if (uVar11 < 10) {
          if (pcVar28 != (char *)0x1) {
            if (9 < pbVar15[1] - 0x30) goto LAB_14015eeca;
            uVar11 = (pbVar15[1] - 0x30) + uVar11 * 10;
            if (pcVar28 != (char *)0x2) {
              if (9 < pbVar15[2] - 0x30) goto LAB_14015eeca;
              uVar11 = (pbVar15[2] - 0x30) + uVar11 * 10;
              if (pcVar28 != (char *)0x3) {
                if (9 < pbVar15[3] - 0x30) goto LAB_14015eeca;
                uVar11 = (pbVar15[3] - 0x30) + uVar11 * 10;
                if (pcVar28 != &DAT_00000004) {
                  if (9 < pbVar15[4] - 0x30) goto LAB_14015eeca;
                  uVar11 = (pbVar15[4] - 0x30) + uVar11 * 10;
                  if (pcVar28 != &DAT_00000005) {
                    if (9 < pbVar15[5] - 0x30) goto LAB_14015eeca;
                    uVar11 = (pbVar15[5] - 0x30) + uVar11 * 10;
                    if (pcVar28 != &DAT_00000006) {
                      if (9 < pbVar15[6] - 0x30) goto LAB_14015eeca;
                      uVar11 = (pbVar15[6] - 0x30) + uVar11 * 10;
                    }
                  }
                }
              }
            }
          }
          goto LAB_14015eec3;
        }
      }
LAB_14015eeca:
      uStack_ed = uVar25 != 0;
      if (bVar5) {
        uStack_ed = 2;
      }
      goto LAB_14015ef11;
    case (char *)0xb:
      if (*(longlong *)((longlong)local_150 + 3) == 0x736574696c655f6e &&
          *local_150 == 0x696c655f6e657665) {
        if (pcVar28 == (char *)0x0) {
          uVar25 = 0;
          bVar5 = true;
          goto LAB_14015eea1;
        }
        if (pcVar28 == (char *)0x1) {
          bVar30 = *pbVar15;
          bVar5 = true;
          uVar25 = 0;
          if ((bVar30 == 0x2b) || (bVar30 == 0x2d)) goto LAB_14015eea1;
        }
        else {
          bVar30 = *pbVar15;
        }
        if (bVar30 == 0x2d) {
          pcVar20 = pcVar28 + -1;
          if (&DAT_00000008 < pcVar28) {
            uVar11 = 0;
            do {
              pbVar15 = pbVar15 + 1;
              iVar26 = (int)((longlong)(int)uVar11 * 10);
              if ((longlong)iVar26 != (longlong)(int)uVar11 * 10) goto code_r0x00014015f0bd;
              uVar25 = *pbVar15 - 0x30;
              if (9 < uVar25) goto LAB_14015ede3;
              uVar11 = iVar26 - uVar25;
              if (SBORROW4(iVar26,uVar25)) {
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eea1;
              }
              pcVar20 = pcVar20 + -1;
            } while (pcVar20 != (char *)0x0);
          }
          else {
            uVar11 = 0;
            if (pcVar20 != (char *)0x0) {
              pcVar20 = (char *)0x1;
              uVar11 = 0;
              do {
                if (9 < pbVar15[(longlong)pcVar20] - 0x30) {
LAB_14015ede3:
                  uVar25 = 0;
                  bVar5 = true;
                  goto LAB_14015eea1;
                }
                uVar11 = uVar11 * 10 - (pbVar15[(longlong)pcVar20] - 0x30);
                pcVar20 = pcVar20 + 1;
              } while (pcVar28 != pcVar20);
            }
          }
LAB_14015ee9a:
          bVar5 = false;
          uVar25 = uVar11;
        }
        else {
          if (bVar30 == 0x2b) {
            pbVar15 = pbVar15 + 1;
            pcVar28 = pcVar28 + -1;
          }
          if (&DAT_00000007 < pcVar28) {
            uVar11 = 0;
            do {
              iVar26 = (int)((longlong)(int)uVar11 * 10);
              if ((longlong)iVar26 != (longlong)(int)uVar11 * 10) {
code_r0x00014015f0bd:
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eea1;
              }
              uVar25 = *pbVar15 - 0x30;
              if (9 < uVar25) goto LAB_14015ede3;
              uVar11 = iVar26 + uVar25;
              if (SCARRY4(iVar26,uVar25)) {
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eea1;
              }
              pbVar15 = pbVar15 + 1;
              pcVar28 = pcVar28 + -1;
            } while (pcVar28 != (char *)0x0);
            goto LAB_14015ee9a;
          }
          if (pcVar28 == (char *)0x0) {
            uVar11 = 0;
            goto LAB_14015ee9a;
          }
          uVar11 = *pbVar15 - 0x30;
          uVar25 = 0;
          bVar5 = true;
          if (uVar11 < 10) {
            if (pcVar28 != (char *)0x1) {
              if (9 < pbVar15[1] - 0x30) goto LAB_14015eea1;
              uVar11 = (pbVar15[1] - 0x30) + uVar11 * 10;
              if (pcVar28 != (char *)0x2) {
                if (9 < pbVar15[2] - 0x30) goto LAB_14015eea1;
                uVar11 = (pbVar15[2] - 0x30) + uVar11 * 10;
                if (pcVar28 != (char *)0x3) {
                  if (9 < pbVar15[3] - 0x30) goto LAB_14015eea1;
                  uVar11 = (pbVar15[3] - 0x30) + uVar11 * 10;
                  if (pcVar28 != &DAT_00000004) {
                    if (9 < pbVar15[4] - 0x30) goto LAB_14015eea1;
                    uVar11 = (pbVar15[4] - 0x30) + uVar11 * 10;
                    if (pcVar28 != &DAT_00000005) {
                      if (9 < pbVar15[5] - 0x30) goto LAB_14015eea1;
                      uVar11 = (pbVar15[5] - 0x30) + uVar11 * 10;
                      if (pcVar28 != &DAT_00000006) {
                        if (9 < pbVar15[6] - 0x30) goto LAB_14015eea1;
                        uVar11 = (pbVar15[6] - 0x30) + uVar11 * 10;
                      }
                    }
                  }
                }
              }
            }
            goto LAB_14015ee9a;
          }
        }
LAB_14015eea1:
        uVar8 = uVar25 != 0;
        if (bVar5) {
          uVar8 = 2;
        }
        uStack_ec = CONCAT31(uStack_ec._1_3_,uVar8);
        goto LAB_14015ef11;
      }
      break;
    case (char *)0xc:
      uVar12 = *local_150 ^ 0x65705f6472696874;
      if ((int)local_150[1] == 0x6e6f7372 && uVar12 == 0) {
        if (pcVar28 == (char *)0x0) goto LAB_14015eb1b;
        if (pcVar28 == (char *)0x1) {
          bVar30 = *pbVar15;
          uStack_f0 = false;
          if ((bVar30 != 0x2b) && (uStack_f0 = false, bVar30 != 0x2d)) goto LAB_14015eae7;
        }
        else {
          bVar30 = *pbVar15;
LAB_14015eae7:
          pcVar20 = (char *)(ulonglong)(bVar30 == 0x2b);
          pbVar15 = pbVar15 + (longlong)pcVar20;
          if ((ulonglong)((longlong)pcVar28 - (longlong)pcVar20) < 3) {
            if (pcVar28 == pcVar20) {
              uVar12 = 0;
            }
            else {
              uVar25 = *pbVar15 - 0x30;
              uVar12 = (ulonglong)uVar25;
              if (9 < uVar25) {
LAB_14015eb1b:
                uStack_f0 = false;
                goto LAB_14015ee87;
              }
              if ((longlong)pcVar28 - (longlong)pcVar20 != 1) {
                if (9 < pbVar15[1] - 0x30) goto LAB_14015eb1b;
                uVar12 = (ulonglong)(byte)((char)(pbVar15[1] - 0x30) + (char)uVar25 * '\n');
              }
            }
LAB_14015ee81:
            uStack_f0 = (byte)uVar12 < 4;
          }
          else {
            lVar29 = 0;
            uVar12 = 0;
            while( true ) {
              uVar10 = (ushort)(byte)uVar12 * 10;
              uVar12 = (ulonglong)uVar10;
              if ((char)(uVar10 >> 8) != '\0') goto LAB_14015eb1b;
              bVar30 = (byte)(pbVar15[lVar29] - 0x30);
              uVar12 = (ulonglong)(byte)((byte)uVar10 + bVar30);
              uStack_f0 = false;
              if ((9 < pbVar15[lVar29] - 0x30) || (CARRY1((byte)uVar10,bVar30))) break;
              lVar29 = lVar29 + 1;
              if (lVar29 == (longlong)pcVar28 - (longlong)pcVar20) goto LAB_14015ee81;
            }
          }
        }
LAB_14015ee87:
        uStack_ef = (undefined1)uVar12;
        goto LAB_14015ef11;
      }
      break;
    case (char *)0xd:
      if (*(longlong *)((longlong)local_150 + 5) == 0x73726f6c6f635f72 &&
          *local_150 == 0x635f726579616c70) {
        if (pcVar28 == (char *)0x0) {
          uVar25 = 0;
          bVar5 = true;
          goto LAB_14015eef3;
        }
        if (pcVar28 == (char *)0x1) {
          bVar30 = *pbVar15;
          bVar5 = true;
          uVar25 = 0;
          if ((bVar30 == 0x2b) || (bVar30 == 0x2d)) goto LAB_14015eef3;
        }
        else {
          bVar30 = *pbVar15;
        }
        if (bVar30 == 0x2d) {
          pcVar20 = pcVar28 + -1;
          if (&DAT_00000008 < pcVar28) {
            uVar11 = 0;
            do {
              pbVar15 = pbVar15 + 1;
              iVar26 = (int)((longlong)(int)uVar11 * 10);
              if ((longlong)iVar26 != (longlong)(int)uVar11 * 10) goto code_r0x00014015f0fd;
              uVar25 = *pbVar15 - 0x30;
              if (9 < uVar25) goto LAB_14015ee55;
              uVar11 = iVar26 - uVar25;
              if (SBORROW4(iVar26,uVar25)) {
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eef3;
              }
              pcVar20 = pcVar20 + -1;
            } while (pcVar20 != (char *)0x0);
          }
          else {
            uVar11 = 0;
            if (pcVar20 != (char *)0x0) {
              pcVar20 = (char *)0x1;
              uVar11 = 0;
              do {
                if (9 < pbVar15[(longlong)pcVar20] - 0x30) {
LAB_14015ee55:
                  uVar25 = 0;
                  bVar5 = true;
                  goto LAB_14015eef3;
                }
                uVar11 = uVar11 * 10 - (pbVar15[(longlong)pcVar20] - 0x30);
                pcVar20 = pcVar20 + 1;
              } while (pcVar28 != pcVar20);
            }
          }
LAB_14015eeec:
          bVar5 = false;
          uVar25 = uVar11;
        }
        else {
          if (bVar30 == 0x2b) {
            pbVar15 = pbVar15 + 1;
            pcVar28 = pcVar28 + -1;
          }
          if (&DAT_00000007 < pcVar28) {
            uVar11 = 0;
            do {
              iVar26 = (int)((longlong)(int)uVar11 * 10);
              if ((longlong)iVar26 != (longlong)(int)uVar11 * 10) {
code_r0x00014015f0fd:
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eef3;
              }
              uVar25 = *pbVar15 - 0x30;
              if (9 < uVar25) goto LAB_14015ee55;
              uVar11 = iVar26 + uVar25;
              if (SCARRY4(iVar26,uVar25)) {
                uVar25 = 0;
                bVar5 = true;
                goto LAB_14015eef3;
              }
              pbVar15 = pbVar15 + 1;
              pcVar28 = pcVar28 + -1;
            } while (pcVar28 != (char *)0x0);
            goto LAB_14015eeec;
          }
          if (pcVar28 == (char *)0x0) {
            uVar11 = 0;
            goto LAB_14015eeec;
          }
          uVar11 = *pbVar15 - 0x30;
          uVar25 = 0;
          bVar5 = true;
          if (uVar11 < 10) {
            if (pcVar28 != (char *)0x1) {
              if (9 < pbVar15[1] - 0x30) goto LAB_14015eef3;
              uVar11 = (pbVar15[1] - 0x30) + uVar11 * 10;
              if (pcVar28 != (char *)0x2) {
                if (9 < pbVar15[2] - 0x30) goto LAB_14015eef3;
                uVar11 = (pbVar15[2] - 0x30) + uVar11 * 10;
                if (pcVar28 != (char *)0x3) {
                  if (9 < pbVar15[3] - 0x30) goto LAB_14015eef3;
                  uVar11 = (pbVar15[3] - 0x30) + uVar11 * 10;
                  if (pcVar28 != &DAT_00000004) {
                    if (9 < pbVar15[4] - 0x30) goto LAB_14015eef3;
                    uVar11 = (pbVar15[4] - 0x30) + uVar11 * 10;
                    if (pcVar28 != &DAT_00000005) {
                      if (9 < pbVar15[5] - 0x30) goto LAB_14015eef3;
                      uVar11 = (pbVar15[5] - 0x30) + uVar11 * 10;
                      if (pcVar28 != &DAT_00000006) {
                        if (9 < pbVar15[6] - 0x30) goto LAB_14015eef3;
                        uVar11 = (pbVar15[6] - 0x30) + uVar11 * 10;
                      }
                    }
                  }
                }
              }
            }
            goto LAB_14015eeec;
          }
        }
LAB_14015eef3:
        uStack_ee = uVar25 != 0;
        if (bVar5) {
          uStack_ee = 2;
        }
        goto LAB_14015ef11;
      }
    }
    if (pcVar28 == (char *)0x0) goto LAB_14015ef11;
    if (pcVar28 == (char *)0x1) {
      bVar30 = *pbVar15;
      if ((bVar30 == 0x2b) || (bVar30 == 0x2d)) goto LAB_14015ef11;
    }
    else {
      bVar30 = *pbVar15;
    }
    if (bVar30 == 0x2d) {
      pcVar20 = pcVar28 + -1;
      if (&DAT_00000008 < pcVar28) {
        uVar25 = 0;
        do {
          pbVar15 = pbVar15 + 1;
          iVar26 = (int)((longlong)(int)uVar25 * 10);
          if ((((longlong)iVar26 != (longlong)(int)uVar25 * 10) ||
              (uVar11 = *pbVar15 - 0x30, 9 < uVar11)) ||
             (uVar25 = iVar26 - uVar11, SBORROW4(iVar26,uVar11))) goto LAB_14015ef11;
          pcVar20 = pcVar20 + -1;
        } while (pcVar20 != (char *)0x0);
      }
      else {
        uVar25 = 0;
        if (pcVar20 != (char *)0x0) {
          pcVar20 = (char *)0x1;
          uVar25 = 0;
          do {
            if (9 < pbVar15[(longlong)pcVar20] - 0x30) goto LAB_14015ef11;
            uVar25 = uVar25 * 10 - (pbVar15[(longlong)pcVar20] - 0x30);
            pcVar20 = pcVar20 + 1;
          } while (pcVar28 != pcVar20);
        }
      }
LAB_14015e8e5:
      if (local_f8 == auStack_110._8_8_) {
        <alloc::raw_vec::RawVec<(alloc::string::String,i32)>>::
        grow_one(auStack_110 + 8);
      }
      lVar18 = lVar29 * 0x20;
      *(char **)(lStack_100 + lVar18) = pcVar21;
      *(ulonglong **)(lStack_100 + 8 + lVar18) = local_150;
      *(char **)(lStack_100 + 0x10 + lVar18) = pcVar21;
      *(uint *)(lStack_100 + 0x18 + lVar18) = uVar25;
      local_f8 = lVar29 + 1;
    }
    else {
      if (bVar30 == 0x2b) {
        pbVar15 = pbVar15 + 1;
        pcVar28 = pcVar28 + -1;
      }
      if (&DAT_00000007 < pcVar28) {
        uVar25 = 0;
        do {
          iVar26 = (int)((longlong)(int)uVar25 * 10);
          if ((((longlong)iVar26 != (longlong)(int)uVar25 * 10) ||
              (uVar11 = *pbVar15 - 0x30, 9 < uVar11)) ||
             (uVar25 = iVar26 + uVar11, SCARRY4(iVar26,uVar11))) goto LAB_14015ef11;
          pbVar15 = pbVar15 + 1;
          pcVar28 = pcVar28 + -1;
        } while (pcVar28 != (char *)0x0);
        goto LAB_14015e8e5;
      }
      if (pcVar28 == (char *)0x0) {
        uVar25 = 0;
        goto LAB_14015e8e5;
      }
      uVar25 = *pbVar15 - 0x30;
      if (uVar25 < 10) {
        if (pcVar28 != (char *)0x1) {
          if (9 < pbVar15[1] - 0x30) goto LAB_14015ef11;
          uVar25 = (pbVar15[1] - 0x30) + uVar25 * 10;
          if (pcVar28 != (char *)0x2) {
            if (9 < pbVar15[2] - 0x30) goto LAB_14015ef11;
            uVar25 = (pbVar15[2] - 0x30) + uVar25 * 10;
            if (pcVar28 != (char *)0x3) {
              if (9 < pbVar15[3] - 0x30) goto LAB_14015ef11;
              uVar25 = (pbVar15[3] - 0x30) + uVar25 * 10;
              if (pcVar28 != &DAT_00000004) {
                if (9 < pbVar15[4] - 0x30) goto LAB_14015ef11;
                uVar25 = (pbVar15[4] - 0x30) + uVar25 * 10;
                if (pcVar28 != &DAT_00000005) {
                  if (9 < pbVar15[5] - 0x30) goto LAB_14015ef11;
                  uVar25 = (pbVar15[5] - 0x30) + uVar25 * 10;
                  if (pcVar28 != &DAT_00000006) {
                    if (9 < pbVar15[6] - 0x30) goto LAB_14015ef11;
                    uVar25 = (pbVar15[6] - 0x30) + uVar25 * 10;
                  }
                }
              }
            }
          }
        }
        goto LAB_14015e8e5;
      }
LAB_14015ef11:
      if (pcVar21 != (char *)0x0) {
LAB_14015ef1f:
        puVar16 = (undefined8 *)__rustc::__rust_dealloc(local_150,pcVar21,1);
        return puVar16;
      }
    }
    if (bVar7) goto LAB_14015f175;
  } while( true );
}


