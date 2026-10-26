
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::host::loadscreen::load */

void __rustcall
host_probe::host::loadscreen::load(ulonglong param_1,undefined8 param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  float fVar4;
  float fVar5;
  ulonglong uVar6;
  undefined *puVar7;
  longlong *plVar8;
  ulonglong *puVar9;
  undefined8 *puVar10;
  longlong *plVar11;
  byte bVar12;
  char cVar13;
  code *pcVar14;
  code *pcVar15;
  ulonglong uVar16;
  longlong lVar17;
  longlong lVar18;
  undefined8 uVar19;
  undefined **ppuVar20;
  ulonglong uVar21;
  longlong *plVar22;
  bool bVar23;
  longlong *local_160;
  code *local_158;
  longlong *local_150;
  ulonglong *local_148;
  undefined8 *local_140;
  ulonglong local_138;
  undefined1 local_129;
  code *local_128;
  code *local_120;
  longlong **local_118;
  float *local_110;
  float *local_108;
  float *local_100;
  float *local_f8;
  code *local_f0;
  undefined8 local_e8;
  undefined4 local_e0;
  ulonglong local_d8;
  longlong *local_d0;
  ulonglong local_c8;
  float local_bc;
  float local_b8 [5];
  float local_a4;
  ulonglong local_a0;
  undefined8 local_98;
  ulonglong local_90;
  undefined4 local_88;
  undefined8 uStack_84;
  undefined4 uStack_7c;
  
  local_a0 = param_1;
  local_98 = param_2;
  std::fs::read::inner(&local_d8,param_1);
  uVar21 = local_c8;
  plVar11 = local_d0;
  uVar6 = local_d8;
  if (local_d8 == 0xffffffffffffffff) {
    local_160 = local_d0;
    local_128 = (code *)&local_a0;
    local_120 = (code *)&<&str_as_core::fmt::Display>::fmt;
    local_118 = &local_160;
    local_110 = (float *)<core::io::error::Error_as_core::fmt::Display>
                         ::fmt;
    alloc::fmt::format::format_inner(&local_148,&DAT_1406d3793,&local_128);
    log(local_140,local_138);
    if (local_148 != (ulonglong *)0x0) {
      __rustc::__rust_dealloc(local_140,local_148,1);
      return;
    }
    if (((uint)local_160 & 3) - 2 < 2) {
      return;
    }
    if (((ulonglong)local_160 & 3) == 0) {
      return;
    }
    local_120 = (code *)((longlong)local_160 + -1);
    local_128 = (code *)CONCAT71(local_128._1_7_,3);
    <core::io::error::CustomOwner_as_core::ops::drop::Drop>::drop
              (&local_120);
    return;
  }
  if ((local_c8 < 0xc) || (*local_d0 != 0x3153544f445252)) {
    local_148 = &local_a0;
    local_140 = &<&str_as_core::fmt::Display>::fmt;
    alloc::fmt::format::format_inner(&local_128,&DAT_1406d3770,&local_148);
    pcVar15 = local_120;
    pcVar14 = local_128;
    log(local_120,local_118);
    if (pcVar14 != (code *)0x0) {
      uVar19 = 1;
      goto LAB_1401ac687;
    }
    goto LAB_1401ac68c;
  }
  plVar22 = (longlong *)((local_c8 - 0xc) / 0xc);
  if ((longlong *)(ulonglong)*(uint *)(local_d0 + 1) <= plVar22) {
    plVar22 = (longlong *)(ulonglong)*(uint *)(local_d0 + 1);
  }
  if (plVar22 == (longlong *)0x0) {
    local_160 = (longlong *)0x0;
    local_158 = (code *)0x4;
    local_150 = (longlong *)0x0;
  }
  else {
    lVar17 = (longlong)plVar22 * 0xc;
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    pcVar14 = (code *)__rustc::__rust_alloc(lVar17,4);
    if (pcVar14 == (code *)0x0) {
      alloc::raw_vec::handle_error(4,lVar17);
      goto LAB_1401ac883;
    }
    local_150 = (longlong *)0x0;
    lVar18 = 0;
    local_160 = plVar22;
    local_158 = pcVar14;
    do {
      plVar8 = local_150;
      if (uVar21 <= lVar18 + 0xfU) {
        uVar16 = lVar18 + 0xc;
        lVar17 = 0x10;
LAB_1401ac6ea:
        core::slice::index::slice_index_fail
                  (uVar16,lVar17 + lVar18,uVar21,
                   &PTR_anon_2ea4320f4837ea28fa7015f50151942e_45_llvm_7975856906012809254_1406d1ee8)
        ;
        goto LAB_1401ac883;
      }
      uVar16 = lVar18 + 0x10;
      if (uVar21 - 3 <= uVar16) {
        lVar17 = 0x14;
        goto LAB_1401ac6ea;
      }
      if (uVar21 <= lVar18 + 0x17U) {
        uVar16 = lVar18 + 0x14;
        lVar17 = 0x18;
        goto LAB_1401ac6ea;
      }
      uVar1 = *(undefined4 *)((longlong)plVar11 + lVar18 + 0xc);
      uVar2 = *(undefined4 *)((longlong)plVar11 + lVar18 + 0x10);
      uVar3 = *(undefined4 *)((longlong)plVar11 + lVar18 + 0x14);
      if (local_150 == local_160) {
        <alloc::raw_vec::RawVec<[f32;3usize]>>::grow_one(&local_160);
      }
      local_150 = (longlong *)((longlong)plVar8 + 1);
      *(undefined4 *)(local_158 + lVar18) = uVar1;
      *(undefined4 *)(local_158 + lVar18 + 4) = uVar2;
      *(undefined4 *)(local_158 + lVar18 + 8) = uVar3;
      lVar18 = lVar18 + 0xc;
    } while (lVar17 != lVar18);
  }
  local_e0 = 0;
  local_e8 = 0;
  local_b8[2] = 0.0;
  local_b8[0] = 0.0;
  local_b8[1] = 0.0;
  local_d8 = 0;
  local_120 = local_158 + (longlong)plVar22 * 0xc;
  local_128 = local_158;
  local_118 = (longlong **)&local_d8;
  <alloc::vec::Vec<f32>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<f32,core::iter::adapters::map::Map<core::slice::iter::Iter<[f32;3usize]>,host_probe::host::loadscreen::load::{closure#1}>>>
  ::from_iter(&local_148,&local_128);
  uVar21 = local_138;
  puVar10 = local_140;
  local_128 = (code *)&local_129;
  if (local_138 < 2) {
    if (local_138 != 0) goto LAB_1401ac12a;
LAB_1401ac835:
    ppuVar20 = &PTR_anon_2ea4320f4837ea28fa7015f50151942e_45_llvm_7975856906012809254_1406d3728;
    uVar16 = 0;
    uVar21 = 0;
    local_128 = (code *)&local_129;
  }
  else {
    if (local_138 < 0x15) {
      core::slice::sort::shared::smallsort::insertion_sort_shift_left::
      <f32,<[f32]>::sort_by<host_probe::host::loadscreen::load::_closure_2_>::_closure_0_>
                (local_140,local_138,1,&local_128);
    }
    else {
      core::slice::sort::stable::driftsort_main::
      <f32,<[f32]>::sort_by<host_probe::host::loadscreen::load::_closure_2_>::_closure_0_,alloc::vec::Vec<f32>>
                (local_140,local_138,&local_128);
    }
LAB_1401ac12a:
    uVar16 = (uVar21 * 0x62) / 100;
    if (uVar16 < uVar21) {
      if (local_d8 < 3) {
        fVar4 = *(float *)((longlong)puVar10 + (uVar21 / 0x32) * 4);
        fVar5 = *(float *)((longlong)puVar10 + uVar16 * 4);
        *(float *)((longlong)&local_e8 + local_d8 * 4) = (fVar4 + fVar5) * 0.5;
        local_b8[local_d8] = fVar5 - fVar4;
        if (local_148 != (ulonglong *)0x0) {
          __rustc::__rust_dealloc(fVar4,(longlong)local_148 << 2,4);
          return;
        }
        local_d8 = 1;
        local_120 = local_158 + (longlong)local_150 * 0xc;
        local_128 = local_158;
        local_118 = (longlong **)&local_d8;
        <alloc::vec::Vec<f32>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<f32,core::iter::adapters::map::Map<core::slice::iter::Iter<[f32;3usize]>,host_probe::host::loadscreen::load::{closure#1}>>>
        ::from_iter(&local_148,&local_128);
        uVar21 = local_138;
        puVar10 = local_140;
        if (local_138 < 2) {
          local_128 = (code *)&local_129;
          if (local_138 == 0) goto LAB_1401ac835;
        }
        else if (local_138 < 0x15) {
          local_128 = (code *)&local_129;
          core::slice::sort::shared::smallsort::insertion_sort_shift_left::
          <f32,<[f32]>::sort_by<host_probe::host::loadscreen::load::_closure_2_>::_closure_0_>
                    (local_140,local_138,1,&local_128);
        }
        else {
          local_128 = (code *)&local_129;
          core::slice::sort::stable::driftsort_main::
          <f32,<[f32]>::sort_by<host_probe::host::loadscreen::load::_closure_2_>::_closure_0_,alloc::vec::Vec<f32>>
                    (local_140,local_138,&local_128);
        }
        uVar16 = (uVar21 * 0x62) / 100;
        if (uVar21 <= uVar16) {
LAB_1401ac85d:
          ppuVar20 = &
                     PTR_anon_2ea4320f4837ea28fa7015f50151942e_45_llvm_7975856906012809254_1406d3740
          ;
          goto LAB_1401ac864;
        }
        if (local_d8 < 3) {
          fVar4 = *(float *)((longlong)puVar10 + (uVar21 / 0x32) * 4);
          fVar5 = *(float *)((longlong)puVar10 + uVar16 * 4);
          *(float *)((longlong)&local_e8 + local_d8 * 4) = (fVar4 + fVar5) * 0.5;
          local_b8[local_d8] = fVar5 - fVar4;
          if (local_148 != (ulonglong *)0x0) {
            __rustc::__rust_dealloc(fVar4,(longlong)local_148 << 2,4);
            return;
          }
          local_d8 = 2;
          local_120 = local_158 + (longlong)local_150 * 0xc;
          local_128 = local_158;
          local_118 = (longlong **)&local_d8;
          <alloc::vec::Vec<f32>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<f32,core::iter::adapters::map::Map<core::slice::iter::Iter<[f32;3usize]>,host_probe::host::loadscreen::load::{closure#1}>>>
          ::from_iter(&local_148,&local_128);
          uVar21 = local_138;
          puVar10 = local_140;
          if (local_138 < 2) {
            local_128 = (code *)&local_129;
            if (local_138 == 0) goto LAB_1401ac835;
          }
          else if (local_138 < 0x15) {
            local_128 = (code *)&local_129;
            core::slice::sort::shared::smallsort::insertion_sort_shift_left::
            <f32,<[f32]>::sort_by<host_probe::host::loadscreen::load::_closure_2_>::_closure_0_>
                      (local_140,local_138,1,&local_128);
          }
          else {
            local_128 = (code *)&local_129;
            core::slice::sort::stable::driftsort_main::
            <f32,<[f32]>::sort_by<host_probe::host::loadscreen::load::_closure_2_>::_closure_0_,alloc::vec::Vec<f32>>
                      (local_140,local_138,&local_128);
          }
          uVar16 = (uVar21 * 0x62) / 100;
          if (uVar21 <= uVar16) goto LAB_1401ac85d;
          if (local_d8 < 3) {
            fVar4 = *(float *)((longlong)puVar10 + (uVar21 / 0x32) * 4);
            fVar5 = *(float *)((longlong)puVar10 + uVar16 * 4);
            *(float *)((longlong)&local_e8 + local_d8 * 4) = (fVar4 + fVar5) * 0.5;
            local_b8[local_d8] = fVar5 - fVar4;
            if (local_148 != (ulonglong *)0x0) {
              __rustc::__rust_dealloc(fVar4,(longlong)local_148 << 2,4);
              return;
            }
            local_bc = SQRT(local_b8[2] * local_b8[2] +
                            local_b8[0] * local_b8[0] + local_b8[1] * local_b8[1]) * 0.45;
            if (local_bc <= 1.0) {
              local_bc = 1.0;
            }
            local_b8[3] = 1.7169889e-16;
            local_b8[4] = (float)local_e8 - local_b8[0] * 0.5;
            local_a4 = local_b8[0] * 0.5 + (float)local_e8;
            local_120 = local_158 + (longlong)local_150 * 0xc;
            local_128 = local_158;
            local_118 = (longlong **)(local_b8 + 4);
            local_110 = &local_a4;
            local_108 = local_b8 + 3;
            local_100 = &local_bc;
            <alloc::vec::Vec<[f32;8usize]>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<[f32;8usize],core::iter::adapters::map::Map<core::slice::iter::Iter<[f32;3usize]>,host_probe::host::loadscreen::load::{closure#4}>>>
            ::from_iter(&local_d8,&local_128);
            local_90 = local_c8;
            local_128 = (code *)&local_90;
            local_120 = <u64_as_core::fmt::Display>::fmt;
            local_118 = (longlong **)&local_a0;
            local_110 = (float *)&<&str_as_core::fmt::Display>::fmt;
            local_108 = (float *)&local_e8;
            local_100 = (float *)<[f32;3usize]as_core::fmt::Debug>::fmt;
            local_f0 = <f32_as_core::fmt::Display>::fmt;
            local_f8 = &local_bc;
            alloc::fmt::format::format_inner(&local_148,&DAT_1406d36d0,&local_128)
            ;
            puVar10 = local_140;
            puVar9 = local_148;
            log(local_140,local_138);
            fVar4 = local_bc;
            uVar21 = local_d8;
            if (puVar9 != (ulonglong *)0x0) {
              __rustc::__rust_dealloc(puVar10,puVar9,1);
              return;
            }
            uStack_84 = local_e8;
            uStack_7c = local_e0;
            LOCK();
            bVar23 = STATE == '\0';
            if (bVar23) {
              STATE = '\x01';
            }
            UNLOCK();
            if (!bVar23) {
              <std::sys::sync::mutex::futex::Mutex>::lock_contended(&STATE);
            }
            puVar7 = __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT;
            if ((*(ulonglong *)
                  __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
                0x7fffffffffffffff) == 0) {
              bVar12 = 0;
            }
            else {
              bVar12 = std::panicking::panic_count::is_zero_slow_path();
              bVar12 = bVar12 ^ 1;
            }
            if (DAT_14069b831 == '\0') {
              if ((DAT_14069b838 != 2) && (DAT_14069b878 != 0)) {
                __rustc::__rust_dealloc(DAT_14069b880,DAT_14069b878 << 5,4);
                return;
              }
              _DAT_14069b838 = 0;
              DAT_14069b878 = uVar21;
              DAT_14069b880 = local_d0;
              DAT_14069b888 = local_c8;
              DAT_14069b898 = 0xffffffff;
              uStack_84._4_4_ = (undefined4)((ulonglong)uStack_84 >> 0x20);
              DAT_14069b8a0._0_4_ = (undefined4)uStack_84;
              DAT_14069b8a0._4_4_ = uStack_84._4_4_;
              DAT_14069b8ac = fVar4;
              _DAT_14069b8b0 = 0x2d;
              _DAT_14069b8b8 = 0;
              _DAT_14069b89c = local_88;
              DAT_14069b8a8 = uStack_7c;
              if (((bVar12 == 0) && ((*(ulonglong *)puVar7 & 0x7fffffffffffffff) != 0)) &&
                 (cVar13 = std::panicking::panic_count::is_zero_slow_path(),
                 cVar13 == '\0')) {
                DAT_14069b831 = '\x01';
              }
              cVar13 = STATE;
              LOCK();
              STATE = '\0';
              UNLOCK();
              if (cVar13 == '\x02') {
                <std::sys::sync::mutex::futex::Mutex>::wake(&STATE);
              }
              if (local_160 != (longlong *)0x0) {
                pcVar14 = (code *)((longlong)local_160 * 0xc);
                uVar19 = 4;
                pcVar15 = local_158;
LAB_1401ac687:
                __rustc::__rust_dealloc(pcVar15,pcVar14,uVar19);
                return;
              }
LAB_1401ac68c:
              if (uVar6 == 0) {
                return;
              }
              __rustc::__rust_dealloc(plVar11,uVar6,1);
              return;
            }
            local_128 = (code *)&STATE;
            uVar6 = (ulonglong)local_120 >> 8;
            local_120 = (code *)CONCAT71((int7)uVar6,bVar12);
            core::result::unwrap_failed
                      ("called `Result::unwrap()` on an `Err` value",0x2b,&local_128,
                       &anon_2ea4320f4837ea28fa7015f50151942e_74_llvm_7975856906012809254,
                       &
                       PTR_anon_2ea4320f4837ea28fa7015f50151942e_45_llvm_7975856906012809254_1406d3710
                      );
            goto LAB_1401ac883;
          }
        }
      }
      ppuVar20 = &PTR_anon_2ea4320f4837ea28fa7015f50151942e_45_llvm_7975856906012809254_1406d3758;
      uVar16 = local_d8;
      uVar21 = 3;
    }
    else {
      ppuVar20 = &PTR_anon_2ea4320f4837ea28fa7015f50151942e_45_llvm_7975856906012809254_1406d3740;
    }
  }
LAB_1401ac864:
  core::panicking::panic_bounds_check(uVar16,uVar21,ppuVar20);
LAB_1401ac883:
                    /* WARNING: Does not return */
  pcVar14 = (code *)invalidInstructionException();
  (*pcVar14)();
}


