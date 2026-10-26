
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::mods::read */

byte ******* __rustcall
menu_overlay::mods::read
          (byte *******param_1,byte *******param_2,byte *******param_3)

{
  undefined1 auVar1 [16];
  code *pcVar2;
  bool bVar3;
  char cVar4;
  byte bVar5;
  longlong lVar6;
  ulonglong uVar7;
  char *pcVar8;
  void *_Src;
  byte ******_Dst;
  undefined **ppuVar9;
  byte *******pppppppbVar10;
  ulonglong uVar11;
  byte bVar12;
  byte *pbVar13;
  longlong lVar14;
  byte bVar15;
  uint uVar16;
  byte *******pppppppbVar17;
  byte *pbVar18;
  undefined8 uVar19;
  byte *******pppppppbVar20;
  uint uVar21;
  uint uVar22;
  byte *pbVar23;
  byte *******pppppppbVar24;
  byte *******local_130;
  byte *******local_128;
  byte *******local_120;
  byte *******local_118;
  undefined8 local_110;
  byte *******local_108;
  byte *******local_100;
  byte *******local_f8;
  byte *******local_f0;
  byte *******local_e8;
  byte *******local_e0;
  byte *******local_d8;
  byte *******local_d0;
  byte *******local_c8;
  code *local_c0;
  byte *******local_b8;
  byte *******local_b0;
  undefined *local_a8 [5];
  undefined8 local_80;
  char *local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined8 uStack_60;
  byte *******local_58;
  byte *******local_50;
  byte *******local_48;
  
  <std::path::Path>::_join(&local_d8,param_2,param_3,&DAT_1401c862f,0xc);
  pppppppbVar24 = local_d0;
  std::fs::read_to_string::inner(&local_130,local_d0,local_c8);
  if (local_d8 != (byte *******)0x0) {
    __rustc::__rust_dealloc(pppppppbVar24,local_d8,1);
  }
  pppppppbVar20 = local_120;
  pppppppbVar24 = local_128;
  if (local_130 == (byte *******)0xffffffffffffffff) {
    if ((1 < ((uint)local_128 & 3) - 2) && (((ulonglong)local_128 & 3) != 0)) {
      local_d0 = (byte *******)((longlong)local_128 + -1);
      local_d8 = (byte *******)CONCAT71(local_d8._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&local_d0);
    }
    *param_1 = (byte ******)0xffffffffffffffff;
    return param_1;
  }
  local_e8 = local_130;
  pppppppbVar17 = (byte *******)0x0;
  do {
    while( true ) {
      pppppppbVar10 = local_120;
      if (pppppppbVar17 == local_120) goto LAB_1400dece8;
      pbVar13 = (byte *)((longlong)pppppppbVar17 + (longlong)local_128);
      bVar5 = *pbVar13;
      pppppppbVar10 = pppppppbVar17;
      if ((-1 < (char)bVar5) || (bVar5 < 0xe0)) goto LAB_1400dece8;
      uVar16 = pbVar13[2] & 0x3f | (pbVar13[1] & 0x3f) << 6;
      if (0xef < bVar5) break;
      pppppppbVar17 = (byte *******)(pbVar13 + -(longlong)local_128 + 3);
      if ((uVar16 | (bVar5 & 0x1f) << 0xc) != 0xfeff) goto LAB_1400dece8;
    }
    pppppppbVar17 = (byte *******)(pbVar13 + -(longlong)local_128 + 4);
  } while ((pbVar13[3] & 0x3f | uVar16 << 6 | (bVar5 & 7) << 0x12) == 0xfeff);
LAB_1400dece8:
  pppppppbVar17 = param_3;
  local_108 = param_1;
  lVar6 = <std::path::Path>::file_name(param_2,param_3);
  if (lVar6 == 0) {
LAB_1400dee12:
    local_110 = (byte *******)((ulonglong)local_110._4_4_ << 0x20);
  }
  else {
    core::str::converts::from_utf8(&local_d8,lVar6,pppppppbVar17);
    if (((int)local_d8 == 1) || (local_c8 == (byte *******)0x0)) goto LAB_1400dee12;
    if (local_c8 == (byte *******)0x1) {
      cVar4 = *(char *)local_d0;
      local_110 = (byte *******)((ulonglong)local_110 & 0xffffffff00000000);
      if ((cVar4 != '+') && (cVar4 != '-')) goto LAB_1400ded60;
    }
    else {
      cVar4 = *(char *)local_d0;
LAB_1400ded60:
      uVar11 = (ulonglong)(cVar4 == '+');
      lVar6 = -uVar11;
      if ((longlong)local_c8 - uVar11 < 0x11) {
        if (local_c8 == (byte *******)uVar11) {
          param_1 = (byte *******)0x0;
        }
        else {
          lVar14 = (longlong)local_c8 + lVar6;
          param_1 = (byte *******)0x0;
          lVar6 = 0;
          do {
            uVar16 = *(byte *)((longlong)local_d0 + lVar6 + uVar11) - 0x30;
            if (9 < uVar16) goto LAB_1400dee12;
            param_1 = (byte *******)((ulonglong)uVar16 + (longlong)param_1 * 10);
            lVar6 = lVar6 + 1;
          } while (lVar6 != lVar14);
        }
      }
      else {
        param_1 = (byte *******)0x0;
        lVar14 = 0;
        do {
          auVar1._8_8_ = 0;
          auVar1._0_8_ = param_1;
          uVar7 = SUB168(auVar1 * ZEXT816(10),0);
          if (SUB168(auVar1 * ZEXT816(10),8) != 0) goto LAB_1400dee12;
          uVar16 = *(byte *)((longlong)local_d0 + lVar14 + uVar11) - 0x30;
          param_1 = (byte *******)(uVar7 + uVar16);
          local_110 = (byte *******)((ulonglong)local_110 & 0xffffffff00000000);
          if ((9 < uVar16) || (CARRY8(uVar7,(ulonglong)uVar16))) goto LAB_1400dee1a;
          lVar14 = lVar14 + 1;
        } while (lVar14 != (longlong)local_c8 + lVar6);
        lVar6 = 0;
      }
      local_110 = (byte *******)
                  CONCAT44(local_110._4_4_,
                           (int)CONCAT71((int7)((ulonglong)lVar6 >> 8),param_1 != (byte *******)0x0)
                          );
    }
  }
LAB_1400dee1a:
  pppppppbVar20 = (byte *******)((longlong)pppppppbVar20 - (longlong)pppppppbVar10);
  pbVar13 = (byte *)((longlong)pppppppbVar10 + (longlong)pppppppbVar24);
  pppppppbVar17 = pppppppbVar20;
  bVar5 = number_after(pbVar13,pppppppbVar20,&DAT_1401c863b);
  local_118 = pppppppbVar24;
  bVar5 = pppppppbVar17 != (byte *******)0x0 & bVar5;
  local_68 = 0;
  uStack_60 = 0;
  local_c8 = (byte *******)0x1;
  local_b8 = (byte *******)0x0;
  local_b0 = (byte *******)&DAT_00000004;
  local_a8[0] = &DAT_1401c864c;
  local_a8[1] = (undefined *)0x6;
  local_a8[2] = &DAT_1401c8628;
  local_a8[3] = (undefined *)0x7;
  local_a8[4] = &DAT_1401c8620;
  local_80 = 8;
  local_78 = "steamapps";
  local_70 = 9;
  local_c0 = (code *)0x4;
  lVar6 = 0;
  local_100 = pppppppbVar20;
  local_e0 = param_1;
  local_d8 = param_2;
  local_d0 = param_3;
  local_50 = pppppppbVar17;
  do {
    pppppppbVar17 = local_c8;
    pppppppbVar20 = local_d0;
    pppppppbVar24 = local_d8;
    local_c0 = local_c0 + -1;
    if (local_c8 == (byte *******)0x0) {
      if (local_d8 == (byte *******)0x0) {
        local_d8 = (byte *******)0x0;
        local_d0 = local_c8;
        break;
      }
      local_d8 = (byte *******)<std::path::Path>::parent(local_d8);
    }
    else {
      local_c8 = (byte *******)0x0;
      pppppppbVar24 =
           (byte *******)
           <std::path::Ancestors_as_core::iter::traits::iterator::Iterator>
           ::nth(&local_d8);
      pppppppbVar20 = pppppppbVar17;
    }
    if (pppppppbVar24 == (byte *******)0x0) break;
    lVar14 = <std::path::Path>::file_name(pppppppbVar24);
    if (lVar14 == 0) {
      pppppppbVar20 = (byte *******)0x1;
      pppppppbVar24 = (byte *******)0x0;
      if (local_b0 == local_b8) break;
    }
    else {
      core::str::converts::from_utf8(&local_130,lVar14,pppppppbVar20);
      pppppppbVar20 = local_128;
      pppppppbVar24 = local_120;
      if ((char)local_130 != '\0') {
        pppppppbVar24 = (byte *******)0x0;
        pppppppbVar20 = (byte *******)0x1;
      }
      if (local_b0 == local_b8) break;
    }
    pppppppbVar17 = (byte *******)((longlong)local_b8 + 1);
    if (pppppppbVar24 == (byte *******)local_a8[(longlong)local_b8 * 2 + 1]) {
      lVar14 = (longlong)local_b8 * 2;
      if (pppppppbVar24 < &DAT_00000010) {
        uVar11 = 1;
        local_b8 = pppppppbVar17;
        if (pppppppbVar24 != (byte *******)0x0) {
          pppppppbVar10 = (byte *******)0x0;
          do {
            if ((byte)(((byte)(*(byte *)((longlong)pppppppbVar20 + (longlong)pppppppbVar10) + 0xbf)
                       < 0x1a) << 5 | *(byte *)((longlong)pppppppbVar20 + (longlong)pppppppbVar10))
                != (byte)(((byte)(*(byte *)((longlong)pppppppbVar10 + (longlong)local_a8[lVar14]) +
                                 0xbf) < 0x1a) << 5 |
                         *(byte *)((longlong)pppppppbVar10 + (longlong)local_a8[lVar14])))
            goto LAB_1400def20;
            pppppppbVar10 = (byte *******)((longlong)pppppppbVar10 + 1);
          } while (pppppppbVar24 != pppppppbVar10);
        }
      }
      else {
        local_b8 = pppppppbVar17;
        bVar12 = <[u8]>::eq_ignore_ascii_case_chunks::<16usize>();
        uVar11 = (ulonglong)bVar12;
      }
    }
    else {
LAB_1400def20:
      uVar11 = 0;
      local_b8 = pppppppbVar17;
    }
    lVar6 = lVar6 + uVar11;
  } while (local_c0 != (code *)0x0);
  pppppppbVar24 = local_100;
  pppppppbVar20 = local_e0;
  if (bVar5 != 0) {
    pppppppbVar20 = local_50;
  }
  if (((ulonglong)local_110 & 1) == 0) {
    local_e0 = local_50;
  }
  if (lVar6 != 4) {
    local_e0 = pppppppbVar20;
  }
  if (bVar5 == 0 && ((ulonglong)local_110 & 1) == 0) {
    *local_108 = (byte ******)0xffffffffffffffff;
    pppppppbVar17 = local_108;
    goto joined_r0x0001400dfebb;
  }
  <core::str::pattern::StrSearcher>::new
            (&local_d8,pbVar13,local_100,&DAT_1401c8652,7);
  <core::str::pattern::StrSearcher_as_core::str::pattern::Searcher>
  ::next_match(&local_130,&local_d8);
  if ((int)local_130 == 1) {
    if (local_128 != (byte *******)0x0) {
      if (local_128 < pppppppbVar24) {
        if ((char)pbVar13[(longlong)local_128] < -0x40) {
LAB_1400df18d:
          core::str::slice_error_fail
                    (pbVar13,pppppppbVar24,local_128,pppppppbVar24,
                     &PTR_s_src_______directory_src_mods_rs_1401c80b8);
          goto LAB_1400dfd0b;
        }
      }
      else if (pppppppbVar24 != local_128) goto LAB_1400df18d;
    }
    lVar6 = (longlong)pppppppbVar24 - (longlong)local_128;
    pcVar8 = (char *)value_after((byte *)((longlong)local_128 + (longlong)pbVar13),lVar6,
                                 "\"Neutral\"",9);
    if (((pcVar8 == (char *)0x0) || (lVar6 == 0)) || (*pcVar8 != '\"')) goto LAB_1400dfad2;
    local_130 = (byte *******)0x0;
    local_128 = (byte *******)0x1;
    local_120 = (byte *******)0x0;
    local_f0 = (byte *******)(pcVar8 + lVar6);
    if (lVar6 != 1) {
      local_f8 = (byte *******)(pcVar8 + 1);
      pppppppbVar24 = (byte *******)0x1;
      pppppppbVar20 = (byte *******)0x0;
      do {
        pppppppbVar17 = (byte *******)((longlong)local_f8 + 1);
        bVar5 = *(byte *)local_f8;
        uVar16 = (uint)bVar5;
        if ((char)bVar5 < '\0') {
          pppppppbVar17 = (byte *******)((longlong)local_f8 + 2);
          uVar21 = *(byte *)((longlong)local_f8 + 1) & 0x3f;
          if (bVar5 < 0xe0) {
            uVar16 = (uVar16 & 0x1f) << 6 | uVar21;
            goto joined_r0x0001400df368;
          }
          pppppppbVar17 = (byte *******)((longlong)local_f8 + 3);
          uVar21 = *(byte *)((longlong)local_f8 + 2) & 0x3f | uVar21 << 6;
          if (0xef < bVar5) {
            pppppppbVar17 = (byte *******)((longlong)local_f8 + 4);
            uVar16 = *(byte *)((longlong)local_f8 + 3) & 0x3f | uVar21 << 6 | (uVar16 & 7) << 0x12;
            goto joined_r0x0001400df368;
          }
          uVar16 = uVar21 | (uVar16 & 0x1f) << 0xc;
          local_f8 = pppppppbVar17;
          if (uVar16 != 0x5c) goto LAB_1400df2cc;
LAB_1400df400:
          if (local_f8 == local_f0) break;
          bVar5 = *(byte *)local_f8;
          uVar16 = (uint)bVar5;
          pppppppbVar17 = (byte *******)((longlong)local_f8 + 1);
          if ((char)bVar5 < '\0') {
            uVar21 = *(byte *)((longlong)local_f8 + 1) & 0x3f;
            if (bVar5 < 0xe0) {
              uVar16 = (uVar16 & 0x1f) << 6 | uVar21;
              pppppppbVar17 = (byte *******)((longlong)local_f8 + 2);
            }
            else {
              uVar21 = *(byte *)((longlong)local_f8 + 2) & 0x3f | uVar21 << 6;
              if (bVar5 < 0xf0) {
                uVar16 = uVar21 | (uVar16 & 0x1f) << 0xc;
                pppppppbVar17 = (byte *******)((longlong)local_f8 + 3);
              }
              else {
                uVar16 = *(byte *)((longlong)local_f8 + 3) & 0x3f | uVar21 << 6 |
                         (uVar16 & 7) << 0x12;
                pppppppbVar17 = (byte *******)((longlong)local_f8 + 4);
              }
            }
          }
          local_f8 = pppppppbVar17;
          switch(uVar16) {
          case 0x6e:
          case 0x72:
          case 0x74:
            if (local_130 == pppppppbVar20) {
              <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
              <alloc::alloc::Global>(&local_130,pppppppbVar20,1,1,1);
            }
            *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) = 0x20;
            pppppppbVar20 = (byte *******)((longlong)pppppppbVar20 + 1);
            pppppppbVar24 = local_128;
            local_120 = pppppppbVar20;
            break;
          default:
            if (uVar16 < 0x80) {
              if (local_130 == pppppppbVar20) {
                <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
                <alloc::alloc::Global>(&local_130,pppppppbVar20,1,1,1);
              }
              *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) = (byte)uVar16;
              lVar6 = 1;
            }
            else {
              uVar11 = 4 - (ulonglong)(uVar16 < 0x10000);
              if (uVar16 < 0x800) {
                uVar11 = 2;
              }
              if ((ulonglong)((longlong)local_130 - (longlong)pppppppbVar20) < uVar11) {
                <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
                <alloc::alloc::Global>(&local_130,pppppppbVar20,uVar11,1,1);
              }
              bVar5 = (byte)uVar16 & 0x3f | 0x80;
              bVar12 = (byte)(uVar16 >> 6);
              if (uVar16 < 0x800) {
LAB_1400df286:
                *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) = bVar12 | 0xc0;
                ((byte *)((longlong)local_128 + 1))[(longlong)pppppppbVar20] = bVar5;
                lVar6 = 2;
              }
              else {
                bVar12 = bVar12 & 0x3f | 0x80;
                bVar15 = (byte)(uVar16 >> 0xc);
                if (uVar16 < 0x10000) {
LAB_1400df3d1:
                  *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) = bVar15 | 0xe0;
                  ((byte *)((longlong)local_128 + 1))[(longlong)pppppppbVar20] = bVar12;
                  ((byte *)((longlong)local_128 + 2))[(longlong)pppppppbVar20] = bVar5;
                  lVar6 = 3;
                }
                else {
                  *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) =
                       (byte)(uVar16 >> 0x12) | 0xf0;
LAB_1400df4ab:
                  ((byte *)((longlong)local_128 + 1))[(longlong)pppppppbVar20] =
                       bVar15 & 0x3f | 0x80;
                  ((byte *)((longlong)local_128 + 2))[(longlong)pppppppbVar20] = bVar12;
                  ((byte *)((longlong)local_128 + 3))[(longlong)pppppppbVar20] = bVar5;
                  lVar6 = 4;
                }
              }
            }
            goto LAB_1400df298;
          case 0x75:
            <alloc::string::String_as_core::iter::traits::collect::FromIterator<char>>
            ::from_iter::
            <core::iter::adapters::take::Take<&mut_core::str::iter::Chars>>
                      (&local_d8,&local_f8,4);
            pppppppbVar17 = local_d0;
            if (local_c8 == (byte *******)0x0) {
              uVar16 = 0;
              bVar3 = true;
              goto LAB_1400df6fc;
            }
            if (local_c8 == (byte *******)0x1) {
              bVar5 = *(byte *)local_d0;
              uVar16 = 0;
              bVar3 = true;
              if ((bVar5 == 0x2b) || (bVar5 == 0x2d)) goto LAB_1400df6fc;
            }
            else {
              bVar5 = *(byte *)local_d0;
            }
            pppppppbVar10 = (byte *******)(ulonglong)(bVar5 == 0x2b);
            uVar11 = (longlong)local_c8 - (longlong)pppppppbVar10;
            pbVar18 = (byte *)((longlong)local_d0 + (longlong)pppppppbVar10);
            if (uVar11 < 9) {
              if (local_c8 == pppppppbVar10) {
                uVar21 = 0;
                goto LAB_1400df6f5;
              }
              bVar5 = *pbVar18;
              uVar21 = bVar5 - 0x30;
              if (0x39 < bVar5) {
                uVar21 = (bVar5 - 0x41 & 0xffffffdf) + 10;
              }
              uVar16 = 0;
              bVar3 = true;
              if (uVar21 < 0x10) {
                if (uVar11 != 1) {
                  bVar5 = pbVar18[1];
                  uVar22 = bVar5 - 0x30;
                  if (0x39 < bVar5) {
                    uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                  }
                  if (0xf < uVar22) goto LAB_1400df6fc;
                  uVar21 = uVar22 | uVar21 << 4;
                  if (uVar11 != 2) {
                    bVar5 = pbVar18[2];
                    uVar22 = bVar5 - 0x30;
                    if (0x39 < bVar5) {
                      uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                    }
                    if (0xf < uVar22) goto LAB_1400df6fc;
                    uVar21 = uVar22 | uVar21 << 4;
                    if (uVar11 != 3) {
                      bVar5 = pbVar18[3];
                      uVar22 = bVar5 - 0x30;
                      if (0x39 < bVar5) {
                        uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                      }
                      if (0xf < uVar22) goto LAB_1400df6fc;
                      uVar21 = uVar22 | uVar21 << 4;
                      if (uVar11 != 4) {
                        bVar5 = pbVar18[4];
                        uVar22 = bVar5 - 0x30;
                        if (0x39 < bVar5) {
                          uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                        }
                        if (0xf < uVar22) goto LAB_1400df6fc;
                        uVar21 = uVar22 | uVar21 << 4;
                        if (uVar11 != 5) {
                          bVar5 = pbVar18[5];
                          uVar22 = bVar5 - 0x30;
                          if (0x39 < bVar5) {
                            uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                          }
                          if (0xf < uVar22) goto LAB_1400df6fc;
                          uVar21 = uVar22 | uVar21 << 4;
                          if (uVar11 != 6) {
                            bVar5 = pbVar18[6];
                            uVar22 = bVar5 - 0x30;
                            if (0x39 < bVar5) {
                              uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                            }
                            if (0xf < uVar22) goto LAB_1400df6fc;
                            uVar21 = uVar22 | uVar21 << 4;
                            if (uVar11 != 7) {
                              bVar5 = pbVar18[7];
                              uVar22 = bVar5 - 0x30;
                              if (0x39 < bVar5) {
                                uVar22 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                              }
                              if (0xf < uVar22) goto LAB_1400df6fc;
                              uVar21 = uVar22 | uVar21 << 4;
                            }
                          }
                        }
                      }
                    }
                  }
                }
                goto LAB_1400df6f5;
              }
            }
            else {
              uVar21 = 0;
              lVar6 = 0;
              do {
                bVar5 = pbVar18[lVar6];
                if (0xfffffff < uVar21) {
                  uVar16 = 0;
                  bVar3 = true;
                  goto LAB_1400df6fc;
                }
                uVar16 = bVar5 - 0x30;
                if (0x39 < bVar5) {
                  uVar16 = (bVar5 - 0x41 & 0xffffffdf) + 10;
                }
                if (0xf < uVar16) {
                  uVar16 = 0;
                  bVar3 = true;
                  goto LAB_1400df6fc;
                }
                uVar21 = uVar16 | uVar21 << 4;
                lVar6 = lVar6 + 1;
              } while (lVar6 != (longlong)local_c8 - (longlong)pppppppbVar10);
LAB_1400df6f5:
              uVar16 = uVar21;
              bVar3 = false;
            }
LAB_1400df6fc:
            if ((uVar16 ^ 0xd800) - 0x110000 < 0xffef0800) {
              uVar16 = 0x3f;
            }
            if (bVar3) {
              uVar16 = 0x3f;
            }
            if (uVar16 < 0x80) {
              if (local_130 == pppppppbVar20) {
                <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
                <alloc::alloc::Global>(&local_130,pppppppbVar20,1,1,1);
              }
              *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) = (byte)uVar16;
              lVar6 = 1;
              pppppppbVar24 = local_128;
            }
            else {
              uVar11 = 4 - (ulonglong)(uVar16 < 0x10000);
              if (uVar16 < 0x800) {
                uVar11 = 2;
              }
              if ((ulonglong)((longlong)local_130 - (longlong)pppppppbVar20) < uVar11) {
                <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
                <alloc::alloc::Global>(&local_130,pppppppbVar20,uVar11,1,1);
                pppppppbVar24 = local_128;
              }
              bVar5 = (byte)uVar16 & 0x3f | 0x80;
              bVar12 = (byte)(uVar16 >> 6);
              if (uVar16 < 0x800) {
                *(byte *)((longlong)pppppppbVar24 + (longlong)pppppppbVar20) = bVar12 | 0xc0;
                ((byte *)((longlong)pppppppbVar24 + 1))[(longlong)pppppppbVar20] = bVar5;
                lVar6 = 2;
              }
              else {
                bVar12 = bVar12 & 0x3f | 0x80;
                bVar15 = (byte)(uVar16 >> 0xc);
                if (uVar16 < 0x10000) {
                  *(byte *)((longlong)pppppppbVar24 + (longlong)pppppppbVar20) = bVar15 | 0xe0;
                  ((byte *)((longlong)pppppppbVar24 + 1))[(longlong)pppppppbVar20] = bVar12;
                  ((byte *)((longlong)pppppppbVar24 + 2))[(longlong)pppppppbVar20] = bVar5;
                  lVar6 = 3;
                }
                else {
                  *(byte *)((longlong)pppppppbVar24 + (longlong)pppppppbVar20) =
                       (byte)(uVar16 >> 0x12) | 0xf0;
                  ((byte *)((longlong)pppppppbVar24 + 1))[(longlong)pppppppbVar20] =
                       bVar15 & 0x3f | 0x80;
                  ((byte *)((longlong)pppppppbVar24 + 2))[(longlong)pppppppbVar20] = bVar12;
                  ((byte *)((longlong)pppppppbVar24 + 3))[(longlong)pppppppbVar20] = bVar5;
                  lVar6 = 4;
                }
              }
            }
            pppppppbVar20 = (byte *******)((longlong)pppppppbVar20 + lVar6);
            local_120 = pppppppbVar20;
            if (local_d8 != (byte *******)0x0) {
              __rustc::__rust_dealloc(pppppppbVar17,local_d8,1);
            }
          }
        }
        else {
joined_r0x0001400df368:
          local_f8 = pppppppbVar17;
          if (uVar16 == 0x5c) goto LAB_1400df400;
LAB_1400df2cc:
          if (uVar16 == 0x22) {
            local_110 = local_128;
            pppppppbVar24 = local_130;
            if (local_130 != (byte *******)0xffffffffffffffff) goto LAB_1400dfad9;
            goto LAB_1400dfad2;
          }
          if (0x7f < uVar16) {
            uVar11 = 4 - (ulonglong)(uVar16 < 0x10000);
            if (uVar16 < 0x800) {
              uVar11 = 2;
            }
            if ((ulonglong)((longlong)local_130 - (longlong)pppppppbVar20) < uVar11) {
              <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
              <alloc::alloc::Global>(&local_130,pppppppbVar20,uVar11,1,1);
            }
            bVar5 = (byte)uVar16 & 0x3f | 0x80;
            bVar12 = (byte)(uVar16 >> 6);
            if (0x7ff < uVar16) {
              bVar12 = bVar12 & 0x3f | 0x80;
              bVar15 = (byte)(uVar16 >> 0xc);
              if (0xffff < uVar16) {
                *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) =
                     (byte)(uVar16 >> 0x12) | 0xf0;
                goto LAB_1400df4ab;
              }
              goto LAB_1400df3d1;
            }
            goto LAB_1400df286;
          }
          if (local_130 == pppppppbVar20) {
            <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
            <alloc::alloc::Global>(&local_130,pppppppbVar20,1,1,1);
          }
          *(byte *)((longlong)local_128 + (longlong)pppppppbVar20) = (byte)uVar16;
          lVar6 = 1;
