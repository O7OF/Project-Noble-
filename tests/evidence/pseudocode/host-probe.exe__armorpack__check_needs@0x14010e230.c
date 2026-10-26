
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::armorpack::check_needs */

void __rustcall
host_probe::armorpack::check_needs
          (undefined8 *param_1,undefined8 *******param_2,undefined8 *******param_3,
          undefined8 param_4,undefined8 param_5)

{
  int *piVar1;
  code *pcVar2;
  undefined8 ******ppppppuVar3;
  ulonglong uVar4;
  void *_Dst;
  bool bVar5;
  undefined *puVar6;
  char cVar7;
  ulonglong uVar8;
  ulonglong uVar9;
  longlong *plVar10;
  ulonglong uVar11;
  ulonglong uVar12;
  undefined8 *******pppppppuVar13;
  undefined8 *******pppppppuVar14;
  longlong lVar15;
  ulonglong uVar16;
  ulonglong uVar17;
  ulonglong uVar18;
  undefined1 *puVar19;
  size_t _Size;
  undefined8 *******pppppppuVar20;
  undefined8 *******pppppppuVar21;
  DWORD dwTlsIndex;
  undefined8 uVar22;
  longlong *plVar23;
  undefined8 *******pppppppuVar24;
  undefined1 *puVar25;
  undefined8 local_1e8;
  uint uStack_1e0;
  undefined4 uStack_1dc;
  undefined7 uStack_1d8;
  undefined1 uStack_1d1;
  undefined4 uStack_1d0;
  undefined4 uStack_1cc;
  undefined8 local_1c8;
  undefined8 *******pppppppuStack_1c0;
  undefined8 *******local_1b0;
  longlong *local_1a8;
  undefined8 *******local_1a0;
  undefined8 *******local_198;
  undefined8 *******pppppppuStack_190;
  undefined8 *******local_188;
  undefined4 uStack_180;
  undefined4 uStack_17c;
  undefined8 *local_170;
  ulonglong local_168;
  ulonglong local_160;
  undefined8 *******local_158;
  undefined8 *******pppppppuStack_150;
  undefined8 *******local_148;
  undefined8 uStack_140;
  undefined8 ******local_138;
  undefined8 local_130;
  longlong local_128;
  undefined8 *******local_120;
  undefined8 *******local_118;
  undefined8 *******local_110;
  undefined4 local_108;
  undefined4 uStack_104;
  undefined4 uStack_100;
  undefined4 uStack_fc;
  undefined8 *******local_f8;
  longlong local_f0;
  undefined8 *******local_e8;
  undefined8 *******local_e0;
  undefined1 local_d8;
  undefined8 *******local_d0;
  undefined8 ******local_c8;
  size_t local_c0;
  undefined4 local_b8;
  undefined4 uStack_b4;
  undefined4 uStack_b0;
  undefined4 uStack_ac;
  undefined8 ******local_a8;
  undefined1 local_a0 [8];
  undefined8 uStack_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined8 uStack_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined8 uStack_68;
  undefined8 local_60;
  
  uStack_1e0 = 0x406415e0;
  uStack_1dc = 1;
  uVar22 = param_4;
  local_170 = param_1;
  local_120 = param_2;
  local_118 = param_3;
  local_1e8 = &local_120;
  alloc::fmt::format::format_inner(&local_f0,&DAT_1406b1976,&local_1e8);
  pppppppuVar14 = local_e8;
  local_d8 = 1;
  std::fs::read_to_string::inner(&local_1e8,local_e8,local_e0);
  local_1b0 = local_1e8;
  if (local_1e8 == (undefined8 *******)0xffffffffffffffff) {
    if ((1 < (uStack_1e0 & 3) - 2) && ((uStack_1e0 & 3) != 0)) {
      local_118 = (undefined8 *******)(CONCAT44(uStack_1dc,uStack_1e0) + -1);
      local_120 = (undefined8 *******)CONCAT71(local_120._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&local_118);
    }
    local_170[2] = 0;
    local_170[1] = 0xffffffffffffffff;
    goto LAB_14010e50b;
  }
  local_1a8 = (longlong *)CONCAT44(uStack_1dc,uStack_1e0);
  uVar4 = CONCAT17(uStack_1d1,uStack_1d8);
  uVar12 = 0;
  do {
    uVar17 = (longlong)local_1a8 + uVar12;
    uVar9 = uVar4;
    if (uVar4 - uVar12 < 0x10) {
      if (uVar4 == uVar12) break;
      uVar16 = 0;
      while (*(char *)(uVar17 + uVar16) != '\n') {
        uVar16 = uVar16 + 1;
        if (uVar4 - uVar12 == uVar16) goto LAB_14010e383;
      }
    }
    else {
      uVar16 = uVar17;
      uVar8 = core::slice::memchr::memchr_aligned(10);
      if ((uVar8 & 1) == 0) break;
    }
    uVar9 = uVar16 + uVar12 + 1;
    if ((uVar12 + uVar16 < uVar4) && (*(char *)(uVar17 + uVar16) == '\n')) {
      local_168 = 0;
      uVar12 = uVar9;
      local_160 = uVar9;
      if (uVar9 != 0) goto LAB_14010e3a1;
      local_168 = 0;
      local_160 = 0;
      goto LAB_14010e478;
    }
    uVar12 = uVar9;
  } while (uVar9 <= uVar4);
LAB_14010e383:
  if (uVar4 != 0) {
    local_168 = CONCAT71((int7)((ulonglong)uVar22 >> 8),1);
    local_160 = 0;
    uVar12 = uVar4;
LAB_14010e3a1:
    plVar23 = local_1a8;
    pppppppuVar21 = local_1b0;
    uVar17 = uVar12;
    if (*(char *)((longlong)local_1a8 + (uVar12 - 1)) == '\n') {
      uVar17 = uVar12 - 1;
      if (uVar17 != 0) {
        if (*(char *)((longlong)local_1a8 + (uVar12 - 2)) != '\r') goto LAB_14010e43c;
        if (uVar12 != 0xb) goto LAB_14010e478;
        goto LAB_14010e455;
      }
    }
    else {
LAB_14010e43c:
      if (uVar17 == 9) {
LAB_14010e455:
        if ((char)local_1a8[1] == '1' && *local_1a8 == 0x20534445454e5252) {
          tag_list(&local_1e8,param_4,param_5);
          puVar6 = 
          __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
          ;
          pppppppuVar20 = (undefined8 *******)CONCAT44(uStack_1dc,uStack_1e0);
          pppppppuVar14 = (undefined8 *******)CONCAT17(uStack_1d1,uStack_1d8);
          pppppppuVar24 = (undefined8 *******)CONCAT44(uStack_1cc,uStack_1d0);
          if ((int)local_1e8 != 1) {
            uStack_140 = pppppppuVar14 + (longlong)pppppppuVar24 * 7;
            local_158 = pppppppuVar14;
            pppppppuStack_150 = pppppppuVar14;
            local_148 = pppppppuVar20;
            if (*(longlong *)
                 __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
                != 0) {
              std::sys::thread_local::guard::windows::enable();
            }
            if (*(int *)(puVar6 + 0x18) == 0) {
              dwTlsIndex = <std::sys::thread_local::key::windows::LazyKey>::init
                                     (
                                     __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
                                     );
            }
            else {
              dwTlsIndex = *(int *)(puVar6 + 0x18) - 1;
            }
            pppppppuVar21 = local_1b0;
            plVar10 = TlsGetValue(dwTlsIndex);
            if (plVar10 < (longlong *)0x2) {
              plVar10 = (longlong *)
                        <std::sys::thread_local::os::Storage<core::cell::Cell<(u64,u64)>,8usize>>
                        ::try_initialize::
                        <<std::hash::random::RandomState>::new::KEYS::__rust_std_internal_init_fn>
                                  (dwTlsIndex,plVar10,0);
              pppppppuVar21 = local_1b0;
            }
            if (plVar10 == (longlong *)0x0) {
              std::thread::local::panic_access_error
                        (&anon_34f15a90fcba3ed6a32c120778b34e05_3_llvm_15918893398050881851);
LAB_14010ede8:
                    /* WARNING: Does not return */
              pcVar2 = (code *)invalidInstructionException();
              (*pcVar2)();
            }
            local_1c8 = (void *)*plVar10;
            pppppppuStack_1c0 = (undefined8 *******)plVar10[1];
            *plVar10 = *plVar10 + 1;
            uStack_1d8 = 0;
            uStack_1d1 = 0;
            uStack_1d0 = 0;
            uStack_1cc = 0;
            local_1e8._0_4_ = 0x406b07b0;
            local_1e8._4_4_ = 1;
            uStack_1e0 = 0;
            uStack_1dc = 0;
            if ((longlong)uStack_140 - (longlong)pppppppuStack_150 != 0) {
              <hashbrown::raw::RawTable<((alloc::string::String,alloc::string::String),())>>
              ::reserve_rehash::
              <hashbrown::map::make_hasher<(alloc::string::String,alloc::string::String),(),std::hash::random::RandomState>::_closure_0_>
                        (&local_1e8,
                         ((ulonglong)((longlong)uStack_140 - (longlong)pppppppuStack_150) >> 3) *
                         0x6db6db6db6db6db7,&local_1c8,1);
            }
            local_188 = local_148;
            uStack_180 = (undefined4)uStack_140;
            uStack_17c = uStack_140._4_4_;
            <alloc::vec::into_iter::IntoIter<(alloc::string::String,alloc::string::String,u32)>as_core::iter::traits::iterator::Iterator>
            ::fold::
            <(),core::iter::adapters::map::map_fold<(alloc::string::String,alloc::string::String,u32),(alloc::string::String,alloc::string::String),(),host_probe::armorpack::map_tags::_closure_0_,core::iter::adapters::map::map_fold<(alloc::string::String,alloc::string::String),((alloc::string::String,alloc::string::String),()),(),<hashbrown::set::HashSet<(alloc::string::String,alloc::string::String),std::hash::random::RandomState>as_core::iter::traits::collect::Extend<(alloc::string::String,alloc::string::String)>>::extend<core::iter::adapters::map::Map<alloc::vec::into_iter::IntoIter<(alloc::string::String,alloc___pack::map_tags::_closure_0_>>::_closure_0_>>::_closure_0_>::_closure_0_>___closure_0_>___closure_0_>
                      (&local_198,&local_1e8);
            pppppppuVar20 = (undefined8 *******)CONCAT44(uStack_1dc,uStack_1e0);
            pppppppuVar14 = (undefined8 *******)CONCAT17(uStack_1d1,uStack_1d8);
            if (local_1e8 != (undefined8 *******)0x0) {
              local_108 = uStack_1d0;
              uStack_104 = uStack_1cc;
              uStack_100 = (undefined4)local_1c8;
              uStack_fc = local_1c8._4_4_;
              local_f8 = pppppppuStack_1c0;
              local_198 = (undefined8 *******)0x0;
              pppppppuStack_190 = (undefined8 *******)&DAT_00000008;
              local_188 = (undefined8 *******)0x0;
              local_120 = local_1e8;
              local_118 = pppppppuVar20;
              local_110 = pppppppuVar14;
              if ((char)local_168 == '\0') {
                local_128 = 0;
LAB_14010e776:
                local_168 = local_168 & 0xffffffff00000000;
                uVar12 = local_160;
                uVar17 = local_160;
joined_r0x00014010e7a1:
                do {
                  local_160 = uVar17;
                  uVar17 = local_160;
                  uVar16 = uVar9;
                  if (uVar4 < uVar9) {
LAB_14010e817:
                    pppppppuVar14 =
                         (undefined8 *******)CONCAT71((int7)((ulonglong)pppppppuVar14 >> 8),1);
                    local_168 = CONCAT44(local_168._4_4_,(int)pppppppuVar14);
                    bVar5 = true;
                    uVar11 = uVar4;
                    uVar9 = uVar16;
                    pppppppuVar21 = local_1b0;
                    local_160 = uVar17;
                    if (uVar4 != uVar17) goto LAB_14010e83a;
                    goto LAB_14010ebc4;
                  }
                  uVar8 = (longlong)plVar23 + uVar9;
                  uVar16 = uVar4;
                  local_160 = uVar12;
                  if (uVar4 - uVar9 < 0x10) {
                    if (uVar4 != uVar9) {
                      uVar18 = 0;
                      do {
                        if (*(char *)(uVar8 + uVar18) == '\n') goto LAB_14010e7ef;
                        uVar18 = uVar18 + 1;
                      } while (uVar4 - uVar9 != uVar18);
                    }
                    goto LAB_14010e817;
                  }
                  pppppppuVar14 =
                       (undefined8 *******)CONCAT71((int7)((ulonglong)pppppppuVar14 >> 8),10);
                  uVar18 = uVar8;
                  uVar12 = core::slice::memchr::memchr_aligned(pppppppuVar14);
                  if ((uVar12 & 1) == 0) goto LAB_14010e817;
LAB_14010e7ef:
                  uVar11 = uVar18 + uVar9 + 1;
                  uVar16 = uVar9 + uVar18;
                  uVar9 = uVar11;
                  uVar12 = local_160;
                } while ((uVar4 <= uVar16) || (*(char *)(uVar8 + uVar18) != '\n'));
                bVar5 = false;
                local_160 = uVar11;
LAB_14010e83a:
                piVar1 = (int *)((longlong)plVar23 + uVar17);
                pppppppuVar20 = (undefined8 *******)(uVar11 - uVar17);
                pppppppuVar21 = pppppppuVar20;
                if ((pppppppuVar20 != (undefined8 *******)0x0) &&
                   (*(char *)((longlong)plVar23 + (uVar11 - 1)) == '\n')) {
                  pppppppuVar14 = (undefined8 *******)((longlong)pppppppuVar20 + -1);
                  if (pppppppuVar14 == (undefined8 *******)0x0) {
                    pppppppuVar21 = (undefined8 *******)0x0;
                  }
                  else {
                    pppppppuVar21 = pppppppuVar14;
                    if (*(char *)((longlong)plVar23 + (uVar11 - 2)) == '\r') {
                      pppppppuVar21 = (undefined8 *******)((longlong)pppppppuVar20 + -2);
                    }
                  }
                }
                pppppppuVar20 = (undefined8 *******)0x0;
                do {
                  puVar19 = (undefined1 *)((longlong)pppppppuVar20 + (longlong)piVar1);
                  if ((undefined1 *)((longlong)pppppppuVar21 - (longlong)pppppppuVar20) <
                      (undefined1 *)0x10) {
                    if (pppppppuVar21 == pppppppuVar20) break;
                    puVar25 = (undefined1 *)0x0;
                    while (puVar19[(longlong)puVar25] != '\t') {
                      puVar25 = puVar25 + 1;
                      if ((undefined1 *)((longlong)pppppppuVar21 - (longlong)pppppppuVar20) ==
                          puVar25) goto LAB_14010e783;
                    }
                  }
                  else {
                    pppppppuVar14 =
                         (undefined8 *******)CONCAT71((int7)((ulonglong)pppppppuVar14 >> 8),9);
                    uVar12 = core::slice::memchr::memchr_aligned(pppppppuVar14);
                    puVar25 = puVar19;
                    if ((uVar12 & 1) == 0) break;
                  }
                  pppppppuVar24 =
                       (undefined8 *******)((longlong)pppppppuVar20 + (longlong)(puVar25 + 1));
                  pppppppuVar20 = (undefined8 *******)(puVar25 + (longlong)pppppppuVar20);
                  if ((pppppppuVar20 < pppppppuVar21) &&
                     (*(char *)((longlong)piVar1 + (longlong)pppppppuVar20) == '\t')) {
                    pppppppuVar13 = pppppppuVar24;
                    if (pppppppuVar24 <= pppppppuVar21) goto LAB_14010e907;
                    break;
                  }
                  pppppppuVar20 = pppppppuVar24;
                } while (pppppppuVar24 <= pppppppuVar21);
                goto LAB_14010e783;
              }
              lVar15 = 0;
              goto LAB_14010ecb7;
            }
            pppppppuVar24 = (undefined8 *******)CONCAT44(uStack_1cc,uStack_1d0);
          }
          local_158 = &local_198;
          pppppppuStack_150 =
               (undefined8 *******)
               <alloc::string::String_as_core::fmt::Display>::fmt
          ;
          local_198 = pppppppuVar20;
          pppppppuStack_190 = pppppppuVar14;
          local_188 = pppppppuVar24;
          alloc::fmt::format::format_inner(&local_1e8,&DAT_1406b0d0d,&local_158);
          if (local_198 != (undefined8 *******)0x0) {
            __rustc::__rust_dealloc(pppppppuStack_190,local_198,1);
            return;
          }
          *local_170 = 1;
          local_170[1] = local_1e8;
          local_170[2] = CONCAT44(uStack_1dc,uStack_1e0);
          local_170[3] = CONCAT17(uStack_1d1,uStack_1d8);
          goto LAB_14010e4f5;
        }
      }
    }
  }
LAB_14010e478:
  plVar23 = local_1a8;
  pppppppuVar21 = local_1b0;
  local_120 = pppppppuVar14;
  local_118 = local_e0;
  uStack_1e0 = 0x406415e0;
  uStack_1dc = 1;
  local_1e8 = &local_120;
  alloc::fmt::format::format_inner(local_a0,&DAT_1406b19f5,&local_1e8);
  local_1e8._7_1_ = local_a0[0];
  uStack_1e0 = local_a0._1_4_;
  uStack_1dc._0_3_ = local_a0._5_3_;
  uStack_1dc._3_1_ = (undefined1)uStack_98;
  uStack_1d8 = (undefined7)((ulonglong)uStack_98 >> 8);
  uStack_1d1 = (undefined1)local_90;
  uStack_1d0 = (undefined4)((ulonglong)local_90 >> 8);
  uStack_1cc._0_3_ = (undefined3)((ulonglong)local_90 >> 0x28);
  *(undefined1 *)local_170 = 1;
  *(int *)((longlong)local_170 + 1) = (int)local_1e8;
  *(undefined4 *)((longlong)local_170 + 5) = local_1e8._4_4_;
  *(uint *)((longlong)local_170 + 9) = uStack_1e0;
  *(undefined4 *)((longlong)local_170 + 0xd) = uStack_1dc;
  local_170[2] = uStack_98;
  local_170[3] = local_90;
  goto LAB_14010e4f5;
  while( true ) {
    pppppppuVar13 = (undefined8 *******)((longlong)local_1a0 + (longlong)(puVar25 + 1));
    pppppppuVar14 = (undefined8 *******)(puVar25 + (longlong)local_1a0);
    if ((pppppppuVar14 < pppppppuVar21) &&
       (*(char *)((longlong)piVar1 + (longlong)pppppppuVar14) == '\t')) {
      local_138 = (undefined8 ******)((longlong)pppppppuVar24 + (longlong)piVar1);
      local_158 = (undefined8 *******)((longlong)piVar1 + (longlong)pppppppuVar13);
      _Size = (longlong)pppppppuVar14 - (longlong)pppppppuVar24;
      pppppppuStack_150 = (undefined8 *******)((longlong)pppppppuVar21 - (longlong)pppppppuVar13);
      local_c8 = local_138;
      local_c0 = _Size;
      <alloc::raw_vec::RawVecInner>::try_allocate_in(&local_1e8,_Size,0,1,1);
      pppppppuVar13 = local_1b0;
      uVar22 = CONCAT44(uStack_1dc,uStack_1e0);
      if ((int)local_1e8 == 1) {
        alloc::raw_vec::handle_error(uVar22,CONCAT17(uStack_1d1,uStack_1d8));
        goto LAB_14010ede8;
      }
      local_1a0 = (undefined8 *******)CONCAT17(uStack_1d1,uStack_1d8);
      local_130 = uVar22;
      if (pppppppuVar14 != pppppppuVar24) {
        memcpy(local_1a0,local_138,_Size);
      }
      pppppppuVar14 = pppppppuStack_150;
      local_d0 = local_158;
      <alloc::raw_vec::RawVecInner>::try_allocate_in
                (&local_1e8,pppppppuStack_150,0,1,1);
      ppppppuVar3 = (undefined8 ******)CONCAT44(uStack_1dc,uStack_1e0);
      if ((char)local_1e8 != '\0') {
        alloc::raw_vec::handle_error(ppppppuVar3,CONCAT17(uStack_1d1,uStack_1d8));
        goto LAB_14010ede8;
      }
      _Dst = (void *)CONCAT17(uStack_1d1,uStack_1d8);
      local_138 = ppppppuVar3;
      if (pppppppuVar14 != (undefined8 *******)0x0) {
        memcpy(_Dst,local_d0,(size_t)pppppppuVar14);
      }
      local_1e8._0_4_ = (int)local_130;
      local_1e8._4_4_ = (undefined4)((ulonglong)local_130 >> 0x20);
      uStack_1e0 = (uint)local_1a0;
      uStack_1dc = (undefined4)((ulonglong)local_1a0 >> 0x20);
      uStack_1d8 = (undefined7)_Size;
      uStack_1d1 = (undefined1)(_Size >> 0x38);
      uStack_1d0 = SUB84(local_138,0);
      uStack_1cc = (undefined4)((ulonglong)local_138 >> 0x20);
      pppppppuStack_1c0 = pppppppuVar14;
      local_1c8 = _Dst;
      cVar7 = <hashbrown::map::HashMap<(alloc::string::String,alloc::string::String),(),std::hash::random::RandomState>>
              ::contains_key::
              <(alloc::string::String,alloc::string::String)>
                        (&local_120);
      pppppppuVar14 = (undefined8 *******)&local_1e8;
      core::ptr::drop_glue::
      <(alloc::string::String,alloc::string::String)>
                (&local_1e8);
      plVar23 = local_1a8;
      pppppppuVar21 = local_1b0;
      if (cVar7 == '\0') {
        if ((pppppppuVar20 == (undefined8 *******)&DAT_00000004) && (*piVar1 == 0x746f6f72)) {
          local_1e8 = &local_c8;
          uStack_1e0 = 0x402428d0;
          uStack_1dc = 1;
          uStack_1d8 = SUB87(&local_158,0);
          uStack_1d1 = (undefined1)((ulonglong)&local_158 >> 0x38);
          uStack_1d0 = 0x402428d0;
          uStack_1cc = 1;
          alloc::fmt::format::format_inner(&local_88,&DAT_1406b19c6,&local_1e8);
          local_1e8._7_1_ = (undefined1)local_88;
          uStack_1e0 = (uint)((ulonglong)local_88 >> 8);
          uStack_1dc._0_3_ = (undefined3)((ulonglong)local_88 >> 0x28);
          uStack_1dc._3_1_ = (undefined1)uStack_80;
          uStack_1d8 = (undefined7)((ulonglong)uStack_80 >> 8);
          uStack_1d1 = (undefined1)local_78;
          uStack_1d0 = (undefined4)((ulonglong)local_78 >> 8);
          uStack_1cc._0_3_ = (undefined3)((ulonglong)local_78 >> 0x28);
          *(undefined1 *)local_170 = 1;
          goto LAB_14010ec55;
        }
        local_1e8 = &local_c8;
        uStack_1e0 = 0x402428d0;
        uStack_1dc = 1;
        uStack_1d8 = SUB87(&local_158,0);
        uStack_1d1 = (undefined1)((ulonglong)&local_158 >> 0x38);
        uStack_1d0 = 0x402428d0;
        uStack_1cc = 1;
        alloc::fmt::format::format_inner(&local_b8,&DAT_1406b19c1,&local_1e8);
        pppppppuVar21 = local_188;
        if (local_188 == local_198) {
          <alloc::raw_vec::RawVec<alloc::string::String>>::
          grow_one(&local_198);
        }
        pppppppuVar14 = (undefined8 *******)((longlong)pppppppuVar21 * 3);
        pppppppuStack_190[(longlong)pppppppuVar21 * 3 + 2] = local_a8;
        pppppppuVar20 = pppppppuStack_190 + (longlong)pppppppuVar21 * 3;
        *(undefined4 *)pppppppuVar20 = local_b8;
        *(undefined4 *)((longlong)pppppppuVar20 + 4) = uStack_b4;
        *(undefined4 *)(pppppppuVar20 + 1) = uStack_b0;
        *(undefined4 *)((longlong)pppppppuVar20 + 0xc) = uStack_ac;
        local_188 = (undefined8 *******)((longlong)pppppppuVar21 + 1);
        pppppppuVar13 = local_1b0;
      }
      local_128 = local_128 + 1;
      pppppppuVar21 = pppppppuVar13;
      plVar23 = local_1a8;
      if ((local_168 & 1) != 0) goto LAB_14010ebc4;
      goto LAB_14010e776;
    }
    pppppppuVar14 = local_1a0;
    local_1a0 = pppppppuVar13;
    if (pppppppuVar21 < pppppppuVar13) break;
LAB_14010e907:
    local_1a0 = pppppppuVar13;
    puVar19 = (undefined1 *)((longlong)local_1a0 + (longlong)piVar1);
    if ((undefined1 *)((longlong)pppppppuVar21 - (longlong)local_1a0) < (undefined1 *)0x10) {
      if (pppppppuVar21 == local_1a0) break;
      puVar25 = (undefined1 *)0x0;
      while (puVar19[(longlong)puVar25] != '\t') {
        puVar25 = puVar25 + 1;
        if ((undefined1 *)((longlong)pppppppuVar21 - (longlong)local_1a0) == puVar25)
        goto LAB_14010e783;
      }
    }
    else {
      pppppppuVar14 = (undefined8 *******)CONCAT71((int7)((ulonglong)pppppppuVar14 >> 8),9);
      uVar12 = core::slice::memchr::memchr_aligned(pppppppuVar14);
      puVar25 = puVar19;
      if ((uVar12 & 1) == 0) break;
    }
  }
LAB_14010e783:
  pppppppuVar21 = local_1b0;
  plVar23 = local_1a8;
  uVar12 = local_160;
  uVar17 = local_160;
  if (bVar5) goto LAB_14010ebc4;
  goto joined_r0x00014010e7a1;
LAB_14010ebc4:
  lVar15 = local_128;
  if (local_188 == (undefined8 *******)0x0) {
LAB_14010ecb7:
    local_170[2] = 1;
    local_170[3] = lVar15;
    local_170[1] = 0xffffffffffffffff;
    core::ptr::drop_glue::
    <alloc::vec::Vec<alloc::string::String>>(&local_198);
    <hashbrown::raw::RawTable<(alloc::string::String,alloc::string::String)>as_core::ops::drop::Drop>
    ::drop(&local_120);
  }
  else {
    local_158 = local_188;
    local_1e8 = &local_158;
    uStack_1e0 = 0x40693280;
    uStack_1dc = 1;
    uStack_1d8 = SUB87(pppppppuStack_190,0);
    uStack_1d1 = (undefined1)((ulonglong)pppppppuStack_190 >> 0x38);
    uStack_1d0 = 0x4011c7a0;
    uStack_1cc = 1;
    alloc::fmt::format::format_inner(&local_70,&DAT_1406b1981,&local_1e8);
    local_1e8._7_1_ = (undefined1)local_70;
    uStack_1e0 = (uint)((ulonglong)local_70 >> 8);
    uStack_1dc._0_3_ = (undefined3)((ulonglong)local_70 >> 0x28);
    uStack_1dc._3_1_ = (undefined1)uStack_68;
    uStack_1d8 = (undefined7)((ulonglong)uStack_68 >> 8);
    uStack_1d1 = (undefined1)local_60;
    uStack_1d0 = (undefined4)((ulonglong)local_60 >> 8);
    uStack_1cc._0_3_ = (undefined3)((ulonglong)local_60 >> 0x28);
    *(undefined1 *)local_170 = 0;
LAB_14010ec55:
    *(int *)((longlong)local_170 + 1) = (int)local_1e8;
    *(undefined4 *)((longlong)local_170 + 5) = local_1e8._4_4_;
    *(uint *)((longlong)local_170 + 9) = uStack_1e0;
    *(undefined4 *)((longlong)local_170 + 0xd) = uStack_1dc;
    local_170[2] = CONCAT71(uStack_1d8,uStack_1dc._3_1_);
    local_170[3] = CONCAT35((undefined3)uStack_1cc,CONCAT41(uStack_1d0,uStack_1d1));
    <alloc::vec::Vec<alloc::vec::Vec<u8>>as_core::ops::drop::Drop>
    ::drop(&local_198);
    if (local_198 != (undefined8 *******)0x0) {
      __rustc::__rust_dealloc(pppppppuStack_190,(longlong)local_198 * 0x18,8);
      return;
    }
    <hashbrown::raw::RawTable<(alloc::string::String,alloc::string::String)>as_core::ops::drop::Drop>
    ::drop(&local_120);
  }
LAB_14010e4f5:
  if (pppppppuVar21 != (undefined8 *******)0x0) {
    __rustc::__rust_dealloc(plVar23,pppppppuVar21,1);
    return;
  }
LAB_14010e50b:
  if (local_f0 == 0) {
    return;
  }
  __rustc::__rust_dealloc(local_e8,local_f0,1);
  return;
}


