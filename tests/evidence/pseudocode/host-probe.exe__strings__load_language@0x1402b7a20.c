
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::strings::load_language */

undefined8 * __rustcall
host_probe::strings::load_language
          (undefined8 *param_1,longlong param_2,int *param_3)

{
  char *pcVar1;
  int iVar2;
  code *pcVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  bool bVar9;
  bool bVar10;
  longlong lVar11;
  int *piVar12;
  bool bVar13;
  undefined8 *puVar14;
  ulonglong uVar15;
  int *piVar16;
  int *piVar17;
  undefined8 uVar18;
  uint uVar19;
  uint uVar20;
  longlong lVar21;
  int *_Size;
  int *piVar22;
  ulonglong uVar23;
  ulonglong uVar24;
  longlong lVar25;
  int *piVar26;
  ushort uVar27;
  byte bVar28;
  undefined1 auVar29 [16];
  undefined1 auVar30 [16];
  undefined1 auVar31 [16];
  undefined1 unaff_XMM6_Ba;
  undefined1 unaff_XMM6_Bb;
  undefined1 unaff_XMM6_Bc;
  undefined1 unaff_XMM6_Bd;
  undefined1 unaff_XMM6_Be;
  undefined1 unaff_XMM6_Bf;
  undefined1 unaff_XMM6_Bg;
  undefined1 unaff_XMM6_Bh;
  undefined1 unaff_XMM6_Bi;
  undefined1 unaff_XMM6_Bj;
  undefined1 unaff_XMM6_Bk;
  undefined1 unaff_XMM6_Bl;
  undefined1 unaff_XMM6_Bm;
  undefined1 unaff_XMM6_Bn;
  undefined1 unaff_XMM6_Bo;
  undefined1 unaff_XMM6_Bp;
  longlong local_1d0;
  ulonglong local_1b8;
  undefined1 (*local_1a8) [16];
  code *pcStack_1a0;
  longlong local_198;
  longlong lStack_190;
  undefined8 local_188;
  longlong lStack_180;
  int *local_178;
  longlong local_170;
  undefined1 local_168 [16];
  undefined **local_150;
  int *local_148;
  longlong local_140;
  int *local_138;
  ulonglong local_130;
  longlong local_128;
  int *local_120;
  int *local_118;
  ulonglong local_110;
  ulonglong local_108;
  int *local_100;
  ulonglong local_f8;
  undefined1 (*local_f0) [16];
  code *pcStack_e8;
  undefined1 *local_e0;
  code *local_d8;
  undefined8 uStack_d0;
  longlong lStack_c8;
  undefined8 local_88;
  longlong lStack_80;
  undefined1 (*local_78) [16];
  code *local_68;
  longlong lStack_60;
  
  lVar25 = param_2;
  std::fs::read::inner((undefined1 (*) [16])&local_140);
  piVar12 = local_138;
  lVar11 = local_140;
  if (local_140 == -1) {
    local_168._0_8_ = local_138;
    local_f0 = (undefined1 (*) [16])&local_128;
    pcStack_e8 = <std::path::Display_as_core::fmt::Display>::fmt;
    local_e0 = local_168;
    local_d8 = <core::io::error::Error_as_core::fmt::Display>::fmt
    ;
    local_128 = param_2;
    local_120 = param_3;
    alloc::fmt::format::format_inner(&local_1a8,&DAT_1406f374c,&local_f0);
    if ((1 < (local_168._0_4_ & 3) - 2) && ((local_168._0_8_ & 3) != 0)) {
      pcStack_e8 = (code *)(local_168._0_8_ - 1);
      local_f0 = (undefined1 (*) [16])CONCAT71(local_f0._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&pcStack_e8);
    }
    *param_1 = local_1a8;
    param_1[1] = pcStack_1a0;
    param_1[2] = local_198;
    return param_1;
  }
  if ((((3 < local_130) && (0x43 < local_130 && *local_138 == -0x6f319486)) &&
      (0x7b < local_130 && local_138[0x10] == -0x6f319486)) && (0x7f < local_130)) {
    uVar19 = local_138[0x1e];
    uVar23 = (ulonglong)uVar19 + 0x80;
    if (uVar23 <= local_130) {
      uVar20 = local_138[0x1f];
      local_188 = <std::thread::local::LocalKey<core::cell::Cell<(u64,u64)>>>
                  ::with::
                  <<std::hash::random::RandomState>::new::_closure_0_,std::hash::random::RandomState>
                            (&anon_abae67b6203ffecafa6256cb9d53fac3_194_llvm_5426317090953290063);
      local_198 = 0;
      lStack_190 = 0;
      local_1a8 = (undefined1 (*) [16])
                  &anon_abae67b6203ffecafa6256cb9d53fac3_195_llvm_5426317090953290063;
      pcStack_1a0 = (code *)0x0;
      lStack_180 = lVar25;
      if ((uVar19 & 0xfffffffc) != 0) {
        local_1b8 = uVar20 + uVar23;
        if (local_130 < uVar20 + uVar23) {
          local_1b8 = local_130;
        }
        local_1b8 = local_1b8 - uVar23;
        iVar5 = -(uint)(CONCAT13(unaff_XMM6_Bd,
                                 CONCAT12(unaff_XMM6_Bc,CONCAT11(unaff_XMM6_Bb,unaff_XMM6_Ba))) ==
                       CONCAT13(unaff_XMM6_Bd,
                                CONCAT12(unaff_XMM6_Bc,CONCAT11(unaff_XMM6_Bb,unaff_XMM6_Ba))));
        iVar6 = -(uint)(CONCAT13(unaff_XMM6_Bh,
                                 CONCAT12(unaff_XMM6_Bg,CONCAT11(unaff_XMM6_Bf,unaff_XMM6_Be))) ==
                       CONCAT13(unaff_XMM6_Bh,
                                CONCAT12(unaff_XMM6_Bg,CONCAT11(unaff_XMM6_Bf,unaff_XMM6_Be))));
        iVar7 = -(uint)(CONCAT13(unaff_XMM6_Bl,
                                 CONCAT12(unaff_XMM6_Bk,CONCAT11(unaff_XMM6_Bj,unaff_XMM6_Bi))) ==
                       CONCAT13(unaff_XMM6_Bl,
                                CONCAT12(unaff_XMM6_Bk,CONCAT11(unaff_XMM6_Bj,unaff_XMM6_Bi))));
        iVar8 = -(uint)(CONCAT13(unaff_XMM6_Bp,
                                 CONCAT12(unaff_XMM6_Bo,CONCAT11(unaff_XMM6_Bn,unaff_XMM6_Bm))) ==
                       CONCAT13(unaff_XMM6_Bp,
                                CONCAT12(unaff_XMM6_Bo,CONCAT11(unaff_XMM6_Bn,unaff_XMM6_Bm))));
        local_150 = &
                    PTR_anon_abae67b6203ffecafa6256cb9d53fac3_100_llvm_5426317090953290063_1406f4fc0
        ;
        local_1d0 = 0;
        bVar13 = false;
        uVar24 = (ulonglong)uVar19 & 0xfffffffc;
        piVar26 = piVar12 + 0x20;
        lVar25 = (longlong)piVar12 + uVar23;
LAB_1402b802e:
        if (!bVar13) {
          local_170 = lVar25;
          if (local_1b8 == 0) {
            bVar13 = true;
            local_1b8 = 0;
            uVar23 = 0;
          }
          else {
            uVar23 = 0;
            do {
              if (*(char *)(lVar25 + uVar23) == '\0') {
                local_1b8 = local_1b8 + ~uVar23;
                bVar13 = false;
                local_170 = uVar23 + 1 + lVar25;
                goto LAB_1402b80ac;
              }
              uVar23 = uVar23 + 1;
            } while (local_1b8 != uVar23);
            bVar13 = true;
            uVar23 = local_1b8;
          }
LAB_1402b80ac:
          local_f8 = uVar24 - 4;
          iVar2 = *piVar26;
          local_100 = piVar26 + 1;
          local_f0 = (undefined1 (*) [16])CONCAT44(local_f0._4_4_,iVar2);
          local_110 = <std::hash::random::RandomState_as_core::hash::BuildHasher>
                      ::hash_one::<&i32>(&local_188,&local_f0);
          local_108 = local_110 >> 0x39;
          bVar28 = (byte)(local_110 >> 0x39);
          auVar29 = ZEXT216(CONCAT11(bVar28,bVar28));
          auVar29 = pshuflw(auVar29,auVar29,0);
          lVar21 = 0;
          uVar24 = local_110;
          while( true ) {
            uVar24 = uVar24 & (ulonglong)pcStack_1a0;
            pcVar1 = *local_1a8 + uVar24;
            auVar31[0] = -(*pcVar1 == auVar29[0]);
            auVar31[1] = -(pcVar1[1] == auVar29[1]);
            auVar31[2] = -(pcVar1[2] == auVar29[2]);
            auVar31[3] = -(pcVar1[3] == auVar29[3]);
            auVar31[4] = -(pcVar1[4] == auVar29[4]);
            auVar31[5] = -(pcVar1[5] == auVar29[5]);
            auVar31[6] = -(pcVar1[6] == auVar29[6]);
            auVar31[7] = -(pcVar1[7] == auVar29[7]);
            auVar31[8] = -(pcVar1[8] == auVar29[0]);
            auVar31[9] = -(pcVar1[9] == auVar29[1]);
            auVar31[10] = -(pcVar1[10] == auVar29[2]);
            auVar31[0xb] = -(pcVar1[0xb] == auVar29[3]);
            auVar31[0xc] = -(pcVar1[0xc] == auVar29[4]);
            auVar31[0xd] = -(pcVar1[0xd] == auVar29[5]);
            auVar31[0xe] = -(pcVar1[0xe] == auVar29[6]);
            auVar31[0xf] = -(pcVar1[0xf] == auVar29[7]);
            uVar27 = (ushort)(SUB161(auVar31 >> 7,0) & 1) |
                     (ushort)(SUB161(auVar31 >> 0xf,0) & 1) << 1 |
                     (ushort)(SUB161(auVar31 >> 0x17,0) & 1) << 2 |
                     (ushort)(SUB161(auVar31 >> 0x1f,0) & 1) << 3 |
                     (ushort)(SUB161(auVar31 >> 0x27,0) & 1) << 4 |
                     (ushort)(SUB161(auVar31 >> 0x2f,0) & 1) << 5 |
                     (ushort)(SUB161(auVar31 >> 0x37,0) & 1) << 6 |
                     (ushort)(SUB161(auVar31 >> 0x3f,0) & 1) << 7 |
                     (ushort)(SUB161(auVar31 >> 0x47,0) & 1) << 8 |
                     (ushort)(SUB161(auVar31 >> 0x4f,0) & 1) << 9 |
                     (ushort)(SUB161(auVar31 >> 0x57,0) & 1) << 10 |
                     (ushort)(SUB161(auVar31 >> 0x5f,0) & 1) << 0xb |
                     (ushort)(SUB161(auVar31 >> 0x67,0) & 1) << 0xc |
                     (ushort)(SUB161(auVar31 >> 0x6f,0) & 1) << 0xd |
                     (ushort)(SUB161(auVar31 >> 0x77,0) & 1) << 0xe |
                     (ushort)(auVar31[0xf] >> 7) << 0xf;
            uVar19 = (uint)uVar27;
            while (uVar27 != 0) {
              uVar20 = 0;
              for (uVar4 = uVar19; (uVar4 & 1) == 0; uVar4 = uVar4 >> 1 | 0x80000000) {
                uVar20 = uVar20 + 1;
              }
              if (*(int *)local_1a8[(uVar20 + uVar24 & (ulonglong)pcStack_1a0) * -2 + -2] == iVar2)
              goto LAB_1402b7fff;
              uVar27 = (ushort)(uVar19 - 1) & (ushort)uVar19;
              uVar19 = CONCAT22((short)(uVar19 - 1 >> 0x10),uVar27);
            }
            auVar30[0] = -(*pcVar1 == (char)iVar5);
            auVar30[1] = -(pcVar1[1] == (char)((uint)iVar5 >> 8));
            auVar30[2] = -(pcVar1[2] == (char)((uint)iVar5 >> 0x10));
            auVar30[3] = -(pcVar1[3] == (char)((uint)iVar5 >> 0x18));
            auVar30[4] = -(pcVar1[4] == (char)iVar6);
            auVar30[5] = -(pcVar1[5] == (char)((uint)iVar6 >> 8));
            auVar30[6] = -(pcVar1[6] == (char)((uint)iVar6 >> 0x10));
            auVar30[7] = -(pcVar1[7] == (char)((uint)iVar6 >> 0x18));
            auVar30[8] = -(pcVar1[8] == (char)iVar7);
            auVar30[9] = -(pcVar1[9] == (char)((uint)iVar7 >> 8));
            auVar30[10] = -(pcVar1[10] == (char)((uint)iVar7 >> 0x10));
            auVar30[0xb] = -(pcVar1[0xb] == (char)((uint)iVar7 >> 0x18));
            auVar30[0xc] = -(pcVar1[0xc] == (char)iVar8);
            auVar30[0xd] = -(pcVar1[0xd] == (char)((uint)iVar8 >> 8));
            auVar30[0xe] = -(pcVar1[0xe] == (char)((uint)iVar8 >> 0x10));
            auVar30[0xf] = -(pcVar1[0xf] == (char)((uint)iVar8 >> 0x18));
            if ((((((((((((((((SUB161(auVar30 >> 7,0) & 1) != 0 ||
                             (SUB161(auVar30 >> 0xf,0) & 1) != 0) ||
                            (SUB161(auVar30 >> 0x17,0) & 1) != 0) ||
                           (SUB161(auVar30 >> 0x1f,0) & 1) != 0) ||
                          (SUB161(auVar30 >> 0x27,0) & 1) != 0) ||
                         (SUB161(auVar30 >> 0x2f,0) & 1) != 0) ||
                        (SUB161(auVar30 >> 0x37,0) & 1) != 0) ||
                       (SUB161(auVar30 >> 0x3f,0) & 1) != 0) || (SUB161(auVar30 >> 0x47,0) & 1) != 0
                      ) || (SUB161(auVar30 >> 0x4f,0) & 1) != 0) ||
                    (SUB161(auVar30 >> 0x57,0) & 1) != 0) || (SUB161(auVar30 >> 0x5f,0) & 1) != 0)
                  || (SUB161(auVar30 >> 0x67,0) & 1) != 0) || (SUB161(auVar30 >> 0x6f,0) & 1) != 0)
                || (SUB161(auVar30 >> 0x77,0) & 1) != 0) || auVar30[0xf] < '\0') break;
            uVar24 = uVar24 + lVar21 + 0x10;
            lVar21 = lVar21 + 0x10;
          }
          if (local_198 == 0) {
            <hashbrown::raw::RawTable<(u32,alloc::vec::Vec<[u8;12usize]>)>>
            ::reserve_rehash::
            <hashbrown::map::make_hasher<u32,alloc::vec::Vec<[u8_12usize]>,std::hash::random::RandomState>::_closure_0_>
                      (&local_1a8,1,&local_188);
          }
          ::<alloc::string::String>::from_utf8_lossy(&local_128,lVar25,uVar23);
          _Size = local_118;
          piVar26 = local_120;
          <core::str::pattern::StrSearcher>::new
                    (&local_f0,local_120,local_118,&DAT_1406f4fb8,3);
          <core::str::pattern::StrSearcher_as_core::str::pattern::Searcher>
          ::next_match(&local_140,&local_f0);
          bVar28 = (byte)local_140;
          local_178 = local_138;
          uStack_d0 = CONCAT44(uStack_d0._4_4_,0x80a4ee);
          piVar16 = (int *)0x0;
          do {
            while( true ) {
              piVar17 = piVar16;
              uVar23 = (longlong)piVar26 + (longlong)piVar17;
              piVar22 = piVar26;
              if ((ulonglong)((longlong)_Size - (longlong)piVar17) < 0x10) {
                if (_Size == piVar17) goto LAB_1402b7e9d;
                uVar24 = 0;
                while (!SBORROW1('\0',*(char *)(uVar23 + uVar24))) {
                  uVar24 = uVar24 + 1;
                  if ((longlong)_Size - (longlong)piVar17 == uVar24) goto LAB_1402b7e9d;
                }
              }
              else {
                uVar15 = core::slice::memchr::memchr_aligned(0x80);
                uVar24 = uVar23;
                if ((uVar15 & 1) == 0) goto LAB_1402b7e9d;
              }
              piVar16 = (int *)((longlong)piVar17 + uVar24 + 1);
              if (piVar16 < (int *)0x3 || _Size < piVar16) break;
              if (*(char *)((longlong)piVar26 + -1 + (longlong)piVar16) == uStack_d0._2_1_ &&
                  *(short *)((longlong)piVar26 + -3 + (longlong)piVar16) == (short)uStack_d0) {
                if ((bVar28 & local_178 < (int *)((longlong)piVar17 + (uVar24 - 2))) == 1) {
                  if ((local_178 != (int *)0x0) &&
                     (*(char *)((longlong)piVar26 + (longlong)local_178) < -0x40)) {
                    piVar16 = (int *)0x0;
                    goto LAB_1402b8313;
                  }
                  if (local_178 == (int *)0x0) goto LAB_1402b8134;
                  piVar17 = (int *)0x0;
                  goto LAB_1402b7ef4;
                }
                goto LAB_1402b7e9d;
              }
            }
          } while (piVar16 <= _Size);
          goto LAB_1402b7e9d;
        }
        goto LAB_1402b828e;
      }
      local_168 = (undefined1  [16])0x0;
      local_1d0 = 0;
      goto LAB_1402b81e8;
    }
  }
LAB_1402b7ac3:
  pcStack_1a0 = <std::path::Display_as_core::fmt::Display>::fmt;
  local_1a8 = (undefined1 (*) [16])&local_140;
  local_140 = param_2;
  local_138 = param_3;
  alloc::fmt::format::format_inner(&local_f0,&DAT_1406f375f,&local_1a8);
  *param_1 = local_f0;
  param_1[1] = pcStack_e8;
  param_1[2] = local_e0;
  local_78 = local_f0;
joined_r0x0001402b8283:
  if (lVar11 == 0) {
    return param_1;
  }
  puVar14 = (undefined8 *)__rustc::__rust_dealloc(piVar12,lVar11,1);
  return puVar14;
LAB_1402b828e:
  local_168._8_8_ = local_198;
  local_168._0_8_ = pcStack_1a0;
  local_88 = local_188;
  lStack_80 = lStack_180;
  if (local_1a8 != (undefined1 (*) [16])0x0) {
LAB_1402b81e8:
    local_68 = (code *)local_168._0_8_;
    lStack_60 = local_168._8_8_;
    uStack_d0 = local_188;
    lStack_c8 = lStack_180;
    pcStack_e8 = (code *)local_168._0_8_;
    local_e0 = (undefined1 *)local_168._8_8_;
    local_f0 = local_1a8;
    local_d8 = (code *)local_1d0;
    local_88 = local_188;
    lStack_80 = lStack_180;
    if (DAT_14069dbe0 != 0) {
      <std::sync::once_lock::OnceLock<std::collections::hash::map::HashMap<u32,alloc::string::String>>>
      ::initialize::
      <<std::sync::once_lock::OnceLock<std::collections::hash::map::HashMap<u32,alloc::string::String>>>::get_or_init<<std::sync::once_lock::OnceLock<std::collections::hash::map::HashMap<u32,alloc::string::String>>>::try_insert::_closure_0_>::_closure_0_,_>
                (&LANGUAGE,&local_f0);
    }
    if (local_f0 != (undefined1 (*) [16])0x0) {
      lStack_180 = lStack_c8;
      lStack_190 = (longlong)local_d8;
      local_188 = uStack_d0;
      pcStack_1a0 = pcStack_e8;
      local_198 = (longlong)local_e0;
      local_1a8 = local_f0;
      <hashbrown::raw::RawTable<(u32,alloc::string::String)>as_core::ops::drop::Drop>
      ::drop(&local_1a8);
    }
    param_1[1] = local_1d0;
    *param_1 = 0xffffffffffffffff;
    goto joined_r0x0001402b8283;
  }
  goto LAB_1402b7ac3;
  while (piVar17 = (int *)((longlong)piVar17 + 1), local_178 != piVar17) {
LAB_1402b7ef4:
    bVar28 = *(byte *)((longlong)piVar26 + (longlong)piVar17);
    bVar10 = (byte)(bVar28 - 0x3a) < 0xf6;
    if (0x40 < bVar28) {
      bVar10 = 0x5a < bVar28;
    }
    bVar9 = 0x7a < bVar28;
    if (bVar28 < 0x61) {
      bVar9 = bVar10;
    }
    if ((bVar28 != 0x5f) && (bVar9)) goto LAB_1402b7e9d;
  }
LAB_1402b8134:
  if ((_Size < piVar16 || (int *)((longlong)_Size - (longlong)piVar16) == (int *)0x0) ||
     (-0x41 < *(char *)((longlong)piVar26 + (longlong)piVar16))) {
    _Size = (int *)((longlong)_Size - (longlong)piVar16);
    piVar22 = (void *)((longlong)piVar16 + (longlong)piVar26);
LAB_1402b7e9d:
    if ((longlong)_Size < 0) {
      uVar18 = 0;
    }
    else {
      if (_Size == (int *)0x0) {
        piVar16 = (int *)0x1;
LAB_1402b7f3d:
        if (0 < local_128) {
          puVar14 = (undefined8 *)__rustc::__rust_dealloc(piVar26,local_128,1);
          return puVar14;
        }
        uVar23 = local_110 & (ulonglong)pcStack_1a0;
        auVar29 = *(undefined1 (*) [16])(*local_1a8 + uVar23);
        uVar27 = (ushort)(SUB161(auVar29 >> 7,0) & 1) | (ushort)(SUB161(auVar29 >> 0xf,0) & 1) << 1
                 | (ushort)(SUB161(auVar29 >> 0x17,0) & 1) << 2 |
                 (ushort)(SUB161(auVar29 >> 0x1f,0) & 1) << 3 |
                 (ushort)(SUB161(auVar29 >> 0x27,0) & 1) << 4 |
                 (ushort)(SUB161(auVar29 >> 0x2f,0) & 1) << 5 |
                 (ushort)(SUB161(auVar29 >> 0x37,0) & 1) << 6 |
                 (ushort)(SUB161(auVar29 >> 0x3f,0) & 1) << 7 |
                 (ushort)(SUB161(auVar29 >> 0x47,0) & 1) << 8 |
                 (ushort)(SUB161(auVar29 >> 0x4f,0) & 1) << 9 |
                 (ushort)(SUB161(auVar29 >> 0x57,0) & 1) << 10 |
                 (ushort)(SUB161(auVar29 >> 0x5f,0) & 1) << 0xb |
                 (ushort)(SUB161(auVar29 >> 0x67,0) & 1) << 0xc |
                 (ushort)(SUB161(auVar29 >> 0x6f,0) & 1) << 0xd |
                 (ushort)(SUB161(auVar29 >> 0x77,0) & 1) << 0xe |
                 (ushort)(byte)(auVar29[0xf] >> 7) << 0xf;
        uVar19 = (uint)uVar27;
        if (uVar27 == 0) {
          lVar25 = 0x10;
          do {
            uVar23 = uVar23 + lVar25 & (ulonglong)pcStack_1a0;
            auVar29 = *(undefined1 (*) [16])(*local_1a8 + uVar23);
            lVar25 = lVar25 + 0x10;
            uVar19 = (uint)(ushort)((ushort)(SUB161(auVar29 >> 7,0) & 1) |
                                    (ushort)(SUB161(auVar29 >> 0xf,0) & 1) << 1 |
                                    (ushort)(SUB161(auVar29 >> 0x17,0) & 1) << 2 |
                                    (ushort)(SUB161(auVar29 >> 0x1f,0) & 1) << 3 |
                                    (ushort)(SUB161(auVar29 >> 0x27,0) & 1) << 4 |
                                    (ushort)(SUB161(auVar29 >> 0x2f,0) & 1) << 5 |
                                    (ushort)(SUB161(auVar29 >> 0x37,0) & 1) << 6 |
                                    (ushort)(SUB161(auVar29 >> 0x3f,0) & 1) << 7 |
                                    (ushort)(SUB161(auVar29 >> 0x47,0) & 1) << 8 |
                                    (ushort)(SUB161(auVar29 >> 0x4f,0) & 1) << 9 |
                                    (ushort)(SUB161(auVar29 >> 0x57,0) & 1) << 10 |
                                    (ushort)(SUB161(auVar29 >> 0x5f,0) & 1) << 0xb |
                                    (ushort)(SUB161(auVar29 >> 0x67,0) & 1) << 0xc |
                                    (ushort)(SUB161(auVar29 >> 0x6f,0) & 1) << 0xd |
                                    (ushort)(SUB161(auVar29 >> 0x77,0) & 1) << 0xe |
                                   (ushort)(byte)(auVar29[0xf] >> 7) << 0xf);
          } while (uVar19 == 0);
        }
        uVar20 = 0;
        for (; (uVar19 & 1) == 0; uVar19 = uVar19 >> 1 | 0x80000000) {
          uVar20 = uVar20 + 1;
        }
        uVar23 = uVar20 + uVar23 & (ulonglong)pcStack_1a0;
        bVar28 = (*local_1a8)[uVar23];
        if (-1 < (char)bVar28) {
          auVar29 = *local_1a8;
          uVar19 = 0;
          for (uVar20 = (uint)(ushort)((ushort)(SUB161(auVar29 >> 7,0) & 1) |
                                       (ushort)(SUB161(auVar29 >> 0xf,0) & 1) << 1 |
                                       (ushort)(SUB161(auVar29 >> 0x17,0) & 1) << 2 |
                                       (ushort)(SUB161(auVar29 >> 0x1f,0) & 1) << 3 |
                                       (ushort)(SUB161(auVar29 >> 0x27,0) & 1) << 4 |
                                       (ushort)(SUB161(auVar29 >> 0x2f,0) & 1) << 5 |
                                       (ushort)(SUB161(auVar29 >> 0x37,0) & 1) << 6 |
                                       (ushort)(SUB161(auVar29 >> 0x3f,0) & 1) << 7 |
                                       (ushort)(SUB161(auVar29 >> 0x47,0) & 1) << 8 |
                                       (ushort)(SUB161(auVar29 >> 0x4f,0) & 1) << 9 |
                                       (ushort)(SUB161(auVar29 >> 0x57,0) & 1) << 10 |
                                       (ushort)(SUB161(auVar29 >> 0x5f,0) & 1) << 0xb |
                                       (ushort)(SUB161(auVar29 >> 0x67,0) & 1) << 0xc |
                                       (ushort)(SUB161(auVar29 >> 0x6f,0) & 1) << 0xd |
                                       (ushort)(SUB161(auVar29 >> 0x77,0) & 1) << 0xe |
                                      (ushort)(byte)(auVar29[0xf] >> 7) << 0xf); (uVar20 & 1) == 0;
              uVar20 = uVar20 >> 1 | 0x80000000) {
            uVar19 = uVar19 + 1;
          }
          uVar23 = (ulonglong)uVar19;
          bVar28 = (*local_1a8)[uVar23];
        }
        (*local_1a8)[uVar23] = (char)local_108;
        local_1a8[1][uVar23 - 0x10 & (ulonglong)pcStack_1a0] = (char)local_108;
        local_198 = local_198 - (ulonglong)(bVar28 & 1);
        *(int *)local_1a8[uVar23 * -2 + -2] = iVar2;
        *(int **)(local_1a8[uVar23 * -2 + -2] + 8) = _Size;
        *(int **)local_1a8[uVar23 * -2 + -1] = piVar16;
        *(int **)(local_1a8[uVar23 * -2 + -1] + 8) = _Size;
        local_1d0 = lStack_190 + 1;
        lStack_190 = local_1d0;
        local_148 = piVar16;
LAB_1402b7fff:
        uVar24 = local_f8;
        piVar26 = local_100;
        lVar25 = local_170;
        if (local_f8 == 0) goto LAB_1402b828e;
        goto LAB_1402b802e;
      }
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      piVar16 = (int *)__rustc::__rust_alloc(_Size,1);
      if (piVar16 != (int *)0x0) {
        memcpy(piVar16,piVar22,(size_t)_Size);
        goto LAB_1402b7f3d;
      }
      uVar18 = 1;
      local_148 = _Size;
    }
    alloc::raw_vec::handle_error(uVar18,local_148);
  }
  else {
    local_178 = _Size;
    local_150 = &PTR_anon_abae67b6203ffecafa6256cb9d53fac3_100_llvm_5426317090953290063_1406f4fd8;
LAB_1402b8313:
    core::str::slice_error_fail(piVar26,_Size,piVar16,local_178,local_150);
  }
                    /* WARNING: Does not return */
  pcVar3 = (code *)invalidInstructionException();
  (*pcVar3)();
}


