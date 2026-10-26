
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::mods::unsafe_files */

undefined4 * __rustcall
menu_overlay::mods::unsafe_files
          (undefined4 *param_1,undefined8 param_2,undefined8 param_3)

{
  byte bVar1;
  code *pcVar2;
  undefined8 uVar3;
  longlong *plVar4;
  char cVar5;
  undefined4 *puVar6;
  ulonglong uVar7;
  HANDLE pvVar8;
  longlong lVar9;
  undefined8 uVar10;
  ulonglong uVar11;
  longlong lVar12;
  longlong *plVar13;
  longlong lVar14;
  ulonglong uVar15;
  undefined8 *puVar16;
  longlong *plVar17;
  short local_ccc;
  undefined1 local_cc9;
  undefined8 local_cc8;
  undefined8 uStack_cc0;
  ulonglong local_cb8;
  undefined8 local_cb0;
  longlong local_ca8;
  HANDLE local_ca0;
  longlong local_c98;
  undefined4 *local_c90;
  longlong local_c88;
  ulonglong local_c80;
  longlong local_c78;
  HANDLE local_c70;
  undefined8 local_c68;
  undefined8 uStack_c60;
  undefined4 *local_c58;
  undefined8 local_c50;
  longlong *local_c48;
  longlong *plStack_c40;
  longlong *local_c38;
  undefined8 auStack_c30 [2];
  byte local_c19;
  HANDLE local_9d0;
  longlong local_9c8;
  undefined4 local_9c0;
  undefined4 uStack_9bc;
  undefined4 local_9b8;
  undefined5 uStack_9b4;
  undefined8 local_788;
  undefined8 uStack_780;
  longlong *local_778;
  undefined8 uStack_770;
  undefined8 local_528;
  longlong *local_2c8;
  longlong *local_2c0;
  longlong *local_2b8 [79];
  
  local_cc8 = 0;
  uStack_cc0 = 8;
  local_cb8 = 0;
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar6 = (undefined4 *)__rustc::__rust_alloc(0x28,8);
  if (puVar6 == (undefined4 *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x28);
                    /* WARNING: Does not return */
    pcVar2 = (code *)invalidInstructionException();
    (*pcVar2)();
  }
  local_c58 = param_1;
  <std::path::Path>::to_path_buf(&local_788,param_2,param_3);
  local_c38 = local_778;
  auStack_c30[0] = uStack_770;
  local_c48 = local_788;
  plStack_c40 = uStack_780;
  *(longlong **)(puVar6 + 4) = local_778;
  *(undefined8 *)(puVar6 + 6) = uStack_770;
  *puVar6 = (undefined4)local_788;
  puVar6[1] = local_788._4_4_;
  puVar6[2] = (undefined4)uStack_780;
  puVar6[3] = uStack_780._4_4_;
  *(undefined8 *)(puVar6 + 8) = 0;
  local_c98 = 1;
  local_c88 = 1;
  local_c80 = 0;
  local_c90 = puVar6;
  do {
    local_c88 = local_c88 + -1;
    local_ca8 = *(longlong *)(local_c90 + local_c88 * 10);
    local_cb0 = *(undefined8 *)(local_c90 + local_c88 * 10 + 2);
    uVar10 = *(undefined8 *)(local_c90 + local_c88 * 10 + 4);
    uVar3 = *(undefined8 *)((longlong)(local_c90 + local_c88 * 10 + 4) + 8);
    lVar9 = *(longlong *)(local_c90 + local_c88 * 10 + 8);
    std::sys::fs::read_dir
              (&local_c48,local_cb0,*(undefined8 *)(local_c90 + local_c88 * 10 + 4));
    plVar17 = plStack_c40;
    plVar13 = local_c48;
    if (local_c48 == (longlong *)0x2) {
      if ((1 < ((uint)plStack_c40 & 3) - 2) && (((ulonglong)plStack_c40 & 3) != 0)) {
        uStack_780 = (longlong *)((longlong)plStack_c40 + -1);
        local_788 = (longlong *)CONCAT71(local_788._1_7_,3);
        <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
        drop(&uStack_780);
      }
      lVar12 = local_ca8;
      uVar15 = local_cb8;
      if (lVar9 == 0) {
        if (local_ca8 != 0) {
          __rustc::__rust_dealloc(local_cb0,local_ca8,1);
        }
      }
      else {
        if (local_cb8 == local_cc8) {
          <alloc::raw_vec::RawVec<std::path::PathBuf>>::grow_one
                    (&local_cc8);
        }
        lVar9 = uVar15 * 0x20;
        *(longlong *)(uStack_cc0 + lVar9) = lVar12;
        *(undefined8 *)(uStack_cc0 + 8 + lVar9) = local_cb0;
        puVar16 = (undefined8 *)(uStack_cc0 + 0x10 + lVar9);
        *puVar16 = uVar10;
        puVar16[1] = uVar3;
        local_cb8 = uVar15 + 1;
      }
    }
    else {
      memcpy(local_2b8,&local_c38,0x260);
      local_2c8 = plVar13;
      local_2c0 = plVar17;
      local_788 = (longlong *)0x0;
      local_528 = 0;
      uVar15 = lVar9 + 1;
      do {
        <std::fs::ReadDir_as_core::iter::traits::iterator::Iterator>
        ::next(&local_c48,&local_2c8);
        plVar17 = local_c38;
        plVar13 = plStack_c40;
        if (local_c48 == (longlong *)0x0) {
          if (local_2c8 == (longlong *)0x0) {
LAB_1400de09f:
            LOCK();
            *local_2b8[0] = *local_2b8[0] + -1;
            UNLOCK();
            if (*local_2b8[0] == 0) {
              <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                        (local_2b8);
            }
          }
          else if ((int)local_2c8 != 2) {
            <std::sys::fs::windows::FindNextFileHandle_as_core::ops::drop::Drop>
            ::drop(&local_2c0);
            goto LAB_1400de09f;
          }
          plVar17 = local_c38;
          plVar13 = plStack_c40;
          plVar4 = local_c48;
          local_2c8 = (longlong *)0x2;
          memcpy(&local_9d0,auStack_c30,0x248);
          if (plVar4 != (longlong *)0x1) break;
        }
        else {
          memcpy(&local_9d0,auStack_c30,0x248);
        }
        if (plVar13 == (longlong *)0x0) {
          if ((1 < ((uint)plVar17 & 3) - 2) && (((ulonglong)plVar17 & 3) != 0)) {
            plStack_c40 = (longlong *)((longlong)plVar17 + -1);
            local_c48 = (longlong *)CONCAT71(local_c48._1_7_,3);
            <core::io::error::CustomOwner_as_core::ops::drop::Drop>
            ::drop(&plStack_c40);
          }
        }
        else {
          memcpy(&local_c38,&local_9d0,0x248);
          local_c48 = plVar13;
          plStack_c40 = plVar17;
          <std::fs::DirEntry>::path(&local_c78,&local_c48);
          pvVar8 = local_c70;
          lVar12 = local_c78;
          lVar9 = local_c88;
          uVar7 = local_cb8;
          local_c80 = local_c80 + 1;
          if ((local_c80 < 0x4e21) &&
             ((byte)((local_c19 & 0x20) >> 5 & (byte)((uint)plStack_c40 >> 10)) == 0)) {
            if (((ulonglong)plStack_c40 & 0x10) == 0) {
              local_ca0 = local_c70;
              local_c50 = local_c68;
              uVar10 = local_c68;
              lVar9 = <std::path::Path>::extension();
              if (lVar9 != 0) {
                core::str::converts::from_utf8(&local_9d0,lVar9,uVar10);
                lVar9 = local_9c8;
                if ((int)local_9d0 != 1) {
                  uVar7 = CONCAT44(uStack_9bc,local_9c0);
                  if (uVar7 < 0x10) {
                    lVar9 = 0;
                    do {
                      if (uVar7 == *(ulonglong *)((longlong)&DAT_1401c83f8 + lVar9)) {
                        if (uVar7 == 0) goto LAB_1400de488;
                        uVar11 = 0;
                        while (bVar1 = *(byte *)(*(longlong *)((longlong)&PTR_s_e_1401c83f0 + lVar9)
                                                + uVar11),
                              (byte)(((byte)(*(byte *)(local_9c8 + uVar11) + 0xbf) < 0x1a) << 5 |
                                    *(byte *)(local_9c8 + uVar11)) ==
                              (byte)(((byte)(bVar1 + 0xbf) < 0x1a) << 5 | bVar1)) {
                          uVar11 = uVar11 + 1;
                          if (uVar7 == uVar11) goto LAB_1400de488;
                        }
                      }
                      lVar9 = lVar9 + 0x10;
                    } while (lVar9 != 0x230);
                  }
                  else {
                    lVar12 = 8;
                    do {
                      if ((uVar7 == *(ulonglong *)((longlong)&PTR_s_e_1401c83f0 + lVar12)) &&
                         (cVar5 = <[u8]>::eq_ignore_ascii_case_chunks::<16usize>
                                            (lVar9,uVar7,
                                             *(undefined8 *)
                                              (
                                              "sysdrvocxcplscrcompifmsimspbatcmdps1psm1vbsvbejsjsewsfwshhtalnkurlregjarpypywshziprar7zcabisoimg"
                                              + lVar12 + 0x5a),uVar7), cVar5 != '\0'))
                      goto LAB_1400de488;
                      lVar12 = lVar12 + 0x10;
                    } while (lVar12 != 0x238);
                  }
                }
              }
              local_ccc = 0;
              local_9d0 = (HANDLE)((ulonglong)local_9d0 & 0xffffffff00000000);
              local_9c8 = 0;
              local_9c0 = 7;
              uStack_9b4 = 0;
              uStack_9bc = 0;
              local_9b8 = 1;
              pvVar8 = local_ca0;
              uVar7 = <std::fs::OpenOptions>::_open
                                (&local_9d0,local_ca0,local_c50);
              if ((uVar7 & 1) == 0) {
                local_9d0 = pvVar8;
                pvVar8 = (HANDLE)alloc::io::read::default_read_exact::
                                 <std::fs::File>(&local_9d0,&local_ccc,2);
                CloseHandle(local_9d0);
                if (pvVar8 != (HANDLE)0x0) goto LAB_1400de294;
                if (local_ccc == 0x5a4d) {
LAB_1400de488:
                  lVar9 = local_c78;
                  uVar7 = local_cb8;
                  if (local_cb8 == local_cc8) {
                    <alloc::raw_vec::RawVec<std::path::PathBuf>>
                    ::grow_one(&local_cc8);
                  }
                  lVar12 = uVar7 * 0x20;
                  *(longlong *)(uStack_cc0 + lVar12) = lVar9;
                  *(HANDLE *)(uStack_cc0 + 8 + lVar12) = local_ca0;
                  puVar16 = (undefined8 *)(uStack_cc0 + 0x10 + lVar12);
                  *puVar16 = local_c68;
                  puVar16[1] = uStack_c60;
                  local_cb8 = uVar7 + 1;
                  goto LAB_1400de32d;
                }
              }
              else {
LAB_1400de294:
                if ((1 < ((uint)pvVar8 & 3) - 2) && (((ulonglong)pvVar8 & 3) != 0)) {
                  local_9c8 = (longlong)pvVar8 - 1;
                  local_9d0 = (HANDLE)CONCAT71(local_9d0._1_7_,3);
                  <core::io::error::CustomOwner_as_core::ops::drop::Drop>
                  ::drop(&local_9c8);
                }
              }
              if (local_c78 != 0) {
                __rustc::__rust_dealloc(local_ca0,local_c78,1);
              }
            }
            else {
              if (0xc < uVar15) goto LAB_1400de2da;
              if (local_c88 == local_c98) {
                <alloc::raw_vec::RawVec<(std::path::PathBuf,usize)>>
                ::grow_one(&local_c98);
              }
              *(longlong *)(local_c90 + lVar9 * 10) = lVar12;
              *(HANDLE *)(local_c90 + lVar9 * 10 + 2) = pvVar8;
              *(undefined8 *)(local_c90 + lVar9 * 10 + 4) = local_c68;
              *(undefined8 *)((longlong)(local_c90 + lVar9 * 10 + 4) + 8) = uStack_c60;
              *(ulonglong *)(local_c90 + lVar9 * 10 + 8) = uVar15;
              local_c88 = lVar9 + 1;
            }
          }
          else {
LAB_1400de2da:
            if (local_cb8 == local_cc8) {
              <alloc::raw_vec::RawVec<std::path::PathBuf>>::
              grow_one(&local_cc8);
            }
            lVar9 = uVar7 * 0x20;
            *(longlong *)(uStack_cc0 + lVar9) = lVar12;
            *(HANDLE *)(uStack_cc0 + 8 + lVar9) = pvVar8;
            puVar16 = (undefined8 *)(uStack_cc0 + 0x10 + lVar9);
            *puVar16 = local_c68;
            puVar16[1] = uStack_c60;
            local_cb8 = uVar7 + 1;
          }
LAB_1400de32d:
          LOCK();
          *local_c48 = *local_c48 + -1;
          UNLOCK();
          if (*local_c48 == 0) {
            <alloc::sync::Arc<std::path::PathBuf>>::drop_slow
                      (&local_c48);
          }
        }
      } while ((int)local_2c8 != 2);
      lVar9 = local_ca8;
      core::ptr::drop_glue::
      <core::iter::adapters::flatten::Flatten<std::fs::ReadDir>>
                (&local_788);
      if (20000 < local_c80) {
        if (lVar9 != 0) {
          __rustc::__rust_dealloc(local_cb0,lVar9,1);
        }
        goto LAB_1400de589;
      }
      if (lVar9 != 0) {
        __rustc::__rust_dealloc(local_cb0,lVar9,1);
      }
    }
    if (local_c88 == 0) {
LAB_1400de589:
      puVar6 = local_c58;
      lVar9 = uStack_cc0;
      if (1 < local_cb8) {
        if (local_cb8 < 0x15) {
          lVar14 = local_cb8 << 5;
          lVar12 = 0x20;
          do {
            core::slice::sort::shared::smallsort::insert_tail::
            <std::path::PathBuf,<std::path::PathBuf_as_core::cmp::PartialOrd>::lt>
                      (lVar9,lVar9 + lVar12);
            lVar12 = lVar12 + 0x20;
          } while (lVar14 != lVar12);
        }
        else {
          core::slice::sort::stable::driftsort_main::
          <std::path::PathBuf,<std::path::PathBuf_as_core::cmp::PartialOrd>::lt,alloc::vec::Vec<std::path::PathBuf>>
                    (uStack_cc0,local_cb8,&local_cc9);
        }
      }
      *(ulonglong *)(puVar6 + 4) = local_cb8;
      *puVar6 = (undefined4)local_cc8;
      puVar6[1] = local_cc8._4_4_;
      puVar6[2] = (undefined4)uStack_cc0;
      puVar6[3] = uStack_cc0._4_4_;
      if (local_c88 != 0) {
        puVar16 = (undefined8 *)(local_c90 + 2);
        lVar9 = local_c88;
        do {
          if (puVar16[-1] != 0) {
            __rustc::__rust_dealloc(*puVar16,puVar16[-1],1);
          }
          puVar16 = puVar16 + 5;
          lVar9 = lVar9 + -1;
        } while (lVar9 != 0);
      }
      if (local_c98 != 0) {
        __rustc::__rust_dealloc(local_c90,local_c98 * 0x28,8);
      }
      return puVar6;
    }
  } while( true );
}


