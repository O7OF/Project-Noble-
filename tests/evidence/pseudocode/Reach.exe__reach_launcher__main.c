
/* WARNING: Type propagation algorithm not settling */
/* reach_launcher::main */

ulonglong reach_launcher::main(void)

{
  int iVar1;
  undefined *******pppppppuVar2;
  undefined *******pppppppuVar3;
  code *pcVar4;
  uint uVar5;
  size_t sVar6;
  undefined ********ppppppppuVar7;
  undefined ********ppppppppuVar8;
  bool bVar9;
  char cVar10;
  undefined1 uVar11;
  byte bVar12;
  HANDLE hMutex;
  void *pvVar13;
  byte *pbVar14;
  LPCWSTR pWVar15;
  undefined *puVar16;
  undefined **ppuVar17;
  undefined7 extraout_var;
  undefined7 extraout_var_00;
  undefined7 extraout_var_01;
  void *pvVar18;
  undefined ********ppppppppuVar19;
  undefined8 *puVar20;
  undefined ********ppppppppuVar21;
  undefined ********ppppppppuVar22;
  longlong lVar23;
  ulonglong uVar24;
  undefined ********ppppppppuVar25;
  byte bVar26;
  int iVar27;
  ulonglong uVar28;
  undefined *******pppppppuVar29;
  undefined ********ppppppppuVar30;
  undefined ********ppppppppuVar31;
  SIZE_T in_R9;
  undefined8 uVar32;
  longlong lVar33;
  undefined ********ppppppppuVar34;
  ulonglong uVar35;
  undefined ********local_410;
  undefined ********local_408;
  undefined ********ppppppppuStack_400;
  undefined ********local_3f8;
  undefined ********ppppppppuStack_3f0;
  char *local_3e8;
  undefined8 local_3e0;
  char *local_3d8;
  undefined8 local_3d0;
  char *local_3c8;
  undefined8 local_3c0;
  char *local_3b8;
  undefined8 local_3b0;
  undefined ********local_3a0;
  undefined ********local_398;
  undefined ********ppppppppuStack_390;
  undefined ********local_388;
  undefined ********ppppppppuStack_380;
  undefined1 local_369;
  undefined ********local_368;
  undefined ********ppppppppuStack_360;
  undefined ********local_358;
  undefined ********ppppppppuStack_350;
  undefined ********local_348;
  undefined ********ppppppppuStack_340;
  undefined ********local_338;
  undefined ********ppppppppuStack_330;
  undefined **local_328;
  code *local_320;
  undefined **local_318;
  code *local_310;
  undefined4 local_29c;
  undefined ********local_228;
  undefined ********ppppppppuStack_220;
  undefined ********local_218;
  undefined ********ppppppppuStack_210;
  undefined ********local_208;
  undefined ********local_200;
  undefined8 local_1f8;
  size_t local_1f0;
  undefined8 *local_1e8;
  size_t local_1e0;
  undefined ********local_1d0;
  undefined ********ppppppppuStack_1c8;
  undefined8 local_1c0;
  undefined ********local_1b8;
  undefined ********ppppppppuStack_1b0;
  undefined ********ppppppppuStack_1a8;
  undefined ********ppppppppuStack_1a0;
  undefined8 local_198;
  HANDLE local_90;
  undefined ********local_88;
  undefined ********ppppppppuStack_80;
  undefined8 local_78;
  undefined ********local_68;
  undefined ********ppppppppuStack_60;
  
  if (steam::is_wine::WINE != 0) {
    <std::sync::once_lock::OnceLock<bool>>::initialize::
    <<std::sync::once_lock::OnceLock<bool>>::get_or_init<reach_launcher::steam::is_wine::_closure_0_>::_closure_0_,_>
              ((uint *)&steam::is_wine::WINE);
  }
  cVar10 = DAT_14015702c;
  std::env::args(&local_1b8);
  lVar23 = 2;
  while (<std::env::Args_as_core::iter::traits::iterator::Iterator>
         ::next((longlong *)&local_348,(longlong)&local_1b8),
        local_348 != (undefined ********)0xffffffffffffffff) {
    if (local_338 == (undefined ********)&DAT_0000000d) {
      pppppppuVar29 = *ppppppppuStack_340;
      lVar33 = *(longlong *)((longlong)ppppppppuStack_340 + 5);
      if (local_348 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuStack_340,local_348,1);
      }
      if (lVar33 == 0x7265646c6f662d6b && pppppppuVar29 == (undefined *******)0x662d6b6369702d2d) {
        if (cVar10 != '\0') {
          std::env::args(&local_348);
          if (lVar23 == 0) goto LAB_14004049d;
          lVar33 = -lVar23;
          goto LAB_14003fa89;
        }
        break;
      }
    }
    else if (local_348 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuStack_340,local_348,1);
    }
    lVar23 = lVar23 + 1;
  }
  if ((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0) >> 5;
    ppppppppuVar34 = ppppppppuStack_1b0 + 1;
    do {
      if (ppppppppuVar34[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar34,ppppppppuVar34[-1],1);
      }
      ppppppppuVar34 = ppppppppuVar34 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  lVar23 = 0;
  if (ppppppppuStack_1a8 != (undefined ********)0x0) {
    lVar23 = (longlong)ppppppppuStack_1a8 << 5;
    __rustc::__rust_dealloc(local_1b8,lVar23,8);
  }
  release::ignore_dev_switches((ulonglong *)&local_348,lVar23);
  ppppppppuVar30 = local_338;
  ppppppppuVar21 = ppppppppuStack_340;
  ppppppppuVar25 = local_348;
  ppppppppuVar34 = ppppppppuStack_340 + (longlong)local_338 * 3;
  local_348 = ppppppppuStack_340;
  local_338 = ppppppppuVar25;
  ppppppppuVar31 = ppppppppuStack_340;
  ppppppppuStack_330 = ppppppppuVar34;
  if (ppppppppuVar30 != (undefined ********)0x0) {
    do {
      ppppppppuVar30 = ppppppppuStack_340 + 3;
      local_398 = (undefined ********)ppppppppuStack_340[1];
      ppppppppuStack_390 = (undefined ********)ppppppppuStack_340[2];
      ppppppppuVar31 = ppppppppuVar30;
      if ((undefined ********)*ppppppppuStack_340 == (undefined ********)0xffffffffffffffff) break;
      ppppppppuStack_400 =
           (undefined ********)
           &<alloc::string::String_as_core::fmt::Display>::fmt;
      local_408 = (undefined ********)&local_1b8;
      local_1b8 = (undefined ********)*ppppppppuStack_340;
      ppppppppuStack_1b0 = local_398;
      ppppppppuStack_1a8 = ppppppppuStack_390;
      std::io::stdio::_eprint((ushort *)&DAT_14015d9a0,(ulonglong)&local_408);
      if (local_1b8 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuStack_1b0,local_1b8,1);
      }
      ppppppppuVar31 = ppppppppuVar34;
      ppppppppuStack_340 = ppppppppuVar30;
    } while (ppppppppuVar30 != ppppppppuVar34);
  }
  ppppppppuStack_340 = ppppppppuVar31;
  if (ppppppppuVar34 != ppppppppuVar31) {
    uVar28 = (ulonglong)((longlong)ppppppppuVar34 - (longlong)ppppppppuVar31) / 0x18;
    ppppppppuVar31 = ppppppppuVar31 + 1;
    do {
      if (ppppppppuVar31[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar31,ppppppppuVar31[-1],1);
      }
      ppppppppuVar31 = ppppppppuVar31 + 3;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (ppppppppuVar25 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(ppppppppuVar21,(longlong)ppppppppuVar25 * 0x18,8);
  }
  std::env::args(&local_1b8);
  lVar23 = -1;
  while (<std::env::Args_as_core::iter::traits::iterator::Iterator>
         ::next((longlong *)&local_348,(longlong)&local_1b8),
        local_348 != (undefined ********)0xffffffffffffffff) {
    if (local_338 == (undefined ********)0xe) {
      pppppppuVar29 = *ppppppppuStack_340;
      bVar9 = *(longlong *)((longlong)ppppppppuStack_340 + 6) != 0x6f6d65642d726574;
      ppppppppuVar31 =
           (undefined ********)
           CONCAT71((int7)((ulonglong)ppppppppuVar31 >> 8),
                    bVar9 || pppppppuVar29 != (undefined *******)0x6574616470752d2d);
      if (local_348 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuStack_340,local_348,1);
      }
      if (!bVar9 && pppppppuVar29 == (undefined *******)0x6574616470752d2d) {
        std::env::args(&local_348);
        if (lVar23 == 0) goto LAB_140040195;
        goto LAB_140040175;
      }
    }
    else if (local_348 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuStack_340,local_348,1);
    }
    lVar23 = lVar23 + -1;
  }
  if ((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0) >> 5;
    ppppppppuVar34 = ppppppppuStack_1b0 + 1;
    do {
      if (ppppppppuVar34[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar34,ppppppppuVar34[-1],1);
      }
      ppppppppuVar34 = ppppppppuVar34 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (ppppppppuStack_1a8 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_1b8,(longlong)ppppppppuStack_1a8 << 5,8);
  }
  std::env::args(&local_348);
  do {
    while( true ) {
      <std::env::Args_as_core::iter::traits::iterator::Iterator>
      ::next((longlong *)&local_1b8,(longlong)&local_348);
      ppppppppuVar34 = local_1b8;
      if (local_1b8 == (undefined ********)0xffffffffffffffff) goto LAB_14003fe75;
      if (ppppppppuStack_1a8 == (undefined ********)&DAT_00000007) break;
      if (local_1b8 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuStack_1b0,local_1b8,1);
      }
    }
    iVar27 = *(int *)ppppppppuStack_1b0;
    iVar1 = *(int *)((longlong)ppppppppuStack_1b0 + 3);
    if (local_1b8 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuStack_1b0,local_1b8,1);
    }
    uVar5 = (uint)(iVar1 != 0x6b636568 || iVar27 != 0x68632d2d);
    ppppppppuVar31 = (undefined ********)(ulonglong)uVar5;
  } while (uVar5 != 0);
