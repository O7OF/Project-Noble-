
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::modmaps::prepare_files */

void __rustcall
host_probe::modmaps::prepare_files
          (undefined8 *param_1,undefined8 param_2,undefined8 param_3,longlong *******param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7)

{
  longlong lVar1;
  undefined1 auVar2 [16];
  undefined1 auVar3 [16];
  char *pcVar4;
  code *pcVar5;
  longlong ******pppppplVar6;
  char acVar7 [16];
  char cVar8;
  longlong lVar9;
  char *pcVar10;
  longlong *******ppppppplVar11;
  longlong *******ppppppplVar12;
  longlong *******ppppppplVar13;
  byte *pbVar14;
  ulonglong uVar15;
  longlong lVar16;
  longlong *******ppppppplVar17;
  ulonglong uVar18;
  undefined1 *puVar19;
  undefined1 *puVar20;
  longlong *******ppppppplVar21;
  longlong *******ppppppplVar22;
  longlong lVar23;
  byte bVar24;
  undefined8 uVar25;
  undefined8 *puVar26;
  bool bVar27;
  undefined1 auVar28 [16];
  undefined1 auVar29 [16];
  longlong *******local_9e8;
  longlong *******ppppppplStack_9e0;
  undefined4 local_9d8;
  undefined4 uStack_9d4;
  undefined4 local_9d0;
  undefined5 uStack_9cc;
  longlong *******local_9a8;
  longlong *******ppppppplStack_9a0;
  longlong *******local_998;
  undefined1 local_979 [9];
  longlong local_970;
  longlong lStack_968;
  longlong local_960;
  longlong ******local_958;
  longlong *******local_950;
  ulonglong local_948;
  longlong *******local_938;
  longlong *******ppppppplStack_930;
  longlong ******local_918;
  undefined8 uStack_910;
  char *local_8f0;
  longlong *****local_8e8;
  undefined8 local_8e0;
  longlong *******local_8d8;
  longlong *******local_8d0;
  longlong *******ppppppplStack_8c8;
  longlong *******local_8b8;
  longlong *******local_8a8;
  longlong *******ppppppplStack_8a0;
  ulonglong local_898;
  undefined8 uStack_890;
  char *local_880;
  undefined8 local_878;
  undefined8 local_870;
  longlong *******local_868;
  longlong ******pppppplStack_860;
  longlong *******local_858;
  longlong *******ppppppplStack_850;
  undefined1 local_848 [592];
  char local_5f8 [24];
  undefined8 uStack_5e0;
  longlong local_5c0;
  undefined8 local_5b8;
  undefined8 local_5b0;
  char local_5a8 [8];
  undefined1 uStack_5a0;
  undefined7 uStack_59f;
  undefined1 uStack_598;
  undefined7 uStack_597;
  undefined8 local_590;
  undefined1 local_568 [16];
  undefined1 auStack_558 [8];
  longlong *******ppppppplStack_550;
  undefined1 local_548 [8];
  code *pcStack_540;
  undefined1 local_538 [16];
  undefined1 local_528 [16];
  longlong ******local_2f8;
  longlong *******local_2f0;
  ulonglong local_2e8;
  longlong ******local_2e0;
  undefined8 uStack_2d8;
  undefined1 local_2d0 [656];
  
  local_970 = 0;
  lStack_968 = 8;
  local_960 = 0;
  <std::path::Path>::_join(local_568,param_4,param_5,&DAT_1406a7bf1,0xc);
  uVar25 = local_568._8_8_;
  read_json(local_5a8,local_568._8_8_,auStack_558);
  acVar7 = s__________________1406a4b40;
  if (local_5a8[0] != -1) {
    uStack_5e0 = local_590;
    local_5f8[0] = local_5a8[0];
    local_8b8 = param_4;
    local_878 = param_2;
    local_870 = param_3;
    if (local_568._0_8_ != 0) {
      __rustc::__rust_dealloc(uVar25,local_568._0_8_,1);
      return;
    }
    local_568 = ZEXT816(2) << 0x40;
    ppppppplStack_550 = (longlong *******)0xd;
    auStack_558 = (undefined1  [8])&DAT_1406a7bfd;
    pcStack_540 = (code *)0xc;
    local_548 = (undefined1  [8])&DAT_1406a7c0a;
    local_538._8_8_ = 0xf;
    local_538._0_8_ = &DAT_1406a7c16;
    local_528._8_8_ = 10;
    local_528._0_8_ = &DAT_1406a7c25;
    lVar9 = 0;
    while( true ) {
      lVar23 = lVar9 + 1;
      local_568._0_8_ = lVar23;
      lVar9 = lVar9 * 0x20;
      if (*(longlong ******)(auStack_558 + lVar9) == (longlong *****)0x0) goto LAB_14008b1bd;
      local_898 = *(ulonglong *)(local_548 + lVar9);
      uStack_890 = *(undefined8 *)(local_548 + lVar9 + 8);
      local_8e0 = *(undefined8 *)(auStack_558 + lVar9 + 8);
      local_2f8 = &local_8e8;
      local_2f0 = (longlong *******)&<&str_as_core::fmt::Display>::fmt;
      local_8e8 = *(longlong ******)(auStack_558 + lVar9);
      alloc::fmt::format::format_inner(&local_868,&DAT_1406a7c91,&local_2f8);
      pppppplVar6 = pppppplStack_860;
      ppppppplVar13 = local_868;
      pcVar10 = (char *)::<serde_json::value::Value>::pointer
                                  (local_5f8,pppppplStack_860,local_858);
      if (pcVar10 != (char *)0x0) break;
      if (ppppppplVar13 != (longlong *******)0x0) {
        __rustc::__rust_dealloc(pppppplVar6,ppppppplVar13,1);
        return;
      }
LAB_14008b1a6:
      lVar9 = 1;
      if (lVar23 == 2) goto LAB_14008b1bd;
    }
    if (ppppppplVar13 != (longlong *******)0x0) {
      __rustc::__rust_dealloc(pppppplVar6,ppppppplVar13,1);
      return;
    }
    if (*pcVar10 != '\x04') {
      local_2f8 = &local_8e8;
      local_2f0 = (longlong *******)&<&str_as_core::fmt::Display>::fmt;
      alloc::fmt::format::format_inner(&local_868,&DAT_1406a5601,&local_2f8);
LAB_14008b967:
      param_1[1] = local_868;
      param_1[2] = pppppplStack_860;
      param_1[3] = local_858;
      *param_1 = 1;
      goto LAB_14008bb9f;
    }
    if (*(longlong *)(pcVar10 + 0x18) == 0) goto LAB_14008b1a6;
    pcVar4 = *(char **)(pcVar10 + 0x10);
    local_8f0 = pcVar4 + *(longlong *)(pcVar10 + 0x18) * 0x20;
    if (*pcVar4 != '\x03') {
      local_2f8 = &local_8e8;
      local_2f0 = (longlong *******)&<&str_as_core::fmt::Display>::fmt;
      alloc::fmt::format::format_inner(&local_868,&DAT_1406a5696,&local_2f8);
      goto LAB_14008b967;
    }
    pcVar10 = pcVar4 + 0x20;
    ppppppplVar13 = *(longlong ********)(pcVar4 + 0x10);
    ppppppplVar22 = *(longlong ********)(pcVar4 + 0x18);
    local_8a8 = ppppppplVar13;
    ppppppplStack_8a0 = ppppppplVar22;
    if ((longlong)ppppppplVar22 < 0) {
      uVar25 = 0;
LAB_14008b301:
      alloc::raw_vec::handle_error(uVar25,ppppppplVar22);
      goto LAB_14008c210;
    }
    if (ppppppplVar22 == (longlong *******)0x0) {
      local_9a8 = (longlong *******)0x0;
      ppppppplStack_9a0 = (longlong *******)0x1;
      local_998 = (longlong *******)0x0;
      goto LAB_14008bb11;
    }
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    uVar25 = 1;
    ppppppplVar11 = (longlong *******)__rustc::__rust_alloc(ppppppplVar22);
    if (ppppppplVar11 == (longlong *******)0x0) goto LAB_14008b301;
    if (0xffffffffffffffe0 < (ulonglong)((longlong)ppppppplVar13 - (longlong)ppppppplVar11) ||
        ppppppplVar22 < &DAT_00000008) {
      ppppppplVar12 = (longlong *******)0x0;
      goto LAB_140089c17;
    }
    if (ppppppplVar22 < &DAT_00000020) {
      ppppppplVar17 = (longlong *******)0x0;
      goto LAB_140089bc5;
    }
    ppppppplVar12 = (longlong *******)((ulonglong)ppppppplVar22 & 0x7fffffffffffffe0);
    ppppppplVar17 = (longlong *******)0x0;
    do {
      auVar2 = *(undefined1 (*) [16])((longlong)ppppppplVar13 + (longlong)ppppppplVar17);
      auVar3 = *(undefined1 (*) [16])((longlong)(ppppppplVar13 + 2) + (longlong)ppppppplVar17);
      auVar28[0] = -(auVar2[0] == '\\');
      auVar28[1] = -(auVar2[1] == '\\');
      auVar28[2] = -(auVar2[2] == '\\');
      auVar28[3] = -(auVar2[3] == '\\');
      auVar28[4] = -(auVar2[4] == '\\');
      auVar28[5] = -(auVar2[5] == '\\');
      auVar28[6] = -(auVar2[6] == '\\');
      auVar28[7] = -(auVar2[7] == '\\');
      auVar28[8] = -(auVar2[8] == '\\');
      auVar28[9] = -(auVar2[9] == '\\');
      auVar28[10] = -(auVar2[10] == '\\');
      auVar28[0xb] = -(auVar2[0xb] == '\\');
      auVar28[0xc] = -(auVar2[0xc] == '\\');
      auVar28[0xd] = -(auVar2[0xd] == '\\');
      auVar28[0xe] = -(auVar2[0xe] == '\\');
      auVar28[0xf] = -(auVar2[0xf] == '\\');
      auVar29[0] = -(auVar3[0] == '\\');
      auVar29[1] = -(auVar3[1] == '\\');
      auVar29[2] = -(auVar3[2] == '\\');
      auVar29[3] = -(auVar3[3] == '\\');
      auVar29[4] = -(auVar3[4] == '\\');
      auVar29[5] = -(auVar3[5] == '\\');
      auVar29[6] = -(auVar3[6] == '\\');
      auVar29[7] = -(auVar3[7] == '\\');
      auVar29[8] = -(auVar3[8] == '\\');
      auVar29[9] = -(auVar3[9] == '\\');
      auVar29[10] = -(auVar3[10] == '\\');
      auVar29[0xb] = -(auVar3[0xb] == '\\');
      auVar29[0xc] = -(auVar3[0xc] == '\\');
      auVar29[0xd] = -(auVar3[0xd] == '\\');
      auVar29[0xe] = -(auVar3[0xe] == '\\');
      auVar29[0xf] = -(auVar3[0xf] == '\\');
      *(undefined1 (*) [16])((longlong)ppppppplVar11 + (longlong)ppppppplVar17) =
           auVar28 & (undefined1  [16])acVar7 | ~auVar28 & auVar2;
      *(undefined1 (*) [16])((longlong)(ppppppplVar11 + 2) + (longlong)ppppppplVar17) =
           auVar29 & (undefined1  [16])acVar7 | ~auVar29 & auVar3;
      ppppppplVar17 = ppppppplVar17 + 4;
    } while (ppppppplVar12 != ppppppplVar17);
    if (ppppppplVar22 != ppppppplVar12) {
      ppppppplVar17 = ppppppplVar12;
      if (((ulonglong)ppppppplVar22 & 0x18) != 0) {
LAB_140089bc5:
        ppppppplVar12 = (longlong *******)((ulonglong)ppppppplVar22 & 0x7ffffffffffffff8);
        do {
          uVar18 = *(ulonglong *)((longlong)ppppppplVar13 + (longlong)ppppppplVar17);
          uVar15 = CONCAT17(-((char)(uVar18 >> 0x38) == '\\'),
                            CONCAT16(-((char)(uVar18 >> 0x30) == '\\'),
                                     CONCAT15(-((char)(uVar18 >> 0x28) == '\\'),
                                              CONCAT14(-((char)(uVar18 >> 0x20) == '\\'),
                                                       CONCAT13(-((char)(uVar18 >> 0x18) == '\\'),
                                                                CONCAT12(-((char)(uVar18 >> 0x10) ==
                                                                          '\\'),CONCAT11(-((char)(
                                                  uVar18 >> 8) == '\\'),-((char)uVar18 == '\\'))))))
                                    ));
          *(ulonglong *)((longlong)ppppppplVar11 + (longlong)ppppppplVar17) =
               uVar15 & 0x2f2f2f2f2f2f2f2f | ~uVar15 & uVar18;
          ppppppplVar17 = ppppppplVar17 + 1;
        } while (ppppppplVar12 != ppppppplVar17);
        if (ppppppplVar22 == ppppppplVar12) goto LAB_140089ca7;
      }
LAB_140089c17:
      ppppppplVar17 = ppppppplVar12;
      for (uVar18 = (ulonglong)ppppppplVar22 & 3; uVar18 != 0; uVar18 = uVar18 - 1) {
        cVar8 = *(char *)((longlong)ppppppplVar13 + (longlong)ppppppplVar17);
        if (cVar8 == '\\') {
          cVar8 = '/';
        }
        *(char *)((longlong)ppppppplVar11 + (longlong)ppppppplVar17) = cVar8;
        ppppppplVar17 = (longlong *******)((longlong)ppppppplVar17 + 1);
      }
      if ((ulonglong)((longlong)ppppppplVar12 - (longlong)ppppppplVar22) < 0xfffffffffffffffd) {
        do {
          cVar8 = *(char *)((longlong)ppppppplVar13 + (longlong)ppppppplVar17);
          if (cVar8 == '\\') {
            cVar8 = '/';
          }
          *(char *)((longlong)ppppppplVar11 + (longlong)ppppppplVar17) = cVar8;
          cVar8 = ((undefined1 *)((longlong)ppppppplVar13 + 1))[(longlong)ppppppplVar17];
          if (cVar8 == '\\') {
            cVar8 = '/';
          }
          ((undefined1 *)((longlong)ppppppplVar11 + 1))[(longlong)ppppppplVar17] = cVar8;
          cVar8 = ((undefined1 *)((longlong)ppppppplVar13 + 2))[(longlong)ppppppplVar17];
          if (cVar8 == '\\') {
            cVar8 = '/';
          }
          ((undefined1 *)((longlong)ppppppplVar11 + 2))[(longlong)ppppppplVar17] = cVar8;
          cVar8 = ((undefined1 *)((longlong)ppppppplVar13 + 3))[(longlong)ppppppplVar17];
          if (cVar8 == '\\') {
            cVar8 = '/';
          }
          ((undefined1 *)((longlong)ppppppplVar11 + 3))[(longlong)ppppppplVar17] = cVar8;
          ppppppplVar17 = (longlong *******)((longlong)ppppppplVar17 + 4);
        } while (ppppppplVar22 != ppppppplVar17);
      }
    }
LAB_140089ca7:
    local_9a8 = ppppppplVar22;
    ppppppplStack_9a0 = ppppppplVar11;
    local_998 = ppppppplVar22;
    local_880 = pcVar10;
    if (*(char *)ppppppplVar11 == '/') {
LAB_14008bb11:
      local_868 = (longlong *******)&local_9a8;
      pppppplStack_860 =
           (longlong ******)
           <alloc::string::String_as_core::fmt::Debug>::fmt;
      alloc::fmt::format::format_inner(&local_958,&DAT_1406a7bd8,&local_868);
      ppppppplVar22 = local_9a8;
      pppppplVar6 = local_958;
    }
    else {
      if (&DAT_0000000f < ppppppplVar22) {
        uVar18 = core::slice::memchr::memchr_aligned
                           (CONCAT71((int7)((ulonglong)ppppppplVar17 >> 8),0x3a),ppppppplVar11,
                            ppppppplVar22);
        if ((uVar18 & 1) == 0) goto LAB_140089d0a;
        goto LAB_14008bb11;
      }
      ppppppplVar13 = (longlong *******)0x0;
      do {
        if (*(char *)((longlong)ppppppplVar11 + (longlong)ppppppplVar13) == ':') goto LAB_14008bb11;
        ppppppplVar13 = (longlong *******)((longlong)ppppppplVar13 + 1);
      } while (ppppppplVar22 != ppppppplVar13);
LAB_140089d0a:
      ppppppplVar13 = (longlong *******)0x0;
      ppppppplVar12 = (longlong *******)0x0;
LAB_140089d3b:
      do {
        bVar27 = true;
        bVar24 = 1;
        ppppppplVar17 = ppppppplVar13;
        do {
          ppppppplVar13 = ppppppplVar17;
          if (ppppppplVar22 < ppppppplVar17) {
LAB_140089dd3:
            lVar9 = (longlong)ppppppplVar22 - (longlong)ppppppplVar12;
            ppppppplVar17 = ppppppplVar12;
            goto joined_r0x000140089dc0;
          }
          puVar20 = (undefined1 *)((longlong)ppppppplVar11 + (longlong)ppppppplVar17);
          ppppppplVar13 = ppppppplVar22;
          if ((undefined1 *)((longlong)ppppppplVar22 - (longlong)ppppppplVar17) < (undefined1 *)0x10
             ) {
            if (ppppppplVar22 == ppppppplVar17) goto LAB_140089dd3;
            puVar19 = (undefined1 *)0x0;
            while (puVar20[(longlong)puVar19] != '/') {
              puVar19 = puVar19 + 1;
              if ((undefined1 *)((longlong)ppppppplVar22 - (longlong)ppppppplVar17) == puVar19)
              goto LAB_140089dd3;
            }
          }
          else {
            uVar18 = core::slice::memchr::memchr_aligned(0x2f);
            puVar19 = puVar20;
            if ((uVar18 & 1) == 0) goto LAB_140089dd3;
          }
          ppppppplVar13 = (longlong *******)((longlong)ppppppplVar17 + (longlong)(puVar19 + 1));
          ppppppplVar21 = (longlong *******)(puVar19 + (longlong)ppppppplVar17);
          ppppppplVar17 = ppppppplVar13;
        } while ((ppppppplVar22 <= ppppppplVar21) ||
                (*(char *)((longlong)ppppppplVar11 + (longlong)ppppppplVar21) != '/'));
        bVar27 = false;
        bVar24 = 0;
        lVar9 = (longlong)ppppppplVar21 - (longlong)ppppppplVar12;
joined_r0x000140089dc0:
        if (lVar9 == 2) {
          bVar27 = *(short *)((longlong)ppppppplVar11 + (longlong)ppppppplVar12) != 0x2e2e;
          ppppppplVar12 = ppppppplVar17;
          if (!(bool)(~bVar24 & bVar27)) {
            if (!bVar27) goto LAB_14008bb11;
            break;
          }
          goto LAB_140089d3b;
        }
        if (lVar9 == 0) goto LAB_14008bb11;
        ppppppplVar12 = ppppppplVar17;
      } while (!bVar27);
      std::sys::fs::canonicalize(&local_868,local_8b8,param_5);
      if (local_868 != (longlong *******)0xffffffffffffffff) {
        <std::path::Path>::_join
                  (&local_868,pppppplStack_860,local_858,ppppppplVar11,ppppppplVar22);
        __rustc::__rust_dealloc(ppppppplVar11,ppppppplVar22,1);
        return;
      }
      local_918 = pppppplStack_860;
      local_9e8 = &local_918;
      ppppppplStack_9e0 =
           (longlong *******)
           <core::io::error::Error_as_core::fmt::Display>::fmt;
      alloc::fmt::format::format_inner(&local_2f8,&DAT_1406a55e6,&local_9e8);
      pppppplVar6 = local_2f8;
      local_950 = local_2f0;
      local_948 = local_2e8;
      if ((1 < ((uint)local_918 & 3) - 2) && (((ulonglong)local_918 & 3) != 0)) {
        ppppppplStack_9e0 = (longlong *******)((longlong)local_918 + -1);
        local_9e8 = (longlong *******)CONCAT71(local_9e8._1_7_,3);
        <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
        drop(&ppppppplStack_9e0);
        pppppplVar6 = local_2f8;
        local_950 = local_2f0;
        local_948 = local_2e8;
      }
    }
    if (ppppppplVar22 != (longlong *******)0x0) {
      __rustc::__rust_dealloc(ppppppplStack_9a0,ppppppplVar22,1);
      return;
    }
    param_1[1] = pppppplVar6;
    param_1[2] = local_950;
    param_1[3] = local_948;
    goto LAB_14008bb98;
  }
  param_1[3] = local_590;
  param_1[1] = CONCAT71(uStack_59f,uStack_5a0);
  param_1[2] = CONCAT71(uStack_597,uStack_598);
  *param_1 = 1;
  if (local_568._0_8_ != 0) {
    __rustc::__rust_dealloc(uVar25,local_568._0_8_,1);
    return;
  }
  goto LAB_14008bbac;
LAB_14008b1bd:
  std::sys::fs::read_dir(local_568,param_6,param_7);
  local_868 = (longlong *******)local_568._0_8_;
  pppppplStack_860 = (longlong ******)local_568._8_8_;
  if ((longlong *******)local_568._0_8_ == (longlong *******)0x2) {
    local_958 = (longlong ******)local_568._8_8_;
    local_868 = &local_958;
    pppppplStack_860 =
         (longlong ******)
         <core::io::error::Error_as_core::fmt::Display>::fmt;
    alloc::fmt::format::format_inner(local_568,&DAT_1406a56c1,&local_868);
    if ((1 < ((uint)local_958 & 3) - 2) && (((ulonglong)local_958 & 3) != 0)) {
      pppppplStack_860 = (longlong ******)((longlong)local_958 + -1);
      local_868 = (longlong *******)CONCAT71(local_868._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&pppppplStack_860);
    }
    local_938 = (longlong *******)local_568._8_8_;
    ppppppplStack_930 = (longlong *******)auStack_558;
    local_8a8 = (longlong *******)local_568._8_8_;
    ppppppplStack_8a0 = (longlong *******)auStack_558;
    ppppppplVar13 = (longlong *******)local_568._0_8_;
  }
  else {
    local_9e8 = (longlong *******)auStack_558;
    ppppppplStack_9e0 = ppppppplStack_550;
    memcpy(local_848,local_548,0x250);
    local_938 = local_9e8;
    ppppppplStack_930 = ppppppplStack_9e0;
    local_858 = local_9e8;
    ppppppplStack_850 = ppppppplStack_9e0;
    while( true ) {
      <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
      ::next(&local_2f8,&local_868);
      uVar18 = local_2e8;
      ppppppplVar13 = local_2f0;
      if ((int)local_2f8 != 1) {
        if (local_868 != (longlong *******)0x0) {
          <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
          ::drop(&pppppplStack_860);
        }
        LOCK();
        *local_858 = (longlong ******)((longlong)*local_858 + -1);
        UNLOCK();
        if (*local_858 == (longlong ******)0x0) {
          <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                    (&local_858);
        }
        goto LAB_14008b705;
      }
      if (local_2f0 == (longlong *******)0x0) break;
      memcpy(local_548,local_2d0,0x238);
      local_918 = local_2e0;
      uStack_910 = uStack_2d8;
      ppppppplStack_550 = (longlong *******)uStack_2d8;
      auStack_558 = (undefined1  [8])local_2e0;
      local_568._8_8_ = uVar18;
      local_568._0_8_ = ppppppplVar13;
      <std::fs::DirEntry>::path(&local_958,local_568);
      LOCK();
      *(longlong *)local_568._0_8_ = *(longlong *)local_568._0_8_ + -1;
      UNLOCK();
      if (*(longlong *)local_568._0_8_ == 0) {
        <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                  (local_568);
      }
      uVar18 = local_948;
      ppppppplVar13 = local_950;
      uVar15 = local_948;
      pbVar14 = (byte *)<std::path::Path>::extension(local_950);
      if (((((uVar15 == 7 && pbVar14 != (byte *)0x0) &&
            ((byte)(((byte)(*pbVar14 + 0xbf) < 0x1a) << 5 | *pbVar14) == 0x6d)) &&
           ((byte)(((byte)(pbVar14[1] + 0xbf) < 0x1a) << 5 | pbVar14[1]) == 0x61)) &&
          (((byte)(((byte)(pbVar14[2] + 0xbf) < 0x1a) << 5 | pbVar14[2]) == 0x70 &&
           ((byte)(((byte)(pbVar14[3] + 0xbf) < 0x1a) << 5 | pbVar14[3]) == 0x69)))) &&
         (((byte)(((byte)(pbVar14[4] + 0xbf) < 0x1a) << 5 | pbVar14[4]) == 0x6e &&
          (((byte)(((byte)(pbVar14[5] + 0xbf) < 0x1a) << 5 | pbVar14[5]) == 0x66 &&
           ((byte)(((byte)(pbVar14[6] + 0xbf) < 0x1a) << 5 | pbVar14[6]) == 0x6f)))))) {
        local_528 = (undefined1  [16])0x0;
        local_538 = (undefined1  [16])0x0;
        _local_548 = (undefined1  [16])0x0;
        _auStack_558 = (undefined1  [16])0x0;
        local_568 = (undefined1  [16])0x0;
        local_9e8 = (longlong *******)((ulonglong)local_9e8 & 0xffffffff00000000);
        ppppppplStack_9e0 = (longlong *******)0x0;
        local_9d8 = 7;
        uStack_9cc = 0;
        uStack_9d4 = 0;
        local_9d0 = 1;
        ppppppplVar22 = ppppppplVar13;
        uVar15 = <std::fs::OpenOptions>::_open(&local_9e8,ppppppplVar13,uVar18);
        if ((uVar15 & 1) == 0) {
          local_9e8 = ppppppplVar22;
          local_8b8 = (longlong *******)
                      alloc::io::read::default_read_exact::
                      <std::fs::File>(&local_9e8,local_568,0x50);
          CloseHandle(local_9e8);
          ppppppplVar22 = local_8b8;
          if (local_8b8 == (longlong *******)0x0) {
            if ((local_568._0_4_ == 0x666c625f) && (local_538._0_4_ == 0x7363636d)) {
              if ((local_528._4_8_ == 0x52520001 && local_528._12_4_ == 0) &&
                 (lVar9 = std::sys::fs::remove_file(ppppppplVar13,uVar18),
                 lVar9 != 0)) {
                clear_generated::_closure_4_(&local_9e8,ppppppplVar13,uVar18,lVar9);
                ppppppplStack_8a0 = (longlong *******)CONCAT44(uStack_9d4,local_9d8);
                local_8a8 = ppppppplStack_9e0;
                if (local_958 != (longlong ******)0x0) {
                  __rustc::__rust_dealloc(ppppppplVar13,local_958,1);
                  return;
                }
                local_958 = (longlong ******)0x0;
                ppppppplVar13 = local_9e8;
                goto joined_r0x00014008ba61;
              }
            }
            goto LAB_14008b650;
          }
        }
        if ((1 < ((uint)ppppppplVar22 & 3) - 2) && (((ulonglong)ppppppplVar22 & 3) != 0)) {
          ppppppplStack_9e0 = (longlong *******)((longlong)ppppppplVar22 + -1);
          local_9e8 = (longlong *******)CONCAT71(local_9e8._1_7_,3);
          <core::io::error::CustomOwner_as_core::ops::drop::Drop>
          ::drop(&ppppppplStack_9e0);
        }
      }
LAB_14008b650:
      if (local_958 != (longlong ******)0x0) {
        __rustc::__rust_dealloc(ppppppplVar13,local_958,1);
        return;
      }
    }
    local_898 = local_2e8;
    local_8d8 = (longlong *******)0x0;
    local_8d0 = (longlong *******)0x1;
    ppppppplStack_8c8 = (longlong *******)0x0;
    local_9d8 = 0x60000020;
    uStack_9d4 = 0;
    local_9e8 = (longlong *******)&local_8d8;
    ppppppplStack_9e0 = (longlong *******)&PTR_<alloc::string::String>_1406a9370;
    cVar8 = <core::io::error::Error_as_core::fmt::Display>::fmt
                      (&local_898,&local_9e8);
    ppppppplVar13 = local_8d8;
    if (cVar8 != '\0') {
      core::result::unwrap_failed
                ("a Display implementation returned an error unexpectedly",0x37,local_979,
                 &DAT_1406a5ba8,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_1406a9428);
LAB_14008c210:
                    /* WARNING: Does not return */
      pcVar5 = (code *)invalidInstructionException();
      (*pcVar5)();
    }
    local_9a8 = local_8d0;
    ppppppplStack_9a0 = ppppppplStack_8c8;
    local_8a8 = local_8d0;
    ppppppplStack_8a0 = ppppppplStack_8c8;
    if ((1 < ((uint)local_2e8 & 3) - 2) && ((local_2e8 & 3) != 0)) {
      ppppppplStack_9e0 = (longlong *******)(local_2e8 - 1);
      local_9e8 = (longlong *******)CONCAT71(local_9e8._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&ppppppplStack_9e0);
      local_8a8 = local_9a8;
      ppppppplStack_8a0 = ppppppplStack_9a0;
    }
joined_r0x00014008ba61:
    if (local_868 != (longlong *******)0x0) {
      <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
      ::drop(&pppppplStack_860);
    }
    LOCK();
    *local_858 = (longlong ******)((longlong)*local_858 + -1);
    UNLOCK();
    if (*local_858 == (longlong ******)0x0) {
      <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                (&local_858);
    }
  }
  if (ppppppplVar13 == (longlong *******)0xffffffffffffffff) {
LAB_14008b705:
    if (local_960 != 0) {
      lVar9 = local_960 * 0x48;
      lVar23 = lStack_968 + 0x40;
      do {
        lVar1 = lVar23 + -0x40;
        local_2f8 = (longlong ******)CONCAT44(local_2f8._4_4_,*(undefined4 *)(lVar23 + -0x10));
        lVar16 = ships_as(*(undefined8 *)(lVar23 + -0x38),*(undefined8 *)(lVar23 + -0x30));
        if (lVar16 == 0) {
          local_568._8_8_ = <u32_as_core::fmt::LowerHex>::fmt;
          local_568._0_8_ = &local_2f8;
          alloc::fmt::format::format_inner(&local_5c0,&DAT_1406a7c2f,local_568);
        }
        else {
          local_568._8_8_ =
               <alloc::string::String_as_core::fmt::Display>::fmt
          ;
          ppppppplStack_550 =
               (longlong *******)
               <host_probe::modmaps::MapKind_as_core::fmt::Debug>
               ::fmt;
          auStack_558 = (undefined1  [8])lVar23;
          pcStack_540 = <u32_as_core::fmt::LowerHex>::fmt;
          local_548 = (undefined1  [8])&local_2f8;
          local_568._0_8_ = lVar1;
          alloc::fmt::format::format_inner(&local_868,&DAT_1406a7c48,local_568);
          pppppplVar6 = pppppplStack_860;
          ppppppplVar13 = local_868;
          host::log(pppppplStack_860,local_858);
          if (ppppppplVar13 != (longlong *******)0x0) {
            __rustc::__rust_dealloc(pppppplVar6,ppppppplVar13,1);
            return;
          }
          local_568._8_8_ =
               <alloc::string::String_as_core::fmt::Display>::fmt
          ;
          local_568._0_8_ = lVar1;
          alloc::fmt::format::format_inner(&local_5c0,&DAT_1406a7c86,local_568);
        }
        uVar25 = local_5b8;
        <std::path::Path>::_join(local_568,param_6,param_7,local_5b8,local_5b0);
        if (local_5c0 != 0) {
          __rustc::__rust_dealloc(uVar25,local_5c0,1);
          return;
        }
        uVar25 = local_568._8_8_;
        lVar16 = std::fs::write::inner
                           (local_568._8_8_,auStack_558,*(undefined8 *)(lVar23 + -0x20),
                            *(undefined8 *)(lVar23 + -0x18));
        if (local_568._0_8_ != 0) {
          __rustc::__rust_dealloc(uVar25,local_568._0_8_,1);
          return;
        }
        if (lVar16 != 0) {
          prepare_files::_closure_7_(local_568,lVar1,lVar16);
          param_1[3] = auStack_558;
          param_1[1] = local_568._0_8_;
          param_1[2] = local_568._8_8_;
          *param_1 = 1;
          goto LAB_14008bb9f;
        }
        lVar23 = lVar23 + 0x48;
        lVar9 = lVar9 + -0x48;
      } while (lVar9 != 0);
    }
    param_1[3] = local_960;
    param_1[1] = local_970;
    param_1[2] = lStack_968;
    *param_1 = 0;
    core::ptr::drop_glue::<serde_json::value::Value>(local_5f8);
    return;
  }
  param_1[2] = local_8a8;
  param_1[3] = ppppppplStack_8a0;
  param_1[1] = ppppppplVar13;
LAB_14008bb98:
  *param_1 = 1;
LAB_14008bb9f:
  core::ptr::drop_glue::<serde_json::value::Value>(local_5f8);
LAB_14008bbac:
  if (local_960 != 0) {
    puVar26 = (undefined8 *)(lStack_968 + 0x20);
    lVar9 = local_960;
    do {
      if (puVar26[-4] != 0) {
        __rustc::__rust_dealloc(puVar26[-3],puVar26[-4],1);
        return;
      }
      if (puVar26[-1] != 0) {
        __rustc::__rust_dealloc(*puVar26,puVar26[-1],1);
        return;
      }
      puVar26 = puVar26 + 9;
      lVar9 = lVar9 + -1;
    } while (lVar9 != 0);
  }
  if (local_970 == 0) {
                    /* WARNING: Read-only address (ram,0x0001406a4b40) is written */
    return;
  }
  __rustc::__rust_dealloc(lStack_968,local_970 * 0x48,8);
  return;
}


