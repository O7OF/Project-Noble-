
/* WARNING: Function: ___chkstk_ms replaced with injection: alloca_probe */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::modlist::scan */

undefined8 __rustcall
menu_overlay::modlist::scan
          (undefined8 param_1,longlong param_2,undefined8 param_3)

{
  longlong lVar1;
  undefined1 auVar2 [16];
  undefined1 auVar3 [16];
  undefined1 auVar4 [16];
  code *pcVar5;
  uint uVar6;
  uint uVar7;
  char acVar8 [16];
  longlong lVar9;
  longlong *plVar10;
  char cVar11;
  int iVar12;
  undefined1 (*pauVar13) [16];
  longlong lVar14;
  longlong *plVar15;
  void *pvVar16;
  undefined1 (**_Dst) [16];
  void *_Buf2;
  longlong *plVar17;
  longlong *plVar18;
  undefined1 (**ppauVar19) [16];
  ulonglong uVar20;
  DWORD DVar21;
  longlong lVar22;
  undefined1 (*pauVar23) [16];
  undefined8 *puVar24;
  undefined1 (*pauVar25) [16];
  undefined *puVar26;
  longlong lVar27;
  undefined8 uVar28;
  undefined1 (*pauVar29) [16];
  ushort uVar30;
  uint uVar31;
  longlong lVar32;
  ulonglong uVar33;
  undefined1 auVar34 [16];
  undefined1 auVar35 [16];
  undefined1 local_16e1;
  longlong *local_16e0;
  longlong *local_16d8;
  undefined1 (*local_16d0) [16];
  longlong *local_16c8;
  ulonglong local_16c0;
  longlong lStack_16b8;
  ulonglong local_16b0;
  undefined1 (**local_16a8) [16];
  longlong *local_16a0;
  ulonglong local_1698;
  longlong lStack_1690;
  ulonglong local_1688;
  undefined1 (*local_1678) [16];
  longlong local_1670;
  longlong *local_1668;
  undefined8 local_1660;
  longlong local_1658;
  undefined8 local_1650;
  longlong local_1648;
  undefined8 local_1640;
  longlong *local_1638;
  undefined8 uStack_1630;
  undefined1 (*local_1618) [16];
  undefined1 (*pauStack_1610) [16];
  longlong *plStack_1608;
  longlong lStack_1600;
  longlong local_15f8;
  longlong lStack_15f0;
  undefined1 (*local_13a8) [16];
  longlong *plStack_13a0;
  undefined8 local_1398 [76];
  undefined1 (*local_1138) [16];
  undefined1 (*pauStack_1130) [16];
  longlong *local_1128;
  longlong lStack_1120;
  longlong local_1118;
  longlong lStack_1110;
  undefined8 local_ed8;
  undefined1 (*local_c78) [16];
  undefined1 local_c70 [8];
  longlong *local_c68 [76];
  longlong local_a08;
  undefined1 local_a00 [8];
  longlong *local_9f8 [76];
  undefined1 (*local_798) [16];
  longlong *local_790;
  undefined1 local_788 [608];
  undefined1 (*local_528) [16];
  undefined1 (*pauStack_520) [16];
  longlong *local_518;
  longlong lStack_510;
  longlong local_508;
  longlong lStack_500;
  longlong local_2e0;
  undefined8 local_2d8;
  undefined8 local_2d0;
  undefined8 uStack_48;
  
  uStack_48 = 0x1400fcf36;
  local_16c0 = 0;
  lStack_16b8 = 8;
  local_16b0 = 0;
  workshop::libraries(&local_1138);
  auVar4 = _s__________1401c91c0;
  acVar8 = s__________________1401c91a0;
  plVar17 = (longlong *)((longlong)pauStack_1130 + (longlong)local_1128 * 4 * 8);
  local_16e0 = (longlong *)pauStack_1130;
  local_16d0 = local_1138;
  pauVar25 = pauStack_1130;
  pauVar23 = local_1138;
  pauVar29 = pauStack_1130;
  local_16c8 = plVar17;
  local_1650 = param_1;
  if (local_1128 == (longlong *)0x0) {
LAB_1400fdb10:
    local_1668 = (longlong *)pauVar25;
    if ((longlong)plVar17 - (longlong)pauVar29 != 0) {
      uVar20 = (ulonglong)((longlong)plVar17 - (longlong)pauVar29) >> 5;
      plVar17 = (longlong *)((longlong)pauVar29 + 8);
      do {
        if (plVar17[-1] != 0) {
          __rustc::__rust_dealloc(*plVar17,plVar17[-1],1);
        }
        plVar17 = plVar17 + 4;
        uVar20 = uVar20 - 1;
      } while (uVar20 != 0);
    }
  }
  else {
    local_1678 = local_1138;
    pauVar13 = pauStack_1130;
    local_1658 = param_2;
    do {
      pauVar29 = (undefined1 (*) [16])((longlong)pauVar13 + 0x20);
      lVar27 = *(longlong *)pauVar13;
      pauVar23 = local_1678;
      local_16d8 = (longlong *)pauVar29;
      if (lVar27 == -1) goto LAB_1400fdb10;
      local_1670 = *(longlong *)((longlong)pauVar13 + 8);
      <core::wtf8::Wtf8>::to_string_lossy
                (&local_1138,local_1670,*(longlong *)((longlong)pauVar13 + 0x10));
      plVar10 = local_1128;
      pauVar23 = pauStack_1130;
      if ((longlong)local_1128 < 0) {
        uVar28 = 0;
LAB_1400fdac7:
        alloc::raw_vec::handle_error(uVar28,plVar10);
        goto LAB_1400fe0e1;
      }
      local_1668 = (longlong *)pauVar25;
      local_1660 = param_3;
      if (local_1128 == (longlong *)0x0) {
        lVar14 = 1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar28 = 1;
        lVar14 = __rustc::__rust_alloc(plVar10,1);
        if (lVar14 == 0) goto LAB_1400fdac7;
        if ((ulonglong)((longlong)pauVar23 - lVar14) < 0xffffffffffffffe1 &&
            (longlong *)0x7 < plVar10) {
          if (plVar10 < (longlong *)0x20) {
            plVar18 = (longlong *)0x0;
          }
          else {
            plVar15 = (longlong *)((ulonglong)plVar10 & 0x7fffffffffffffe0);
            plVar18 = (longlong *)0x0;
            do {
              auVar2 = *(undefined1 (*) [16])((longlong)pauVar23 + (longlong)plVar18);
              auVar3 = *(undefined1 (*) [16])((longlong)pauVar23 + (longlong)(plVar18 + 2));
              auVar34[0] = -(auVar2[0] == '/');
              auVar34[1] = -(auVar2[1] == '/');
              auVar34[2] = -(auVar2[2] == '/');
              auVar34[3] = -(auVar2[3] == '/');
              auVar34[4] = -(auVar2[4] == '/');
              auVar34[5] = -(auVar2[5] == '/');
              auVar34[6] = -(auVar2[6] == '/');
              auVar34[7] = -(auVar2[7] == '/');
              auVar34[8] = -(auVar2[8] == '/');
              auVar34[9] = -(auVar2[9] == '/');
              auVar34[10] = -(auVar2[10] == '/');
              auVar34[0xb] = -(auVar2[0xb] == '/');
              auVar34[0xc] = -(auVar2[0xc] == '/');
              auVar34[0xd] = -(auVar2[0xd] == '/');
              auVar34[0xe] = -(auVar2[0xe] == '/');
              auVar34[0xf] = -(auVar2[0xf] == '/');
              auVar35[0] = -(auVar3[0] == '/');
              auVar35[1] = -(auVar3[1] == '/');
              auVar35[2] = -(auVar3[2] == '/');
              auVar35[3] = -(auVar3[3] == '/');
              auVar35[4] = -(auVar3[4] == '/');
              auVar35[5] = -(auVar3[5] == '/');
              auVar35[6] = -(auVar3[6] == '/');
              auVar35[7] = -(auVar3[7] == '/');
              auVar35[8] = -(auVar3[8] == '/');
              auVar35[9] = -(auVar3[9] == '/');
              auVar35[10] = -(auVar3[10] == '/');
              auVar35[0xb] = -(auVar3[0xb] == '/');
              auVar35[0xc] = -(auVar3[0xc] == '/');
              auVar35[0xd] = -(auVar3[0xd] == '/');
              auVar35[0xe] = -(auVar3[0xe] == '/');
              auVar35[0xf] = -(auVar3[0xf] == '/');
              *(undefined1 (*) [16])(lVar14 + (longlong)plVar18) =
                   auVar34 & (undefined1  [16])acVar8 | ~auVar34 & auVar2;
              *(undefined1 (*) [16])(lVar14 + 0x10 + (longlong)plVar18) =
                   auVar35 & (undefined1  [16])acVar8 | ~auVar35 & auVar3;
              plVar18 = plVar18 + 4;
            } while (plVar15 != plVar18);
            if (plVar10 == plVar15) goto LAB_1400fd257;
            plVar18 = plVar15;
            if (((ulonglong)plVar10 & 0x18) == 0) goto LAB_1400fd1c6;
          }
          plVar15 = (longlong *)((ulonglong)plVar10 & 0x7ffffffffffffff8);
          do {
            uVar20 = *(ulonglong *)((longlong)pauVar23 + (longlong)plVar18);
            uVar33 = CONCAT17(-((char)(uVar20 >> 0x38) == '/'),
                              CONCAT16(-((char)(uVar20 >> 0x30) == '/'),
                                       CONCAT15(-((char)(uVar20 >> 0x28) == '/'),
                                                CONCAT14(-((char)(uVar20 >> 0x20) == '/'),
                                                         CONCAT13(-((char)(uVar20 >> 0x18) == '/'),
                                                                  CONCAT12(-((char)(uVar20 >> 0x10)
                                                                            == '/'),CONCAT11(-((char
                                                  )(uVar20 >> 8) == '/'),-((char)uVar20 == '/'))))))
                                      ));
            *(ulonglong *)(lVar14 + (longlong)plVar18) = uVar33 & auVar4._0_8_ | ~uVar33 & uVar20;
            plVar18 = plVar18 + 1;
          } while (plVar15 != plVar18);
          if (plVar10 == plVar15) goto LAB_1400fd257;
        }
        else {
          plVar15 = (longlong *)0x0;
        }
LAB_1400fd1c6:
        plVar18 = plVar15;
        for (uVar20 = (ulonglong)plVar10 & 3; uVar20 != 0; uVar20 = uVar20 - 1) {
          cVar11 = *(char *)((longlong)pauVar23 + (longlong)plVar18);
          if (*(char *)((longlong)pauVar23 + (longlong)plVar18) == '/') {
            cVar11 = '\\';
          }
          *(char *)(lVar14 + (longlong)plVar18) = cVar11;
          plVar18 = (longlong *)((longlong)plVar18 + 1);
        }
        if ((ulonglong)((longlong)plVar15 - (longlong)plVar10) < 0xfffffffffffffffd) {
          do {
            cVar11 = *(char *)((longlong)pauVar23 + (longlong)plVar18);
            if (*(char *)((longlong)pauVar23 + (longlong)plVar18) == '/') {
              cVar11 = '\\';
            }
            *(char *)(lVar14 + (longlong)plVar18) = cVar11;
            cVar11 = *(char *)((longlong)pauVar23 + (longlong)plVar18 + 1U);
            if (cVar11 == '/') {
              cVar11 = '\\';
            }
            *(char *)(lVar14 + 1 + (longlong)plVar18) = cVar11;
            cVar11 = *(char *)((longlong)pauVar23 + (longlong)plVar18 + 2U);
            if (cVar11 == '/') {
              cVar11 = '\\';
            }
            *(char *)(lVar14 + 2 + (longlong)plVar18) = cVar11;
            cVar11 = *(char *)((longlong)pauVar23 + (longlong)plVar18 + 3U);
            if (cVar11 == '/') {
              cVar11 = '\\';
            }
            *(char *)(lVar14 + 3 + (longlong)plVar18) = cVar11;
            plVar18 = (longlong *)((longlong)plVar18 + 4);
          } while (plVar10 != plVar18);
        }
      }
LAB_1400fd257:
      if (0 < (longlong)local_1138) {
        __rustc::__rust_dealloc(pauVar23,local_1138,1);
      }
      <std::path::Path>::_join(&local_1138,lVar14,plVar10,"steamapps",9);
      local_16a0 = (longlong *)pauStack_1130;
      <std::path::Path>::_join(&local_13a8,pauStack_1130,local_1128,"workshop",8);
      plVar18 = plStack_13a0;
      <std::path::Path>::_join(&local_1618,plStack_13a0,local_1398[0],"content",7)
      ;
      plVar15 = plStack_1608;
      pauVar25 = pauStack_1610;
      ppauVar19 = &local_528;
      pvVar16 = (void *)<u32>::_fmt(0xee75a,&local_528,10);
      if ((longlong)ppauVar19 < 0) {
        uVar28 = 0;
        ppauVar19 = local_16a8;
LAB_1400fdadf:
        local_16a8 = ppauVar19;
        alloc::raw_vec::handle_error(uVar28,local_16a8);
        goto LAB_1400fe0e1;
      }
      if (ppauVar19 == (undefined1 (**) [16])0x0) {
        _Dst = (undefined1 (**) [16])0x1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar28 = 1;
        _Dst = (undefined1 (**) [16])__rustc::__rust_alloc(ppauVar19,1);
        if (_Dst == (undefined1 (**) [16])0x0) goto LAB_1400fdadf;
        memcpy(_Dst,pvVar16,(size_t)ppauVar19);
      }
      local_16a8 = _Dst;
      <std::path::Path>::_join(&local_1698,pauVar25,plVar15,_Dst,ppauVar19);
      if (ppauVar19 != (undefined1 (**) [16])0x0) {
        __rustc::__rust_dealloc(local_16a8,ppauVar19,1);
      }
      if (local_1618 != (undefined1 (*) [16])0x0) {
        __rustc::__rust_dealloc(pauVar25,local_1618,1);
      }
      if (local_13a8 != (undefined1 (*) [16])0x0) {
        __rustc::__rust_dealloc(plVar18,local_13a8,1);
      }
      if (local_1138 != (undefined1 (*) [16])0x0) {
        __rustc::__rust_dealloc(local_16a0,local_1138,1);
      }
      lVar9 = lStack_1690;
      std::sys::fs::read_dir(&local_1138,lStack_1690,local_1688);
      pauVar23 = pauStack_1130;
      pauVar25 = local_1138;
      if (local_1138 == (undefined1 (*) [16])0x2) {
        local_13a8 = (undefined1 (*) [16])0x2;
        plStack_13a0 = (longlong *)pauStack_1130;
        core::ptr::drop_glue::
        <core::result::Result<std::fs::ReadDir,core::io::error::Error>>
                  (&local_13a8);
      }
      else {
        memcpy(local_1398,&local_1128,0x260);
        local_13a8 = pauVar25;
        plStack_13a0 = (longlong *)pauVar23;
        memcpy(&local_1618,&plStack_13a0,0x268);
      }
      memcpy(&local_790,&local_1618,0x268);
      local_1138 = (undefined1 (*) [16])0x0;
      local_ed8 = 0;
      local_c78 = (undefined1 (*) [16])0x2;
      local_a08 = 2;
      local_798 = pauVar25;
      do {
        if (local_c78 == (undefined1 (*) [16])0x2) {
LAB_1400fd64e:
          do {
            ppauVar19 = &local_528;
            if (local_798 != (undefined1 (*) [16])0xffffffffffffffff) {
              local_528 = local_798;
              memcpy(&local_13a8,&local_790,0x268);
              ppauVar19 = &local_798;
            }
            *ppauVar19 = (undefined1 (*) [16])0x2;
            if (local_528 == (undefined1 (*) [16])0x2) {
              if ((int)local_a08 == 2) goto LAB_1400fda30;
              <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
              ::next(&local_1618,&local_a08);
              plVar15 = plStack_1608;
              pauVar23 = pauStack_1610;
              if (local_1618 != (undefined1 (*) [16])0x0) {
                memcpy(&local_2e0,&lStack_1600,0x248);
                goto LAB_1400fd820;
              }
              if (local_a08 == 0) {
LAB_1400fd7b7:
                LOCK();
                *local_9f8[0] = *local_9f8[0] + -1;
                UNLOCK();
                if (*local_9f8[0] == 0) {
                  <alloc::sync::Arc<std::path::PathBuf>>::
                  drop_slow(local_9f8);
                }
              }
              else if ((int)local_a08 != 2) {
                <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
                ::drop(local_a00);
                goto LAB_1400fd7b7;
              }
              plVar15 = plStack_1608;
              pauVar23 = pauStack_1610;
              pauVar25 = local_1618;
              local_a08 = 2;
              memcpy(&local_2e0,&lStack_1600,0x248);
              if (((ulonglong)pauVar25 & 1) == 0) goto LAB_1400fda30;
              goto LAB_1400fd820;
            }
            local_c78 = local_528;
            memcpy(local_c70,&local_13a8,0x268);
            <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
            ::next(&local_1618,&local_c78);
            if (local_1618 != (undefined1 (*) [16])0x0) goto LAB_1400fd55b;
            if (local_c78 == (undefined1 (*) [16])0x0) {
LAB_1400fd70d:
              LOCK();
              *local_c68[0] = *local_c68[0] + -1;
              UNLOCK();
              if (*local_c68[0] == 0) {
                <alloc::sync::Arc<std::path::PathBuf>>::
                drop_slow(local_c68);
              }
            }
            else if ((int)local_c78 != 2) {
              <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
              ::drop(local_c70);
              goto LAB_1400fd70d;
            }
            plVar15 = plStack_1608;
            pauVar23 = pauStack_1610;
            pauVar25 = local_1618;
            local_c78 = (undefined1 (*) [16])0x2;
            memcpy(&local_528,&lStack_1600,0x248);
          } while (pauVar25 != (undefined1 (*) [16])0x1);
        }
        else {
          <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
          ::next(&local_1618,&local_c78);
          if ((char)local_1618 == '\0') {
            if (local_c78 == (undefined1 (*) [16])0x0) {
LAB_1400fd5b5:
              LOCK();
              *local_c68[0] = *local_c68[0] + -1;
              UNLOCK();
              if (*local_c68[0] == 0) {
                <alloc::sync::Arc<std::path::PathBuf>>::
                drop_slow(local_c68);
              }
            }
            else if ((int)local_c78 != 2) {
              <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
              ::drop(local_c70);
              goto LAB_1400fd5b5;
            }
            plVar15 = plStack_1608;
            pauVar23 = pauStack_1610;
            pauVar25 = local_1618;
            local_c78 = (undefined1 (*) [16])0x2;
            memcpy(&local_528,&lStack_1600,0x248);
            if (((ulonglong)pauVar25 & 1) == 0) goto LAB_1400fd64e;
          }
          else {
LAB_1400fd55b:
            plVar15 = plStack_1608;
            pauVar23 = pauStack_1610;
            memcpy(&local_528,&lStack_1600,0x248);
          }
        }
        memcpy(&local_2e0,&local_528,0x248);
LAB_1400fd820:
        if (pauVar23 == (undefined1 (*) [16])0x0) {
          if ((1 < ((uint)plVar15 & 3) - 2) && (((ulonglong)plVar15 & 3) != 0)) {
            plStack_13a0 = (longlong *)((longlong)plVar15 + -1);
            local_13a8 = (undefined1 (*) [16])CONCAT71(local_13a8._1_7_,3);
            <core::io::error::CustomOwner_as_core::ops::drop::Drop>
            ::drop(&plStack_13a0);
          }
        }
        else {
          memcpy(local_1398,&local_2e0,0x248);
          local_13a8 = pauVar23;
          plStack_13a0 = plVar15;
          <std::fs::DirEntry>::path(&local_1648,&local_13a8);
          uVar28 = local_1640;
          local_16a0 = local_1638;
          cVar11 = <std::path::Path>::is_dir();
          lVar1 = lStack_16b8;
          if (cVar11 == '\0') {
LAB_1400fd95a:
            if (local_1648 != 0) {
              __rustc::__rust_dealloc(uVar28,local_1648,1);
            }
            LOCK();
            *(longlong *)*local_13a8 = *(longlong *)*local_13a8 + -1;
            lVar1 = *(longlong *)*local_13a8;
            UNLOCK();
          }
          else {
            if (local_16b0 == 0) {
              uVar20 = 0;
            }
            else {
              lVar32 = local_16b0 << 5;
              lVar22 = 0;
              do {
                plVar15 = *(longlong **)(lVar1 + 0x10 + lVar22);
                pvVar16 = (void *)<std::path::Path>::file_name
                                            (*(undefined8 *)(lVar1 + 8 + lVar22));
                plVar18 = local_16a0;
                _Buf2 = (void *)<std::path::Path>::file_name(uVar28);
                if (pvVar16 == (void *)0x0) {
                  if (_Buf2 == (void *)0x0) goto LAB_1400fd95a;
                }
                else if (((_Buf2 != (void *)0x0) && (plVar15 == plVar18)) &&
                        (iVar12 = memcmp(pvVar16,_Buf2,(size_t)plVar15), iVar12 == 0))
                goto LAB_1400fd95a;
                lVar22 = lVar22 + 0x20;
                uVar20 = local_16b0;
              } while (lVar32 != lVar22);
            }
            lVar1 = local_1648;
            if (uVar20 == local_16c0) {
              <alloc::raw_vec::RawVec<std::path::PathBuf>>::
              grow_one(&local_16c0);
            }
            lVar22 = uVar20 * 0x20;
            *(longlong *)(lStack_16b8 + lVar22) = lVar1;
            *(undefined8 *)(lStack_16b8 + 8 + lVar22) = uVar28;
            puVar24 = (undefined8 *)(lStack_16b8 + 0x10 + lVar22);
            *puVar24 = local_1638;
            puVar24[1] = uStack_1630;
            local_16b0 = uVar20 + 1;
            LOCK();
            *(longlong *)*local_13a8 = *(longlong *)*local_13a8 + -1;
            lVar1 = *(longlong *)*local_13a8;
            UNLOCK();
          }
          if (lVar1 == 0) {
            <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                      (&local_13a8);
          }
        }
      } while (local_c78 != (undefined1 (*) [16])0xffffffffffffffff);
LAB_1400fda30:
      core::ptr::drop_glue::
      <core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>>
                (&local_1138);
      if (local_1698 != 0) {
        __rustc::__rust_dealloc(lVar9,local_1698,1);
      }
      param_2 = local_1658;
      if (plVar10 != (longlong *)0x0) {
        __rustc::__rust_dealloc(lVar14,plVar10,1);
      }
      if (lVar27 != 0) {
        __rustc::__rust_dealloc(local_1670,lVar27,1);
      }
      pauVar13 = pauVar29;
      pauVar25 = (undefined1 (*) [16])local_1668;
      pauVar23 = local_1678;
      param_3 = local_1660;
    } while (pauVar29 != (undefined1 (*) [16])plVar17);
  }
  if (pauVar23 != (undefined1 (*) [16])0x0) {
    __rustc::__rust_dealloc(local_1668,(longlong)pauVar23 << 5,8);
  }
  uVar28 = local_1650;
  local_1138 = (undefined1 (*) [16])&local_16e1;
  if (1 < local_16b0) {
    if (local_16b0 < 0x15) {
      core::slice::sort::shared::smallsort::insertion_sort_shift_left::
      <std::path::PathBuf,<[std::path::PathBuf]>::sort_by_key<alloc::string::String,menu_overlay::modlist::workshop_items::_closure_1_>::_closure_0_>
                (lStack_16b8,local_16b0,1,&local_1138);
    }
    else {
      core::slice::sort::stable::driftsort_main::
      <std::path::PathBuf,<[std::path::PathBuf]>::sort_by_key<alloc::string::String,menu_overlay::modlist::workshop_items::_closure_1_>::_closure_0_,alloc::vec::Vec<std::path::PathBuf>>
                (lStack_16b8,local_16b0,&local_1138);
    }
  }
  uVar20 = local_16b0;
  lVar27 = lStack_16b8;
  puVar26 = 
  __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
  ;
  local_1698 = local_16c0;
  lStack_1690 = lStack_16b8;
  local_1688 = local_16b0;
  if (param_2 == 0) {
    if (*(longlong *)
         __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
        != 0) {
      std::sys::thread_local::guard::windows::enable();
    }
    if (*(int *)(puVar26 + 0x18) == 0) {
      DVar21 = <std::sys::thread_local::key::windows::LazyKey>::init
                         (
                         __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
                         );
      plVar17 = TlsGetValue(DVar21);
    }
    else {
      DVar21 = *(int *)(puVar26 + 0x18) - 1;
      plVar17 = TlsGetValue(DVar21);
    }
    if (plVar17 < (longlong *)0x2) {
                    /* WARNING: Read-only address (ram,0x0001401c91a0) is written */
                    /* WARNING: Read-only address (ram,0x0001401c91c0) is written */
      plVar17 = (longlong *)
                <std::sys::thread_local::os::Storage<core::cell::Cell<(u64,u64)>,8usize>>
                ::try_initialize::
                <<std::hash::random::RandomState>::new::KEYS::__rust_std_internal_init_fn>
                          (DVar21,plVar17,0);
    }
    if (plVar17 == (longlong *)0x0) {
      std::thread::local::panic_access_error
                (&anon_69a0f54b37228b0631890ef9dd0e8866_2_llvm_9738004379497355289);
      goto LAB_1400fe0e1;
    }
    local_1118 = *plVar17;
    lStack_1110 = plVar17[1];
    *plVar17 = *plVar17 + 1;
    local_1128 = (longlong *)0x0;
    lStack_1120 = 0;
    local_1138 = (undefined1 (*) [16])&DAT_1401c9690;
    pauStack_1130 = (undefined1 (*) [16])0x0;
  }
  else {
    <std::path::Path>::_join(&local_2e0,param_2,param_3,&DAT_1401c98d9,9);
    <std::path::Path>::_join(&local_13a8,local_2d8,local_2d0,&DAT_1401c98b8,4);
    plVar17 = plStack_13a0;
    std::sys::fs::read_dir(&local_1138,plStack_13a0,local_1398[0]);
    pauVar29 = local_1138;
    if (local_1138 != (undefined1 (*) [16])0x2) {
      memcpy(&local_1618,&local_1128,0x260);
    }
    if (local_13a8 != (undefined1 (*) [16])0x0) {
      __rustc::__rust_dealloc(plVar17,local_13a8,1);
    }
    local_1138 = pauVar29;
    memcpy(&local_1128,&local_1618,0x260);
    if (pauVar29 == (undefined1 (*) [16])0x2) {
      core::ptr::drop_glue::
      <core::result::Result<std::fs::ReadDir,core::io::error::Error>>
                (&local_1138);
    }
    else {
      memcpy(&local_13a8,&local_1618,0x260);
    }
    local_1138 = (undefined1 (*) [16])0x0;
    local_ed8 = 0;
    local_c78 = (undefined1 (*) [16])0x2;
    local_a08 = 2;
    local_798 = pauVar29;
    local_790 = (longlong *)pauStack_1130;
    memcpy(local_788,&local_13a8,0x260);
    puVar26 = 
    __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
    ;
    if (*(longlong *)
         __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
        != 0) {
      std::sys::thread_local::guard::windows::enable();
    }
    if (*(int *)(puVar26 + 0x18) == 0) {
      DVar21 = <std::sys::thread_local::key::windows::LazyKey>::init
                         (
                         __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
                         );
      plVar17 = TlsGetValue(DVar21);
    }
    else {
      DVar21 = *(int *)(puVar26 + 0x18) - 1;
      plVar17 = TlsGetValue(DVar21);
    }
    if (plVar17 < (longlong *)0x2) {
                    /* WARNING: Read-only address (ram,0x0001401c91a0) is written */
                    /* WARNING: Read-only address (ram,0x0001401c91c0) is written */
      plVar17 = (longlong *)
                <std::sys::thread_local::os::Storage<core::cell::Cell<(u64,u64)>,8usize>>
                ::try_initialize::
                <<std::hash::random::RandomState>::new::KEYS::__rust_std_internal_init_fn>
                          (DVar21,plVar17,0);
    }
    if (plVar17 == (longlong *)0x0) {
      std::thread::local::panic_access_error
                (&anon_69a0f54b37228b0631890ef9dd0e8866_2_llvm_9738004379497355289);
LAB_1400fe0e1:
                    /* WARNING: Does not return */
      pcVar5 = (code *)invalidInstructionException();
      (*pcVar5)();
    }
    local_15f8 = *plVar17;
    lStack_15f0 = plVar17[1];
    *plVar17 = *plVar17 + 1;
    plStack_1608 = (longlong *)0x0;
    lStack_1600 = 0;
    local_1618 = (undefined1 (*) [16])&DAT_1401c9690;
    pauStack_1610 = (undefined1 (*) [16])0x0;
    <core::iter::adapters::map::Map<core::iter::adapters::map::Map<core::iter::adapters::filter::Filter<core::iter::adapters::map::Map<core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>,menu_overlay::modlist::shipped_maps::{closure#0}>,menu_overlay::modlist::shipped_maps::{closure#1}>,menu_overlay::modlist::shipped_maps::{closure#2}>,<hashbrown::set::HashSet<alloc::string::String,std::hash::random::RandomState>as_core::iter::traits::collect::Extend<alloc::string::String>>::extend<core::iter::adapters::map::Map<core::iter::adapters::filter::Filter<core::iter::adapters::map::Map<core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>,menu_overlay::modlist::shipped_maps::{closure#0}>,menu_overlay::modlist::shipped_maps::{closure#1}>,menu_overlay::modlist::shipped_maps::{closure#2}>>::{closure#0}>as_core::iter::traits::iterator::Iterator>
    ::fold::
    <(),core::iter::traits::iterator::Iterator::for_each::call<(alloc::string::String,()),<hashbrown::map::HashMap<alloc::string::String,(),std::hash::random::RandomState>as_core::iter::traits::collect::Extend<(alloc::string::String,())>>::extend<core::iter::adapters::map::Map<core::iter::adapters::map::Map<core::iter::adapters::filter::Filter<core::iter::adapters::map::Map<core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>,menu_overlay::modlist::shipped_maps::_closure_0_>,menu_overlay::modlist::shipped_maps::_closure_1_>,menu_overlay::modlist::shipped_maps::_closure_2_>,<hashbrown::set::HashSet<alloc::string::String,std::hash::random::RandomState>as_core::iter::traits::collect::Extend<alloc::string::String>>::extend<core::iter::adapters::map::Map<core::iter::adapters::filter::Filter<core::iter::adapters::map::Map<core::iter::adapters::flatten::Flatten<core::iter::adapters::flatten::Flatten<core::result::IntoIter<std::fs::ReadDir>>>,menu_overlay::modlist::shipped_maps::_closure_0_>,menu_overlay::modlist::shipped_maps::_closure_1_>,menu_overlay::modlist::shipped_maps::_closure_2_>>::_closure_0_>>::_closure_0_>::_closure_0_>
              (&local_1138,&local_1618);
    local_508 = local_15f8;
    lStack_500 = lStack_15f0;
    local_518 = plStack_1608;
    lStack_510 = lStack_1600;
    local_528 = local_1618;
    pauStack_520 = pauStack_1610;
    if (local_2e0 != 0) {
      __rustc::__rust_dealloc(local_2d8,local_2e0,1);
    }
    local_1138 = local_528;
    pauStack_1130 = pauStack_520;
    local_1128 = local_518;
    lStack_1120 = lStack_510;
    local_1118 = local_508;
    lStack_1110 = lStack_500;
  }
  scan_items(uVar28,lVar27,uVar20,&local_1138);
  pauVar29 = pauStack_1130;
  if (pauStack_1130 != (undefined1 (*) [16])0x0) {
    if (lStack_1120 != 0) {
      auVar4 = *local_1138;
      pauVar25 = local_1138 + 1;
      uVar31 = ~(uint)(ushort)((ushort)(SUB161(auVar4 >> 7,0) & 1) |
                               (ushort)(SUB161(auVar4 >> 0xf,0) & 1) << 1 |
                               (ushort)(SUB161(auVar4 >> 0x17,0) & 1) << 2 |
                               (ushort)(SUB161(auVar4 >> 0x1f,0) & 1) << 3 |
                               (ushort)(SUB161(auVar4 >> 0x27,0) & 1) << 4 |
                               (ushort)(SUB161(auVar4 >> 0x2f,0) & 1) << 5 |
                               (ushort)(SUB161(auVar4 >> 0x37,0) & 1) << 6 |
                               (ushort)(SUB161(auVar4 >> 0x3f,0) & 1) << 7 |
                               (ushort)(SUB161(auVar4 >> 0x47,0) & 1) << 8 |
                               (ushort)(SUB161(auVar4 >> 0x4f,0) & 1) << 9 |
                               (ushort)(SUB161(auVar4 >> 0x57,0) & 1) << 10 |
                               (ushort)(SUB161(auVar4 >> 0x5f,0) & 1) << 0xb |
                               (ushort)(SUB161(auVar4 >> 0x67,0) & 1) << 0xc |
                               (ushort)(SUB161(auVar4 >> 0x6f,0) & 1) << 0xd |
                               (ushort)(SUB161(auVar4 >> 0x77,0) & 1) << 0xe |
                              (ushort)(byte)(auVar4[0xf] >> 7) << 0xf);
      pauVar23 = local_1138;
      lVar27 = lStack_1120;
      do {
        uVar30 = (ushort)uVar31;
        while (uVar30 == 0) {
          auVar4 = *pauVar25;
          pauVar23 = pauVar23 + -0x18;
          pauVar25 = pauVar25 + 1;
          uVar30 = ((ushort)(SUB161(auVar4 >> 7,0) & 1) | (ushort)(SUB161(auVar4 >> 0xf,0) & 1) << 1
                    | (ushort)(SUB161(auVar4 >> 0x17,0) & 1) << 2 |
                    (ushort)(SUB161(auVar4 >> 0x1f,0) & 1) << 3 |
                    (ushort)(SUB161(auVar4 >> 0x27,0) & 1) << 4 |
                    (ushort)(SUB161(auVar4 >> 0x2f,0) & 1) << 5 |
                    (ushort)(SUB161(auVar4 >> 0x37,0) & 1) << 6 |
                    (ushort)(SUB161(auVar4 >> 0x3f,0) & 1) << 7 |
                    (ushort)(SUB161(auVar4 >> 0x47,0) & 1) << 8 |
                    (ushort)(SUB161(auVar4 >> 0x4f,0) & 1) << 9 |
                    (ushort)(SUB161(auVar4 >> 0x57,0) & 1) << 10 |
                    (ushort)(SUB161(auVar4 >> 0x5f,0) & 1) << 0xb |
                    (ushort)(SUB161(auVar4 >> 0x67,0) & 1) << 0xc |
                    (ushort)(SUB161(auVar4 >> 0x6f,0) & 1) << 0xd |
                    (ushort)(SUB161(auVar4 >> 0x77,0) & 1) << 0xe |
                   (ushort)(byte)(auVar4[0xf] >> 7) << 0xf) ^ 0xffff;
          uVar31 = (uint)uVar30;
        }
        uVar6 = 0;
        for (uVar7 = uVar31; (uVar7 & 1) == 0; uVar7 = uVar7 >> 1 | 0x80000000) {
          uVar6 = uVar6 + 1;
        }
        lVar14 = *(longlong *)((longlong)pauVar23 + (ulonglong)uVar6 * -0x18 + -0x18);
        if (lVar14 != 0) {
          __rustc::__rust_dealloc
                    (*(undefined8 *)((longlong)pauVar23 + (ulonglong)uVar6 * -0x18 + -0x10),lVar14,1
                    );
        }
        lVar27 = lVar27 + -1;
        uVar31 = uVar31 - 1 & uVar31;
      } while (lVar27 != 0);
    }
    uVar20 = (longlong)pauVar29 * 0x18 & 0xfffffffffffffff0;
    puVar26 = pauVar29[3] + uVar20 + 1;
    if (puVar26 != (undefined *)0x0) {
      __rustc::__rust_dealloc
                ((undefined *)((longlong)local_1138 + (-0x20 - uVar20)),puVar26,0x10);
    }
  }
  lVar27 = lStack_1690;
  if (local_1688 != 0) {
    puVar24 = (undefined8 *)(lStack_1690 + 8);
    uVar20 = local_1688;
    do {
      if (puVar24[-1] != 0) {
        __rustc::__rust_dealloc(*puVar24,puVar24[-1],1);
      }
      puVar24 = puVar24 + 4;
      uVar20 = uVar20 - 1;
    } while (uVar20 != 0);
  }
  if (local_1698 != 0) {
    __rustc::__rust_dealloc(lVar27,local_1698 << 5,8);
  }
  return uVar28;
}