LAB_14003fe75:
  ppppppppuVar25 = ppppppppuStack_340;
  if ((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340) >> 5;
    ppppppppuVar25 = ppppppppuStack_340 + 1;
    do {
      if (ppppppppuVar25[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar25,ppppppppuVar25[-1],1);
      }
      ppppppppuVar25 = ppppppppuVar25 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (local_338 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
  }
  steam::wine_version(&local_1b8);
  if (local_1b8 != (undefined ********)0xffffffffffffffff) {
    local_338 = ppppppppuStack_1a8;
    local_348 = local_1b8;
    ppppppppuStack_340 = ppppppppuStack_1b0;
    ppppppppuStack_400 =
         (undefined ********)
         &<alloc::string::String_as_core::fmt::Display>::fmt;
    local_408 = (undefined ********)&local_348;
    std::io::stdio::_eprint((ushort *)&DAT_14015d4e9,(ulonglong)&local_408);
    if (local_348 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuStack_340,local_348,1);
    }
  }
  hMutex = running::lock_named(0x14015b1bf,0x1d,900000);
  if (hMutex == (HANDLE)0x0) {
    local_348 = (undefined ********)
                &PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388;
    ppppppppuStack_340 = (undefined ********)<&str_as_core::fmt::Display>::fmt;
    local_338 = (undefined ********)
                &PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388;
    ppppppppuStack_330 = (undefined ********)<&str_as_core::fmt::Display>::fmt;
    alloc::fmt::format::format_inner
              ((ulonglong *)&local_1b8,(ushort *)&DAT_14015d930,(ulonglong)&local_348);
    ppppppppuVar34 = ppppppppuStack_1b0;
    message((undefined *)ppppppppuStack_1b0,(longlong)ppppppppuStack_1a8);
    if (local_1b8 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar34,local_1b8,1);
    }
    return 7;
  }
  local_90 = hMutex;
  std::env::current_exe((ulonglong *)&local_348);
  ppppppppuVar30 = ppppppppuStack_340;
  ppppppppuVar21 = local_348;
  if (local_348 == (undefined ********)0xffffffffffffffff) {
    if ((1 < ((uint)ppppppppuStack_340 & 3) - 2) && (((ulonglong)ppppppppuStack_340 & 3) != 0)) {
      ppppppppuStack_1b0 = (undefined ********)((longlong)ppppppppuStack_340 + -1);
      local_1b8 = (undefined ********)CONCAT71(local_1b8._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop((longlong *)&ppppppppuStack_1b0);
    }
  }
  else {
    ppppppppuVar19 = local_338;
    pvVar13 = (void *)<std::path::Path>::parent
                                ((byte *)ppppppppuStack_340,(ulonglong)local_338);
    if (pvVar13 == (void *)0x0) {
      ppppppppuVar19 = (undefined ********)0xffffffffffffffff;
    }
    else {
      <std::path::Path>::to_path_buf
                ((size_t *)&local_348,pvVar13,(size_t)ppppppppuVar19);
      ppppppppuVar19 = local_348;
      ppppppppuVar25 = ppppppppuStack_340;
      ppppppppuVar31 = local_338;
    }
    if (ppppppppuVar21 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar30,ppppppppuVar21,1);
    }
    if (ppppppppuVar19 != (undefined ********)0xffffffffffffffff) {
      bVar9 = update::unfinished(ppppppppuVar25,(size_t)ppppppppuVar31);
      if (bVar9) {
        update::recover((ulonglong *)&local_368,ppppppppuVar25,(size_t)ppppppppuVar31);
        if (local_368 != (undefined ********)0xffffffffffffffff) {
          local_3f8 = local_358;
          local_408 = local_368;
          ppppppppuStack_400 = ppppppppuStack_360;
          local_348 = (undefined ********)
                      &
                      PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
          ;
          ppppppppuStack_340 =
               (undefined ********)<&str_as_core::fmt::Display>::fmt;
          local_338 = (undefined ********)&local_408;
          ppppppppuStack_330 =
               (undefined ********)
               &<alloc::string::String_as_core::fmt::Display>::
                fmt;
          local_328 = &
                      PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
          ;
          local_320 = <&str_as_core::fmt::Display>::fmt;
          local_318 = &
                      PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
          ;
          local_310 = <&str_as_core::fmt::Display>::fmt;
          alloc::fmt::format::format_inner
                    ((ulonglong *)&local_1b8,(ushort *)&DAT_14015d5a4,(ulonglong)&local_348);
          ppppppppuVar31 = ppppppppuStack_1b0;
          ppppppppuVar34 = local_1b8;
          message((undefined *)ppppppppuStack_1b0,(longlong)ppppppppuStack_1a8);
          if (ppppppppuVar34 != (undefined ********)0x0) {
            __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
          }
          if (local_408 != (undefined ********)0x0) {
            __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
          }
          uVar28 = 7;
LAB_14004193c:
          if (ppppppppuVar19 != (undefined ********)0x0) {
            __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar19,1);
          }
          ReleaseMutex(hMutex);
          CloseHandle(hMutex);
          return uVar28;
        }
        if ((char)ppppppppuStack_360 != '\0') {
          <std::path::Path>::_join
                    (&local_1b8,ppppppppuVar25,(size_t)ppppppppuVar31,
                     (undefined8 *******)&DAT_14015d538,&DAT_00000015);
          <std::process::Command>::new::<std::path::PathBuf>
                    (&local_348,(longlong *)&local_1b8);
          std::env::args(&local_408);
          ppppppppuStack_1a8 = local_3f8;
          ppppppppuStack_1a0 = ppppppppuStack_3f0;
          local_1b8 = local_408;
          ppppppppuStack_1b0 = ppppppppuStack_400;
          local_198 = 1;
          <std::process::Command>::args::
          <core::iter::adapters::skip::Skip<std::env::Args>,alloc::string::String>
                    ((longlong)&local_348,&local_1b8);
          <std::process::Command>::spawn((longlong *)&local_1b8,&local_348);
          if ((int)local_1b8 == 2) {
            local_228 = ppppppppuStack_1b0;
            local_408 = (undefined ********)
                        &
                        PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
            ;
            ppppppppuStack_400 =
                 (undefined ********)<&str_as_core::fmt::Display>::fmt;
            local_3f8 = (undefined ********)&local_228;
            ppppppppuStack_3f0 =
                 (undefined ********)
                 <core::io::error::Error_as_core::fmt::Display>::
                 fmt;
            alloc::fmt::format::format_inner
                      ((ulonglong *)&local_398,(ushort *)&DAT_14015d54d,(ulonglong)&local_408);
            ppppppppuVar31 = ppppppppuStack_390;
            ppppppppuVar34 = local_398;
            message((undefined *)ppppppppuStack_390,(longlong)local_388);
            if (ppppppppuVar34 != (undefined ********)0x0) {
              __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
            }
            core::ptr::drop_glue::<core::io::error::Error>
                      ((ulonglong)local_228);
            uVar28 = 7;
          }
          else {
            uVar28 = 0;
            core::ptr::drop_glue::
            <core::result::Result<std::process::Child,core::io::error::Error>>
                      ((longlong *)&local_1b8);
          }
          core::ptr::drop_glue::<std::process::Command>
                    ((longlong *)&local_348);
          goto LAB_14004193c;
        }
      }
      if (ppppppppuVar19 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar19,1);
      }
    }
  }
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);
  package::root(&local_1f0);
  sVar6 = local_1e0;
  puVar20 = local_1e8;
  if ((ppppppppuVar34 == (undefined ********)0xffffffffffffffff) &&
     (local_1f0 != 0xffffffffffffffff)) {
    update::clean_up(local_1e8,local_1e0);
    update::latest((size_t *)&local_348);
    if (local_348 != (undefined ********)0xffffffffffffffff) {
      uVar32 = 0;
      bVar9 = update::is_newer((byte *)ppppppppuStack_340,(longlong)local_338,(byte *)"0.1.24\r\n",8
                              );
      if (bVar9) {
        memcpy(&local_1b8,&local_348,0x120);
        if (local_1b8 != (undefined ********)0xffffffffffffffff) {
          std::env::args(&local_348);
LAB_140040a1c:
          while (<std::env::Args_as_core::iter::traits::iterator::Iterator>
                 ::next((longlong *)&local_408,(longlong)&local_348),
                local_408 != (undefined ********)0xffffffffffffffff) {
            if (local_3f8 == (undefined ********)&DAT_00000008) goto code_r0x000140040a43;
            if (local_408 != (undefined ********)0x0) {
              __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
            }
          }
          if ((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340 != 0) {
            uVar28 = (ulonglong)((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340) >> 5;
            ppppppppuVar31 = ppppppppuStack_340 + 1;
            do {
              if (ppppppppuVar31[-1] != (undefined *******)0x0) {
                __rustc::__rust_dealloc(*ppppppppuVar31,ppppppppuVar31[-1],1);
              }
              ppppppppuVar31 = ppppppppuVar31 + 4;
              uVar28 = uVar28 - 1;
            } while (uVar28 != 0);
          }
          if (local_338 != (undefined ********)0x0) {
            __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
          }
        }
        goto LAB_140040a8b;
      }
      core::ptr::drop_glue::<reach_launcher::update::Release>
                ((longlong *)&local_348);
    }
  }
  local_1b8 = (undefined ********)0xffffffffffffffff;
LAB_140040a8b:
  std::env::args(&local_348);
  local_328 = (undefined **)0x0;
  <std::env::Args_as_core::iter::traits::iterator::Iterator>::
  next((longlong *)&local_408,(longlong)&local_348);
  if (local_408 != (undefined ********)0xffffffffffffffff) {
    if (local_408 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
    }
    while (<std::env::Args_as_core::iter::traits::iterator::Iterator>
           ::next((longlong *)&local_408,(longlong)&local_348), ppppppppuVar25 = ppppppppuStack_400,
          ppppppppuVar31 = local_408, local_408 != (undefined ********)0xffffffffffffffff) {
      if ((local_3f8 < (undefined ********)0x2) || (*(short *)ppppppppuStack_400 != 0x2d2d)) {
        steam::windows_path((ulonglong *)&local_408,(short *)ppppppppuStack_400,(size_t)local_3f8);
        if (ppppppppuVar31 != (undefined ********)0x0) {
          __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
        }
        ppppppppuVar21 = local_3f8;
        ppppppppuVar25 = ppppppppuStack_400;
        ppppppppuVar31 = local_408;
        steam::mcc_from_pick((ulonglong *)&local_398,(byte *)ppppppppuStack_400,(size_t)local_3f8);
        if (steam::is_wine::WINE != 0) {
          <std::sync::once_lock::OnceLock<bool>>::initialize::
          <<std::sync::once_lock::OnceLock<bool>>::get_or_init<reach_launcher::steam::is_wine::_closure_0_>::_closure_0_,_>
                    ((uint *)&steam::is_wine::WINE);
        }
        cVar10 = DAT_14015702c;
        remembered_mcc_file(&local_408);
        ppppppppuVar8 = ppppppppuStack_3f0;
        ppppppppuVar7 = local_3f8;
        ppppppppuVar19 = ppppppppuStack_400;
        ppppppppuVar30 = local_408;
        if (((cVar10 == '\0') || (local_398 == (undefined ********)0xffffffffffffffff)) ||
           (local_408 == (undefined ********)0xffffffffffffffff)) {
          if ((undefined1 *)((longlong)local_408 + -1) < (undefined1 *)0xfffffffffffffffe) {
            __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
          }
        }
        else {
          ppppppppuVar22 = local_3f8;
          pWVar15 = (LPCWSTR)<std::path::Path>::parent
                                       ((byte *)ppppppppuStack_400,(ulonglong)local_3f8);
          if (pWVar15 == (LPCWSTR)0x0) {
            core::option::unwrap_failed();
            goto LAB_140042029;
          }
          local_408._0_1_ = 1;
          pWVar15 = <std::fs::DirBuilder>::_create
                              ((char *)&local_408,pWVar15,(ulonglong)ppppppppuVar22);
          if (((pWVar15 != (LPCWSTR)0x0) && (1 < ((uint)pWVar15 & 3) - 2)) &&
             (((ulonglong)pWVar15 & 3) != 0)) {
            ppppppppuStack_400 = (undefined ********)((longlong)pWVar15 + -1);
            local_408 = (undefined ********)CONCAT71(local_408._1_7_,3);
            <core::io::error::CustomOwner_as_core::ops::drop::Drop>
            ::drop((longlong *)&ppppppppuStack_400);
          }
          local_68 = ppppppppuStack_390;
          ppppppppuStack_60 = local_388;
          local_1d0 = (undefined ********)0x0;
          ppppppppuStack_1c8 = (undefined ********)0x1;
          local_1c0 = 0;
          local_1f8 = 0x60000020;
          local_208 = (undefined ********)&local_1d0;
          local_200 = (undefined ********)&PTR_<alloc::string::String>_14015dbe8;
          local_408 = ppppppppuVar30;
          ppppppppuStack_400 = ppppppppuVar19;
          local_3f8 = ppppppppuVar7;
          ppppppppuStack_3f0 = ppppppppuVar8;
          cVar10 = <std::path::Display_as_core::fmt::Display>::
                   fmt((longlong *)&local_68,&local_208);
          if (cVar10 != '\0') {
            core::result::unwrap_failed
                      ("a Display implementation returned an error unexpectedly",0x37,&local_369,
                       &DAT_14015dbc8,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015dca0);
            goto LAB_140042029;
          }
          local_78 = local_1c0;
          local_88 = local_1d0;
          ppppppppuStack_80 = ppppppppuStack_1c8;
          puVar16 = std::fs::write::
                    <std::path::PathBuf,alloc::string::String>
                              ((longlong *)&local_408,(longlong *)&local_88);
          if (((puVar16 != (undefined *)0x0) && (1 < ((uint)puVar16 & 3) - 2)) &&
             (((ulonglong)puVar16 & 3) != 0)) {
            ppppppppuStack_400 = (undefined ********)(puVar16 + -1);
            local_408 = (undefined ********)CONCAT71(local_408._1_7_,3);
            <core::io::error::CustomOwner_as_core::ops::drop::Drop>
            ::drop((longlong *)&ppppppppuStack_400);
          }
        }
        ppppppppuVar19 = ppppppppuStack_390;
        ppppppppuVar30 = ppppppppuVar31;
        ppppppppuStack_220 = ppppppppuVar25;
        if ((local_398 != (undefined ********)0xffffffffffffffff) &&
           (ppppppppuVar30 = local_398, ppppppppuVar21 = local_388,
           ppppppppuStack_220 = ppppppppuVar19, ppppppppuVar31 != (undefined ********)0x0)) {
          __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
        }
        local_228 = ppppppppuVar30;
        local_218 = ppppppppuVar21;
        <alloc::vec::into_iter::IntoIter<std::path::PathBuf>as_core::ops::drop::Drop>
        ::drop(&local_348);
        goto joined_r0x000140040eda;
      }
      if (local_408 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
      }
    }
  }
  if ((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340) >> 5;
    ppppppppuVar31 = ppppppppuStack_340 + 1;
    do {
      if (ppppppppuVar31[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar31,ppppppppuVar31[-1],1);
      }
      ppppppppuVar31 = ppppppppuVar31 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (local_338 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
  }
  remembered_mcc_file(&local_348);
  ppppppppuVar25 = ppppppppuStack_340;
  ppppppppuVar31 = local_348;
  if (local_348 == (undefined ********)0xffffffffffffffff) {
LAB_140040f1c:
    local_368 = (undefined ********)0xffffffffffffffff;
LAB_140040f28:
    if (DAT_140157020 != 0) {
      <std::sync::once_lock::OnceLock<core::option::Option<std::path::PathBuf>>>
      ::initialize::
      <<std::sync::once_lock::OnceLock<core::option::Option<std::path::PathBuf>>>::get_or_init<reach_launcher::steam::steam_root::_closure_0_>::_closure_0_,_>
                (0x140157000);
    }
    sVar6 = DAT_140157010;
    pvVar13 = DAT_140157008;
    if (DAT_140157000 != -1) {
      if ((longlong)DAT_140157010 < 0) {
        lVar23 = 0;
LAB_140040f53:
        alloc::raw_vec::handle_error(lVar23,sVar6);
        goto LAB_140042029;
      }
      if (DAT_140157010 == 0) {
        pvVar18 = (LPVOID)0x1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        lVar23 = 1;
        pvVar18 = (void *)__rustc::__rust_alloc(sVar6,1);
        if (pvVar18 == (void *)0x0) goto LAB_140040f53;
        memcpy(pvVar18,pvVar13,sVar6);
      }
      steam::find_mcc(&local_348,pvVar18,sVar6);
      if (sVar6 != 0) {
        __rustc::__rust_dealloc(pvVar18,sVar6,1);
      }
      goto LAB_14004103d;
    }
LAB_14004106a:
    steam::default_mcc((longlong *)&local_228);
  }
  else {
    std::fs::read_to_string::inner
              ((longlong *)&local_408,(longlong)ppppppppuStack_340,local_338);
    if (ppppppppuVar31 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
    }
    ppppppppuVar25 = ppppppppuStack_400;
    ppppppppuVar31 = local_408;
    if (local_408 == (undefined ********)0xffffffffffffffff) {
      if ((1 < ((uint)ppppppppuStack_400 & 3) - 2) && (((ulonglong)ppppppppuStack_400 & 3) != 0)) {
        ppppppppuStack_390 = (undefined ********)((longlong)ppppppppuStack_400 + -1);
        local_398 = (undefined ********)CONCAT71(local_398._1_7_,3);
        <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
        drop((longlong *)&ppppppppuStack_390);
      }
      goto LAB_140040f1c;
    }
    ppppppppuVar21 = local_3f8;
    pbVar14 = <str>::trim_matches::<<char>::is_whitespace>
                        ((byte *)ppppppppuStack_400,(longlong)local_3f8);
    steam::mcc_from_pick((ulonglong *)&local_368,pbVar14,(size_t)ppppppppuVar21);
    if (ppppppppuVar31 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
    }
    if (local_368 == (undefined ********)0xffffffffffffffff) goto LAB_140040f28;
    local_338 = local_358;
    ppppppppuStack_330 = ppppppppuStack_350;
    local_348 = local_368;
    ppppppppuStack_340 = ppppppppuStack_360;
LAB_14004103d:
    if (local_348 == (undefined ********)0xffffffffffffffff) goto LAB_14004106a;
    local_218 = local_338;
    ppppppppuStack_210 = ppppppppuStack_330;
    local_228 = local_348;
    ppppppppuStack_220 = ppppppppuStack_340;
  }
  ppppppppuVar30 = local_228;
  if (local_228 != (undefined ********)0xffffffffffffffff) {
LAB_140041089:
    ppppppppuVar21 = local_218;
    ppppppppuVar25 = ppppppppuStack_220;
    <std::path::Path>::_join
              (&local_348,ppppppppuStack_220,(size_t)local_218,(undefined8 *******)"haloreach",
               &DAT_00000009);
    ppppppppuVar31 = ppppppppuStack_340;
    <std::path::Path>::_join
              (&local_398,ppppppppuStack_340,(size_t)local_338,(undefined8 *******)"haloreach.dll",
               &DAT_0000000d);
    if (local_348 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar31,local_348,1);
    }
    ppppppppuVar31 = ppppppppuStack_390;
    build::sha256_file((ulonglong *)&local_368,ppppppppuStack_390,(longlong)local_388);
    ppppppppuVar7 = ppppppppuStack_360;
    ppppppppuVar19 = local_368;
    if (local_368 == (undefined ********)0xffffffffffffffff) {
      local_208 = ppppppppuStack_360;
      local_228 = ppppppppuVar31;
      ppppppppuStack_220 = local_388;
      local_348 = (undefined ********)&local_228;
      ppppppppuStack_340 =
           (undefined ********)
           <std::path::Display_as_core::fmt::Display>::fmt;
      local_338 = (undefined ********)&local_208;
      ppppppppuStack_330 =
           (undefined ********)
           <core::io::error::Error_as_core::fmt::Display>::fmt;
      alloc::fmt::format::format_inner
                ((ulonglong *)&local_408,(ushort *)&DAT_14015d8fa,(ulonglong)&local_348);
      ppppppppuVar31 = ppppppppuStack_400;
      ppppppppuVar34 = local_408;
      message((undefined *)ppppppppuStack_400,(longlong)local_3f8);
      if (ppppppppuVar34 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
      }
      uVar11 = main::_closure_7_((longlong *)&local_1f0,(longlong *)&local_1b8,3);
      uVar28 = CONCAT71(extraout_var,uVar11) & 0xffffffff;
      if ((1 < ((uint)local_208 & 3) - 2) && (((ulonglong)local_208 & 3) != 0)) {
        ppppppppuStack_340 = (undefined ********)((longlong)local_208 + -1);
        local_348 = (undefined ********)CONCAT71(local_348._1_7_,3);
        <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
        drop((longlong *)&ppppppppuStack_340);
      }
      bVar12 = 1;
    }
    else {
      ppuVar17 = (undefined **)build::match_known((byte *)ppppppppuStack_360,(ulonglong)local_358);
      puVar20 = local_1e8;
      sVar6 = local_1f0;
      if (ppuVar17 == (undefined **)0x0) {
        local_408 = (undefined ********)
                    &
                    PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
        ;
        ppppppppuStack_400 = (undefined ********)<&str_as_core::fmt::Display>::fmt;
        alloc::fmt::format::format_inner
                  ((ulonglong *)&local_348,(ushort *)&DAT_14015d877,(ulonglong)&local_408);
        ppppppppuVar31 = ppppppppuStack_340;
        ppppppppuVar34 = local_348;
        message((undefined *)ppppppppuStack_340,(longlong)local_338);
        if (ppppppppuVar34 != (undefined ********)0x0) {
          __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
        }
        uVar11 = main::_closure_7_((longlong *)&local_1f0,(longlong *)&local_1b8,4);
        uVar28 = CONCAT71(extraout_var_01,uVar11) & 0xffffffff;
joined_r0x000140041397:
        bVar26 = 1;
        bVar12 = 1;
      }
      else {
        if (ppppppppuVar34 != (undefined ********)0xffffffffffffffff) {
          build::read_build_tag(&local_348,ppppppppuVar25,(size_t)ppppppppuVar21);
          if (local_348 == (undefined ********)0xffffffffffffffff) {
            main::_closure_8_(&local_408);
          }
          else {
            local_3f8 = local_338;
            local_408 = local_348;
            ppppppppuStack_400 = ppppppppuStack_340;
          }
          local_368 = ppppppppuVar25;
          ppppppppuStack_360 = ppppppppuVar21;
          local_318 = ppuVar17 + 4;
          ppppppppuStack_340 =
               (undefined ********)
               <std::path::Display_as_core::fmt::Display>::fmt;
          local_338 = (undefined ********)&local_408;
          ppppppppuStack_330 =
               (undefined ********)
               &<alloc::string::String_as_core::fmt::Display>::
                fmt;
          local_320 = <&str_as_core::fmt::Display>::fmt;
          local_310 = <u32_as_core::fmt::Display>::fmt;
          local_348 = (undefined ********)&local_368;
          local_328 = ppuVar17;
          std::io::stdio::_print((ushort *)&DAT_14015d834,(ulonglong)&local_348);
          if (local_408 != (undefined ********)0x0) {
            __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
          }
          uVar28 = 0;
          goto joined_r0x000140041397;
        }
        if (local_1f0 == 0xffffffffffffffff) {
          run_menus_script(&local_348);
          ppppppppuVar21 = local_338;
          ppppppppuVar34 = local_348;
          if (local_348 != (undefined ********)0xffffffffffffffff) {
            local_3a0 = ppppppppuStack_340;
            <std::sys::process::windows::Command>::new
                      ((size_t *)&local_348,"powershell",10);
            local_408 = (undefined ********)0x14015d6a5;
            ppppppppuStack_400 = (undefined ********)&DAT_0000000a;
            local_3f8 = (undefined ********)0x14015d6af;
            ppppppppuStack_3f0 = (undefined ********)&DAT_00000010;
            local_3e8 = "Bypass";
            local_3e0 = 6;
            local_3d8 = "-WindowStyle";
            local_3d0 = 0xc;
            local_3c8 = "Hidden";
            local_3c0 = 6;
            local_3b8 = "-Filesrc\\main.rs";
            local_3b0 = 5;
            <std::process::Command>::args::<[&str_6usize],&str>
                      ((longlong)&local_348,&local_408);
            <std::sys::process::windows::Command>::arg
                      ((longlong)&local_348,local_3a0,(size_t)ppppppppuVar21);
            pvVar13 = (void *)<std::path::Path>::parent
                                        ((byte *)local_3a0,(ulonglong)ppppppppuVar21);
            if (pvVar13 != (void *)0x0) {
              <std::sys::process::windows::Command>::cwd
                        ((longlong)&local_348,pvVar13,(size_t)ppppppppuVar21);
              local_29c = 0x8000000;
              <std::process::Command>::spawn((longlong *)&local_408,&local_348);
              core::ptr::drop_glue::<std::process::Command>
                        ((longlong *)&local_348);
              if ((int)local_408 == 2) {
                local_228 = ppppppppuStack_400;
                local_368 = (undefined ********)&local_228;
                ppppppppuStack_360 =
                     (undefined ********)
                     <core::io::error::Error_as_core::fmt::Display>
                     ::fmt;
                alloc::fmt::format::format_inner
                          ((ulonglong *)&local_348,(ushort *)&DAT_14015d700,(ulonglong)&local_368);
                ppppppppuVar8 = ppppppppuStack_340;
                ppppppppuVar21 = local_348;
                message((undefined *)ppppppppuStack_340,(longlong)local_338);
                if (ppppppppuVar21 != (undefined ********)0x0) {
                  __rustc::__rust_dealloc(ppppppppuVar8,ppppppppuVar21,1);
                }
                core::ptr::drop_glue::<core::io::error::Error>
                          ((ulonglong)local_228);
                uVar28 = 6;
              }
              else {
                uVar28 = 0;
                core::ptr::drop_glue::
                <core::result::Result<std::process::Child,core::io::error::Error>>
                          ((longlong *)&local_408);
              }
              if (ppppppppuVar34 != (undefined ********)0x0) {
                __rustc::__rust_dealloc(local_3a0,ppppppppuVar34,1);
              }
              if (ppppppppuVar19 != (undefined ********)0x0) {
                __rustc::__rust_dealloc(ppppppppuVar7,ppppppppuVar19,1);
              }
              if (local_398 != (undefined ********)0x0) {
                __rustc::__rust_dealloc(ppppppppuVar31,local_398,1);
              }
              if (ppppppppuVar30 != (undefined ********)0x0) {
                __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar30,1);
              }
              if (local_1b8 == (undefined ********)0xffffffffffffffff) {
                return uVar28;
              }
              core::ptr::drop_glue::
              <reach_launcher::update::Release>((longlong *)&local_1b8);
              return uVar28;
            }
            core::option::unwrap_failed();
LAB_140042029:
                    /* WARNING: Does not return */
            pcVar4 = (code *)invalidInstructionException();
            (*pcVar4)();
          }
          std::env::current_exe((ulonglong *)&local_408);
          if (local_408 == (undefined ********)0xffffffffffffffff) {
            ppppppppuVar34 = (undefined ********)0xffffffffffffffff;
            core::ptr::drop_glue::
            <core::result::Result<std::path::PathBuf,core::io::error::Error>>
                      (-1,ppppppppuStack_400);
LAB_140041f29:
            message(&DAT_14015d71e,0x46);
          }
          else {
            ppppppppuStack_330 = ppppppppuStack_3f0;
            ppppppppuStack_340 = ppppppppuStack_400;
            local_338 = local_3f8;
            local_348 = local_408;
            main::_closure_10_((size_t *)&local_408,(longlong *)&local_348);
            ppppppppuVar34 = local_408;
            if (local_408 == (undefined ********)0xffffffffffffffff) {
              ppppppppuVar34 = (undefined ********)0xffffffffffffffff;
              goto LAB_140041f29;
            }
            bVar12 = package::looks_like_package(ppppppppuStack_400,(size_t)local_3f8);
            if (bVar12 == 0) goto LAB_140041f29;
            local_348 = (undefined ********)
                        &
                        PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
            ;
            ppppppppuStack_340 =
                 (undefined ********)<&str_as_core::fmt::Display>::fmt;
            local_338 = (undefined ********)
                        &
                        PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388
            ;
            ppppppppuStack_330 =
                 (undefined ********)<&str_as_core::fmt::Display>::fmt;
            alloc::fmt::format::format_inner
                      ((ulonglong *)&local_368,(ushort *)&DAT_14015d764,(ulonglong)&local_348);
            ppppppppuVar21 = ppppppppuStack_360;
            ppppppppuVar31 = local_368;
            message((undefined *)ppppppppuStack_360,(longlong)local_358);
            if (ppppppppuVar31 != (undefined ********)0x0) {
              __rustc::__rust_dealloc(ppppppppuVar21,ppppppppuVar31,1);
            }
          }
          if ((undefined1 *)((longlong)ppppppppuVar34 + -1) < (undefined1 *)0xfffffffffffffffe) {
            __rustc::__rust_dealloc(ppppppppuStack_400,ppppppppuVar34,1);
          }
          uVar28 = 5;
          goto joined_r0x000140041397;
        }
        if (steam::is_wine::WINE != 0) {
          <std::sync::once_lock::OnceLock<bool>>::initialize::
          <<std::sync::once_lock::OnceLock<bool>>::get_or_init<reach_launcher::steam::is_wine::_closure_0_>::_closure_0_,_>
                    ((uint *)&steam::is_wine::WINE);
        }
        if (DAT_14015702c != '\0') {
          proton::prepare_webview2(puVar20,local_1e0);
        }
        ppppppppuVar34 = ppppppppuStack_1b0;
        if (local_1b8 == (undefined ********)0xffffffffffffffff) {
          ppppppppuVar34 = (undefined ********)0x0;
        }
        package::start((ulonglong *)&local_348,puVar20,local_1e0,(undefined *)ppppppppuVar25,
                       ppppppppuVar21,(longlong)ppppppppuVar34,(longlong)ppppppppuStack_1a8);
        ppppppppuVar31 = ppppppppuStack_340;
        ppppppppuVar34 = local_348;
        if (local_348 == (undefined ********)0xffffffffffffffff) {
          uVar28 = 0;
        }
        else {
          message((undefined *)ppppppppuStack_340,(longlong)local_338);
          uVar28 = 6;
          if (ppppppppuVar34 != (undefined ********)0x0) {
            __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
          }
        }
        if (sVar6 != 0) {
          __rustc::__rust_dealloc(puVar20,sVar6,1);
        }
        bVar26 = 0;
        bVar12 = 0;
      }
      if (ppppppppuVar19 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuVar7,ppppppppuVar19,1);
        bVar12 = bVar26;
      }
    }
    if (local_398 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuStack_390,local_398,1);
    }
    if (ppppppppuVar30 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar30,1);
    }
    if (local_1b8 != (undefined ********)0xffffffffffffffff) {
      core::ptr::drop_glue::<reach_launcher::update::Release>
                ((longlong *)&local_1b8);
    }
    if (!(bool)(bVar12 & local_1f0 != 0xffffffffffffffff)) {
      return uVar28;
    }
    goto LAB_1400415e3;
  }
  if (ppppppppuVar34 == (undefined ********)0xffffffffffffffff) {
    local_408 = (undefined ********)
                &PTR_anon_0ba670aeb02bc321804ac4e2f5d0109e_14_llvm_15297295612839780106_14015d388;
    ppppppppuStack_400 = (undefined ********)<&str_as_core::fmt::Display>::fmt;
    alloc::fmt::format::format_inner
              ((ulonglong *)&local_348,(ushort *)&DAT_14015d9c0,(ulonglong)&local_408);
    ppppppppuVar25 = ppppppppuStack_340;
    ppppppppuVar31 = local_348;
    message((undefined *)ppppppppuStack_340,(longlong)local_338);
    if (ppppppppuVar31 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
    }
    if (DAT_140157020 != 0) {
      <std::sync::once_lock::OnceLock<core::option::Option<std::path::PathBuf>>>
      ::initialize::
      <<std::sync::once_lock::OnceLock<core::option::Option<std::path::PathBuf>>>::get_or_init<reach_launcher::steam::steam_root::_closure_0_>::_closure_0_,_>
                (0x140157000);
    }
    sVar6 = DAT_140157010;
    pvVar13 = DAT_140157008;
    if (DAT_140157000 == -1) goto LAB_140041c22;
    if ((longlong)DAT_140157010 < 0) {
      lVar23 = 0;
LAB_1400414a0:
      alloc::raw_vec::handle_error(lVar23,sVar6);
      goto LAB_140042029;
    }
    if (DAT_140157010 == 0) {
      pvVar18 = (LPVOID)0x1;
    }
    else {
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      lVar23 = 1;
      pvVar18 = (void *)__rustc::__rust_alloc(sVar6,1);
      if (pvVar18 == (void *)0x0) goto LAB_1400414a0;
      memcpy(pvVar18,pvVar13,sVar6);
    }
    <std::path::Path>::_join
              (&local_348,pvVar18,sVar6,(undefined8 *******)"steamapps",&DAT_00000009);
    ppppppppuVar31 = ppppppppuStack_340;
    <std::path::Path>::_join
              (&local_408,ppppppppuStack_340,(size_t)local_338,(undefined8 *******)"common",
               &DAT_00000006);
    if (local_348 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar31,local_348,1);
    }
    if (sVar6 != 0) {
      __rustc::__rust_dealloc(pvVar18,sVar6,1);
    }
    ppppppppuVar21 = local_3f8;
    ppppppppuVar25 = ppppppppuStack_400;
    ppppppppuVar31 = local_408;
    if (local_408 == (undefined ********)0xffffffffffffffff) {
LAB_140041c22:
      std::sys::env::windows::getenv((longlong *)&local_348,0x14015d32e,0x11);
      if (local_348 == (undefined ********)0xffffffffffffffff) {
        <core::wtf8::Wtf8>::to_owned((size_t *)&local_408,"C:\\",3);
        ppppppppuVar25 = ppppppppuStack_400;
        ppppppppuVar31 = local_408;
        ppppppppuVar21 = local_3f8;
      }
      else {
        local_3f8 = local_338;
        ppppppppuStack_3f0 = ppppppppuStack_330;
        local_408 = local_348;
        ppppppppuStack_400 = ppppppppuStack_340;
        ppppppppuVar25 = ppppppppuStack_340;
        ppppppppuVar31 = local_348;
        ppppppppuVar21 = local_338;
      }
    }
    else {
      bVar12 = <std::path::Path>::is_dir
                         ((longlong)ppppppppuStack_400,(longlong)local_3f8);
      if (bVar12 == 0) {
        if (ppppppppuVar31 != (undefined ********)0x0) {
          __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
        }
        goto LAB_140041c22;
      }
    }
    if (steam::is_wine::WINE != 0) {
      <std::sync::once_lock::OnceLock<bool>>::initialize::
      <<std::sync::once_lock::OnceLock<bool>>::get_or_init<reach_launcher::steam::is_wine::_closure_0_>::_closure_0_,_>
                ((uint *)&steam::is_wine::WINE);
    }
    if ((DAT_14015702c == '\0') ||
       (proton::pick_folder_showing_dot_files
                  ((size_t *)&local_348,ppppppppuVar25,(size_t)ppppppppuVar21),
       local_348 == (undefined ********)0xfffffffffffffffe)) {
      pick::folder((ulonglong *)&local_408,0x14015d342,0x32,(byte *)ppppppppuVar25,
                   (ulonglong)ppppppppuVar21);
    }
    else {
      local_3f8 = local_338;
      ppppppppuStack_3f0 = ppppppppuStack_330;
      local_408 = local_348;
      ppppppppuStack_400 = ppppppppuStack_340;
    }
    ppppppppuVar19 = local_3f8;
    ppppppppuVar30 = ppppppppuStack_400;
    ppppppppuVar21 = local_408;
    if (local_408 == (undefined ********)0xffffffffffffffff) {
      if (ppppppppuVar31 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
      }
    }
    else {
      steam::mcc_from_pick((ulonglong *)&local_398,(byte *)ppppppppuStack_400,(size_t)local_3f8);
      ppppppppuVar8 = local_388;
      ppppppppuVar7 = ppppppppuStack_390;
      if (local_398 == (undefined ********)0xffffffffffffffff) {
        local_208 = ppppppppuVar30;
        local_200 = ppppppppuVar19;
        local_408 = (undefined ********)&local_208;
        ppppppppuStack_400 =
             (undefined ********)
             <std::path::Display_as_core::fmt::Display>::fmt;
        alloc::fmt::format::format_inner
                  ((ulonglong *)&local_348,(ushort *)&DAT_14015daf1,(ulonglong)&local_408);
        ppppppppuVar7 = ppppppppuStack_340;
        ppppppppuVar19 = local_348;
        message((undefined *)ppppppppuStack_340,(longlong)local_338);
        if (ppppppppuVar19 != (undefined ********)0x0) {
          __rustc::__rust_dealloc(ppppppppuVar7,ppppppppuVar19,1);
        }
      }
      else {
        remembered_mcc_file(&local_408);
        if (local_408 != (undefined ********)0xffffffffffffffff) {
          local_3a0 = ppppppppuStack_400;
          ppppppppuVar19 = local_3f8;
          pWVar15 = (LPCWSTR)<std::path::Path>::parent
                                       ((byte *)ppppppppuStack_400,(ulonglong)local_3f8);
          if (pWVar15 == (LPCWSTR)0x0) {
            core::option::unwrap_failed();
            goto LAB_140042029;
          }
          local_348 = (undefined ********)CONCAT71(local_348._1_7_,1);
          pWVar15 = <std::fs::DirBuilder>::_create
                              ((char *)&local_348,pWVar15,(ulonglong)ppppppppuVar19);
          core::ptr::drop_glue::
          <core::result::Result<(),core::io::error::Error>>
                    ((ulonglong)pWVar15);
          local_338 = local_3f8;
          ppppppppuStack_330 = ppppppppuStack_3f0;
          local_348 = local_408;
          ppppppppuStack_340 = ppppppppuStack_400;
          local_1d0 = ppppppppuVar7;
          ppppppppuStack_1c8 = ppppppppuVar8;
          <std::path::Display_as_alloc::string::SpecToString>::
          spec_to_string(&local_208,(longlong *)&local_1d0);
          puVar16 = std::fs::write::
                    <std::path::PathBuf,alloc::string::String>
                              ((longlong *)&local_348,(longlong *)&local_208);
          core::ptr::drop_glue::
          <core::result::Result<(),core::io::error::Error>>
                    ((ulonglong)puVar16);
        }
        local_228 = local_398;
        ppppppppuStack_220 = ppppppppuStack_390;
        local_218 = local_388;
        ppppppppuStack_210 = ppppppppuStack_380;
      }
      if (ppppppppuVar21 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuVar30,ppppppppuVar21,1);
      }
      ppppppppuVar30 = local_228;
      if (ppppppppuVar31 != (undefined ********)0x0) {
        __rustc::__rust_dealloc(ppppppppuVar25,ppppppppuVar31,1);
        ppppppppuVar30 = local_228;
      }
