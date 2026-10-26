
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::workshop::fetch */

void __rustcall
menu_overlay::workshop::fetch
          (undefined8 *param_1,longlong param_2,ulonglong param_3,undefined8 param_4,
          undefined8 param_5)

{
  undefined1 auVar1 [16];
  undefined1 auVar2 [16];
  code *pcVar3;
  HANDLE hObject;
  longlong *plVar4;
  char cVar5;
  byte bVar6;
  undefined8 *puVar7;
  undefined8 *puVar8;
  longlong *******ppppppplVar9;
  longlong ******pppppplVar10;
  ulonglong uVar11;
  ulonglong uVar12;
  ulonglong uVar13;
  ulonglong uVar14;
  ulonglong uVar15;
  undefined4 uVar16;
  uint uVar17;
  undefined4 uVar18;
  longlong lVar19;
  undefined8 uVar20;
  ulonglong uVar21;
  char *pcVar22;
  longlong lVar23;
  undefined1 *puVar24;
  char *pcVar25;
  ulonglong uVar26;
  undefined1 *puVar27;
  undefined1 *puVar28;
  longlong lVar29;
  longlong *******ppppppplVar30;
  undefined1 *puVar31;
  bool bVar32;
  undefined8 local_238;
  longlong ******pppppplStack_230;
  longlong ******local_228;
  longlong ******local_220;
  longlong *******local_218;
  undefined8 uStack_210;
  undefined8 uStack_208;
  longlong ******local_200;
  undefined8 uStack_1f8;
  undefined8 uStack_1f0;
  undefined8 local_1e8;
  undefined8 uStack_1e0;
  undefined1 local_1d8;
  undefined2 local_1d0;
  longlong local_1c8;
  uint local_1bc;
  undefined8 *local_1b8;
  longlong local_1b0;
  longlong ******local_1a8;
  code *local_1a0;
  undefined8 local_198;
  longlong *******local_190;
  longlong ******local_188;
  longlong ******local_180;
  longlong ******local_178;
  longlong ******local_170;
  undefined8 local_168;
  longlong ******local_160;
  undefined **local_158;
  longlong *******local_150;
  undefined8 local_148;
  undefined8 uStack_140;
  longlong local_138;
  undefined8 uStack_130;
  undefined1 local_128;
  longlong ******local_120;
  undefined4 local_118;
  undefined4 uStack_114;
  undefined4 uStack_110;
  undefined4 uStack_10c;
  longlong local_108;
  undefined8 local_100;
  undefined4 local_f8;
  longlong *******local_f0;
  undefined4 uStack_e8;
  undefined4 uStack_e4;
  undefined8 uStack_e0;
  undefined8 uStack_d8;
  undefined4 local_d0;
  undefined4 uStack_cc;
  undefined4 uStack_c8;
  undefined4 uStack_c4;
  undefined4 local_c0;
  undefined4 uStack_bc;
  undefined4 uStack_b8;
  undefined4 uStack_b4;
  undefined8 local_b0;
  undefined4 local_a8;
  undefined4 local_a0;
  undefined4 uStack_9c;
  undefined4 uStack_98;
  undefined4 uStack_94;
  longlong ******local_90;
  undefined4 local_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  longlong ******local_78;
  undefined4 local_70;
  undefined4 uStack_6c;
  undefined4 uStack_68;
  undefined4 uStack_64;
  longlong ******local_60;
  undefined8 local_58;
  undefined8 uStack_50;
  undefined8 local_48;
  
  uVar20 = REQUEST;
  lVar23 = param_3 * 8;
  lVar29 = param_3 * 0x18;
  local_198 = param_4;
  if (param_3 == 0) {
    puVar7 = (undefined8 *)&DAT_00000008;
  }
  else {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar7 = (undefined8 *)__rustc::__rust_alloc(lVar29);
    if (puVar7 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(8,lVar29);
                    /* WARNING: Does not return */
      pcVar3 = (code *)invalidInstructionException();
      (*pcVar3)();
    }
    if (param_3 - 1 < 7) {
      uVar15 = 0;
    }
    else {
      uVar15 = 0;
      puVar8 = puVar7;
      do {
        *puVar8 = 0;
        puVar8[1] = 0;
        *(undefined1 *)(puVar8 + 2) = 0;
        puVar8[3] = 0;
        puVar8[4] = 0;
        *(undefined1 *)(puVar8 + 5) = 0;
        puVar8[6] = 0;
        puVar8[7] = 0;
        *(undefined1 *)(puVar8 + 8) = 0;
        puVar8[9] = 0;
        puVar8[10] = 0;
        *(undefined1 *)(puVar8 + 0xb) = 0;
        puVar8[0xc] = 0;
        puVar8[0xd] = 0;
        *(undefined1 *)(puVar8 + 0xe) = 0;
        puVar8[0xf] = 0;
        puVar8[0x10] = 0;
        *(undefined1 *)(puVar8 + 0x11) = 0;
        puVar8[0x12] = 0;
        puVar8[0x13] = 0;
        *(undefined1 *)(puVar8 + 0x14) = 0;
        puVar8[0x15] = 0;
        puVar8[0x16] = 0;
        *(undefined1 *)(puVar8 + 0x17) = 0;
        uVar15 = uVar15 + 8;
        puVar8 = puVar8 + 0x18;
      } while ((param_3 & 0x3fffffffffffff8) != uVar15);
      if ((param_3 & 7) == 0) goto LAB_14005dfd2;
    }
    lVar19 = 0;
    do {
      puVar8 = (undefined8 *)((longlong)puVar7 + lVar19 + uVar15 * 0x18);
      *puVar8 = 0;
      puVar8[1] = 0;
      *(undefined1 *)((longlong)puVar7 + lVar19 + uVar15 * 0x18 + 0x10) = 0;
      lVar19 = lVar19 + 0x18;
    } while ((ulonglong)(((uint)param_3 & 7) << 3) * 3 != lVar19);
  }
LAB_14005dfd2:
  progress_json(&local_218,uVar20,param_2,param_3,puVar7,param_3,&DAT_1401c0c9c,0x24);
  uStack_140 = <alloc::string::String_as_core::fmt::Display>::fmt
  ;
  local_148 = (longlong *******)&local_218;
  alloc::fmt::format::format_inner(&local_a0,&DAT_1401c0788,&local_148);
  if (local_218 != (longlong *******)0x0) {
    __rustc::__rust_dealloc(uStack_210,local_218,1);
  }
  local_200 = local_90;
  uStack_210 = (code *)CONCAT44(uStack_9c,local_a0);
  uStack_208 = (longlong ******)CONCAT44(uStack_94,uStack_98);
  local_218 = (longlong *******)0x8000000000000000;
  online::push(&local_218,local_198,param_5);
  LOCK();
  bVar32 = CHILD == '\0';
  if (bVar32) {
    CHILD = '\x01';
  }
  UNLOCK();
  if (!bVar32) {
    <std::sys::sync::mutex::futex::Mutex>::lock_contended(&CHILD);
  }
  if ((*(ulonglong *)__imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
      0x7fffffffffffffff) == 0) {
    uVar17 = 0;
  }
  else {
    uVar17 = std::panicking::panic_count::is_zero_slow_path();
    uVar17 = uVar17 ^ 1;
  }
  cVar5 = (char)uVar17;
  if (DAT_1401b8669 != '\0') {
    local_218 = (longlong *******)&CHILD;
    uStack_210._1_7_ = (undefined7)((ulonglong)uStack_210 >> 8);
    uStack_210 = (code *)CONCAT71(uStack_210._1_7_,cVar5);
    local_1c8 = lVar29;
    core::result::unwrap_failed
              ("called `Result::unwrap()` on an `Err` value",0x2b,&local_218,
               &
               PTR_<std::sync::poison::PoisonError<std::sync::poison::mutex::MutexGuard<core::option::Option<std::process::Child>>>>_1401c01e8
               ,&PTR_s_src_workshop_rs_1401c0cc0);
    goto LAB_14005f755;
  }
  if (CANCEL == '\0') {
    local_168 = uVar20;
    local_150 = (longlong *******)0x8000000000000000;
    if (param_3 == 0) {
      lVar19 = 8;
    }
    else {
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      uVar20 = 8;
      lVar19 = __rustc::__rust_alloc(lVar23,8);
      if (lVar19 == 0) goto LAB_14005f6c5;
      uVar15 = (ulonglong)((uint)param_3 & 3);
      if (param_3 - 1 < 3) {
        uVar21 = 0;
      }
      else {
        puVar8 = (undefined8 *)(param_2 + 0xd8);
        uVar21 = 0;
        do {
          *(undefined8 *)(lVar19 + uVar21 * 8) = puVar8[-0x15];
          *(undefined8 *)(lVar19 + 8 + uVar21 * 8) = puVar8[-0xe];
          *(undefined8 *)(lVar19 + 0x10 + uVar21 * 8) = puVar8[-7];
          *(undefined8 *)(lVar19 + 0x18 + uVar21 * 8) = *puVar8;
          uVar21 = uVar21 + 4;
          puVar8 = puVar8 + 0x1c;
        } while ((param_3 & 0x3fffffffffffffc) != uVar21);
        if (uVar15 == 0) goto LAB_14005e293;
      }
      puVar8 = (undefined8 *)(uVar21 * 0x38 + param_2 + 0x30);
      uVar26 = 0;
      do {
        *(undefined8 *)(lVar19 + uVar21 * 8 + uVar26 * 8) = *puVar8;
        uVar26 = uVar26 + 1;
        puVar8 = puVar8 + 7;
      } while (uVar15 != uVar26);
    }
LAB_14005e293:
    spawn_fetch_child(&local_218,lVar19,param_3);
    ppppppplVar9 = DAT_1401b8670;
    if (local_218 != (longlong *******)0x2) {
      uVar18 = SUB84(&uStack_210,0);
      local_d0 = (undefined4)uStack_1f8;
      uStack_cc = uStack_1f8._4_4_;
      uStack_c8 = (undefined4)uStack_1f0;
      uStack_c4 = uStack_1f0._4_4_;
      local_c0 = (undefined4)local_1e8;
      uStack_bc = local_1e8._4_4_;
      uStack_b8 = (undefined4)uStack_1e0;
      uStack_b4 = uStack_1e0._4_4_;
      uStack_e8 = (undefined4)uStack_210;
      uStack_e4 = uStack_210._4_4_;
      local_178 = local_200;
      uStack_d8 = local_200;
      local_f0 = local_218;
      uStack_e0 = 0;
      local_1b0 = param_2;
      if ((int)uStack_208 != 1) {
        local_1c8 = lVar29;
        local_1bc = uVar17;
        core::option::unwrap_failed(&PTR_s_src_workshop_rs_1401c0cd8);
LAB_14005f755:
                    /* WARNING: Does not return */
        pcVar3 = (code *)invalidInstructionException();
        (*pcVar3)();
      }
      local_1c8 = lVar29;
      local_1bc = uVar17;
      if (DAT_1401b8670 != (longlong *******)0x2) {
        hObject = (HANDLE)CONCAT44(DAT_1401b86a8._4_4_,(undefined4)DAT_1401b86a8);
        CloseHandle((HANDLE)CONCAT44(DAT_1401b86a0._4_4_,(undefined4)DAT_1401b86a0));
        CloseHandle(hObject);
        if (ppppppplVar9 != (longlong *******)0x0) {
          CloseHandle((HANDLE)CONCAT44(DAT_1401b8678._4_4_,(undefined4)DAT_1401b8678));
        }
        if (CONCAT44(DAT_1401b8680._4_4_,(undefined4)DAT_1401b8680) != 0) {
          CloseHandle((HANDLE)CONCAT44(DAT_1401b8688._4_4_,(undefined4)DAT_1401b8688));
        }
        if (CONCAT44(DAT_1401b8690._4_4_,(undefined4)DAT_1401b8690) != 0) {
          CloseHandle((HANDLE)CONCAT44(DAT_1401b8698._4_4_,(undefined4)DAT_1401b8698));
        }
      }
      DAT_1401b86a0._0_4_ = local_c0;
      DAT_1401b86a0._4_4_ = uStack_bc;
      DAT_1401b86a8._0_4_ = uStack_b8;
      DAT_1401b86a8._4_4_ = uStack_b4;
      DAT_1401b8690._0_4_ = local_d0;
      DAT_1401b8690._4_4_ = uStack_cc;
      DAT_1401b8698._0_4_ = uStack_c8;
      DAT_1401b8698._4_4_ = uStack_c4;
      DAT_1401b8680._0_4_ = (undefined4)uStack_e0;
      DAT_1401b8680._4_4_ = uStack_e0._4_4_;
      DAT_1401b8688._0_4_ = (undefined4)uStack_d8;
      DAT_1401b8688._4_4_ = uStack_d8._4_4_;
      DAT_1401b8670 = local_f0;
      DAT_1401b8678._0_4_ = uStack_e8;
      DAT_1401b8678._4_4_ = uStack_e4;
      if (((cVar5 == '\0') &&
          ((*(ulonglong *)
             __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
           0x7fffffffffffffff) != 0)) &&
         (cVar5 = std::panicking::panic_count::is_zero_slow_path(), cVar5 == '\0')
         ) {
        DAT_1401b8669 = '\x01';
      }
      cVar5 = CHILD;
      LOCK();
      CHILD = '\0';
      UNLOCK();
      local_1b8 = param_1;
      if (cVar5 == '\x02') {
        <std::sys::sync::mutex::futex::Mutex>::wake(&CHILD);
      }
      uVar20 = <std::time::Instant>::now();
      uVar16 = uVar18;
      local_100 = <std::time::Instant>::now();
      local_f8 = uVar16;
      local_b0 = uVar20;
      local_a8 = uVar18;
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      ppppppplVar9 = (longlong *******)__rustc::__rust_alloc(0x2000,1);
      if (ppppppplVar9 == (longlong *******)0x0) {
        alloc::raw_vec::handle_error(1,0x2000);
        goto LAB_14005f755;
      }
      local_138 = 0;
      uStack_130 = 0;
      local_128 = 0;
      uStack_140 = (code *)0x2000;
      local_120 = local_178;
      ppppppplVar30 = (longlong *******)0xffffffffffffffff;
      local_158 = &PTR_s_src_workshop_rs_1401c0d50;
      local_148 = ppppppplVar9;
      while (<alloc::io::util::Lines<alloc::io::buffered::bufreader::BufReader<std::process::ChildStdout>>as_core::iter::traits::iterator::Iterator>
             ::next(&local_218,&local_148), pppppplVar10 = (longlong ******)uStack_210,
            local_218 != (longlong *******)0xfffffffffffffffe) {
        if (local_218 == (longlong *******)0xffffffffffffffff) {
          if ((1 < ((uint)uStack_210 & 3) - 2) && (((ulonglong)uStack_210 & 3) != 0)) {
            uStack_210 = (code *)((longlong)uStack_210 + -1);
            local_218 = (longlong *******)0xffffffffffffff03;
            <core::io::error::CustomOwner_as_core::ops::drop::Drop>
            ::drop(&uStack_210);
          }
          break;
        }
        local_190 = local_218;
        local_188 = (longlong ******)uStack_210;
        local_180 = uStack_208;
        local_218 = (longlong *******)0x3;
        uStack_210 = (code *)0x0;
        local_200 = pppppplVar10;
        uStack_1f8 = uStack_208;
        uStack_1f0 = (code *)0x0;
        local_1e8 = uStack_208;
        uStack_1e0 = (code *)0x2000000020;
        local_1d8 = 1;
        local_1d0 = 1;
        <alloc::vec::Vec<&str>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<&str,core::str::iter::SplitN<char>>>
        ::from_iter(&local_118,&local_218);
        plVar4 = (longlong *)CONCAT44(uStack_10c,uStack_110);
        if (local_108 == 0) goto LAB_14005e950;
        if (local_108 == 3) {
          lVar23 = plVar4[1];
          uVar15 = param_3;
          if (lVar23 == 9) {
            if ((char)((longlong *)*plVar4)[1] != 'd' || *(longlong *)*plVar4 != 0x656c6c6174736e69)
            goto LAB_14005e950;
            local_160 = (longlong ******)(plVar4 + 2);
            local_1a8 = (longlong ******)(plVar4 + 4);
            local_218 = &local_160;
            uStack_210 = <&&str_as_core::fmt::Display>::fmt;
            uStack_208 = (longlong ******)&local_1a8;
            local_200 = (longlong ******)<&&str_as_core::fmt::Display>::fmt;
            alloc::fmt::format::format_inner(&local_238,&DAT_1401c0d32,&local_218)
            ;
            pppppplVar10 = pppppplStack_230;
            ppppppplVar9 = local_238;
            logmask::mask_ips(&local_218,pppppplStack_230,local_228);
            pppppplStack_230 =
                 (longlong ******)
                 <alloc::string::String_as_core::fmt::Display>::
                 fmt;
            local_238 = (longlong *******)&local_218;
            std::io::stdio::_eprint
                      (&anon_b00e009db4843eda5d84d24e1a206c57_526_llvm_7972633909693502697,
                       &local_238);
            lVar23 = local_1b0;
            if (local_218 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(uStack_210,local_218,1);
            }
            if (ppppppplVar9 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(pppppplVar10,ppppppplVar9,1);
            }
            lVar23 = fetch::_closure_3_(lVar23,param_3,plVar4[2],plVar4[3]);
            if (lVar23 == 1) {
              if (param_3 <= uVar15) goto LAB_14005f73e;
              lVar23 = puVar7[uVar15 * 3 + 1] + (ulonglong)(puVar7[uVar15 * 3 + 1] == 0);
              puVar7[uVar15 * 3] = lVar23;
              puVar7[uVar15 * 3 + 1] = lVar23;
              *(undefined1 *)(puVar7 + uVar15 * 3 + 2) = 1;
              progress_json(&local_218,local_168,local_1b0,param_3,puVar7,param_3,"Installed.",10);
              pppppplStack_230 =
                   (longlong ******)
                   <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
              local_238 = (longlong *******)&local_218;
              alloc::fmt::format::format_inner
                        (&local_70,&DAT_1401c0788,&local_238);
              ppppppplVar9 = local_150;
              if (local_218 != (longlong *******)0x0) {
                __rustc::__rust_dealloc(uStack_210,local_218,1);
              }
              local_200 = local_60;
              uStack_210 = (code *)CONCAT44(uStack_6c,local_70);
              uStack_208 = (longlong ******)CONCAT44(uStack_64,uStack_68);
              local_218 = ppppppplVar9;
              online::push(&local_218,local_198,param_5);
            }
          }
          else {
            if (lVar23 != 8) goto LAB_14005e6f5;
            if (*(longlong *)*plVar4 != 0x73736572676f7270) goto LAB_14005e950;
            local_160 = (longlong ******)(plVar4 + 2);
            pcVar22 = (char *)plVar4[4];
            puVar27 = (undefined1 *)plVar4[5];
            puVar31 = (undefined1 *)0x0;
            do {
              puVar24 = puVar31 + (longlong)pcVar22;
              if (&DAT_0000000f < puVar27 + -(longlong)puVar31) {
                uVar21 = core::slice::memchr::memchr_aligned(0x20);
                puVar28 = puVar24;
                if ((uVar21 & 1) == 0) break;
              }
              else {
                if (puVar27 == puVar31) break;
                puVar28 = (undefined1 *)0x0;
                while (puVar24[(longlong)puVar28] != ' ') {
                  puVar28 = puVar28 + 1;
                  if (puVar27 + -(longlong)puVar31 == puVar28) goto LAB_14005ecca;
                }
              }
              puVar24 = puVar31 + (longlong)puVar28 + 1;
              puVar28 = puVar28 + (longlong)puVar31;
              if ((puVar28 < puVar27) && (pcVar22[(longlong)puVar28] == ' ')) {
                uVar21 = (longlong)puVar27 - (longlong)puVar24;
                pcVar25 = puVar24 + (longlong)pcVar22;
                goto LAB_14005ece0;
              }
              puVar31 = puVar24;
            } while (puVar24 <= puVar27);
LAB_14005ecca:
            pcVar22 = (char *)plVar4[4];
            puVar28 = (undefined1 *)plVar4[5];
            uVar21 = 1;
            pcVar25 = "0";
LAB_14005ece0:
            lVar23 = fetch::_closure_3_(local_1b0);
            if (lVar23 != 1) goto LAB_14005e9f8;
            if (puVar28 == (undefined1 *)0x0) {
joined_r0x00014005ed3d:
              uVar26 = 0;
            }
            else if (puVar28 == (undefined1 *)0x1) {
              cVar5 = *pcVar22;
              if (cVar5 == '+') goto joined_r0x00014005ed3d;
              uVar26 = 0;
              if (cVar5 != '-') goto LAB_14005ed54;
            }
            else {
              cVar5 = *pcVar22;
LAB_14005ed54:
              puVar27 = (undefined1 *)(ulonglong)(cVar5 == '+');
              if ((ulonglong)((longlong)puVar28 - (longlong)puVar27) < 0x11) {
                if (puVar28 == puVar27) goto joined_r0x00014005ed3d;
                lVar23 = 0;
                uVar26 = 0;
                do {
                  uVar17 = (byte)(pcVar22 + (longlong)puVar27)[lVar23] - 0x30;
                  if (9 < uVar17) {
                    uVar26 = 0;
                    break;
                  }
                  uVar26 = (ulonglong)uVar17 + uVar26 * 10;
                  lVar23 = lVar23 + 1;
                } while (lVar23 != (longlong)puVar28 + -(longlong)puVar27);
              }
              else {
                lVar23 = 0;
                uVar12 = 0;
                do {
                  auVar1._8_8_ = 0;
                  auVar1._0_8_ = uVar12;
                  uVar11 = SUB168(auVar1 * ZEXT816(10),0);
                  if (SUB168(auVar1 * ZEXT816(10),8) != 0) goto joined_r0x00014005ed3d;
                  uVar17 = (byte)(pcVar22 + (longlong)puVar27)[lVar23] - 0x30;
                  uVar12 = uVar11 + uVar17;
                  if (9 < uVar17) goto joined_r0x00014005ed3d;
                  uVar26 = 0;
                } while ((!CARRY8(uVar11,(ulonglong)uVar17)) &&
                        (lVar23 = lVar23 + 1, uVar26 = uVar12,
                        lVar23 != (longlong)puVar28 + -(longlong)puVar27));
              }
            }
            uVar12 = 0;
            if (uVar21 != 0) {
              if (uVar21 == 1) {
                cVar5 = *pcVar25;
                if (cVar5 != '+') {
                  uVar12 = 0;
                  if (cVar5 == '-') goto joined_r0x00014005ee88;
                  goto LAB_14005ee2a;
                }
LAB_14005eed2:
                uVar12 = 0;
              }
              else {
                cVar5 = *pcVar25;
LAB_14005ee2a:
                uVar11 = (ulonglong)(cVar5 == '+');
                if (uVar21 - uVar11 < 0x11) {
                  if (uVar21 == uVar11) goto LAB_14005eed2;
                  lVar23 = 0;
                  uVar12 = 0;
                  do {
                    if (9 < (byte)pcVar25[lVar23 + uVar11] - 0x30) goto LAB_14005eed2;
                    uVar12 = (ulonglong)((byte)pcVar25[lVar23 + uVar11] - 0x30) + uVar12 * 10;
                    lVar23 = lVar23 + 1;
                  } while (lVar23 != uVar21 + -uVar11);
                }
                else {
                  lVar23 = 0;
                  uVar14 = 0;
                  do {
                    auVar2._8_8_ = 0;
                    auVar2._0_8_ = uVar14;
                    uVar13 = SUB168(auVar2 * ZEXT816(10),0);
                    if (SUB168(auVar2 * ZEXT816(10),8) != 0) goto LAB_14005eed2;
                    uVar17 = (byte)pcVar25[lVar23 + uVar11] - 0x30;
                    uVar14 = uVar13 + uVar17;
                    if (9 < uVar17) goto LAB_14005eed2;
                    uVar12 = 0;
                  } while ((!CARRY8(uVar13,(ulonglong)uVar17)) &&
                          (lVar23 = lVar23 + 1, uVar12 = uVar14, lVar23 != uVar21 + -uVar11));
                }
              }
            }
joined_r0x00014005ee88:
            if (param_3 <= uVar15) {
              local_158 = &PTR_s_src_workshop_rs_1401c0d78;
LAB_14005f73e:
              core::panicking::panic_bounds_check(uVar15,param_3,local_158);
              goto LAB_14005f755;
            }
            puVar7[uVar15 * 3] = uVar26;
            puVar7[uVar15 * 3 + 1] = uVar12;
            *(undefined1 *)(puVar7 + uVar15 * 3 + 2) = 0;
            progress_json(&local_218,local_168,local_1b0,param_3,puVar7,param_3,&DAT_1401c076f,0x19)
            ;
            pppppplStack_230 =
                 (longlong ******)
                 <alloc::string::String_as_core::fmt::Display>::
                 fmt;
            local_238 = (longlong *******)&local_218;
            alloc::fmt::format::format_inner(&local_88,&DAT_1401c0788,&local_238);
            if (local_218 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(uStack_210,local_218,1);
            }
            local_200 = local_78;
            uStack_210 = (code *)CONCAT44(uStack_84,local_88);
            uStack_208 = (longlong ******)CONCAT44(uStack_7c,uStack_80);
            local_218 = local_150;
            uVar20 = local_198;
            online::push(&local_218,local_198,param_5);
            uVar18 = (undefined4)uVar20;
            uVar21 = <std::time::Instant>::elapsed(&local_100);
            if (9 < uVar21) {
              local_100 = <std::time::Instant>::now();
              local_f8 = uVar18;
              local_1a8 = (longlong ******)
                          <std::time::Instant>::elapsed(&local_b0);
              uStack_208 = (longlong ******)(puVar7 + uVar15 * 3);
              uStack_1f8 = (longlong ******)(puVar7 + uVar15 * 3 + 1);
              local_218 = &local_160;
              uStack_210 = <&&str_as_core::fmt::Display>::fmt;
              local_200 = (longlong ******)<u64_as_core::fmt::Display>::fmt;
              uStack_1f0 = <u64_as_core::fmt::Display>::fmt;
              local_1e8 = (longlong ******)&local_1a8;
              uStack_1e0 = <u64_as_core::fmt::Display>::fmt;
              alloc::fmt::format::format_inner
                        (&local_238,&DAT_1401c0d90,&local_218);
              pppppplVar10 = pppppplStack_230;
              ppppppplVar9 = local_238;
              logmask::mask_ips(&local_218,pppppplStack_230,local_228);
              pppppplStack_230 =
                   (longlong ******)
                   <alloc::string::String_as_core::fmt::Display>
                   ::fmt;
              local_238 = (longlong *******)&local_218;
              std::io::stdio::_eprint
                        (&anon_b00e009db4843eda5d84d24e1a206c57_526_llvm_7972633909693502697,
                         &local_238);
              goto LAB_14005e9c2;
            }
          }
        }
        else {
          lVar23 = plVar4[1];
LAB_14005e6f5:
          if (lVar23 == 0xc) {
            if ((int)((longlong *)*plVar4)[1] != 0x6970612d ||
                *(longlong *)*plVar4 != 0x6d616574732d6f6e) goto LAB_14005e950;
            alloc::str::join_generic_copy::<str,u8,&str>
                      (&local_218,plVar4 + 2,local_108 + -1,&DAT_1401c0435,1);
            local_228 = uStack_208;
            local_238 = local_218;
            pppppplStack_230 = (longlong ******)uStack_210;
            local_1a8 = (longlong ******)&local_238;
            local_1a0 = <alloc::string::String_as_core::fmt::Display>
                        ::fmt;
            alloc::fmt::format::format_inner(&local_218,&DAT_1401c0dc5,&local_1a8)
            ;
            if (local_238 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(pppppplStack_230,local_238,1);
            }
            pppppplVar10 = (longlong ******)uStack_210;
            ppppppplVar9 = local_218;
            logmask::mask_ips(&local_218,uStack_210,uStack_208);
            pppppplStack_230 =
                 (longlong ******)
                 <alloc::string::String_as_core::fmt::Display>::
                 fmt;
            local_238 = (longlong *******)&local_218;
            std::io::stdio::_eprint
                      (&anon_b00e009db4843eda5d84d24e1a206c57_526_llvm_7972633909693502697,
                       &local_238);
            if (local_218 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(uStack_210,local_218,1);
            }
            if (ppppppplVar9 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(pppppplVar10,ppppppplVar9,1);
            }
            __rustc::__rust_no_alloc_shim_is_unstable_v2();
            pppppplVar10 = (longlong ******)__rustc::__rust_alloc(9,1);
            if (pppppplVar10 == (longlong ******)0x0) {
              alloc::raw_vec::handle_error(1,9);
              goto LAB_14005f755;
            }
            *pppppplVar10 = (longlong *****)0x6361626c6c616600;
            *(code *)(pppppplVar10 + 1) = (code)0x6b;
            local_170 = (longlong ******)&DAT_00000009;
            if ((undefined1 *)((longlong)ppppppplVar30 + -1) < (undefined1 *)0xfffffffffffffffe) {
              __rustc::__rust_dealloc(local_220,ppppppplVar30,1);
            }
            ppppppplVar30 = (longlong *******)&DAT_00000009;
            local_220 = pppppplVar10;
          }
          else if ((lVar23 == 5) &&
                  ((char)((int *)*plVar4)[1] == 'r' && *(int *)*plVar4 == 0x6f727265)) {
            alloc::str::join_generic_copy::<str,u8,&str>
                      (&local_218,plVar4 + 2,local_108 + -1,&DAT_1401c0435,1);
            pppppplVar10 = (longlong ******)uStack_210;
            ppppppplVar9 = local_218;
            local_170 = uStack_208;
            if ((undefined1 *)((longlong)ppppppplVar30 + -1) < (undefined1 *)0xfffffffffffffffe) {
              __rustc::__rust_dealloc(local_220,ppppppplVar30,1);
            }
            local_220 = pppppplVar10;
            ppppppplVar30 = ppppppplVar9;
          }
          else {
LAB_14005e950:
            local_238 = (longlong *******)&local_190;
            pppppplStack_230 =
                 (longlong ******)
                 <alloc::string::String_as_core::fmt::Display>::
                 fmt;
            alloc::fmt::format::format_inner(&local_218,&DAT_1401c0db8,&local_238)
            ;
            pppppplVar10 = (longlong ******)uStack_210;
            ppppppplVar9 = local_218;
            logmask::mask_ips(&local_218,uStack_210,uStack_208);
            pppppplStack_230 =
                 (longlong ******)
                 <alloc::string::String_as_core::fmt::Display>::
                 fmt;
            local_238 = (longlong *******)&local_218;
            std::io::stdio::_eprint
                      (&anon_b00e009db4843eda5d84d24e1a206c57_526_llvm_7972633909693502697,
                       &local_238);
LAB_14005e9c2:
            if (local_218 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(uStack_210,local_218,1);
            }
            if (ppppppplVar9 != (longlong *******)0x0) {
              __rustc::__rust_dealloc(pppppplVar10,ppppppplVar9,1);
            }
          }
        }
LAB_14005e9f8:
        if (CONCAT44(uStack_114,local_118) != 0) {
          __rustc::__rust_dealloc
                    (CONCAT44(uStack_10c,uStack_110),CONCAT44(uStack_114,local_118) << 4,8);
        }
        if (local_190 != (longlong *******)0x0) {
          __rustc::__rust_dealloc(local_188,local_190,1);
        }
      }
      if ((longlong ******)uStack_140 != (longlong ******)0x0) {
        __rustc::__rust_dealloc(local_148,uStack_140,1);
      }
      CloseHandle(local_120);
      LOCK();
      bVar32 = CHILD == '\0';
      if (bVar32) {
        CHILD = '\x01';
      }
      UNLOCK();
      if (!bVar32) {
        <std::sys::sync::mutex::futex::Mutex>::lock_contended(&CHILD);
      }
      if ((*(ulonglong *)__imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT
          & 0x7fffffffffffffff) == 0) {
        bVar6 = 0;
        ppppppplVar9 = DAT_1401b8670;
      }
      else {
        bVar6 = std::panicking::panic_count::is_zero_slow_path();
        bVar6 = bVar6 ^ 1;
        ppppppplVar9 = DAT_1401b8670;
      }
      if (DAT_1401b8669 != '\0') {
        local_218 = (longlong *******)&CHILD;
        uStack_210._1_7_ = (undefined7)((ulonglong)uStack_210 >> 8);
        uStack_210 = (code *)CONCAT71(uStack_210._1_7_,bVar6);
        DAT_1401b8670 = ppppppplVar9;
        core::result::unwrap_failed
                  ("called `Result::unwrap()` on an `Err` value",0x2b,&local_218,
                   &
                   PTR_<std::sync::poison::PoisonError<std::sync::poison::mutex::MutexGuard<core::option::Option<std::process::Child>>>>_1401c01e8
                   ,&PTR_s_src_workshop_rs_1401c0cf0);
        goto LAB_14005f755;
      }
      DAT_1401b8670 = (longlong *******)0x2;
      if (ppppppplVar9 == (longlong *******)0x2) {
        uVar15 = (ulonglong)local_238 >> 0x20;
        local_238 = (longlong *******)CONCAT44((int)uVar15,2);
        ppppppplVar9 = local_238;
        pppppplVar10 = pppppplStack_230;
      }
      else {
        uStack_1e0 = (code *)CONCAT44(DAT_1401b86a8._4_4_,(undefined4)DAT_1401b86a8);
        uStack_1f0 = (code *)CONCAT44(DAT_1401b8698._4_4_,(undefined4)DAT_1401b8698);
        local_1e8 = (longlong ******)CONCAT44(DAT_1401b86a0._4_4_,(undefined4)DAT_1401b86a0);
        local_200 = (longlong ******)CONCAT44(DAT_1401b8688._4_4_,(undefined4)DAT_1401b8688);
        uStack_1f8 = (longlong ******)CONCAT44(DAT_1401b8690._4_4_,(undefined4)DAT_1401b8690);
        uStack_210 = (code *)CONCAT44(DAT_1401b8678._4_4_,(undefined4)DAT_1401b8678);
        uStack_208 = (longlong ******)CONCAT44(DAT_1401b8680._4_4_,(undefined4)DAT_1401b8680);
        local_218 = ppppppplVar9;
        <std::process::Child>::wait(&local_148,&local_218);
        pcVar3 = uStack_1e0;
        CloseHandle(local_1e8);
        CloseHandle(pcVar3);
        if (local_218 != (longlong *******)0x0) {
          CloseHandle(uStack_210);
        }
        if (uStack_208 != (longlong ******)0x0) {
          CloseHandle(local_200);
        }
        ppppppplVar9 = local_148;
        pppppplVar10 = (longlong ******)uStack_140;
        if (uStack_1f8 != (longlong ******)0x0) {
          CloseHandle(uStack_1f0);
          ppppppplVar9 = local_148;
          pppppplVar10 = (longlong ******)uStack_140;
        }
      }
      pppppplStack_230 = pppppplVar10;
      local_238 = ppppppplVar9;
      if (((bVar6 == 0) &&
          ((*(ulonglong *)
             __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
           0x7fffffffffffffff) != 0)) &&
         (cVar5 = std::panicking::panic_count::is_zero_slow_path(), cVar5 == '\0')
         ) {
        DAT_1401b8669 = '\x01';
      }
      cVar5 = CHILD;
      LOCK();
      CHILD = '\0';
      UNLOCK();
      if (cVar5 == '\x02') {
        <std::sys::sync::mutex::futex::Mutex>::wake();
      }
      if (CANCEL == '\0') {
        if (ppppppplVar30 == (longlong *******)0xffffffffffffffff) {
          if (param_3 != 0) {
            lVar23 = 0;
            do {
              if (*(char *)((longlong)puVar7 + lVar23 + 0x10) == '\0') {
                local_218 = (longlong *******)&local_238;
                uStack_210 = <core::option::Option<core::result::Result<std::process::ExitStatus,core::io::error::Error>>as_core::fmt::Debug>
                             ::fmt;
                alloc::fmt::format::format_inner
                          (&local_58,&DAT_1401c0d08,&local_218);
                local_1b8[2] = local_48;
                *local_1b8 = local_58;
                local_1b8[1] = uStack_50;
                goto LAB_14005f4c0;
              }
              lVar23 = lVar23 + 0x18;
            } while (param_3 * 0x18 != lVar23);
          }
          *local_1b8 = 0xffffffffffffffff;
        }
        else if ((local_170 == (longlong ******)&DAT_00000009) &&
                (*(code *)(local_220 + 1) == (code)0x6b &&
                 *local_220 == (longlong *****)0x6361626c6c616600)) {
          fetch_through_steam_pages
                    (local_1b8,local_168,local_1b0,param_3,puVar7,param_3,local_198,param_5);
          if (ppppppplVar30 != (longlong *******)0x0) {
            __rustc::__rust_dealloc(local_220,ppppppplVar30,1);
          }
        }
        else {
          *local_1b8 = ppppppplVar30;
          local_1b8[1] = local_220;
          local_1b8[2] = local_170;
        }
LAB_14005f4c0:
        if ((((int)local_238 == 1) && (1 < ((uint)pppppplStack_230 & 3) - 2)) &&
           (((ulonglong)pppppplStack_230 & 3) != 0)) {
          uStack_210 = (code *)((longlong)pppppplStack_230 + -1);
          local_218 = (longlong *******)CONCAT71(local_218._1_7_,3);
          <core::io::error::CustomOwner_as_core::ops::drop::Drop>
          ::drop(&uStack_210);
        }
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        puVar8 = (undefined8 *)__rustc::__rust_alloc(9,1);
        if (puVar8 == (undefined8 *)0x0) {
          alloc::raw_vec::handle_error(1,9);
          goto LAB_14005f755;
        }
        *puVar8 = 0x656c6c65636e6163;
        *(undefined1 *)(puVar8 + 1) = 100;
        *local_1b8 = 9;
        local_1b8[1] = puVar8;
        local_1b8[2] = 9;
        if ((((int)local_238 == 1) && (1 < ((uint)pppppplStack_230 & 3) - 2)) &&
           (((ulonglong)pppppplStack_230 & 3) != 0)) {
          uStack_210 = (code *)((longlong)pppppplStack_230 + -1);
          local_218 = (longlong *******)CONCAT71(local_218._1_7_,3);
          <core::io::error::CustomOwner_as_core::ops::drop::Drop>
          ::drop(&uStack_210);
        }
        if ((undefined1 *)((longlong)ppppppplVar30 + -1) < (undefined1 *)0xfffffffffffffffe) {
          __rustc::__rust_dealloc(local_220,ppppppplVar30,1);
        }
      }
      lVar29 = local_1c8;
      if (param_3 == 0) {
        return;
      }
      __rustc::__rust_dealloc(lVar19,param_3 * 8,8);
      goto LAB_14005e17f;
    }
    local_1a8 = (longlong ******)uStack_210;
    local_190 = &local_1a8;
    local_188 = (longlong ******)
                <core::io::error::Error_as_core::fmt::Display>::
                fmt;
    alloc::fmt::format::format_inner(&local_148,&DAT_1401c0193,&local_190);
    if ((1 < ((uint)local_1a8 & 3) - 2) && (((ulonglong)local_1a8 & 3) != 0)) {
      local_188 = (longlong ******)((longlong)local_1a8 + -1);
      local_190 = (longlong *******)CONCAT71(local_190._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&local_188);
    }
    local_108 = local_138;
    local_118 = (undefined4)local_148;
    uStack_114 = local_148._4_4_;
    uStack_110 = (undefined4)uStack_140;
    uStack_10c = uStack_140._4_4_;
    param_1[2] = local_138;
    *(undefined4 *)param_1 = (undefined4)local_148;
    *(undefined4 *)((longlong)param_1 + 4) = local_148._4_4_;
    *(undefined4 *)(param_1 + 1) = (undefined4)uStack_140;
    *(undefined4 *)((longlong)param_1 + 0xc) = uStack_140._4_4_;
    if (param_3 != 0) {
      __rustc::__rust_dealloc(lVar19,param_3 * 8,8);
    }
  }
  else {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    lVar23 = 9;
    uVar20 = 1;
    puVar8 = (undefined8 *)__rustc::__rust_alloc(9,1);
    if (puVar8 == (undefined8 *)0x0) {
LAB_14005f6c5:
      local_1c8 = lVar29;
      local_1bc = uVar17;
      alloc::raw_vec::handle_error(uVar20,lVar23);
      goto LAB_14005f755;
    }
    *puVar8 = 0x656c6c65636e6163;
    *(undefined1 *)(puVar8 + 1) = 100;
    *param_1 = 9;
    param_1[1] = puVar8;
    param_1[2] = 9;
  }
  if (((cVar5 == '\0') &&
      ((*(ulonglong *)__imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
       0x7fffffffffffffff) != 0)) &&
     (cVar5 = std::panicking::panic_count::is_zero_slow_path(), cVar5 == '\0')) {
    DAT_1401b8669 = '\x01';
  }
  cVar5 = CHILD;
  LOCK();
  CHILD = '\0';
  UNLOCK();
  if (cVar5 == '\x02') {
    <std::sys::sync::mutex::futex::Mutex>::wake(&CHILD);
  }
LAB_14005e17f:
  if (param_3 != 0) {
    __rustc::__rust_dealloc(puVar7,lVar29,8);
  }
  return;
}


