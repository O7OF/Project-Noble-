
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::workshop::check_and_fetch */

undefined8 * __rustcall
menu_overlay::workshop::check_and_fetch
          (undefined8 *param_1,undefined8 param_2,undefined8 param_3,byte ******param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7)

{
  code *pcVar1;
  longlong lVar2;
  byte bVar3;
  byte bVar4;
  char cVar5;
  ulonglong uVar6;
  longlong lVar7;
  byte *pbVar8;
  char *pcVar9;
  longlong lVar10;
  longlong *plVar11;
  undefined8 uVar12;
  undefined8 *puVar13;
  byte *******pppppppbVar14;
  uint uVar15;
  byte *******pppppppbVar16;
  byte *******pppppppbVar17;
  byte *******pppppppbVar18;
  byte *******pppppppbVar19;
  byte *pbVar20;
  uint uVar21;
  byte *pbVar22;
  byte *******pppppppbVar23;
  byte *******pppppppbVar24;
  byte *******pppppppbVar25;
  bool bVar26;
  bool bVar27;
  byte *******local_258;
  byte *******local_250;
  undefined8 uStack_248;
  undefined8 uStack_240;
  byte *******local_238;
  undefined2 local_22a;
  byte *******local_228;
  undefined8 local_220;
  undefined8 uStack_218;
  byte *******local_210;
  undefined8 local_208;
  undefined8 uStack_200;
  byte *******local_1f8;
  undefined8 uStack_1f0;
  undefined8 local_1e8;
  undefined8 uStack_1e0;
  undefined4 local_1d8;
  undefined4 uStack_1d4;
  undefined4 uStack_1d0;
  undefined4 uStack_1cc;
  undefined8 local_1c8;
  undefined8 uStack_1c0;
  undefined8 local_1b8;
  undefined8 uStack_1b0;
  undefined8 local_1a8;
  undefined8 local_1a0;
  byte *******local_198;
  undefined2 local_190;
  byte *******local_188;
  undefined8 uStack_180;
  undefined8 local_178;
  undefined4 uStack_170;
  undefined4 uStack_16c;
  undefined4 local_168;
  undefined4 uStack_164;
  undefined4 uStack_160;
  undefined4 uStack_15c;
  undefined4 local_158;
  undefined4 uStack_154;
  undefined4 uStack_150;
  undefined4 uStack_14c;
  undefined8 local_148;
  undefined8 uStack_140;
  undefined8 local_138;
  undefined8 uStack_130;
  undefined8 local_128;
  ulonglong local_118;
  undefined8 local_110;
  undefined4 local_108;
  undefined4 uStack_104;
  undefined4 uStack_100;
  undefined4 uStack_fc;
  byte *******local_f8;
  byte **local_e8;
  code *local_e0;
  undefined4 local_d8;
  undefined4 uStack_d4;
  undefined4 uStack_d0;
  undefined4 uStack_cc;
  byte *******local_c8;
  byte *******local_c0;
  undefined8 local_b8;
  longlong local_b0;
  byte ******local_a8;
  undefined8 local_a0;
  byte *local_98;
  byte *******local_90;
  undefined4 local_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  byte *******local_78;
  undefined4 local_70;
  undefined4 uStack_6c;
  undefined4 uStack_68;
  undefined4 uStack_64;
  undefined8 local_60;
  undefined4 local_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  undefined8 local_48;
  
  local_a0 = param_5;
  uVar12 = param_2;
  local_a8 = param_4;
  uVar6 = online::browser_port();
  if ((uVar6 & 1) == 0) {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    pcVar9 = (char *)__rustc::__rust_alloc(0x70,1);
    if (pcVar9 != (char *)0x0) {
      builtin_strncpy(pcVar9,
                      "The server browser isn\'t running, so its mod requirements couldn\'t be checked. Restart the client and try again."
                      ,0x70);
      param_1[1] = 0x70;
      param_1[2] = pcVar9;
      param_1[3] = 0x70;
      *param_1 = 1;
      return param_1;
    }
    alloc::raw_vec::handle_error(1,0x70);
                    /* WARNING: Does not return */
    pcVar1 = (code *)invalidInstructionException();
    (*pcVar1)();
  }
  local_22a = (undefined2)uVar12;
  <str>::replace::<char>(&local_188,param_2,param_3,0x25,&DAT_1401c0991,3);
  pppppppbVar17 = (byte *******)uStack_180;
  <str>::replace::<char>(&local_250,uStack_180,local_178,0x26,&DAT_1401c0994,3);
  pppppppbVar18 = (byte *******)uStack_248;
  <str>::replace::<char>(&local_228,uStack_248,uStack_240,0x3d,&DAT_1401c0997,3);
  local_208 = (byte *******)&local_22a;
  uStack_200 = <u16_as_core::fmt::Display>::fmt;
  uStack_1f0 = <alloc::string::String_as_core::fmt::Display>::fmt
  ;
  local_1e8 = &local_a8;
  uStack_1e0 = (code *)&<&str_as_core::fmt::Display>::fmt;
  local_1f8 = (byte *******)&local_228;
  alloc::fmt::format::format_inner(&local_108,&DAT_1401c099a,&local_208);
  if (local_228 != (byte *******)0x0) {
    __rustc::__rust_dealloc(local_220,local_228,1);
  }
  if (local_250 != (byte *******)0x0) {
    __rustc::__rust_dealloc(pppppppbVar18,local_250,1);
  }
  if (local_188 != (byte *******)0x0) {
    __rustc::__rust_dealloc(pppppppbVar17,local_188,1);
  }
  lVar2 = CONCAT44(uStack_104,local_108);
  local_110 = CONCAT44(uStack_fc,uStack_100);
  online::http_with_timeout(&local_250,&DAT_1401c09c7,3,local_110,local_f8,0);
  pppppppbVar17 = uStack_240;
  pppppppbVar18 = (byte *******)uStack_248;
  if ((int)local_250 != 1) {
    parse_wanted(&local_228,uStack_240,local_238);
    if (pppppppbVar18 != (byte *******)0x0) {
      __rustc::__rust_dealloc(pppppppbVar17,pppppppbVar18,1);
    }
    pppppppbVar17 = local_210;
    pppppppbVar18 = uStack_218;
    if ((int)local_228 != 1) {
      local_78 = local_210;
      local_d8 = (undefined4)local_220;
      uStack_d4 = local_220._4_4_;
      uStack_d0 = (undefined4)uStack_218;
      uStack_cc = uStack_218._4_4_;
      local_c8 = local_210;
      lVar7 = (longlong)local_210 * 0x38;
      while( true ) {
        LOCK();
        bVar26 = PRE == '\0';
        if (bVar26) {
          PRE = '\x01';
        }
        UNLOCK();
        if (!bVar26) {
          <std::sys::sync::mutex::futex::Mutex>::lock_contended(&PRE);
        }
        if ((*(ulonglong *)
              __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
            0x7fffffffffffffff) == 0) {
          bVar3 = 0;
        }
        else {
          bVar3 = std::panicking::panic_count::is_zero_slow_path();
          bVar3 = bVar3 ^ 1;
        }
        if (DAT_1401b85f1 != '\0') {
          local_208 = (byte *******)&PRE;
          uStack_200._1_7_ = (undefined7)((ulonglong)uStack_200 >> 8);
          uStack_200 = (code *)CONCAT71(uStack_200._1_7_,bVar3);
          core::result::unwrap_failed
                    ("called `Result::unwrap()` on an `Err` value",0x2b,&local_208,
                     &
                     PTR_<std::sync::poison::PoisonError<std::sync::poison::mutex::MutexGuard<core::option::Option<std::process::Child>>>>_1401c0258
                     ,&PTR_s_src_workshop_rs_1401c0b18);
          goto LAB_14005c73a;
        }
        if (DAT_1401b8610 == -1) break;
        if (pppppppbVar17 == (byte *******)0x0) {
          bVar26 = true;
        }
        else {
          lVar10 = 0;
          do {
            bVar27 = *(byte ********)((longlong)pppppppbVar18 + lVar10 + 0x30) == DAT_1401b8640;
            bVar26 = !bVar27;
            if (bVar27) break;
            bVar27 = lVar7 + -0x38 != lVar10;
            lVar10 = lVar10 + 0x38;
          } while (bVar27);
        }
        if (((bVar3 == 0) &&
            ((*(ulonglong *)
               __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
             0x7fffffffffffffff) != 0)) &&
           (cVar5 = std::panicking::panic_count::is_zero_slow_path(),
           cVar5 == '\0')) {
          DAT_1401b85f1 = '\x01';
        }
        cVar5 = PRE;
        LOCK();
        PRE = '\0';
        UNLOCK();
        if (cVar5 == '\x02') {
          <std::sys::sync::mutex::futex::Mutex>::wake(&PRE);
        }
        if (!bVar26) goto LAB_14005b9e4;
        if (CANCEL != '\0') goto LAB_14005c022;
        pre_stop(1,0);
        std::thread::functions::sleep(0);
      }
      if (((bVar3 == 0) &&
          ((*(ulonglong *)
             __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
           0x7fffffffffffffff) != 0)) &&
         (cVar5 = std::panicking::panic_count::is_zero_slow_path(), cVar5 == '\0')
         ) {
        DAT_1401b85f1 = '\x01';
      }
      cVar5 = PRE;
      LOCK();
      PRE = '\0';
      UNLOCK();
      if (cVar5 == '\x02') {
        <std::sys::sync::mutex::futex::Mutex>::wake(&PRE);
      }
LAB_14005b9e4:
      if (pppppppbVar17 != (byte *******)0x1) {
        if (pppppppbVar17 != (byte *******)0x0) {
          local_188 = pppppppbVar17;
          uStack_200 = <u64_as_core::fmt::Display>::fmt;
          local_208 = (byte *******)&local_188;
          alloc::fmt::format::format_inner(&local_70,&DAT_1401c0a62,&local_208);
          param_1[3] = local_60;
          *(undefined4 *)(param_1 + 1) = local_70;
          *(undefined4 *)((longlong)param_1 + 0xc) = uStack_6c;
          *(undefined4 *)(param_1 + 2) = uStack_68;
          *(undefined4 *)((longlong)param_1 + 0x14) = uStack_64;
          *param_1 = 1;
          goto LAB_14005c070;
        }
        param_1[1] = 0xffffffffffffffff;
        *param_1 = 0;
        goto LAB_14005c0ba;
      }
      local_208 = pppppppbVar18 + 6;
      local_1f8 = pppppppbVar18 + 3;
      uStack_200 = <u64_as_core::fmt::Display>::fmt;
      uStack_1f0 = <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
      uStack_1e0 = <alloc::string::String_as_core::fmt::Debug>::
                   fmt;
      local_1e8 = pppppppbVar18;
      alloc::fmt::format::format_inner(&local_188,&DAT_1401c09ca,&local_208);
      pppppppbVar24 = (byte *******)uStack_180;
      pppppppbVar14 = local_188;
      logmask::mask_ips(&local_208,uStack_180,local_178);
      uStack_180 = <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
      local_188 = (byte *******)&local_208;
      std::io::stdio::_eprint
                (&anon_b00e009db4843eda5d84d24e1a206c57_526_llvm_7972633909693502697,&local_188);
      if (local_208 != (byte *******)0x0) {
        __rustc::__rust_dealloc(uStack_200,local_208,1);
      }
      if (pppppppbVar14 != (byte *******)0x0) {
        __rustc::__rust_dealloc(pppppppbVar24,pppppppbVar14,1);
      }
      libraries(&local_228);
      pppppppbVar24 = uStack_218;
      pppppppbVar14 = (byte *******)local_220;
      installed_for_in(&local_208,local_220,uStack_218,pppppppbVar18);
      pppppppbVar16 = local_1f8;
      pppppppbVar25 = (byte *******)uStack_200;
      pppppppbVar19 = local_208;
      if (local_208 != (byte *******)0xffffffffffffffff) {
        local_258 = (byte *******)uStack_200;
        mismatch(&local_188,uStack_200,local_1f8,pppppppbVar18);
        if (local_188 == (byte *******)0xffffffffffffffff) {
          <core::wtf8::Wtf8>::to_string_lossy
                    (&local_250,pppppppbVar25,pppppppbVar16);
          pppppppbVar24 = uStack_240;
          pppppppbVar14 = (byte *******)uStack_248;
          pppppppbVar23 = (byte *******)uStack_248;
          pppppppbVar16 = local_250;
          if (local_250 == (byte *******)0xffffffffffffffff) {
            <alloc::raw_vec::RawVecInner>::try_allocate_in
                      (&local_188,uStack_240,0,1,1);
            pppppppbVar23 = local_178;
            pppppppbVar16 = (byte *******)uStack_180;
            if ((int)local_188 == 1) {
              alloc::raw_vec::handle_error(uStack_180,local_178);
              goto LAB_14005c73a;
            }
            if (pppppppbVar24 == (byte *******)0x0) {
              pppppppbVar24 = (byte *******)0x0;
            }
            else {
              memcpy(local_178,pppppppbVar14,(size_t)pppppppbVar24);
            }
          }
          param_1[1] = pppppppbVar16;
          param_1[2] = pppppppbVar23;
          param_1[3] = pppppppbVar24;
          *param_1 = 0;
          if (pppppppbVar19 != (byte *******)0x0) {
            __rustc::__rust_dealloc(pppppppbVar25,pppppppbVar19,1);
          }
          core::ptr::drop_glue::
          <alloc::vec::Vec<std::path::PathBuf>>(&local_228);
          goto LAB_14005c070;
        }
        if (local_188 != (byte *******)0x0) {
          __rustc::__rust_dealloc(uStack_180,local_188,1);
        }
        if (pppppppbVar19 != (byte *******)0x0) {
          __rustc::__rust_dealloc(pppppppbVar25,pppppppbVar19,1);
        }
      }
      if (pppppppbVar24 != (byte *******)0x0) {
        pppppppbVar19 = pppppppbVar14 + 1;
        do {
          if (pppppppbVar19[-1] != (byte ******)0x0) {
            __rustc::__rust_dealloc(*pppppppbVar19,pppppppbVar19[-1],1);
          }
          pppppppbVar19 = pppppppbVar19 + 4;
          pppppppbVar24 = (byte *******)((longlong)pppppppbVar24 + -1);
        } while (pppppppbVar24 != (byte *******)0x0);
      }
      if (local_228 != (byte *******)0x0) {
        __rustc::__rust_dealloc(pppppppbVar14,(longlong)local_228 << 5,8);
      }
      local_b8 = REQUEST;
      local_118 = 0;
      local_c0 = (byte *******)0x8000000000000000;
      bVar3 = 0;
      while( true ) {
        LOCK();
        bVar26 = PRE == '\0';
        if (bVar26) {
          PRE = '\x01';
        }
        UNLOCK();
        if (!bVar26) {
          <std::sys::sync::mutex::futex::Mutex>::lock_contended(&PRE);
        }
        if ((*(ulonglong *)
              __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
            0x7fffffffffffffff) == 0) {
          bVar4 = 0;
          pppppppbVar14 = DAT_1401b8640;
          lVar7 = DAT_1401b8658;
          pppppppbVar24 = DAT_1401b8660;
        }
        else {
          bVar4 = std::panicking::panic_count::is_zero_slow_path();
          bVar4 = bVar4 ^ 1;
          pppppppbVar14 = DAT_1401b8640;
          lVar7 = DAT_1401b8658;
          pppppppbVar24 = DAT_1401b8660;
        }
        DAT_1401b8640 = pppppppbVar14;
        DAT_1401b8658 = lVar7;
        DAT_1401b8660 = pppppppbVar24;
        if (DAT_1401b85f1 != '\0') {
          local_208 = (byte *******)&PRE;
          uStack_200._1_7_ = (undefined7)((ulonglong)uStack_200 >> 8);
          uStack_200 = (code *)CONCAT71(uStack_200._1_7_,bVar4);
          core::result::unwrap_failed
                    ("called `Result::unwrap()` on an `Err` value",0x2b,&local_208,
                     &
                     PTR_<std::sync::poison::PoisonError<std::sync::poison::mutex::MutexGuard<core::option::Option<std::process::Child>>>>_1401c0258
                     ,&PTR_s_src_workshop_rs_1401c06f8);
          goto LAB_14005c73a;
        }
        if (DAT_1401b8610 == -1) goto LAB_14005c0e2;
        if (((bVar4 == 0) &&
            ((*(ulonglong *)
               __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
             0x7fffffffffffffff) != 0)) &&
           (cVar5 = std::panicking::panic_count::is_zero_slow_path(),
           cVar5 == '\0')) {
          DAT_1401b85f1 = '\x01';
        }
        cVar5 = PRE;
        LOCK();
        PRE = '\0';
        UNLOCK();
        if (cVar5 == '\x02') {
          <std::sys::sync::mutex::futex::Mutex>::wake(&PRE);
        }
        local_228 = pppppppbVar14;
        if (CANCEL != '\0') break;
        lVar10 = 0;
        do {
          if (*(byte ********)((longlong)pppppppbVar18 + lVar10 + 0x30) == pppppppbVar14) {
            bVar4 = bVar3 & local_b0 == lVar7;
            bVar3 = 1;
            if ((bool)(local_258 == pppppppbVar24 & bVar4)) goto LAB_14005bca9;
            if ((local_118 & 1) == 0) {
              local_250 = (byte *******)&local_228;
              uStack_248 = <u64_as_core::fmt::Display>::fmt;
              alloc::fmt::format::format_inner
                        (&local_208,&DAT_1401c0710,&local_250);
              pppppppbVar19 = (byte *******)uStack_200;
              pppppppbVar14 = local_208;
              logmask::mask_ips(&local_208,uStack_200,local_1f8);
              uStack_248 = <alloc::string::String_as_core::fmt::Display>
                           ::fmt;
              local_250 = (byte *******)&local_208;
              std::io::stdio::_eprint
                        (&anon_b00e009db4843eda5d84d24e1a206c57_526_llvm_7972633909693502697,
                         &local_250);
              if (local_208 != (byte *******)0x0) {
                __rustc::__rust_dealloc(uStack_200,local_208,1);
              }
              if (pppppppbVar14 != (byte *******)0x0) {
                __rustc::__rust_dealloc(pppppppbVar19,pppppppbVar14,1);
              }
            }
            __rustc::__rust_no_alloc_shim_is_unstable_v2();
            plVar11 = (longlong *)__rustc::__rust_alloc(0x18,8);
            if (plVar11 == (longlong *)0x0) {
              alloc::raw_vec::handle_error(8,0x18);
              goto LAB_14005c73a;
            }
            lVar10 = 0;
            if ((byte *******)pppppppbVar18[6] == local_228) {
              lVar10 = lVar7;
            }
            pppppppbVar14 = (byte *******)0x0;
            if ((byte *******)pppppppbVar18[6] == local_228) {
              pppppppbVar14 = pppppppbVar24;
            }
            *plVar11 = lVar10;
            plVar11[1] = (longlong)pppppppbVar14;
            *(undefined1 *)(plVar11 + 2) = 0;
            progress_json(&local_208,local_b8,pppppppbVar18,1,plVar11,1,&DAT_1401c076f,0x19);
            uStack_248 = <alloc::string::String_as_core::fmt::Display>
                         ::fmt;
            local_250 = (byte *******)&local_208;
            alloc::fmt::format::format_inner(&local_188,&DAT_1401c0788,&local_250)
            ;
            if (local_208 != (byte *******)0x0) {
              __rustc::__rust_dealloc(uStack_200,local_208,1);
            }
            uStack_1f0 = (code *)local_178;
            uStack_200 = (code *)local_188;
            local_1f8 = (byte *******)uStack_180;
            local_208 = local_c0;
            online::push(&local_208,param_6,param_7);
            uVar12 = __rustc::__rust_dealloc(plVar11,0x18,8);
            bVar3 = 1;
            local_118 = CONCAT71((int7)((ulonglong)uVar12 >> 8),1);
            local_258 = pppppppbVar24;
            local_b0 = lVar7;
            goto LAB_14005bca9;
          }
          lVar10 = lVar10 + 0x38;
        } while (lVar10 != 0x38);
        pre_stop(1,0);
LAB_14005bca9:
        std::thread::functions::sleep(0);
      }
LAB_14005c022:
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      puVar13 = (undefined8 *)__rustc::__rust_alloc(9,1);
      if (puVar13 == (undefined8 *)0x0) {
        alloc::raw_vec::handle_error(1,9);
        goto LAB_14005c73a;
      }
      *puVar13 = 0x656c6c65636e6163;
      *(undefined1 *)(puVar13 + 1) = 100;
      param_1[1] = 9;
      param_1[2] = puVar13;
      param_1[3] = 9;
LAB_14005c064:
      *param_1 = 1;
      goto LAB_14005c06b;
    }
    goto LAB_14005b950;
  }
  local_f8 = local_238;
  local_108 = (undefined4)uStack_248;
  uStack_104 = uStack_248._4_4_;
  uStack_100 = (undefined4)uStack_240;
  uStack_fc = uStack_240._4_4_;
  <core::str::pattern::StrSearcher>::new
            (&local_188,uStack_240,local_238,&DAT_1401c0088,0xc);
  local_1a8 = local_128;
  local_1b8 = local_138;
  uStack_1b0 = uStack_130;
  local_1c8 = local_148;
  uStack_1c0 = uStack_140;
  local_1d8 = local_158;
  uStack_1d4 = uStack_154;
  uStack_1d0 = uStack_150;
  uStack_1cc = uStack_14c;
  local_1e8 = (byte *******)CONCAT44(uStack_164,local_168);
  uStack_1e0 = (code *)CONCAT44(uStack_15c,uStack_160);
  local_1f8 = local_178;
  uStack_1f0 = (code *)CONCAT44(uStack_16c,uStack_170);
  local_208 = local_188;
  uStack_200 = uStack_180;
  local_1a0 = 0;
  local_198 = local_238;
  local_190 = 1;
  lVar7 = <core::str::iter::SplitInternal<&str>>::next(&local_208);
  if ((lVar7 == 0) ||
     (pbVar8 = (byte *)<core::str::iter::SplitInternal<&str>>::next(&local_208),
     pbVar8 == (byte *)0x0)) {
LAB_14005b8cf:
    local_e0 = <alloc::string::String_as_core::fmt::Display>::fmt
    ;
    local_e8 = (byte **)&local_108;
    alloc::fmt::format::format_inner(&local_188,&DAT_1401c00f5,&local_e8);
  }
  else {
    pppppppbVar18 = (byte *******)0x0;
    pbVar20 = pbVar8;
    do {
      pppppppbVar17 = uStack_240;
      if (pbVar20 == pbVar8 + (longlong)uStack_240) break;
      bVar3 = *pbVar20;
      uVar21 = (uint)bVar3;
      if ((char)bVar3 < '\0') {
        if (bVar3 < 0xe0) {
          pbVar22 = pbVar20 + 2;
          uVar21 = (uVar21 & 0x1f) << 6 | pbVar20[1] & 0x3f;
        }
        else {
          uVar15 = pbVar20[2] & 0x3f | (pbVar20[1] & 0x3f) << 6;
          if (bVar3 < 0xf0) {
            pbVar22 = pbVar20 + 3;
            uVar21 = uVar15 | (uVar21 & 0x1f) << 0xc;
          }
          else {
            pbVar22 = pbVar20 + 4;
            uVar21 = pbVar20[3] & 0x3f | uVar15 << 6 | (uVar21 & 7) << 0x12;
          }
        }
      }
      else {
        pbVar22 = pbVar20 + 1;
      }
      pppppppbVar17 = pppppppbVar18;
      pppppppbVar18 = (byte *******)(pbVar22 + ((longlong)pppppppbVar18 - (longlong)pbVar20));
      pbVar20 = pbVar22;
    } while (0xfffffff5 < uVar21 - 0x3a);
    if (pppppppbVar17 == (byte *******)0x0) goto LAB_14005b8cf;
    local_e8 = &local_98;
    local_e0 = (code *)&<&str_as_core::fmt::Display>::fmt;
    local_98 = pbVar8;
    local_90 = pppppppbVar17;
    alloc::fmt::format::format_inner(&local_188,&DAT_1401c0094,&local_e8);
  }
  if (CONCAT44(uStack_104,local_108) != 0) {
    __rustc::__rust_dealloc
              (CONCAT44(uStack_fc,uStack_100),CONCAT44(uStack_104,local_108),1);
  }
  local_238 = local_178;
  uStack_248 = (code *)local_188;
  uStack_240 = (byte *******)uStack_180;
  local_220 = (code *)local_188;
  uStack_218 = (byte *******)uStack_180;
  local_210 = local_178;
LAB_14005b950:
  local_78 = local_210;
  local_88 = (undefined4)local_220;
  uStack_84 = local_220._4_4_;
  uStack_80 = (undefined4)uStack_218;
  uStack_7c = uStack_218._4_4_;
  param_1[3] = local_210;
  *(undefined4 *)(param_1 + 1) = (undefined4)local_220;
  *(undefined4 *)((longlong)param_1 + 0xc) = local_220._4_4_;
  *(undefined4 *)(param_1 + 2) = (undefined4)uStack_218;
  *(undefined4 *)((longlong)param_1 + 0x14) = uStack_218._4_4_;
  *param_1 = 1;
LAB_14005b97f:
  if (lVar2 != 0) {
    __rustc::__rust_dealloc(local_110,lVar2,1);
  }
  return param_1;
LAB_14005c0e2:
  if (((bVar4 == 0) &&
      ((*(ulonglong *)__imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
       0x7fffffffffffffff) != 0)) &&
     (cVar5 = std::panicking::panic_count::is_zero_slow_path(), cVar5 == '\0')) {
    DAT_1401b85f1 = '\x01';
  }
  cVar5 = PRE;
  LOCK();
  PRE = '\0';
  UNLOCK();
  if (cVar5 == '\x02') {
    <std::sys::sync::mutex::futex::Mutex>::wake(&PRE);
  }
  if ((char)local_118 == '\x01') {
    libraries(&local_228);
    installed_for_in(&local_208,local_220,uStack_218,pppppppbVar18);
    <core::option::Option<std::path::PathBuf>>::filter::
    <menu_overlay::workshop::check_and_fetch::_closure_0_>
              (&local_188,&local_208,pppppppbVar18);
    pppppppbVar24 = (byte *******)uStack_180;
    pppppppbVar14 = local_188;
    if (local_188 == (byte *******)0xffffffffffffffff) {
      core::ptr::drop_glue::
      <alloc::vec::Vec<std::path::PathBuf>>(&local_228);
      goto LAB_14005c2a9;
    }
    <core::wtf8::Wtf8>::to_string_lossy(&local_250,uStack_180,local_178);
    pppppppbVar25 = uStack_240;
    pppppppbVar19 = (byte *******)uStack_248;
    pppppppbVar16 = local_250;
    pppppppbVar23 = (byte *******)uStack_248;
    if (local_250 == (byte *******)0xffffffffffffffff) {
      <alloc::raw_vec::RawVecInner>::try_allocate_in(&local_208,uStack_240,0,1,1);
      pppppppbVar23 = local_1f8;
      pppppppbVar16 = (byte *******)uStack_200;
      if ((int)local_208 == 1) {
        alloc::raw_vec::handle_error(uStack_200,local_1f8);
        goto LAB_14005c73a;
      }
      if (pppppppbVar25 == (byte *******)0x0) {
        pppppppbVar25 = (byte *******)0x0;
      }
      else {
        memcpy(local_1f8,pppppppbVar19,(size_t)pppppppbVar25);
      }
    }
    param_1[1] = pppppppbVar16;
    param_1[2] = pppppppbVar23;
    param_1[3] = pppppppbVar25;
    *param_1 = 0;
    if (pppppppbVar14 != (byte *******)0x0) {
      __rustc::__rust_dealloc(pppppppbVar24,pppppppbVar14,1);
    }
    core::ptr::drop_glue::
    <alloc::vec::Vec<std::path::PathBuf>>(&local_228);
  }
  else {
LAB_14005c2a9:
    fetch(&local_208,pppppppbVar18,1,param_6,param_7);
    if (local_208 != (byte *******)0xffffffffffffffff) {
      param_1[3] = local_1f8;
      *(int *)(param_1 + 1) = (int)local_208;
      *(undefined4 *)((longlong)param_1 + 0xc) = local_208._4_4_;
      *(undefined4 *)(param_1 + 2) = (undefined4)uStack_200;
      *(undefined4 *)((longlong)param_1 + 0x14) = uStack_200._4_4_;
      goto LAB_14005c064;
    }
    libraries(&local_250);
    installed_for_in(&local_208,uStack_248,uStack_240,pppppppbVar18);
    pppppppbVar19 = local_1f8;
    pppppppbVar24 = (byte *******)uStack_200;
    pppppppbVar14 = local_208;
    if (local_208 == (byte *******)0xffffffffffffffff) {
      local_228 = pppppppbVar18 + 3;
      local_220 = <alloc::string::String_as_core::fmt::Display>::
                  fmt;
      alloc::fmt::format::format_inner(&local_188,&DAT_1401c013b,&local_228);
      param_1[1] = local_188;
      *(undefined4 *)(param_1 + 2) = (undefined4)uStack_180;
      *(undefined4 *)((longlong)param_1 + 0x14) = uStack_180._4_4_;
      *(undefined4 *)(param_1 + 3) = (undefined4)local_178;
      *(undefined4 *)((longlong)param_1 + 0x1c) = local_178._4_4_;
      *param_1 = 1;
      core::ptr::drop_glue::
      <alloc::vec::Vec<std::path::PathBuf>>(&local_250);
    }
    else {
      core::ptr::drop_glue::
      <alloc::vec::Vec<std::path::PathBuf>>(&local_250);
      mismatch(&local_250,pppppppbVar24,pppppppbVar19,pppppppbVar18);
      if (local_250 == (byte *******)0xffffffffffffffff) {
        <core::wtf8::Wtf8>::to_string_lossy(&local_188,pppppppbVar24,pppppppbVar19)
        ;
        pppppppbVar25 = local_178;
        pppppppbVar19 = (byte *******)uStack_180;
        pppppppbVar17 = (byte *******)uStack_180;
        pppppppbVar18 = local_188;
        if (local_188 == (byte *******)0xffffffffffffffff) {
          <alloc::raw_vec::RawVecInner>::try_allocate_in
                    (&local_208,local_178,0,1,1);
          pppppppbVar17 = local_1f8;
          pppppppbVar18 = (byte *******)uStack_200;
          if ((int)local_208 == 1) {
            alloc::raw_vec::handle_error(uStack_200,local_1f8);
LAB_14005c73a:
                    /* WARNING: Does not return */
            pcVar1 = (code *)invalidInstructionException();
            (*pcVar1)();
          }
          if (pppppppbVar25 == (byte *******)0x0) {
            pppppppbVar25 = (byte *******)0x0;
          }
          else {
            memcpy(local_1f8,pppppppbVar19,(size_t)pppppppbVar25);
          }
        }
        param_1[1] = pppppppbVar18;
        param_1[2] = pppppppbVar17;
        param_1[3] = pppppppbVar25;
        *param_1 = 0;
        if (pppppppbVar14 != (byte *******)0x0) {
          __rustc::__rust_dealloc(pppppppbVar24,pppppppbVar14,1);
        }
        core::ptr::drop_glue::
        <alloc::vec::Vec<menu_overlay::workshop::Wanted>>
                  (&local_d8);
        goto LAB_14005b97f;
      }
      local_178 = uStack_240;
      local_188 = local_250;
      uStack_180 = uStack_248;
      local_208 = pppppppbVar18 + 3;
      uStack_200 = <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
      uStack_1f0 = <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
      local_1e8 = (byte *******)&local_188;
      uStack_1e0 = <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
      local_1f8 = pppppppbVar18;
      alloc::fmt::format::format_inner(&local_58,&DAT_1401c0a07,&local_208);
      param_1[3] = local_48;
      *(undefined4 *)(param_1 + 1) = local_58;
      *(undefined4 *)((longlong)param_1 + 0xc) = uStack_54;
      *(undefined4 *)(param_1 + 2) = uStack_50;
      *(undefined4 *)((longlong)param_1 + 0x14) = uStack_4c;
      *param_1 = 1;
      if (local_188 != (byte *******)0x0) {
        __rustc::__rust_dealloc(uStack_180,local_188,1);
      }
      if (pppppppbVar14 != (byte *******)0x0) {
        __rustc::__rust_dealloc(pppppppbVar24,pppppppbVar14,1);
      }
    }
  }
LAB_14005c06b:
  if (pppppppbVar17 != (byte *******)0x0) {
LAB_14005c070:
    pppppppbVar14 = pppppppbVar18 + 4;
    do {
      if (pppppppbVar14[-4] != (byte ******)0x0) {
        __rustc::__rust_dealloc(pppppppbVar14[-3],pppppppbVar14[-4],1);
      }
      if (pppppppbVar14[-1] != (byte ******)0x0) {
        __rustc::__rust_dealloc(*pppppppbVar14,pppppppbVar14[-1],1);
      }
      pppppppbVar14 = pppppppbVar14 + 7;
      pppppppbVar17 = (byte *******)((longlong)pppppppbVar17 + -1);
    } while (pppppppbVar17 != (byte *******)0x0);
  }
LAB_14005c0ba:
  if (CONCAT44(uStack_d4,local_d8) != 0) {
    __rustc::__rust_dealloc(pppppppbVar18,CONCAT44(uStack_d4,local_d8) * 0x38,8);
  }
  goto LAB_14005b97f;
}