joined_r0x000140040eda:
      if (ppppppppuVar30 != (undefined ********)0xffffffffffffffff) goto LAB_140041089;
    }
  }
  else {
    std::io::stdio::_eprint
              ((ushort *)"MCC not found in the Steam libraries\n",0x4b);
  }
  uVar11 = main::_closure_7_((longlong *)&local_1f0,(longlong *)&local_1b8,2);
  uVar28 = CONCAT71(extraout_var_00,uVar11) & 0xffffffff;
LAB_1400412e1:
  if (local_1b8 != (undefined ********)0xffffffffffffffff) {
    core::ptr::drop_glue::<reach_launcher::update::Release>
              ((longlong *)&local_1b8);
  }
  if (local_1f0 == 0xffffffffffffffff) {
    return uVar28;
  }
LAB_1400415e3:
  if (local_1f0 != 0) {
    __rustc::__rust_dealloc(local_1e8,local_1f0,1);
  }
  return uVar28;
  while (lVar23 = lVar23 + 1, lVar23 != 0) {
LAB_140040175:
    <std::env::Args_as_core::iter::traits::iterator::Iterator>::
    next((longlong *)&local_408,(longlong)&local_348);
    if (local_408 != (undefined ********)0x0) {
      if (local_408 == (undefined ********)0xffffffffffffffff) goto LAB_14004026d;
      __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
    }
  }