LAB_1400df298:
          pppppppbVar20 = (byte *******)((longlong)pppppppbVar20 + lVar6);
          pppppppbVar24 = local_128;
          local_120 = pppppppbVar20;
        }
      } while (local_f8 != local_f0);
    }
    if (local_130 != (byte *******)0x0) {
      __rustc::__rust_dealloc(local_128,local_130,1);
    }
    pppppppbVar20 = (byte *******)0x0;
    local_110 = (byte *******)0x1;
    pppppppbVar24 = (byte *******)0x0;
  }
  else {
LAB_1400dfad2:
    pppppppbVar20 = (byte *******)0x0;
    local_110 = (byte *******)0x1;
    pppppppbVar24 = (byte *******)0x0;
  }
LAB_1400dfad9:
  pppppppbVar17 = local_108;
  local_d0 = (byte *******)((longlong)pppppppbVar20 + (longlong)local_110);
  local_c8 = (byte *******)&DAT_00000030;
  local_d8 = local_110;
  <alloc::string::String_as_core::iter::traits::collect::FromIterator<char>>
  ::from_iter::
  <core::iter::adapters::take::Take<core::iter::adapters::filter::Filter<core::str::iter::Chars,menu_overlay::gamesetup::share_fields::_closure_2_::_closure_3_>>>
            (&local_130,&local_d8);
  pppppppbVar20 = local_128;
  pppppppbVar10 = local_120;
  _Src = (void *)<str>::trim_matches::<<char>::is_whitespace>(local_128);
  if ((longlong)pppppppbVar10 < 0) {
    uVar19 = 0;
LAB_1400dfb28:
    alloc::raw_vec::handle_error(uVar19);
    goto LAB_1400dfd0b;
  }
  local_108 = pppppppbVar10;
  if (pppppppbVar10 == (byte *******)0x0) {
    _Dst = (byte ******)0x1;
  }
  else {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    uVar19 = 1;
    _Dst = (byte ******)__rustc::__rust_alloc(pppppppbVar10,1);
    if (_Dst == (byte ******)0x0) goto LAB_1400dfb28;
    memcpy(_Dst,_Src,(size_t)pppppppbVar10);
  }
  if (local_130 != (byte *******)0x0) {
    __rustc::__rust_dealloc(pppppppbVar20,local_130,1);
  }
  if (pppppppbVar24 != (byte *******)0x0) {
    __rustc::__rust_dealloc(local_110,pppppppbVar24,1);
  }
  <core::str::pattern::StrSearcher>::new
            (&local_d8,pbVar13,local_100,&DAT_1401c8659,0xc);
  <core::str::pattern::StrSearcher_as_core::str::pattern::Searcher>
  ::next_match();
  if ((int)local_130 == 1) {
    if (local_128 != (byte *******)0x0) {
      if (local_100 <= local_128) {
        if (local_100 != local_128) goto LAB_1400dfc0a;
        goto LAB_1400dfc42;
      }
      if (-0x41 < (char)pbVar13[(longlong)local_128]) goto LAB_1400dfc42;
LAB_1400dfc0a:
      ppuVar9 = &PTR_s_src_______directory_src_mods_rs_1401c80e0;
      pppppppbVar20 = local_128;
      pppppppbVar24 = local_100;
LAB_1400dfcf9:
      core::str::slice_error_fail
                (pbVar13,local_100,pppppppbVar20,pppppppbVar24,ppuVar9);
LAB_1400dfd0b:
                    /* WARNING: Does not return */
      pcVar2 = (code *)invalidInstructionException();
      (*pcVar2)();
    }
LAB_1400dfc42:
    local_100 = (byte *******)((longlong)local_100 - (longlong)local_128);
    pbVar13 = pbVar13 + (longlong)local_128;
    pppppppbVar20 = (byte *******)0x0;
    do {
      while( true ) {
        pbVar18 = pbVar13 + (longlong)pppppppbVar20;
        if (&DAT_0000000f < (byte *)((longlong)local_100 - (longlong)pppppppbVar20)) {
          uVar11 = core::slice::memchr::memchr_aligned(0x7d);
          pbVar23 = pbVar18;
          pppppppbVar24 = local_100;
          if ((uVar11 & 1) == 0) goto joined_r0x0001400dfcda;
        }
        else {
          pppppppbVar24 = local_100;
          if (local_100 == pppppppbVar20) goto joined_r0x0001400dfcda;
          pbVar23 = (byte *)0x0;
          while (pbVar18[(longlong)pbVar23] != 0x7d) {
            pbVar23 = pbVar23 + 1;
            if ((byte *)((longlong)local_100 - (longlong)pppppppbVar20) == pbVar23)
            goto joined_r0x0001400dfcda;
          }
        }
        pppppppbVar10 = (byte *******)(pbVar23 + (longlong)pppppppbVar20 + 1);
        pppppppbVar24 = (byte *******)(pbVar23 + (longlong)pppppppbVar20);
        pppppppbVar20 = pppppppbVar10;
        if (local_100 <= pppppppbVar24) break;
        if ((local_100 < pppppppbVar10) || (pbVar13[(longlong)pppppppbVar24] == 0x7d)) {
          pppppppbVar20 = local_100;
          if (pbVar13[(longlong)pppppppbVar24] != 0x7d) goto LAB_1400dfd1a;
          goto joined_r0x0001400dfcda;
        }
      }
      pppppppbVar24 = local_100;
    } while (pppppppbVar10 <= local_100);
joined_r0x0001400dfcda:
    if (pppppppbVar24 == (byte *******)0x0) {
      pppppppbVar20 = (byte *******)0x0;
    }
    else if (pppppppbVar24 < local_100) {
      pppppppbVar20 = pppppppbVar24;
      if ((char)pbVar13[(longlong)pppppppbVar24] < -0x40) goto LAB_1400dfcef;
    }
    else {
      pppppppbVar20 = local_100;
      if (pppppppbVar24 != local_100) {
LAB_1400dfcef:
        ppuVar9 = &PTR_s_src_______directory_src_mods_rs_1401c80f8;
        pppppppbVar20 = (byte *******)0x0;
        goto LAB_1400dfcf9;
      }
    }
LAB_1400dfd1a:
    pppppppbVar24 = pppppppbVar20;
    lVar6 = number_after(pbVar13,pppppppbVar20,&DAT_1401c8110,7);
    pppppppbVar10 = pppppppbVar20;
    uVar11 = number_after(pbVar13,pppppppbVar20,&DAT_1401c8117,7);
    lVar14 = number_after(pbVar13,pppppppbVar20,&DAT_1401c811e,7);
    if ((lVar6 == 1) && ((uVar11 & 1) != 0)) {
      if (lVar14 == 1) {
        local_d8 = (byte *******)&local_48;
        local_d0 = (byte *******)<u64_as_core::fmt::Display>::fmt;
        local_c8 = (byte *******)&local_58;
        local_c0 = <u64_as_core::fmt::Display>::fmt;
        local_b8 = (byte *******)&local_f8;
        local_b0 = (byte *******)<u64_as_core::fmt::Display>::fmt;
        local_f8 = pppppppbVar20;
        local_58 = pppppppbVar10;
        local_48 = pppppppbVar24;
        alloc::fmt::format::format_inner(&local_130,&DAT_1401c812a,&local_d8);
        pppppppbVar24 = local_128;
        pppppppbVar20 = local_130;
        pppppppbVar10 = local_120;
      }
      else {
        local_d8 = (byte *******)&local_58;
        local_d0 = (byte *******)<u64_as_core::fmt::Display>::fmt;
        local_c8 = (byte *******)&local_f8;
        local_c0 = <u64_as_core::fmt::Display>::fmt;
        local_f8 = pppppppbVar10;
        local_58 = pppppppbVar24;
        alloc::fmt::format::format_inner(&local_130,&DAT_1401c8125,&local_d8);
        pppppppbVar24 = local_128;
        pppppppbVar20 = local_130;
        pppppppbVar10 = local_120;
      }
    }
    else {
      local_130 = (byte *******)0x0;
      local_128 = (byte *******)0x1;
      local_120 = (byte *******)0x0;
      pppppppbVar24 = local_128;
      pppppppbVar20 = local_130;
      pppppppbVar10 = local_120;
    }
  }
  else {
    pppppppbVar24 = (byte *******)0x1;
    pppppppbVar20 = (byte *******)0x0;
    pppppppbVar10 = (byte *******)0x0;
  }
  *pppppppbVar17 = (byte ******)local_108;
  pppppppbVar17[1] = _Dst;
  pppppppbVar17[2] = (byte ******)local_108;
  pppppppbVar17[3] = (byte ******)pppppppbVar20;
  pppppppbVar17[4] = (byte ******)pppppppbVar24;
  pppppppbVar17[5] = (byte ******)pppppppbVar10;
  pppppppbVar17[6] = (byte ******)local_e0;
joined_r0x0001400dfebb:
  if (local_e8 != (byte *******)0x0) {
    __rustc::__rust_dealloc(local_118,local_e8,1);
  }
  return pppppppbVar17;
}


