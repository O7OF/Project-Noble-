
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::set_game_variant */

longlong * __rustcall
host_probe::set_game_variant
          (longlong *param_1,longlong param_2,ulonglong param_3,undefined8 param_4,
          undefined8 param_5,longlong param_6,longlong ***param_7,code ***param_8)

{
  undefined2 uVar1;
  undefined2 uVar2;
  undefined1 auVar3 [16];
  code *pcVar4;
  longlong ***ppplVar5;
  undefined1 auVar6 [14];
  undefined1 auVar7 [12];
  unkbyte10 Var8;
  undefined1 auVar9 [12];
  undefined1 auVar10 [14];
  undefined1 auVar11 [12];
  undefined1 auVar12 [12];
  undefined1 auVar13 [14];
  undefined1 auVar14 [12];
  undefined1 auVar15 [12];
  undefined1 auVar16 [14];
  undefined1 auVar17 [12];
  undefined1 auVar18 [12];
  longlong ***ppplVar19;
  code **ppcVar20;
  code **ppcVar21;
  code **ppcVar22;
  char cVar23;
  longlong *plVar24;
  void *_Src;
  longlong lVar25;
  void *_Dst;
  void *_Dst_00;
  byte bVar26;
  code ***pppcVar27;
  undefined8 uVar28;
  ulonglong uVar29;
  longlong ***ppplVar30;
  longlong ***ppplVar31;
  longlong ***ppplVar32;
  undefined1 auVar33 [16];
  longlong lVar34;
  undefined1 auVar35 [16];
  longlong lVar36;
  longlong lVar37;
  undefined1 auVar38 [16];
  longlong lVar39;
  undefined1 auVar40 [16];
  undefined1 auVar41 [16];
  undefined1 auVar49 [16];
  undefined1 auVar52 [16];
  undefined1 auVar54 [16];
  undefined1 auVar55 [16];
  undefined1 auVar56 [16];
  undefined1 auVar64 [16];
  undefined1 auVar67 [16];
  undefined1 auVar68 [16];
  undefined1 auVar76 [16];
  undefined1 auVar79 [16];
  undefined1 auVar80 [16];
  undefined1 auVar88 [16];
  longlong ***local_1d8;
  code **ppcStack_1d0;
  code ***local_1c8;
  code **ppcStack_1c0;
  longlong ***local_1b8;
  undefined1 auStack_1b0 [16];
  longlong ***ppplStack_1a0;
  undefined2 local_198;
  undefined6 uStack_196;
  ulonglong uStack_190;
  code **local_188;
  longlong ***local_180;
  longlong ***local_178;
  code **local_170;
  longlong ***local_168;
  code **local_160;
  longlong ***local_158;
  longlong ***ppplStack_150;
  longlong ***local_148;
  code **ppcStack_140;
  longlong **local_138;
  undefined8 uStack_130;
  longlong local_128;
  longlong ***ppplStack_120;
  longlong local_118;
  undefined8 uStack_110;
  code **local_100;
  longlong ***local_f8;
  undefined8 local_f0;
  longlong **local_e8;
  code **ppcStack_e0;
  longlong local_d8;
  longlong **local_d0;
  code *local_c8;
  code *pcStack_c0;
  code *local_b8;
  code *pcStack_b0;
  longlong ***local_a8;
  undefined8 local_a0;
  longlong local_98;
  undefined8 local_90;
  undefined8 local_88;
  longlong local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined1 auVar42 [16];
  undefined1 auVar43 [16];
  undefined1 auVar44 [16];
  undefined1 auVar50 [16];
  undefined1 auVar45 [16];
  undefined1 auVar46 [16];
  undefined1 auVar51 [16];
  undefined1 auVar47 [16];
  undefined1 auVar48 [16];
  undefined2 uVar53;
  undefined1 auVar57 [16];
  undefined1 auVar69 [16];
  undefined1 auVar58 [16];
  undefined1 auVar70 [16];
  undefined1 auVar59 [16];
  undefined1 auVar65 [16];
  undefined1 auVar71 [16];
  undefined1 auVar77 [16];
  undefined1 auVar60 [16];
  undefined1 auVar72 [16];
  undefined1 auVar61 [16];
  undefined1 auVar66 [16];
  undefined1 auVar73 [16];
  undefined1 auVar78 [16];
  undefined1 auVar62 [16];
  undefined1 auVar74 [16];
  undefined1 auVar63 [16];
  undefined1 auVar75 [16];
  undefined1 auVar81 [16];
  undefined1 auVar82 [16];
  undefined1 auVar83 [16];
  undefined1 auVar89 [16];
  undefined1 auVar84 [16];
  undefined1 auVar85 [16];
  undefined1 auVar90 [16];
  undefined1 auVar86 [16];
  undefined1 auVar87 [16];
  
  local_188 = (code **)param_7;
  local_180 = (longlong ***)param_8;
  cVar23 = gametype::is_ours(param_7,param_8);
  if (cVar23 != '\0') {
    uVar28 = DAT_14069b4a0;
    if (DAT_14069b4b8 != 0) {
      uVar28 = 0;
    }
    gametype::load_in(&local_1d8,param_7,param_8,uVar28,DAT_14069b4a8);
    if ((code ***)local_1d8 == (code ***)0xffffffffffffffff) {
      local_148 = (longlong ***)ppcStack_1c0;
      local_158 = (longlong ***)ppcStack_1d0;
      ppplStack_150 = (longlong ***)local_1c8;
      set_game_variant::_closure_0_(&local_158);
                    /* WARNING: Does not return */
      pcVar4 = (code *)invalidInstructionException();
      (*pcVar4)();
    }
    local_118 = CONCAT62(uStack_196,local_198);
    uStack_110 = uStack_190;
    local_128 = auStack_1b0._8_8_;
    ppplStack_120 = ppplStack_1a0;
    local_138 = (longlong **)local_1b8;
    uStack_130 = auStack_1b0._0_8_;
    local_148 = (longlong ***)local_1c8;
    ppcStack_140 = ppcStack_1c0;
    local_158 = local_1d8;
    ppplStack_150 = (longlong ***)ppcStack_1d0;
    set_game_variant(&local_e8,param_2,param_3,param_4,param_5,param_6,local_1b8,auStack_1b0._0_8_);
    if (0xff03 < param_3) {
      gametype::apply(&local_1d8,param_2 + 0x300,0xfc04,&local_158);
      ppcVar20 = ppcStack_1d0;
      ppplVar19 = local_1d8;
      ppplVar30 = (longlong ***)(ppcStack_1d0 + (longlong)local_1c8 * 3);
      local_178 = (longlong ***)ppcStack_1d0;
      local_170 = ppcStack_1d0;
      local_168 = local_1d8;
      ppplVar32 = (longlong ***)ppcStack_1d0;
      local_160 = (code **)ppplVar30;
      if (local_1c8 != (code ***)0x0) {
        ppplVar5 = (longlong ***)ppcStack_1d0;
        do {
          ppplVar31 = ppplVar5 + 3;
          local_b8 = (code *)ppplVar5[1];
          pcStack_b0 = (code *)ppplVar5[2];
          ppplVar32 = ppplVar31;
          if ((code **)*ppplVar5 == (code **)0xffffffffffffffff) break;
          local_1d8 = (longlong ***)&local_188;
          ppcStack_1d0 = (code **)&<&str_as_core::fmt::Display>::fmt;
          local_1c8 = (code ***)&local_d0;
          ppcStack_1c0 = (code **)<alloc::string::String_as_core::fmt::Display>
                                  ::fmt;
          local_d0 = *ppplVar5;
          local_c8 = local_b8;
          pcStack_c0 = pcStack_b0;
          alloc::fmt::format::format_inner(&local_100,&DAT_1406d5b27,&local_1d8);
          ppplVar32 = local_f8;
          ppcVar22 = local_100;
          host::log(local_f8,local_f0);
          if ((longlong ***)ppcVar22 != (longlong ***)0x0) {
            plVar24 = (longlong *)__rustc::__rust_dealloc(ppplVar32,ppcVar22,1);
            return plVar24;
          }
          if ((code **)local_d0 != (code **)0x0) {
            plVar24 = (longlong *)__rustc::__rust_dealloc(local_c8,local_d0,1);
            return plVar24;
          }
          ppplVar32 = ppplVar30;
          ppplVar5 = ppplVar31;
        } while (ppplVar31 != ppplVar30);
      }
      auVar3._8_8_ = 0;
      auVar3._0_8_ = (longlong)ppplVar30 - (longlong)ppplVar32;
      uVar28 = SUB168(auVar3 * ZEXT816(0xaaaaaaaaaaaaaaab),8);
      local_170 = (code **)ppplVar32;
      if (ppplVar30 != ppplVar32) {
        uVar29 = (ulonglong)((longlong)ppplVar30 - (longlong)ppplVar32) / 0x18;
        ppplVar32 = ppplVar32 + 1;
        do {
          if (ppplVar32[-1] != (longlong **)0x0) {
            plVar24 = (longlong *)
                      __rustc::__rust_dealloc(*ppplVar32,ppplVar32[-1],1);
            return plVar24;
          }
          ppplVar32 = ppplVar32 + 3;
          uVar29 = uVar29 - 1;
          uVar28 = 0;
        } while (uVar29 != 0);
      }
      if ((code ***)ppplVar19 != (code ***)0x0) {
        plVar24 = (longlong *)
                  __rustc::__rust_dealloc(ppcVar20,(longlong)ppplVar19 * 0x18,8);
        return plVar24;
      }
      if ((*(uint *)(param_2 + 0xc) | 4) == 5) {
        bVar26 = 0;
      }
      else {
        bVar26 = *(byte *)(param_2 + 0x5bc);
      }
      matchrules::for_game
                (bVar26 & 1,CONCAT71((int7)((ulonglong)uVar28 >> 8),*(int *)(param_2 + 0x300) == 1),
                 uStack_110._2_1_,uStack_110 & 0xff,uStack_110 >> 8 & 0xff,uStack_110 >> 0x18 & 0xff
                 ,uStack_110 >> 0x20 & 0xff);
      local_1b8 = (longlong ***)&ppcStack_140;
      local_1d8 = (longlong ***)&local_188;
      ppcStack_1d0 = (code **)&<&str_as_core::fmt::Display>::fmt;
      ppcStack_1c0 = (code **)<alloc::string::String_as_core::fmt::Display>
                              ::fmt;
      auStack_1b0._0_8_ =
           <alloc::string::String_as_core::fmt::Display>::fmt;
      local_1c8 = (code ***)&local_158;
      alloc::fmt::format::format_inner(&local_178,&DAT_1406d5b08,&local_1d8);
      ppcVar20 = local_170;
      ppplVar19 = local_178;
      host::log(local_170,local_168);
      if (ppplVar19 != (longlong ***)0x0) {
        plVar24 = (longlong *)__rustc::__rust_dealloc(ppcVar20,ppplVar19,1);
        return plVar24;
      }
      if ((code ***)local_148 == (code ***)0x0) {
        param_1[2] = local_d8;
        *param_1 = CONCAT71(local_e8._1_7_,(char)local_e8);
        param_1[1] = (longlong)ppcStack_e0;
        pppcVar27 = (code ***)local_158;
        ppcStack_e0 = (code **)ppplStack_150;
      }
      else {
        param_1[2] = (longlong)local_148;
        *param_1 = (longlong)local_158;
        param_1[1] = (longlong)ppplStack_150;
        pppcVar27 = (code ***)CONCAT71(local_e8._1_7_,(char)local_e8);
      }
      if (pppcVar27 != (code ***)0x0) {
        plVar24 = (longlong *)__rustc::__rust_dealloc(ppcStack_e0,pppcVar27,1);
        return plVar24;
      }
      if ((longlong ***)ppcStack_140 != (longlong ***)0x0) {
        plVar24 = (longlong *)__rustc::__rust_dealloc(local_138,ppcStack_140,1);
        return plVar24;
      }
      if (local_118 != 0) {
        pppcVar27 = (code ***)(ppplStack_120 + 1);
        do {
          if (pppcVar27[-1] != (code **)0x0) {
            plVar24 = (longlong *)
                      __rustc::__rust_dealloc(*pppcVar27,pppcVar27[-1],1);
            return plVar24;
          }
          pppcVar27 = pppcVar27 + 4;
          local_118 = local_118 + -1;
        } while (local_118 != 0);
      }
      if (local_128 != 0) {
        plVar24 = (longlong *)
                  __rustc::__rust_dealloc(ppplStack_120,local_128 << 5,8);
        return plVar24;
      }
      return param_1;
    }
    core::slice::index::slice_index_fail
              (0x300,0xff04,param_3,&PTR_s_src_main_rs_1406d5af0);
    goto LAB_1401c4280;
  }
  cVar23 = <std::path::Path>::is_file(param_7,param_8);
  local_a0 = param_4;
  if (cVar23 == '\0') {
    <std::path::Path>::_join(&local_1d8,param_4,param_5,&DAT_1406d59f1,9);
    ppcVar20 = ppcStack_1d0;
    <std::path::Path>::_join(&local_158,ppcStack_1d0,local_1c8,&DAT_1406d59fa,0xd)
    ;
    ppplVar32 = local_148;
    ppplVar19 = ppplStack_150;
    pppcVar27 = (code ***)local_180;
    local_100 = (code **)::<str>::trim_end_matches::<&str>(local_188,local_180,&DAT_1406d5a07,4);
    local_178 = (longlong ***)&local_100;
    local_170 = (code **)&<&str_as_core::fmt::Display>::fmt;
    local_f8 = (longlong ***)pppcVar27;
    alloc::fmt::format::format_inner(&local_98,&DAT_1406d5a0b,&local_178);
    <std::path::Path>::_join(&local_178,ppplVar19,ppplVar32,local_90,local_88);
    if (local_98 != 0) {
      plVar24 = (longlong *)__rustc::__rust_dealloc(local_90,local_98,1);
      return plVar24;
    }
    if ((code ***)local_158 != (code ***)0x0) {
      plVar24 = (longlong *)__rustc::__rust_dealloc(ppplVar19,local_158,1);
      return plVar24;
    }
    if ((code ***)local_1d8 != (code ***)0x0) {
      plVar24 = (longlong *)__rustc::__rust_dealloc(ppcVar20,local_1d8,1);
      return plVar24;
    }
  }
  else {
    <core::wtf8::Wtf8>::to_owned(&local_178,param_7,param_8);
  }
  ppcVar22 = local_170;
  std::fs::read::inner(&local_1d8,local_170,local_168);
  ppcVar20 = ppcStack_1d0;
  if ((code ***)local_1d8 == (code ***)0xffffffffffffffff) {
    set_game_variant::_closure_1_(ppcVar22,local_168,ppcStack_1d0);
    goto LAB_1401c4280;
  }
  local_a8 = (longlong ***)local_1c8;
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  _Src = (void *)__rustc::__rust_alloc_zeroed(0xfc04,1);
  if (_Src == (void *)0x0) {
    alloc::raw_vec::handle_error(1,0xfc04);
    goto LAB_1401c4280;
  }
  local_b8 = (code *)((ulonglong)local_b8 & 0xffffffffffffff00);
  local_e8._0_1_ = (*(code *)(param_6 + 0x373a0))(ppcVar20,_Src,&local_b8);
  if (param_3 < 0xff04) {
    core::slice::index::slice_index_fail
              (0x300,0xff04,param_3,&PTR_s_src_main_rs_1406d5a18);
    goto LAB_1401c4280;
  }
  lVar34 = 0;
  lVar36 = 0;
  lVar25 = 0;
  auVar33._8_4_ = 0xffffffff;
  auVar33._0_8_ = 0xffffffffffffffff;
  auVar33._12_4_ = 0xffffffff;
  lVar37 = 0;
  lVar39 = 0;
  while( true ) {
    uVar53 = *(undefined2 *)((longlong)_Src + lVar25 + 2);
    uVar1 = *(undefined2 *)(param_2 + 0x300 + lVar25);
    auVar55[0] = -((char)uVar1 == (char)*(undefined2 *)((longlong)_Src + lVar25));
    auVar55[1] = -((char)((ushort)uVar1 >> 8) ==
                  (char)((ushort)*(undefined2 *)((longlong)_Src + lVar25) >> 8));
    auVar55[2] = 0xff;
    auVar55[3] = 0xff;
    auVar55[4] = 0xff;
    auVar55[5] = 0xff;
    auVar55[6] = 0xff;
    auVar55[7] = 0xff;
    auVar55[8] = 0xff;
    auVar55[9] = 0xff;
    auVar55[10] = 0xff;
    auVar55[0xb] = 0xff;
    auVar55[0xc] = 0xff;
    auVar55[0xd] = 0xff;
    auVar55[0xe] = 0xff;
    auVar55[0xf] = 0xff;
    uVar1 = *(undefined2 *)(param_2 + 0x302 + lVar25);
    auVar79[0] = -((char)uVar1 == (char)uVar53);
    auVar79[1] = -((char)((ushort)uVar1 >> 8) == (char)((ushort)uVar53 >> 8));
    auVar79[2] = 0xff;
    auVar79[3] = 0xff;
    auVar79[4] = 0xff;
    auVar79[5] = 0xff;
    auVar79[6] = 0xff;
    auVar79[7] = 0xff;
    auVar79[8] = 0xff;
    auVar79[9] = 0xff;
    auVar79[10] = 0xff;
    auVar79[0xb] = 0xff;
    auVar79[0xc] = 0xff;
    auVar79[0xd] = 0xff;
    auVar79[0xe] = 0xff;
    auVar79[0xf] = 0xff;
    auVar55 = auVar55 ^ auVar33;
    auVar63._0_14_ = auVar55._0_14_;
    auVar63[0xe] = auVar55[7];
    auVar63[0xf] = auVar55[7];
    auVar62._14_2_ = auVar63._14_2_;
    auVar62._0_13_ = auVar55._0_13_;
    auVar62[0xd] = auVar55[6];
    auVar61._13_3_ = auVar62._13_3_;
    auVar61._0_12_ = auVar55._0_12_;
    auVar61[0xc] = auVar55[6];
    auVar60._12_4_ = auVar61._12_4_;
    auVar60._0_11_ = auVar55._0_11_;
    auVar60[0xb] = auVar55[5];
    auVar59._11_5_ = auVar60._11_5_;
    auVar59._0_10_ = auVar55._0_10_;
    auVar59[10] = auVar55[5];
    auVar58._10_6_ = auVar59._10_6_;
    auVar58._0_9_ = auVar55._0_9_;
    auVar58[9] = auVar55[4];
    auVar57._9_7_ = auVar58._9_7_;
    auVar57._0_8_ = auVar55._0_8_;
    auVar57[8] = auVar55[4];
    Var8 = CONCAT91(CONCAT81(auVar57._8_8_,auVar55[3]),auVar55[3]);
    auVar7._2_10_ = Var8;
    auVar7[1] = auVar55[2];
    auVar7[0] = auVar55[2];
    auVar6._2_12_ = auVar7;
    auVar6[1] = auVar55[1];
    auVar6[0] = auVar55[1];
    auVar56._0_2_ = CONCAT11(auVar55[0],auVar55[0]);
    auVar56._2_14_ = auVar6;
    uVar53 = (undefined2)Var8;
    auVar66._0_12_ = auVar56._0_12_;
    auVar66._12_2_ = uVar53;
    auVar66._14_2_ = uVar53;
    auVar65._12_4_ = auVar66._12_4_;
    auVar65._0_10_ = auVar56._0_10_;
    auVar65._10_2_ = auVar7._0_2_;
    auVar64._10_6_ = auVar65._10_6_;
    auVar64._0_8_ = auVar56._0_8_;
    auVar64._8_2_ = auVar7._0_2_;
    auVar9._4_8_ = auVar64._8_8_;
    auVar9._2_2_ = auVar6._0_2_;
    auVar9._0_2_ = auVar6._0_2_;
    auVar35._2_2_ = auVar56._0_2_;
    auVar35._0_2_ = auVar56._0_2_;
    auVar35._4_4_ = auVar9._0_4_;
    auVar35._8_4_ = auVar9._0_4_;
    auVar35._12_4_ = auVar65._12_4_;
    lVar34 = SUB168(auVar35 & _DAT_1406d4830,0) + lVar34;
    lVar36 = SUB168(auVar35 & _DAT_1406d4830,8) + lVar36;
    auVar79 = auVar79 ^ auVar33;
    auVar87._0_14_ = auVar79._0_14_;
    auVar87[0xe] = auVar79[7];
    auVar87[0xf] = auVar79[7];
    auVar86._14_2_ = auVar87._14_2_;
    auVar86._0_13_ = auVar79._0_13_;
    auVar86[0xd] = auVar79[6];
    auVar85._13_3_ = auVar86._13_3_;
    auVar85._0_12_ = auVar79._0_12_;
    auVar85[0xc] = auVar79[6];
    auVar84._12_4_ = auVar85._12_4_;
    auVar84._0_11_ = auVar79._0_11_;
    auVar84[0xb] = auVar79[5];
    auVar83._11_5_ = auVar84._11_5_;
    auVar83._0_10_ = auVar79._0_10_;
    auVar83[10] = auVar79[5];
    auVar82._10_6_ = auVar83._10_6_;
    auVar82._0_9_ = auVar79._0_9_;
    auVar82[9] = auVar79[4];
    auVar81._9_7_ = auVar82._9_7_;
    auVar81._0_8_ = auVar79._0_8_;
    auVar81[8] = auVar79[4];
    Var8 = CONCAT91(CONCAT81(auVar81._8_8_,auVar79[3]),auVar79[3]);
    auVar11._2_10_ = Var8;
    auVar11[1] = auVar79[2];
    auVar11[0] = auVar79[2];
    auVar10._2_12_ = auVar11;
    auVar10[1] = auVar79[1];
    auVar10[0] = auVar79[1];
    auVar80._0_2_ = CONCAT11(auVar79[0],auVar79[0]);
    auVar80._2_14_ = auVar10;
    uVar53 = (undefined2)Var8;
    auVar90._0_12_ = auVar80._0_12_;
    auVar90._12_2_ = uVar53;
    auVar90._14_2_ = uVar53;
    auVar89._12_4_ = auVar90._12_4_;
    auVar89._0_10_ = auVar80._0_10_;
    auVar89._10_2_ = auVar11._0_2_;
    auVar88._10_6_ = auVar89._10_6_;
    auVar88._0_8_ = auVar80._0_8_;
    auVar88._8_2_ = auVar11._0_2_;
    auVar12._4_8_ = auVar88._8_8_;
    auVar12._2_2_ = auVar10._0_2_;
    auVar12._0_2_ = auVar10._0_2_;
    auVar38._2_2_ = auVar80._0_2_;
    auVar38._0_2_ = auVar80._0_2_;
    auVar38._4_4_ = auVar12._0_4_;
    auVar38._8_4_ = auVar12._0_4_;
    auVar38._12_4_ = auVar89._12_4_;
    lVar37 = SUB168(auVar38 & _DAT_1406d4830,0) + lVar37;
    lVar39 = SUB168(auVar38 & _DAT_1406d4830,8) + lVar39;
    if (lVar25 == 0xfc00) break;
    uVar53 = *(undefined2 *)((longlong)_Src + lVar25 + 4);
    uVar1 = *(undefined2 *)((longlong)_Src + lVar25 + 6);
    uVar2 = *(undefined2 *)(param_2 + 0x304 + lVar25);
    auVar67[0] = -((char)uVar2 == (char)uVar53);
    auVar67[1] = -((char)((ushort)uVar2 >> 8) == (char)((ushort)uVar53 >> 8));
    auVar67[2] = 0xff;
    auVar67[3] = 0xff;
    auVar67[4] = 0xff;
    auVar67[5] = 0xff;
    auVar67[6] = 0xff;
    auVar67[7] = 0xff;
    auVar67[8] = 0xff;
    auVar67[9] = 0xff;
    auVar67[10] = 0xff;
    auVar67[0xb] = 0xff;
    auVar67[0xc] = 0xff;
    auVar67[0xd] = 0xff;
    auVar67[0xe] = 0xff;
    auVar67[0xf] = 0xff;
    uVar53 = *(undefined2 *)(param_2 + 0x306 + lVar25);
    auVar40[0] = -((char)uVar53 == (char)uVar1);
    auVar40[1] = -((char)((ushort)uVar53 >> 8) == (char)((ushort)uVar1 >> 8));
    auVar40[2] = 0xff;
    auVar40[3] = 0xff;
    auVar40[4] = 0xff;
    auVar40[5] = 0xff;
    auVar40[6] = 0xff;
    auVar40[7] = 0xff;
    auVar40[8] = 0xff;
    auVar40[9] = 0xff;
    auVar40[10] = 0xff;
    auVar40[0xb] = 0xff;
    auVar40[0xc] = 0xff;
    auVar40[0xd] = 0xff;
    auVar40[0xe] = 0xff;
    auVar40[0xf] = 0xff;
    auVar67 = auVar67 ^ auVar33;
    auVar75._0_14_ = auVar67._0_14_;
    auVar75[0xe] = auVar67[7];
    auVar75[0xf] = auVar67[7];
    auVar74._14_2_ = auVar75._14_2_;
    auVar74._0_13_ = auVar67._0_13_;
    auVar74[0xd] = auVar67[6];
    auVar73._13_3_ = auVar74._13_3_;
    auVar73._0_12_ = auVar67._0_12_;
    auVar73[0xc] = auVar67[6];
    auVar72._12_4_ = auVar73._12_4_;
    auVar72._0_11_ = auVar67._0_11_;
    auVar72[0xb] = auVar67[5];
    auVar71._11_5_ = auVar72._11_5_;
    auVar71._0_10_ = auVar67._0_10_;
    auVar71[10] = auVar67[5];
    auVar70._10_6_ = auVar71._10_6_;
    auVar70._0_9_ = auVar67._0_9_;
    auVar70[9] = auVar67[4];
    auVar69._9_7_ = auVar70._9_7_;
    auVar69._0_8_ = auVar67._0_8_;
    auVar69[8] = auVar67[4];
    Var8 = CONCAT91(CONCAT81(auVar69._8_8_,auVar67[3]),auVar67[3]);
    auVar14._2_10_ = Var8;
    auVar14[1] = auVar67[2];
    auVar14[0] = auVar67[2];
    auVar13._2_12_ = auVar14;
    auVar13[1] = auVar67[1];
    auVar13[0] = auVar67[1];
    auVar68._0_2_ = CONCAT11(auVar67[0],auVar67[0]);
    auVar68._2_14_ = auVar13;
    uVar53 = (undefined2)Var8;
    auVar78._0_12_ = auVar68._0_12_;
    auVar78._12_2_ = uVar53;
    auVar78._14_2_ = uVar53;
    auVar77._12_4_ = auVar78._12_4_;
    auVar77._0_10_ = auVar68._0_10_;
    auVar77._10_2_ = auVar14._0_2_;
    auVar76._10_6_ = auVar77._10_6_;
    auVar76._0_8_ = auVar68._0_8_;
    auVar76._8_2_ = auVar14._0_2_;
    auVar15._4_8_ = auVar76._8_8_;
    auVar15._2_2_ = auVar13._0_2_;
    auVar15._0_2_ = auVar13._0_2_;
    auVar54._2_2_ = auVar68._0_2_;
    auVar54._0_2_ = auVar68._0_2_;
    auVar54._4_4_ = auVar15._0_4_;
    auVar54._8_4_ = auVar15._0_4_;
    auVar54._12_4_ = auVar77._12_4_;
    auVar40 = auVar40 ^ auVar33;
    auVar48._0_14_ = auVar40._0_14_;
    auVar48[0xe] = auVar40[7];
    auVar48[0xf] = auVar40[7];
    auVar47._14_2_ = auVar48._14_2_;
    auVar47._0_13_ = auVar40._0_13_;
    auVar47[0xd] = auVar40[6];
    auVar46._13_3_ = auVar47._13_3_;
    auVar46._0_12_ = auVar40._0_12_;
    auVar46[0xc] = auVar40[6];
    auVar45._12_4_ = auVar46._12_4_;
    auVar45._0_11_ = auVar40._0_11_;
    auVar45[0xb] = auVar40[5];
    auVar44._11_5_ = auVar45._11_5_;
    auVar44._0_10_ = auVar40._0_10_;
    auVar44[10] = auVar40[5];
    auVar43._10_6_ = auVar44._10_6_;
    auVar43._0_9_ = auVar40._0_9_;
    auVar43[9] = auVar40[4];
    auVar42._9_7_ = auVar43._9_7_;
    auVar42._0_8_ = auVar40._0_8_;
    auVar42[8] = auVar40[4];
    Var8 = CONCAT91(CONCAT81(auVar42._8_8_,auVar40[3]),auVar40[3]);
    auVar17._2_10_ = Var8;
    auVar17[1] = auVar40[2];
    auVar17[0] = auVar40[2];
    auVar16._2_12_ = auVar17;
    auVar16[1] = auVar40[1];
    auVar16[0] = auVar40[1];
    auVar41._0_2_ = CONCAT11(auVar40[0],auVar40[0]);
    auVar41._2_14_ = auVar16;
    uVar53 = (undefined2)Var8;
    auVar51._0_12_ = auVar41._0_12_;
    auVar51._12_2_ = uVar53;
    auVar51._14_2_ = uVar53;
    auVar50._12_4_ = auVar51._12_4_;
    auVar50._0_10_ = auVar41._0_10_;
    auVar50._10_2_ = auVar17._0_2_;
    auVar49._10_6_ = auVar50._10_6_;
    auVar49._0_8_ = auVar41._0_8_;
    auVar49._8_2_ = auVar17._0_2_;
    auVar18._4_8_ = auVar49._8_8_;
    auVar18._2_2_ = auVar16._0_2_;
    auVar18._0_2_ = auVar16._0_2_;
    auVar52._2_2_ = auVar41._0_2_;
    auVar52._0_2_ = auVar41._0_2_;
    auVar52._4_4_ = auVar18._0_4_;
    auVar52._8_4_ = auVar52._4_4_;
    auVar52._12_4_ = auVar50._12_4_;
    lVar34 = lVar34 + SUB168(auVar54 & _DAT_1406d4830,0);
    lVar36 = lVar36 + SUB168(auVar54 & _DAT_1406d4830,8);
    lVar37 = lVar37 + SUB168(auVar52 & _DAT_1406d4830,0);
    lVar39 = lVar39 + SUB168(auVar52 & _DAT_1406d4830,8);
    lVar25 = lVar25 + 8;
  }
  local_d0 = (longlong **)(lVar39 + lVar36 + lVar37 + lVar34);
  local_100 = ppcVar22;
  local_f8 = local_168;
  ppcStack_1d0 = (code **)<std::path::Display_as_core::fmt::Display>
                          ::fmt;
  local_1c8 = (code ***)&local_e8;
  ppcStack_1c0 = (code **)<u8_as_core::fmt::Display>::fmt;
  auStack_1b0._8_8_ = &local_d0;
  auStack_1b0._0_8_ = <u8_as_core::fmt::Display>::fmt;
  ppplStack_1a0 = (longlong ***)<u64_as_core::fmt::Display>::fmt;
  local_1d8 = (longlong ***)&local_100;
  local_1b8 = (longlong ***)&local_b8;
  alloc::fmt::format::format_inner(&local_158,&DAT_1406d5a30,&local_1d8);
  ppplVar32 = ppplStack_150;
  ppplVar19 = local_158;
  host::log(ppplStack_150,local_148);
  if ((code ***)ppplVar19 != (code ***)0x0) {
    plVar24 = (longlong *)__rustc::__rust_dealloc(ppplVar32,ppplVar19,1);
    return plVar24;
  }
  if (((char)local_e8 == '\0') || ((char)local_b8 == '\0')) {
    core::panicking::panic_fmt
              ("Reach could not decode this game variant",0x51,&PTR_s_src_main_rs_1406d5ab0);
    goto LAB_1401c4280;
  }
  std::sys::env::windows::getenv(&local_1d8,&DAT_1406d5ac8,0xf);
  ppplVar19 = local_180;
  ppcVar21 = local_188;
  pppcVar27 = (code ***)local_1d8;
  if ((code ***)local_1d8 != (code ***)0xffffffffffffffff) {
    if ((code ***)local_1d8 != (code ***)0x0) {
      plVar24 = (longlong *)__rustc::__rust_dealloc(ppcStack_1d0,local_1d8,1);
      return plVar24;
    }
    local_1b8 = (longlong ***)((longlong)local_188 + (longlong)local_180);
    local_1d8 = (longlong ***)0x2f0000005c;
    ppcStack_1d0 = local_188;
    local_1c8 = (code ***)local_180;
    ppcStack_1c0 = local_188;
    auStack_1b0 = (undefined1  [16])0x0;
    ppplStack_1a0 = (longlong ***)param_8;
    local_198 = 1;
    local_100 = (code **)<core::str::iter::SplitInternal<[char;2usize]>>::next_back
                                   (&local_1d8);
    local_f8 = (longlong ***)pppcVar27;
    if ((longlong ***)local_100 == (longlong ***)0x0) {
      local_f8 = ppplVar19;
      local_100 = ppcVar21;
    }
    ppplStack_150 = (longlong ***)&<&str_as_core::fmt::Display>::fmt;
    local_158 = (longlong ***)&local_100;
    alloc::fmt::format::format_inner(&local_80,&DAT_1406d5ad7,&local_158);
    uVar29 = std::fs::write::inner(local_78,local_70,_Src,0xfc04);
    if (local_80 != 0) {
      plVar24 = (longlong *)__rustc::__rust_dealloc(local_78,local_80,1);
      return plVar24;
    }
    if (((uVar29 != 0) && (1 < ((uint)uVar29 & 3) - 2)) && ((uVar29 & 3) != 0)) {
      ppcStack_1d0 = (code **)(uVar29 - 1);
      local_1d8 = (longlong ***)CONCAT71(local_1d8._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&ppcStack_1d0);
    }
  }
  memcpy((int *)(param_2 + 0x300),_Src,0xfc04);
  if ((*(uint *)(param_2 + 0xc) | 4) == 5) {
    bVar26 = 0;
  }
  else {
    bVar26 = *(byte *)(param_2 + 0x5bc);
  }
  matchrules::for_game(bVar26 & 1,*(int *)(param_2 + 0x300) == 1,2,0);
  cVar23 = gametype::is_own_file(local_188,local_180,local_a0,param_5);
  if ((cVar23 == '\0') ||
     (gametype::title_of(&local_1d8,ppcVar20,local_a8),
     (code ***)local_1d8 == (code ***)0xffffffffffffffff)) {
    gametype::display_name(&local_158,_Src,0xfc04);
    ppplVar19 = local_180;
    ppcVar20 = local_188;
    if ((code ***)local_158 == (code ***)0xffffffffffffffff) {
      pppcVar27 = (code ***)local_168;
      lVar25 = <std::path::Path>::file_stem(ppcVar22,local_168);
      if ((longlong)ppplVar19 < 0) {
        uVar28 = 0;
LAB_1401c40a4:
        alloc::raw_vec::handle_error(uVar28,ppplVar19);
        goto LAB_1401c4280;
      }
      if ((code ***)ppplVar19 == (code ***)0x0) {
        _Dst = (void *)0x1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar28 = 1;
        _Dst = (void *)__rustc::__rust_alloc(ppplVar19,1);
        if (_Dst == (void *)0x0) goto LAB_1401c40a4;
        memcpy(_Dst,ppcVar20,(size_t)ppplVar19);
      }
      if (lVar25 == 0) {
                    /* WARNING: Read-only address (ram,0x0001406d4830) is written */
        *param_1 = (longlong)ppplVar19;
        param_1[1] = (longlong)_Dst;
        param_1[2] = (longlong)ppplVar19;
        goto LAB_1401c3d7b;
      }
                    /* WARNING: Read-only address (ram,0x0001406d4830) is written */
      <core::wtf8::Wtf8>::to_string_lossy(&local_1d8,lVar25,pppcVar27);
      pppcVar27 = local_1c8;
      ppcVar20 = ppcStack_1d0;
      if ((code ***)local_1d8 != (code ***)0xffffffffffffffff) {
        param_1[2] = (longlong)local_1c8;
        *param_1 = (longlong)local_1d8;
        param_1[1] = (longlong)ppcStack_1d0;
        if ((code ***)ppplVar19 == (code ***)0x0) goto LAB_1401c3d7b;
        goto LAB_1401c41c1;
      }
      if ((longlong)local_1c8 < 0) {
        uVar28 = 0;
LAB_1401c415c:
        alloc::raw_vec::handle_error(uVar28,pppcVar27);
LAB_1401c4280:
                    /* WARNING: Does not return */
        pcVar4 = (code *)invalidInstructionException();
        (*pcVar4)();
      }
      if (local_1c8 == (code ***)0x0) {
        _Dst_00 = (void *)0x1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar28 = 1;
        _Dst_00 = (void *)__rustc::__rust_alloc(pppcVar27,1);
        if (_Dst_00 == (void *)0x0) goto LAB_1401c415c;
        memcpy(_Dst_00,ppcVar20,(size_t)pppcVar27);
      }
      *param_1 = (longlong)pppcVar27;
      param_1[1] = (longlong)_Dst_00;
      param_1[2] = (longlong)pppcVar27;
      if ((code ***)ppplVar19 != (code ***)0x0) {
LAB_1401c41c1:
        plVar24 = (longlong *)__rustc::__rust_dealloc(_Dst,ppplVar19,1);
        return plVar24;
      }
      goto LAB_1401c3d7b;
    }
  }
  else {
    local_158 = local_1d8;
    ppplStack_150 = (longlong ***)ppcStack_1d0;
    local_148 = (longlong ***)local_1c8;
  }
  param_1[2] = (longlong)local_148;
  *param_1 = (longlong)local_158;
  param_1[1] = (longlong)ppplStack_150;
LAB_1401c3d7b:
  plVar24 = (longlong *)__rustc::__rust_dealloc(_Src,0xfc04,1);
  return plVar24;
}