LAB_140040195:
  <std::env::Args_as_core::iter::traits::iterator::Iterator>::
  next((longlong *)&local_408,(longlong)&local_348);
  if (local_408 == (undefined ********)0xffffffffffffffff) {
LAB_14004026d:
    local_3a0 = (undefined ********)0x1;
    local_410 = (undefined ********)0x0;
    uVar28 = (longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340;
  }
  else {
    uVar28 = (longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340;
    local_3a0 = ppppppppuStack_400;
    local_410 = local_408;
  }
  if (uVar28 != 0) {
    uVar28 = uVar28 >> 5;
    ppppppppuVar34 = ppppppppuStack_340 + 1;
    do {
      if (ppppppppuVar34[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar34,ppppppppuVar34[-1],1);
      }
      ppppppppuVar34 = ppppppppuVar34 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (local_338 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
  }
  <reach_launcher::progress::Window>::headed(&local_348,"UPDATING",8,in_R9);
  uVar28 = 0;
  iVar27 = 0;
  do {
    local_368 = (undefined ********)CONCAT44(local_368._4_4_,(int)(uVar28 / 0x78));
    local_398 = (undefined ********)&local_368;
    ppppppppuStack_390 = (undefined ********)<i32_as_core::fmt::Display>::fmt;
    alloc::fmt::format::format_inner
              ((ulonglong *)&local_408,(ushort *)&DAT_14015d97e,(ulonglong)&local_398);
    ppppppppuVar31 = ppppppppuStack_400;
    ppppppppuVar34 = local_408;
    <reach_launcher::progress::Window>::set
              ((longlong *)&local_348,(float)iVar27 / 120.0,ppppppppuStack_400,(size_t)local_3f8);
    if (ppppppppuVar34 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
    }
    puVar20 = (undefined8 *)0x2faf080;
    std::thread::functions::sleep(0,50000000);
    iVar27 = iVar27 + 1;
    uVar28 = (ulonglong)((int)uVar28 + 0x176);
  } while (iVar27 != 0x79);
  core::ptr::drop_glue::<reach_launcher::progress::Window>
            (&local_348,puVar20);
  if (local_410 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_3a0,local_410,1);
  }
  if ((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0) >> 5;
    ppppppppuVar34 = ppppppppuStack_1b0 + 1;
    do {
      if (ppppppppuVar34[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar34,ppppppppuVar34[-1],1);
      }
      ppppppppuVar34 = ppppppppuVar34 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  goto LAB_140040782;
code_r0x000140040a43:
  pppppppuVar29 = *ppppppppuStack_400;
  if (local_408 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
  }
  if (pppppppuVar29 == (undefined *******)0x6574616470752d2d) goto LAB_140041604;
  goto LAB_140040a1c;
LAB_140041604:
  if ((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340) >> 5;
    ppppppppuVar31 = ppppppppuStack_340 + 1;
    do {
      if (ppppppppuVar31[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar31,ppppppppuVar31[-1],1);
      }
      ppppppppuVar31 = ppppppppuVar31 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (local_338 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
  }
  package::wait_for_menus_to_close();
  uVar24 = install_update(puVar20,sVar6,(longlong)&local_1b8,
                          CONCAT71((int7)((ulonglong)uVar32 >> 8),1));
  uVar28 = 0;
  if ((char)uVar24 == '\0') {
    bVar9 = update::unfinished(puVar20,sVar6);
    uVar28 = 7;
    if (!bVar9) goto LAB_140040a8b;
  }
  goto LAB_1400412e1;
LAB_14003fa89:
  do {
    <std::env::Args_as_core::iter::traits::iterator::Iterator>::
    next((longlong *)&local_408,(longlong)&local_348);
    if (local_408 != (undefined ********)0x0) {
      if (local_408 == (undefined ********)0xffffffffffffffff) {
        local_3f8 = (undefined ********)0xffffffffffffffff;
        ppppppppuVar34 = (undefined ********)0x0;
        ppppppppuVar31 = (undefined ********)0xffffffffffffffff;
        goto LAB_1400404e8;
      }
      __rustc::__rust_dealloc(ppppppppuStack_400,local_408,1);
    }
    lVar33 = lVar33 + 1;
  } while (lVar33 != 0);
LAB_14004049d:
  <std::env::Args_as_core::iter::traits::iterator::Iterator>::
  next((longlong *)&local_408,(longlong)&local_348);
  ppppppppuVar34 = ppppppppuStack_400;
  ppppppppuVar31 = local_408;
  if (local_408 == (undefined ********)0xffffffffffffffff) {
    ppppppppuVar34 = (undefined ********)0x0;
  }
LAB_1400404e8:
  proton::put_back_dot_files((longlong)ppppppppuVar34,(longlong)local_3f8);
  if ((ppppppppuVar31 != (undefined ********)0xffffffffffffffff) &&
     (ppppppppuVar31 != (undefined ********)0x0)) {
    __rustc::__rust_dealloc(ppppppppuStack_400,ppppppppuVar31,1);
  }
  if ((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340) >> 5;
    ppppppppuVar34 = ppppppppuStack_340 + 1;
    do {
      if (ppppppppuVar34[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar34,ppppppppuVar34[-1],1);
      }
      ppppppppuVar34 = ppppppppuVar34 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (local_338 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
  }
  std::env::args_os(&local_348);
  ppppppppuVar34 = ppppppppuStack_330;
  uVar24 = lVar23 - 1;
  uVar35 = (ulonglong)((longlong)ppppppppuStack_330 - (longlong)ppppppppuStack_340) >> 5;
  uVar28 = uVar35;
  if (uVar24 < uVar35) {
    uVar28 = uVar24;
  }
  ppppppppuVar31 = ppppppppuStack_340 + uVar28 * 4;
  if (uVar28 != 0) {
    uVar28 = uVar35;
    if (uVar24 < uVar35) {
      uVar28 = uVar24;
    }
    ppppppppuVar25 = ppppppppuStack_340 + 1;
    do {
      if (ppppppppuVar25[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar25,ppppppppuVar25[-1],1);
      }
      ppppppppuVar25 = ppppppppuVar25 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  pppppppuVar29 = (undefined *******)0x1;
  ppppppppuVar25 = ppppppppuVar31;
  if (ppppppppuVar31 == ppppppppuVar34 || uVar35 < uVar24) {
joined_r0x00014004063d:
    pppppppuVar3 = (undefined *******)0x0;
    pppppppuVar2 = (undefined *******)0x0;
  }
  else {
    ppppppppuVar25 = ppppppppuVar31 + 4;
    pppppppuVar2 = *ppppppppuVar31;
    if (pppppppuVar2 == (undefined *******)0xffffffffffffffff) goto joined_r0x00014004063d;
    pppppppuVar29 = ppppppppuVar31[1];
    pppppppuVar3 = ppppppppuVar31[2];
  }
  uVar28 = (longlong)ppppppppuVar34 - (longlong)ppppppppuVar25;
  if (uVar28 != 0) {
    uVar28 = uVar28 >> 5;
    ppppppppuVar25 = ppppppppuVar25 + 1;
    do {
      if (ppppppppuVar25[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar25,ppppppppuVar25[-1],1);
      }
      ppppppppuVar25 = ppppppppuVar25 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
  if (local_338 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_348,(longlong)local_338 << 5,8);
  }
  pick::folder((ulonglong *)&local_348,0x14015d342,0x32,(byte *)pppppppuVar29,
               (ulonglong)pppppppuVar3);
  ppppppppuVar31 = ppppppppuStack_340;
  ppppppppuVar34 = local_348;
  if (local_348 != (undefined ********)0xffffffffffffffff) {
    local_398 = ppppppppuStack_340;
    ppppppppuStack_390 = local_338;
    local_408 = (undefined ********)&local_398;
    ppppppppuStack_400 =
         (undefined ********)
         <std::path::Display_as_core::fmt::Display>::fmt;
    std::io::stdio::_print((ushort *)&DAT_14015d9bc,(ulonglong)&local_408);
    if (ppppppppuVar34 != (undefined ********)0x0) {
      __rustc::__rust_dealloc(ppppppppuVar31,ppppppppuVar34,1);
    }
  }
  if (pppppppuVar2 != (undefined *******)0x0) {
    __rustc::__rust_dealloc(pppppppuVar29,pppppppuVar2,1);
  }
  if ((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0 != 0) {
    uVar28 = (ulonglong)((longlong)ppppppppuStack_1a0 - (longlong)ppppppppuStack_1b0) >> 5;
    ppppppppuVar34 = ppppppppuStack_1b0 + 1;
    do {
      if (ppppppppuVar34[-1] != (undefined *******)0x0) {
        __rustc::__rust_dealloc(*ppppppppuVar34,ppppppppuVar34[-1],1);
      }
      ppppppppuVar34 = ppppppppuVar34 + 4;
      uVar28 = uVar28 - 1;
    } while (uVar28 != 0);
  }
LAB_140040782:
  if (ppppppppuStack_1a8 != (undefined ********)0x0) {
    __rustc::__rust_dealloc(local_1b8,(longlong)ppppppppuStack_1a8 << 5,8);
  }
  return 0;
}


